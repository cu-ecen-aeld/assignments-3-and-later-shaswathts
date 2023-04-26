#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define DESIRED_INTERVAL 3  //1 second

int get_tv_cur_minus_given(struct timeval *tv, struct timeval *tp_given, int *sign)
{
    struct timeval tp_cur;

    gettimeofday(&tp_cur, NULL);

    tv->tv_sec  = tp_cur.tv_sec - tp_given->tv_sec;
    tv->tv_usec = tp_cur.tv_usec - tp_given->tv_usec;

    if(tv->tv_sec > 0) {
        *sign = 1;
        if(tv->tv_usec < 0) {
            tv->tv_sec--;
            tv->tv_usec = 1000000 + tv->tv_usec;
        }
    }
    else if (tv->tv_sec == 0) 
    {
        if(tv->tv_usec == 0)
            *sign = 0;

        else if(tv->tv_usec < 0) {
            *sign = -1;
            tv->tv_usec *= -1;
        }
        
        else
            *sign = 1;
    }
    else {
        *sign = -1;
        if(tv->tv_usec > 0) {
            tv->tv_sec++;
            tv->tv_usec = 1000000 - tv->tv_usec;
        }
        
        else if(tv->tv_usec < 0)
            tv->tv_usec *= -1;
        
        return 0;
    }
}

int main()
{
    struct timeval tv_last_run;
    struct timeval tv_diff;
    int sign = 0;

    gettimeofday(&tv_last_run, NULL);

    while(1)
    {

        get_tv_cur_minus_given(&tv_diff, &tv_last_run, &sign);

        // printf("Time: %ld\n", tv_diff.tv_sec);
        if(tv_diff.tv_sec > DESIRED_INTERVAL) {
            gettimeofday(&tv_last_run, NULL);
            printf("call the func here : %ld\n", tv_diff.tv_sec);
        }
    }

    return 0;
}