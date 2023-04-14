#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netdb.h>
#include <syslog.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <arpa/inet.h>

#define PORT "9000"
#define LOCALHOST "localhost"
#define BACKLOG 10
#define MAXDATASIZE 1024
#define OUTFILE "/tmp/var/aesdsocket"

static bool sigint = false;
static bool sigterm = false;


void signal_handler(int signal_number)
{
    switch (signal_number)
    {
    case SIGINT:
        sigint = true;
        break;

    case SIGTERM:
        sigterm = true;
        break;
    
    default:
        break;
    }
}


void make_daemon()
{
    pid_t pid = fork();

    /* Fork to create child */
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }

    /* If fork then let parent terminate */
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    /* If parent terminate success then the child 
     * owns the session
     */
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    /* Fork off for the second time*/
    pid = fork();

    /* An error occurred */
    if (pid < 0)
        exit(EXIT_FAILURE);

    /* Success: Let the parent terminate */
    if (pid > 0)
        exit(EXIT_SUCCESS);

    /* Set new file permissions */
    umask(0);

    /* Change the working directory to the root directory */
    /* or another appropriated directory */
    chdir("/");

    /* Open the log file */
    openlog ("aesdsocket_daemon", LOG_PID, LOG_DAEMON);
}


char *read_from_file(void) 
{
    FILE       *File_fd;
    char       *buffer              = NULL;
    size_t      bytes_read          = 0;
    long        file_size           = 0;
    int         ret                 = 0;

    File_fd = fopen(OUTFILE, "r");
    if (File_fd == NULL) {
        fprintf(stderr, "Failed: to open file (%s)\n", strerror(errno));
        exit(0);
    }

    ret = fseek(File_fd, 0, SEEK_END);
    if (ret == -1) {
        fprintf(stderr, "Failed: to seek file (%s)\n", strerror(errno));
        exit(0);
    }

    file_size = ftell(File_fd);
    if (file_size == -1) {
        fprintf(stderr, "Failed: to get file size (%s)\n", strerror(errno));
        exit(0);
    }

    ret = fseek(File_fd, 0, SEEK_SET);
    if (ret == -1) {
        fprintf(stderr, "Failed: to seek file (%s)\n", strerror(errno));
        exit(0);
    }

    buffer = malloc(file_size);
    if (buffer == NULL) {
        fprintf(stderr, "Failed: to malloc buffer (%s)\n", strerror(errno));
        exit(0);
    }

    bytes_read = fread(buffer, sizeof(char), file_size, File_fd);
    if (bytes_read < file_size) {
        fprintf(stderr, "Failed: to read from file (%s)\n", strerror(errno));
        exit(0);
    }
    fclose(File_fd);

    buffer[bytes_read - 1] = '\n';
    buffer[bytes_read] = '\0';

    return buffer;
}


int write_to_file(char *string) 
{
    FILE       *outFile_fd;
    size_t      bytes_wrote = 0;
    int         str_len = 0;

    printf("String recived: %s -> writing to file %s\n", string, OUTFILE);

    system("mkdir -p /tmp/var");

    outFile_fd = fopen(OUTFILE, "a+");
    if (outFile_fd == NULL) {
        fprintf(stderr, "Failed: to open file (%s)\n", strerror(errno));
        return 1;
    }

    str_len = strlen(string);
    bytes_wrote = fwrite(string, sizeof(char), str_len, outFile_fd);

    if (bytes_wrote < str_len) {
        fprintf(stderr, "Failed: to write to file (%s)\n", strerror(errno));
        return 1;
    }
    fclose(outFile_fd);
    
    return 0;
}


