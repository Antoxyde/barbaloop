#include <pthread.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>

typedef void (*actualizer)(char*);

typedef struct actualizer_args_ {
    actualizer func;
    pthread_mutex_t lock;
    size_t sleeptime;
    char* value;
} actualizer_args;

void actualize_time(char* value) {
    time_t rawtime = time(NULL);

    struct tm *info;
    info = localtime(&rawtime);
    strftime(value, 80, "%a %x - %X", info);
}

/*
void actualize_ram(char* value) {
    struct rusage usage;
    int err;

    err = getrusage(&usage);
}
*/

void *thread_main(void* args_void) {

    actualizer_args* args = (actualizer_args*) args_void;

    while (1) {

        pthread_mutex_lock(&(args->lock));
        args->func(args->value);
        pthread_mutex_unlock(&(args->lock));

        sleep(args->sleeptime);
    }

}

int main() {

    pthread_mutex_t time_lock;
    char time_value[80];
    actualizer_args time_args = {&actualize_time, time_lock, 1, time_value};

    pthread_t time_thread;
    int err;

    err = pthread_create(&time_thread, NULL, thread_main, (void*)(&time_args));

    while (1) {

        sleep(1);
        pthread_mutex_lock(&time_lock);
        printf("Time: %s\n", time_value);
        pthread_mutex_unlock(&time_lock);
    }

    return 0;
}

