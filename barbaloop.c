#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>
#include <stdlib.h>
#include <string.h>

#define VALUE_MAXSIZE 80
#define PATH_BATT_FULL "/sys/class/power_supply/BAT1/energy_full"
#define PATH_BATT_NOW "/sys/class/power_supply/BAT1/energy_now"
#define PATH_BATT_STATUS "/sys/class/power_supply/BAT1/energy_now"

typedef void (*actualizer)(char*);

typedef struct actualizer_args_ {
    actualizer func;
    pthread_mutex_t* lock;
    size_t sleeptime;
    char* value;
} actualizer_args;


void actualize_time(char* value) {

    time_t rawtime = time(NULL);

    struct tm *info;
    info = localtime(&rawtime);
    strftime(value, VALUE_MAXSIZE, "%a %x - %X", info);

}

/*
void actualize_ram(char* value) {
    struct rusage usage;
    int err;

    err = getrusage(&usage);
}
*/

void actualize_battery(char* value) {

    char* path_full = "/sys/class/power_supply/BAT1/energy_full";
    char* path_now = "/sys/class/power_supply/BAT1/energy_now";
    FILE* fd_full = NULL;
    FILE* fd_now = NULL;

    if ((fd_now = fopen(PATH_BATT_NOW, "r")) == NULL || (fd_full = fopen(PATH_BATT_FULL, "r")) == NULL) {
        fprintf(stderr, "Can't open /sys/class");
        exit(1);
    }

    double value_now, value_full;
    fscanf(fd_now, "%lf", &value_now);
    fscanf(fd_full, "%lf", &value_full);

    snprintf(value, VALUE_MAXSIZE, "%lf%%", (value_now / value_full) * 100.0 );

    fclose(fd_now);
    fclose(fd_full);
}


void *thread_main(void* args_void) {

    actualizer_args* args = (actualizer_args*) args_void;

    while (1) {

        pthread_mutex_lock(args->lock);
        args->func(args->value);
        pthread_mutex_unlock(args->lock);

        sleep(args->sleeptime);
    }

}

actualizer_args* create_args(size_t sleeptime, actualizer func) {

    pthread_mutex_t* lock = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(lock, NULL);

    actualizer_args* args = (actualizer_args*)malloc(sizeof(actualizer_args));
    args->sleeptime = sleeptime;
    args->func = func;
    args->value = calloc(1, VALUE_MAXSIZE);
    args->lock = lock;

    return args;
}

void destroy_args(actualizer_args* args) {
    if (args) {

        if (args->lock)
            free(args->lock);

        if (args->value)
            free(args->value);

        free(args);
    }
}

int main() {

    int err;

    pthread_t time_thread, battery_thread;

    actualizer_args* time_args = create_args(1, &actualize_time);
    actualizer_args* battery_args = create_args(5, &actualize_battery);

    err = pthread_create(&time_thread, NULL, thread_main, (void*)(time_args));
    err = pthread_create(&battery_thread, NULL, thread_main, (void*)(battery_args));

    while (1) {

        sleep(1);
        pthread_mutex_lock(time_args->lock);
        pthread_mutex_lock(battery_args->lock);
        printf("Time: %s, Battery: %s\n", time_args->value, battery_args->value);
        pthread_mutex_unlock(time_args->lock);
        pthread_mutex_unlock(battery_args->lock);

    }

    destroy_args(time_args);
    destroy_args(battery_args);

    return 0;
}
