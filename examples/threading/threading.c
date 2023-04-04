#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)

void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    struct thread_data* thread_func_args = (struct thread_data *) thread_param;

    /* Wait */
    usleep(thread_func_args->wait_to_obtain_ms * 1000); 
    
    /* obtain mutex */
    if (pthread_mutex_lock(thread_func_args->mutex_lock) != 0) {
        ERROR_LOG("Failed to lock mutex\n");
        pthread_exit(0);
    }
    
    /* Wait */
    usleep(thread_func_args->wait_to_release_ms * 1000);
    
    /* release mutex */
    if (pthread_mutex_unlock(thread_func_args->mutex_lock) != 0) {
        ERROR_LOG("Failed to unlock mutex\n");
        pthread_exit(0);
    }

    thread_func_args->thread_complete_success = true;

    return (void *)thread_func_args;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex, int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */

    /* Allocate memory for thread data */
    struct thread_data *t_data = malloc(sizeof(struct thread_data));
    if (t_data == NULL) {
        ERROR_LOG("Failed to allocate memory for thread data\n");
        return false;
    }

    /* Setup thread data to be shared to threadfunc */
    t_data->mutex_lock = mutex;
    t_data->wait_to_obtain_ms = wait_to_obtain_ms;
    t_data->wait_to_release_ms = wait_to_release_ms;
    t_data->thread_complete_success = false;

    if (pthread_create(thread, NULL, threadfunc, (void *)t_data) == 0) {
        return true;
    }
    ERROR_LOG("Failed to create thread :[%s]", strerror(errno));

    return false;
}

