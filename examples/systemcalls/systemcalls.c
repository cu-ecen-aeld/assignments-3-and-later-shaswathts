#include "systemcalls.h"

/**
 * @param cmd the command to execute with system()
 * @return true if the command in @param cmd was executed
 *   successfully using the system() call, false if an error occurred,
 *   either in invocation of the system() call, or if a non-zero return
 *   value was returned by the command issued in @param cmd.
*/
bool do_system(const char *cmd)
{

/*
 * TODO  add your code here
 *  Call the system() function with the command set in the cmd
 *   and return a boolean true if the system() call completed with success
 *   or false() if it returned a failure
*/
    // if (cmd == NULL) {
    //     fprintf(stderr, "The command passed to syste() is NULL");
    //     return EXIT_FAILURE;
    // }

    // int status = system(cmd);
    // if (status != -1) { // -1 means an error with the call itself
    //     int ret = WEXITSTATUS(status);
    //     fprintf(stderr, "The system() call had no issue, %d, %d\n", ret, WIFEXITED(ret));
    //     // exit(EXIT_SUCCESS);
    // }

    int status = system(cmd);
    if (status != -1) { // -1 means an error with the call itself    
        if (WIFEXITED(status) && !WEXITSTATUS(status)) {
            printf("program execution successful\n"); 
            return true;
        }
        
        else if (WIFEXITED(status) && WEXITSTATUS(status)) { 
            if (WEXITSTATUS(status) == 127) { 
                // execv failed 
                printf("execv failed\n"); 
                return false;
            } 
            else {
                printf("program terminated normally,"
                " but returned a non-zero status\n");
                return false;
            }		 
        } 
        else
            printf("program didn't terminate normally\n");			 
    } 
    
    else { 
        // waitpid() failed 
        printf("waitpid() failed\n"); 
    } 
    return false;
}

/**
* @param count -The numbers of variables passed to the function. The variables are command to execute.
*   followed by arguments to pass to the command
*   Since exec() does not perform path expansion, the command to execute needs
*   to be an absolute path.
* @param ... - A list of 1 or more arguments after the @param count argument.
*   The first is always the full path to the command to execute with execv()
*   The remaining arguments are a list of arguments to pass to the command in execv()
* @return true if the command @param ... with arguments @param arguments were executed successfully
*   using the execv() call, false if an error occurred, either in invocation of the
*   fork, waitpid, or execv() command, or if a non-zero return value was returned
*   by the command issued in @param arguments with the specified arguments.
*/
bool do_exec(int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];

/*
 * TODO:
 *   Execute a system command by calling fork, execv(),
 *   and wait instead of system (see LSP page 161).
 *   Use the command[0] as the full path to the command to execute
 *   (first argument to execv), and use the remaining arguments
 *   as second argument to the execv() command.
 *
*/

    pid_t pid;
    int status;

    fflush(stdout);
    pid = fork();
    if (pid == -1) {
        // fprintf(stderr, "Failed to create fork(): %s\n", strerror(errno));
        return false; 
    }

    else if (pid == 0) {
        // In child process. Call execv() with the path & arguments list
        // the execv() only return if error occured & the return value is -1
        // fprintf(stderr, "In child process, pid = %u\n", getpid()); 

        // the execv() only return if error occured. 
        // The return value is -1 
        execv(command[0], command); 
        exit(errno);
    }

    else {
        // In parent process. wait for execv()/child to finish its process
        // fprintf(stderr, "In parent process, pid = %u\n", getpid()); 

        if (waitpid(pid, &status, 0) > 0) {
            if (WIFEXITED(status) && !WEXITSTATUS(status)) {
			    fprintf(stderr, "program execution successful\n");
			}

			else if (WIFEXITED(status) && WEXITSTATUS(status)) { 
				if (WEXITSTATUS(status) == 127) { 

					// execv failed
					fprintf(stderr, "execv failed: %s\n", strerror(errno));
                    return false;
				} 
				else {
					fprintf(stderr, "Program terminated normally, but returned a non-zero status\n");
                    return false;
                }		 
			} 
			else {
			    fprintf(stderr, "Program didn't terminate normally\n");
            }
		} 
		else { 
            // waitpid() failed 
            printf("waitpid() failed\n"); 
		} 
    }
    va_end(args);
    return true;
}

/**
* @param outputfile - The full path to the file to write with command output.
*   This file will be closed at completion of the function call.
* All other parameters, see do_exec above
*/
bool do_exec_redirect(const char *outputfile, int count, ...)
{
    va_list args;
    va_start(args, count);
    char * command[count+1];
    int i;
    for(i=0; i<count; i++)
    {
        command[i] = va_arg(args, char *);
    }
    command[count] = NULL;
    // this line is to avoid a compile warning before your implementation is complete
    // and may be removed
    command[count] = command[count];


/*
 * TODO
 *   Call execv, but first using https://stackoverflow.com/a/13784315/1446624 as a refernce,
 *   redirect standard out to a file specified by outputfile.
 *   The rest of the behaviour is same as do_exec()
 *
*/
    pid_t pid;
    int status;

    int fd = open(outputfile , O_WRONLY | O_TRUNC | O_CREAT, 0644);
    if (fd < 0) { 
        fprintf(stderr, "Failed to open Fd for file: %s: %s", outputfile, strerror(errno));
        return EXIT_FAILURE;
    }

    fflush(stdout);
    pid = fork();
    if (pid == -1) {
        fprintf(stderr, "Failed to create fork(): %s\n", strerror(errno));
        return EXIT_FAILURE;
    }

    else if (pid == 0) {
        // In child process. Call execv() with the path & arguments list
        // the execv() only return if error occured & the return value is -1
        // fprintf(stderr, "In child process, pid = %u\n", getpid());

        // Redirect standard out to a file specified by outputfile.
        if (dup2(fd, 1) < 0) { 
            fprintf(stderr, "Failed to redirect standard out to a file specified by outputfile.\n"); 
            return EXIT_FAILURE;
        }
        close(fd); 
        execv(command[0], command);
        exit(errno);
    }

    else {
        // In parent process. wait for execv()/child to finish its process
        // fprintf(stderr, "In parent process, pid = %u\n", getpid()); 

        if (waitpid(pid, &status, 0) > 0) {
            if (WIFEXITED(status) && !WEXITSTATUS(status)) {
			    fprintf(stderr, "program execution successful\n");
			}

			else if (WIFEXITED(status) && WEXITSTATUS(status)) { 
				if (WEXITSTATUS(status) == 127) { 

					// execv failed
					fprintf(stderr, "execv failed: %s\n", strerror(errno));
                    return false;
				} 
				else {
					fprintf(stderr, "Program terminated normally, but returned a non-zero status\n");
                    return false;
                }		 
			} 
			else {
			    fprintf(stderr, "Program didn't terminate normally\n");
            }
		} 
		else { 
            // waitpid() failed 
            printf("waitpid() failed\n"); 
		} 
    }
    va_end(args);

    return true;
}
