#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define IF_WRITE

int main (int argc, char *argv[])
{
    int f_id;
    int error;
    char *string_buffer;
    ssize_t nmemb;

    /* Setting up syslog facility */
    openlog("Logs", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Start logging for assignment 2");

    if (argc < 2) {
        syslog(LOG_ERR, "Error: missing command line arguments path to writefile\n");
        return EXIT_FAILURE;
    }

    else if (argc < 3) {
        syslog(LOG_ERR, "Error: missing command line arguments writeString\n");
        return EXIT_FAILURE;
    }

    /* 
     * Assuming the directory and subdirectory exists
     * 
     * Consider \param argv[1] as path to <writefile>
    */
	syslog(LOG_INFO, "Opening file: %s\n", argv[1]);
    // f_id = open(argv[1], O_RDWR|O_CREAT, S_IRWXU|S_IRWXG|S_IRWXO);
    f_id = open(argv[1], O_RDONLY);
    if (f_id < 0) {
        error = errno;
        fprintf(stderr, "File open error: %s\n", strerror(error));
        syslog(LOG_ERR, "Opening file: %s failed\n", strerror(error));
    }
    else {
        syslog(LOG_INFO, "Opening file: %s success\n", argv[1]);
        fprintf(stderr, "Opening file: %s success\n", argv[1]);
    }

    
    /* 
     * Read the file if created during the open syscall
    */
    #ifdef IF_READ
    unsigned long firectl;
    // string_buffer = (char *)malloc(strlen(argv[2]));
    nmemb = read(f_id, &firectl, sizeof(unsigned long));
    if (nmemb != -1) {
        fprintf(stderr, "Read %ld & %ld bytes from file %s\n", firectl, nmemb, argv[1]);
        syslog(LOG_DEBUG, "Read %ld & %ld bytes from file %s\n", firectl, nmemb, argv[1]);
    }

    if( nmemb == -1 ) {
        printf("Error with read(), errno is %d (%s)\n", errno,strerror(errno));
        syslog(LOG_DEBUG, "Error with read(), errno is %d (%s)\n", errno,strerror(errno));
    }

    else {
        fprintf(stderr, "Read 0 bytes (empty file)\n");
        syslog(LOG_DEBUG, "Read 0 bytes (empty file)\n");
    }
    #endif

    /* 
     * Consider \param argv[2] as <string> to be written into <writefile>
    */
    #ifdef IF_WRITE
    string_buffer = (char *)malloc(strlen(argv[2]));
    sprintf(string_buffer, "%s", argv[2]);
    nmemb = write(f_id, string_buffer, strlen(string_buffer));
    if (nmemb <= 0) {
        fprintf(stderr, "Number of data written: %ld\n", nmemb);
        syslog(LOG_ERR, "Size of data written: %ld to file: %s\n", nmemb, argv[1]);
    }
    fprintf(stderr, "Writing string %s to file %s\n", argv[2], argv[1]);
    syslog(LOG_DEBUG, "Writing string %s to file %s\n", argv[2], argv[1]);
    #endif
    
    /* Close the file after work done */
    error = close(f_id);
    if (error < 0) {
        error = errno;
        fprintf(stderr, "%s\n", strerror(error));
        syslog(LOG_ERR, "Failed to close fd: %s\n", strerror(error));
    }
    syslog(LOG_INFO, "Closing fd\n");
    closelog();
    
	return EXIT_SUCCESS;
}