int main(int argc, char **argv) 
{
    /* Setting up syslog facility */
    openlog("Logs", LOG_PID, LOG_USER);
    syslog(LOG_INFO, "Start logging for assignment 5 (aesdsocket-server)");

    /* Opens a stream socket bound to port 9000, 
     * failing and returning -1 if any of the socket connection steps fail. 
     */
    int                            opt                      = 0;
    int                            socket_fd                = 0;
    int                            getaddr_fd               = 0;
    int                            yes                      = 1;
    int                            listen_socket            = 0;
    int                            incomming_fd             = 0;
    ssize_t                        num_bytes                = 0;
    struct addrinfo                *result                  = NULL;
    struct addrinfo                *p_res                   = NULL;
    struct addrinfo                hints;
    struct sockaddr_storage        incomming_addr;
    socklen_t                      addr_size                = 0;
    bool                           start_daemeon            = false;
    char                           ipstr[INET6_ADDRSTRLEN]  = {0};
    char                           buf[MAXDATASIZE]         = {0};
    void                           *addr                    = NULL;
    void                           *str_read                = NULL;
    int                            ret                      = -1;

    /* Options handler for starting dameon */
    while ((opt = getopt(argc, argv, "d")) != -1) {
        switch (opt) {
            case 'd':
                start_daemeon = true;
                break;
            default: /* '?' */
                start_daemeon = false;
                fprintf(stderr, "Usage: %s -d [To run as daemon]\n", argv[0]);
                return -1;
        }
    }

    /* Get socket address */
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddr_fd = getaddrinfo(LOCALHOST, PORT, &hints, &result);
    if (getaddr_fd != 0) {
        syslog(LOG_ERR, "Failed: To get socket address (%s)\n", strerror(errno));
        return -1;
    } else {
        printf("Getting address for socket success\n");
    }

    /* Creating the socket for localhost */
    socket_fd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (socket_fd == -1) {
        syslog(LOG_ERR, "Failed: To create socket (%s)\n", strerror(errno));
        return -1;
    } else {
        printf("Creating socket success\n");
    }

    // loop through all the results and bind to the first we can
    for(p_res = result; p_res != NULL; p_res = result->ai_next) {
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(socket_fd, p_res->ai_addr, p_res->ai_addrlen) == -1) {
            close(socket_fd);
            perror("server: bind");
            continue;
        }
        break;
    }

    freeaddrinfo(result);

    /* Listen and accept */
    listen_socket = listen(socket_fd, BACKLOG);
    if (listen_socket == -1) {
        syslog(LOG_ERR, "Failed: to listen for incomming address (%s)\n", strerror(errno));
        return -1;
    }

    /* Signal handler */
    struct sigaction sa = { 
        .sa_handler = signal_handler, 
        .sa_flags = SA_NODEFER | SA_RESETHAND 
    };
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGINT, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Error (%s)\n", strerror(errno));
        exit(1);
    }

    else if (sigaction(SIGTERM, &sa, NULL) == -1) {
        syslog(LOG_ERR, "Error (%s)\n", strerror(errno));
        exit(1);
    }

    if (start_daemeon) {
        make_daemon();
    }

    do 
    {
        addr_size = sizeof(incomming_addr);
        incomming_fd = accept(socket_fd, (struct sockaddr *)&incomming_addr, &addr_size);
        if (incomming_fd == -1) {
            syslog(LOG_ERR, "Failed: to accept from incomming address (%s)\n", strerror(errno));
        } 

        if (sigint || sigterm) {
            syslog(LOG_DEBUG, "Caught signal. Hence exiting\n");
            
            close(socket_fd);

            syslog(LOG_INFO, "Closing socket\n");
            closelog();

            if (remove(OUTFILE) == 0) {
                printf("Removed file successfully\n");
            } else {
                printf("Failed to remove file with error: %s\n", strerror(errno));
            }
            exit(0);
        }

        if (incomming_addr.ss_family == AF_INET) {
            struct sockaddr_in *ipv4 = (struct sockaddr_in *)&incomming_addr;
            addr = &(ipv4->sin_addr);
        }

        else {
            struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)&incomming_addr;
            addr = &(ipv6->sin6_addr);
        }

        /* Convert the IP address to string */
        if (inet_ntop(incomming_addr.ss_family, addr, ipstr, sizeof(ipstr)) == NULL) {
            fprintf(stderr, "Failed: Convert the IP address to string (%s)\n", strerror(errno));
            return -1;
        } else {
            syslog(LOG_INFO, "Accepted connection from %s\n", ipstr);
            fprintf(stderr, "\nAccepted connection from %s\n", ipstr);
        }

        num_bytes = recv(incomming_fd, buf, MAXDATASIZE-1, 0);
        if (num_bytes <= 0) {
            syslog(LOG_ERR, "Failed: To recive stream from socket (%s)\n", strerror(errno));
            return -1;
        }

        if (buf[num_bytes - 1] == '\n') {
            ret = write_to_file(buf);
            if (ret == 0)
                memset(buf, 0, num_bytes);
        }

        else if (((num_bytes % (MAXDATASIZE-1)) == 0) && (buf[num_bytes-1] != '\n')) {
            fprintf(stderr, "Need to resize or write in blocks or each char\n");
            while (buf[num_bytes-1] != '\n')
            {    
                ret = write_to_file(buf);
                if (ret == 0) {
                    memset(buf, 0, num_bytes);
                    num_bytes = recv(incomming_fd, buf, MAXDATASIZE-1, 0);
                    if (num_bytes <= 0) {
                        syslog(LOG_ERR, "Failed: To recive stream from socket (%s)\n", strerror(errno));
                        return -1;
                    }
                }
            }
            ret = write_to_file(buf);
            if (ret == 0)
                memset(buf, 0, MAXDATASIZE);
        }

        else {
            fprintf(stderr, "Size dosent match..!\n");
            return -1;
        }

        str_read = read_from_file();
        if (str_read != NULL) {
            fprintf(stderr, "\n%s -> String read from file..\n", (char *)str_read);
            num_bytes = send(incomming_fd, str_read, strlen((char *)str_read), 0);
            if (num_bytes == -1) {
                syslog(LOG_ERR, "Failed: To recive stream from socket (%s)\n", strerror(errno));
                fprintf(stderr, "Failed: To send stream to socket (%s)\n", strerror(errno));
                return -1;
            }
            close(incomming_fd);
            fprintf(stderr, "Closed connection from %s\n", ipstr);
        }
        free(str_read);
    } while ((!sigint) || (!sigterm));    

    return 0;
}
