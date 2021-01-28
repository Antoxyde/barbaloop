#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <stdint.h>
#include <X11/Xlib.h>

#include <time.h> // date
#include <sys/ioctl.h> // wifi
#include <sys/types.h> // wifi
#include <sys/socket.h> // wifi
#include <linux/wireless.h> // wifi
#include <errno.h> // temp

void update_statusbar(void);

void actualize_wifi(char* buf);
void actualize_load(char* buf);
void actualize_ram(char* buf);
void actualize_battery(char* buf);
void actualize_time(char* buf);

typedef struct actualizer {
    void (*func)(char* buf);
    int signal;
    int time;
} actualizer_t;

#include "config.h"
#include "pa.h"
#include <pthread.h> // pulseaudio


#define LENGTH(X) (sizeof(X)/sizeof(X[0]))

static char status[LENGTH(actualizers) + 1][MAX_STATUS_SIZE];
static int continue_looping = 1;

void actualize_time(char *buf) {
    char lbuf[64];
    time_t rawtime = time(NULL);
    struct tm *info;
    info = localtime(&rawtime);
    strftime(lbuf, 64 , CONF_TIME_FORMAT, info);
    snprintf(buf, MAX_STATUS_SIZE, "%s%s", CONF_COLOR_TIME, lbuf);
}


int read_file(char* path, char buf[32]) {

    FILE* fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "Can't open %s for reading.\n", path);
        return 0;
    }
    
    fread(buf, 1, 32, fp);
    fclose(fp);
    return 1;
}

void actualize_battery(char* buf) {
    
    char lbuf[32];
    char state[32];
    char color[11];
    int last_capacity = -1;
    int remaining_capacity = -1;
    int present_rate = -1;
    
    if (!read_file(CONF_BATT_PATH "status", state)) return;

    if (!read_file(CONF_BATT_PATH "charge_now", lbuf)) return;
    sscanf(lbuf, "%d", &remaining_capacity);
    remaining_capacity /= 1000;

    if (!read_file(CONF_BATT_PATH "charge_full", lbuf)) return;
    sscanf(lbuf, "%d", &last_capacity);
    last_capacity /= 1000;

    if (!read_file(CONF_BATT_PATH "current_now", lbuf)) return;
    sscanf(lbuf, "%d", &present_rate);
    present_rate /= 1000;

    int percentage = 100 * remaining_capacity / last_capacity;
    int seconds;

    if (!strcmp(state, "Charging\n")) {
        strncpy(color, CONF_COLOR_BATT_CHARGING, 11);
        seconds = 3600 * (last_capacity - remaining_capacity) / present_rate;
    } else if (!strcmp(state, "Discharging\n")) {

        seconds = 3600 *  remaining_capacity / present_rate;

        if (percentage < CONF_LIMIT_BATT_LOW) {
            strncpy(color, CONF_COLOR_BATT_DISCHARGING_LOW, 11);
            system("notify-send -c urgent " CONF_LOW_BATTERY_MESSAGE);
        } else {
            strncpy(color, CONF_COLOR_BATT_DISCHARGING, 11);
        }
    } else {
        strncpy(color, CONF_COLOR_BATT_UNKNOWN, 11);
    }
     
    int hours = seconds / 3600;
    seconds -= 3600 * hours;
    int minutes = seconds / 60;
    
    snprintf(buf, MAX_STATUS_SIZE, "%s%d%% %02dh%02d", color, percentage, hours, minutes);
}

void actualize_ram(char* buf) {
    int memtotal, memavailable;

    FILE* fp = fopen("/proc/meminfo", "r");
    if (!fp) return; // TODO : friendly message

    fscanf(fp, "MemTotal: %d kB\nMemFree: %*d kB\nMemAvailable: %d kB", &memtotal, &memavailable);
    fclose(fp);

    int used = memtotal - memavailable;
    double used_mb = (double)used / 1024.0;
    
    double used_gb = used_mb / 1024.0;

    if (used_gb > 1) {
        if (used_gb > CONF_LIMIT_RAM_HIGH) {
            snprintf(buf, MAX_STATUS_SIZE, "%s%0.2fGi", CONF_COLOR_RAM_HIGH, used_gb);
        } else {
            snprintf(buf, MAX_STATUS_SIZE, "%s%0.2fGi", CONF_COLOR_RAM, used_gb);
        }
    } else {
        snprintf(buf, MAX_STATUS_SIZE, "%s%0.0fMi", CONF_COLOR_RAM, used_mb);
    }
}

