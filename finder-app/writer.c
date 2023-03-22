#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <string.h>
#include <unistd.h>

int main (int argc, char *argv[])
{
    FILE *f_id;
    int error;
    char *string_buffer;
    size_t nmemb;

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
    f_id = fopen(argv[1], "w+");
    if (f_id == NULL) {
        error = errno;
        fprintf(stderr, "%s\n", strerror(error));
        syslog(LOG_ERR, "Opening file: %s\n", strerror(error));
    }
    syslog(LOG_INFO, "Opening file: %s success\n", argv[1]);

    /* 
     * Consider \param argv[2] as <string> to be written into <writefile>
    */
    string_buffer = (char *)malloc(strlen(argv[2]));
    sprintf(string_buffer, "%s", argv[2]);
    nmemb = fwrite(string_buffer, strlen(string_buffer), 1, f_id);
    if (nmemb <= 0) {
        fprintf(stderr, "Number of data written: %ld\n", nmemb);
        syslog(LOG_ERR, "Size of data written: %ld to file: %s\n", nmemb, argv[1]);
    }
    fprintf(stderr, "Writing string %s to file %s\n", argv[2], argv[1]);
    syslog(LOG_DEBUG, "Writing string %s to file %s\n", argv[2], argv[1]);
    
    /* Close the file after work done */
    error = fclose(f_id);
    if (error < 0) {
        error = errno;
        fprintf(stderr, "%s\n", strerror(error));
        syslog(LOG_ERR, "Failed to close fd: %s\n", strerror(error));
    }
    syslog(LOG_INFO, "Closing fd\n");
    closelog();
    
	return EXIT_SUCCESS;
}