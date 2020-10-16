#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <signal.h>

typedef void (*actualizer)(char*);

void actualize_battery(char*);
void actualize_time(char*);

typedef struct actualizer_ {
    actualizer func;
    int sleeptime;
    int signal;
    int iscmd;
    char* cmd;
} actualizer_t;

#include "config.h"

#define STATUS_MAXLEN 80
#define LENGTH(X) (sizeof(X) / sizeof (X[0]))
#define BAR_MAXLEN (STATUS_MAXLEN + 4) * LENGTH(actualizers)

static char status[LENGTH(actualizers)][STATUS_MAXLEN] = {0};
static char bar[BAR_MAXLEN];
static char oldbar[BAR_MAXLEN];
static int continue_looping = 1;

void update_status() {

    int i;
    strncpy(oldbar, bar, BAR_MAXLEN);
    memset(bar, 0, BAR_MAXLEN);

    for (i = 0; i < LENGTH(actualizers) - 1; i++) {
        strncat(bar, CONF_DELIMITER_BEFORE, 2);
        strncat(bar, status[i], STATUS_MAXLEN + 1);
        strncat(bar, CONF_DELIMITER_AFTER, 2);
        strncat(bar, CONF_DELIMITER, 2);

    }

    strncat(bar, CONF_DELIMITER_BEFORE, 2);
    strncat(bar, status[i], STATUS_MAXLEN);
    strncat(bar, CONF_DELIMITER_AFTER, 2);

    //printf("%s\n", bar);
    //fflush(stdout);

    if (strncmp(oldbar, bar, BAR_MAXLEN)) {
        Display *dpy = XOpenDisplay(NULL);

        if (!dpy) {
            fprintf(stderr, "Fatal: could not open X display.\n");
            exit(1);
        }

        int screen = DefaultScreen(dpy);
        Window root = RootWindow(dpy, screen);
        XStoreName(dpy, root, bar);
        XCloseDisplay(dpy);
    }
}

void cleanup(char* value) {

    char* dst = value;
    char* src = value;

    while (*src) {

        if (*src < 0x20 || *src > 0x7f) {
            src++;
            *dst = *src;
        }

        dst++;
        src++;
    }
}

void actualize_time(char* value) {

    time_t rawtime = time(NULL);
    struct tm *info;
    info = localtime(&rawtime);
    strftime(value, STATUS_MAXLEN, CONF_TIME_FORMAT, info);

}

/*
void actualize_ram(char* value) {
    struct rusage usage;
    int err;

    err = getrusage(&usage);
}
*/

void actualize_battery(char* value) {

    FILE* fd_full = NULL;
    FILE* fd_now = NULL;

    if ((fd_now = fopen(CONF_BATTERY_PATH"/energy_now", "r")) == NULL || (fd_full = fopen(CONF_BATTERY_PATH"/energy_full", "r")) == NULL) {
        strcpy(value, "Can't open /sys/class");
    } else {

        double value_now, value_full;
        fscanf(fd_now, "%lf", &value_now);
        fscanf(fd_full, "%lf", &value_full);

        snprintf(value, STATUS_MAXLEN, CONF_BATTERY_FORMAT, (value_now / value_full) * 100.0 );
    }

    if (fd_now)
        fclose(fd_now);

    if (fd_full)
        fclose(fd_full);
}


void exec_cmd(char* cmd, char* value) {

    FILE *file_cmd = popen(cmd,"r");
    if (!file_cmd)
        return;

    fgets(value, STATUS_MAXLEN, file_cmd);
    value[STATUS_MAXLEN - 1] = '\0';

    cleanup(value);

    pclose(file_cmd);
}

void run_loop() {

    int i, time = 0, atleast_one = 0;

    // Run every actualizer a first time
    for  (i = 0; i < LENGTH(actualizers); i++) {
        if (actualizers[i].iscmd)
            exec_cmd(actualizers[i].cmd, status[i]);
        else
            actualizers[i].func(status[i]);
    }
    update_status();


    while (continue_looping) {

        atleast_one = 0;

        for  (i = 0; i < LENGTH(actualizers); i++) {
            if (actualizers[i].sleeptime && time % actualizers[i].sleeptime == 0) {

                if (actualizers[i].iscmd)
                    exec_cmd(actualizers[i].cmd, status[i]);
                else
                    actualizers[i].func(status[i]);

                atleast_one = 1;
            }
        }

        if (atleast_one)
            update_status();

        sleep(1);
        time++;
    }
}

void sig_handler(int signum) {

    int atleast_one = 0;

    for (int i = 0; i < LENGTH(actualizers); i++) {
        if (actualizers[i].signal == signum || signum == CONF_CATCHALL_SIGNUM) {
            atleast_one = 1;
            if (actualizers[i].iscmd) {
                exec_cmd(actualizers[i].cmd, status[i]);
            } else {
                actualizers[i].func(status[i]);
            }
        }
    }

    if (atleast_one)
        update_status();
}

void sigterm_handler(int signum) {
    printf("Caught sigterm/sigint, leaving.\n");
    continue_looping = 0;
    exit(0);
}

int main() {

    // setup signals
    for (int i = 0; i < LENGTH(actualizers); i++)
        signal(actualizers[i].signal, sig_handler);

    signal(CONF_CATCHALL_SIGNUM, sig_handler);


    signal(SIGTERM, sigterm_handler);
	signal(SIGINT, sigterm_handler);

    run_loop();

    return 0;
}
