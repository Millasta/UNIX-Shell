#include "shell_fct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

pid_t *pids;
int nbChildren;

int execCd(char** args, int nbArgs) {
    char *path = NULL;
    int pathSize = 0;

    errno = 0;

    // Get the size of the path
    int i;
    for(i = 1 ; i < nbArgs; i++)
        pathSize += strlen(args[i]);
    pathSize += (nbArgs - 1);
    path = (char*)malloc((pathSize)+1 * sizeof(char));

    // Get all the args into a unique string for the path
    i = 0;
    int j;
    for(j = 1 ; j < nbArgs ; j++) {
        int k;
        for(k = 0 ; k < strlen(args[j]) ; k++) {
            while(args[j][k] == '\'' || args[j][k] == '\"') // Eliminate ' and " char
                k++;
            path[i] = args[j][k];
            i++;
        }
        path[i] = ' ';
        i++;
    }

    path[pathSize - 1] = '\0';

    int cdStatus = chdir(path);
    free(path);
    return cdStatus;
}

//Alarm handler
void alarmHandler(int sigNum)
{
	printf("Alarm handled: killing child processes\n");
    int i;
    for(i = 0 ; i < nbChildren ; i++)
	   kill(pids[i], SIGKILL);
}

int exec_command(Cmd* my_cmd){
    // cd
    if(strcmp(my_cmd->cmdMembersArgs[0][0], "cd") == 0) {
        execCd(my_cmd->cmdMembersArgs[0], my_cmd->nbMembersArgs[0]);
    }
    // exit
    else if(strcmp(my_cmd->cmdMembersArgs[0][0], "exit") == 0) {
        return MYSHELL_FCT_EXIT; // exit
    }
    // others..
    else {
        nbChildren = my_cmd->nbCmdMembers;
        pids = (pid_t*)malloc(sizeof(pid_t) * nbChildren);
        int status[nbChildren];
        int pipes[nbChildren][2];
        int i;

        // Create children processes
        for(i = 0 ; i < nbChildren ; i++) {
            pipe(pipes[i]);
            pids[i] = fork();
            if(pids[i] < 0) {
                exit(errno); // ERROR
            }
            else if(pids[i] == 0){ // Exec the member's command
                // connect pipes
                if(i != 0) {
                    close(pipes[i-1][1]);
                    dup2(pipes[i-1][0], 0);
                    close(pipes[i-1][0]);

                    close(pipes[i][0]);
                    dup2(pipes[i][1], 1);
                    close(pipes[i][1]);
                }
                else {
                    close(pipes[i][0]);

                    if(my_cmd->redirection[0][STDIN_FILENO] != NULL) {
                        int fdIn = open(my_cmd->redirection[0][STDIN_FILENO], O_RDONLY);
                        if(fdIn == -1)
                            printf("Error when openning %s : %s\n", my_cmd->redirection[0][STDIN_FILENO], strerror(errno));
                        else {
                            dup2(fdIn, 0);
                            close(fdIn);
                        }
                    }
                    dup2(pipes[i][1], 1);
                    close(pipes[i][1]);
                }
                if(i == (nbChildren-1)) {
                    int fdOut = -1;
                    if(my_cmd->redirection[i][STDOUT_FILENO] != NULL) {
                        if(my_cmd->redirectionType[i][STDOUT_FILENO] == APPEND) {
                            fdOut = open(my_cmd->redirection[i][STDOUT_FILENO], O_CREAT | O_WRONLY | O_APPEND, S_IRWXU);
                        }
                        else if(my_cmd->redirectionType[i][STDOUT_FILENO] == OVERRIDE) {
                            fdOut = open(my_cmd->redirection[i][STDOUT_FILENO], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                        }
                        else {
                            printf("Error while redirecting output to %s\n", my_cmd->redirection[i][STDOUT_FILENO]);
                        }
                        if(fdOut == -1)
                            printf("Error when openning %s : %s\n", my_cmd->redirection[0][STDOUT_FILENO], strerror(errno));
                        dup2(fdOut, 1);
                        close(fdOut);
                    }
                    if(my_cmd->redirection[i][STDERR_FILENO] != NULL) {
                        if(my_cmd->redirectionType[i][STDERR_FILENO] == APPEND) {
                            fdOut = open(my_cmd->redirection[i][STDERR_FILENO], O_CREAT | O_WRONLY | O_APPEND, S_IRWXU);
                        }
                        else if(my_cmd->redirectionType[i][STDERR_FILENO] == OVERRIDE) {
                            fdOut = open(my_cmd->redirection[i][STDERR_FILENO], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                        }
                        else {
                            printf("Error while redirecting output to %s\n", my_cmd->redirection[i][STDERR_FILENO]);
                        }
                        if(fdOut == -1)
                            printf("Error when openning %s : %s\n", my_cmd->redirection[0][STDERR_FILENO], strerror(errno));
                        dup2(fdOut, 2);
                        close(fdOut);
                    }

                }
                if(execvp(my_cmd->cmdMembersArgs[i][0], my_cmd->cmdMembersArgs[i]) == -1) {
                    perror("execvp");
                    exit(errno);
                }

                exit(EXIT_SUCCESS);
            }
            else {
                if(i != 0) {
                    close(pipes[i-1][0]);
                    close(pipes[i-1][1]);
                }
            }
        }

        int savedStdIn = dup(0);

        close(pipes[i-1][1]);
        dup2(pipes[i-1][0], 0);
        close(pipes[i-1][0]);

        // If the process didn't respond for 5 seconds, triggers the alarm
        signal(SIGALRM, alarmHandler);
        alarm(5);

        for(i = 0 ; i < nbChildren ; i++) { // Wait all the children to finish
            waitpid(pids[i], &status[i], 0);
            if(!WIFEXITED(status[i]))
                printf("Process %d didn't exit normally !\n", pids[i]);
        }

        alarm(0); // Disabling the alarm if everything's ok

        char buffer[10];
        while(fgets(buffer, 10, stdin) != NULL)
            printf("%s", buffer);

        free(pids);

        dup2(savedStdIn, 0);
        close(savedStdIn);
    }
    return 0; // SUCCESS
}