void actualize_load(char* buf) {
    float load;

    FILE* fp = fopen("/proc/loadavg", "r");
    if (!fp) return; // TODO : friendly message

    fscanf(fp, "%f", &load);
    fclose(fp);

    if (load > CONF_LIMIT_LOAD_HIGH) {
        snprintf(buf, MAX_STATUS_SIZE, "%s%0.2f", CONF_COLOR_LOAD_HIGH, load);
    } else {
        snprintf(buf, MAX_STATUS_SIZE, "%s%0.2f", CONF_COLOR_LOAD, load);
    }
}

void actualize_wifi(char* buf) {

    FILE* fp = fopen("/sys/class/net/" CONF_WIFI_IFACE "/operstate", "r");
    if (!fp) return; // TODO: friendly message

    char status[5];
    fscanf(fp, "%s", status);
    fclose(fp);

    if (!strcmp(status, "down")) {
        snprintf(buf, MAX_STATUS_SIZE, "%sdown", CONF_WIFI_COLOR_DOWN);
        return;
    }
    
    char ssid[IW_ESSID_MAX_SIZE + 1];
    bzero(ssid, IW_ESSID_MAX_SIZE + 1);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd == -1) return;

    struct iwreq iwr;
    iwr.u.essid.pointer = ssid;
    iwr.u.essid.flags = 0;
    iwr.u.essid.length = IW_ESSID_MAX_SIZE;
    strcpy(iwr.ifr_name, CONF_WIFI_IFACE);

    if (ioctl(fd, SIOCGIWESSID, &iwr)) {
		fprintf(stderr, "ioctl SIOCGIWESSID : %s\n",strerror(errno));
	} else {
        snprintf(buf, MAX_STATUS_SIZE, "%s%s", CONF_WIFI_COLOR, ssid);
    }
}

void update_statusbar() {
    
    char statusbar[LENGTH(actualizers) * MAX_STATUS_SIZE] = {};

    int i;
    for (i = 0; i < LENGTH(actualizers) + 1; i++) {
        strncat(statusbar, status[i], MAX_STATUS_SIZE);
        strncat(statusbar, " ", 2);
    }
   
    Display *dpy = XOpenDisplay(NULL);

    if (!dpy) {
        fprintf(stderr, "Fatal: could not open X display.\n");
        exit(1);
    }
    
    int screen = DefaultScreen(dpy);
    Window root = RootWindow(dpy, screen);
    XStoreName(dpy, root, statusbar);
    XCloseDisplay(dpy);
}


void actualize_all(int elapsed) {
    int hasChanged = 0;
    for (int i = 0; i < LENGTH(actualizers); i++) {
        if (elapsed == 0 || (actualizers[i].time && elapsed % actualizers[i].time == 0)) {
            actualizers[i].func(status[i+1]);
            hasChanged = 1;
        }
    }
    
    if (hasChanged) {
        update_statusbar();
    }
}

void sighandler(int signo) {
    printf("Received signal %d (%s)\n", signo, strsignal(signo));
    switch(signo) {
        case SIGTERM:
            continue_looping = 0;
            break;
        case CONF_CATCHALL_SIGNUM:
            actualize_all(0); 
            break;
        default:
            fprintf(stderr, "Unhandled signal %d.\n", signo);
            break;
    }

    int hasChanged = 0;
    for (int i = 0; i < LENGTH(actualizers); i++) {
        if (actualizers[i].signal == signo) {
            actualizers[i].func(status[i+1]);
            hasChanged = 1;
        }
    }

    if (hasChanged) {
        update_statusbar();
    }
}

void register_signals() {
    signal(SIGTERM, sighandler);
    signal(CONF_CATCHALL_SIGNUM, sighandler);

    for (int i = 0; i < LENGTH(actualizers); i++) {
        if (actualizers[i].signal) {
            signal(actualizers[i].signal, sighandler);
        }
    }
}

int main(void) {

    register_signals();
    
    pthread_t tid;
    pthread_create(&tid, NULL, pa_setup, status[0]);

    int elapsed = 0;
    while (continue_looping) {
        actualize_all(elapsed); 
        usleep(1000000);
        elapsed += 1;
    }

    return 0;
}

