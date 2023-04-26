#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>


#define WAIT_SEC 2

void *do_timestamp(void *data)
{
    time_t t = time(NULL);
    if (t == (time_t)(-1)) {
        exit(1);
    }
    printf("Current time is %ld\n", t);

    time_t current = t;
    struct tm date;

    while(1)
    {
        if ((current - t) % WAIT_SEC == 0) {  // Check difference
            if (gmtime_r(&current, &date) == NULL) {
                exit(1);
            }
            fprintf(stderr, "Timestamp:%d-%02d-%02d %02d:%02d:%02d\n", date.tm_year + 1900, date.tm_mon + 1, date.tm_mday, date.tm_hour, date.tm_min, date.tm_sec);
            sleep(1);               // To limit amount of output if needed
        }
        current = time(NULL);      // Update current time
    }

    return NULL;
}


int main(int argc, char *argv[]) 
{
    pthread_t tid;
    pthread_create(&tid, NULL, do_timestamp, NULL);
    pthread_join(tid, NULL);

    return 0;
}