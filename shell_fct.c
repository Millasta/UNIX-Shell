#include "shell_fct.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>

pid_t *pids;	// Pids of the children
int nbChildren;	// Number of children required

int execCd(char **args, int nbArgs) {
    char *path = NULL;
    int pathSize = 0;
	int cdStatus = 0;
    int i = 0;
	int j = 0;
    errno = 0;

    // Gets the size of the path
    for (i = 1 ; i < nbArgs; i++)
        pathSize += strlen(args[i]);
    pathSize += (nbArgs - 1);
    path = (char*)malloc((pathSize)+1 * sizeof(char));

    // Gets all the args into a unique string for the path

    for (j = 1, i = 0; j < nbArgs ; j++) {
        int k = 0;
        for (k = 0 ; k < strlen(args[j]) ; k++) {
            while(args[j][k] == '\'' || args[j][k] == '\"') // Eliminate ' and " char
                k++;
            path[i] = args[j][k];
            i++;
        }
        path[i] = ' ';
        i++;
    }

    path[pathSize - 1] = '\0';
    cdStatus = chdir(path);
    free(path);
    return cdStatus;
}

// Alarm handler
void alarmHandler(int sigNum)
{
	printf("Alarm handled: killing child processes\n");
    int i;
    for (i = 0 ; i < nbChildren ; i++)
	   kill(pids[i], SIGKILL);
}

// Executes a command
int exec_command(Cmd* my_cmd){

    /* Handling of non exec-able commands */
    if (strcmp(my_cmd->cmdMembersArgs[0][0], "cd") == 0) {			// cd command
        execCd(my_cmd->cmdMembersArgs[0], my_cmd->nbMembersArgs[0]);
    }
    else if (strcmp(my_cmd->cmdMembersArgs[0][0], "exit") == 0) {	// exit command
        return MYSHELL_FCT_EXIT; 									
    }
    else { /* Handling of exec-able commands */
		nbChildren = my_cmd->nbCmdMembers;
		pids = (pid_t*)malloc(sizeof(pid_t) * nbChildren);

		int pipes[nbChildren][2];
        int status[nbChildren];
        int childIdx = 0;
		int savedStdIn;
		int fdIn;
		int fdOut;
		char buffer[24];

        /* Creates children processes */
        for (childIdx = 0 ; childIdx < nbChildren ; childIdx++) {

			/* Creates a pipe */
			if (pipe(pipes[childIdx]) != 0) {
				perror("pipe");
				exit(errno);
			}

			/* Creates a child process */
			if ((pids[childIdx] = fork()) < 0) {
				perror("fork");
				exit(errno);
			}

			/* Current child's pattern */
            if (pids[childIdx] == 0) {

                /* Connects pipes for the current non-first-child */
                if (childIdx > 0) {

					/* Connects the non-first-child's STDIN with the previous tube's input */
                    close(pipes[childIdx - 1][1]);
                    if (dup2(pipes[childIdx - 1][0], STDIN_FILENO) == -1) {
						perror("dup2");
						exit(errno);
					}
                    close(pipes[childIdx - 1][0]);

					if (childIdx < nbChildren - 1) {
						/* Connects the non-first-non-last-child's STDOUT with the current tube's output */
		                close(pipes[childIdx][0]);
		                if (dup2(pipes[childIdx][1], STDOUT_FILENO) == -1) {
							perror("dup2");
							exit(errno);
						}
		                close(pipes[childIdx][1]);
					}
                }
                else {
					/* For the first child, check if there is an input redirection */
                    if (my_cmd->redirection[0][STDIN_FILENO] != NULL) {

						/* Opens correctly the file */
                        fdIn = open(my_cmd->redirection[0][STDIN_FILENO], O_RDONLY);
                        if (fdIn == -1) {
                            printf("Error when openning %s : %s\n", my_cmd->redirection[0][STDIN_FILENO], strerror(errno));
						}
                        else {
							/* Connects the first-child's STDIN with the file */
                            if (dup2(fdIn, STDIN_FILENO) == -1) {
								perror("dup2");
								exit(errno);
							}
                            close(fdIn);
                        }
                    }

					/* Connects the first-child's STDOUT with the current tube's output */
					close(pipes[childIdx][0]);
                    if (dup2(pipes[childIdx][1], STDOUT_FILENO) == -1) {
						perror("dup2");
						exit(errno);
					}
                    close(pipes[childIdx][1]);
                }

                if (childIdx == nbChildren - 1) {
					/* For the last child, check if there is an output or error redirection */
                    if (my_cmd->redirection[childIdx][STDOUT_FILENO] != NULL) {

						/* Opens correctly the file */
                        if (my_cmd->redirectionType[childIdx][STDOUT_FILENO] == APPEND) {
                            fdOut = open(my_cmd->redirection[childIdx][STDOUT_FILENO], O_CREAT | O_WRONLY | O_APPEND, S_IRWXU);
                        }
                        else if (my_cmd->redirectionType[childIdx][STDOUT_FILENO] == OVERRIDE) {
                            fdOut = open(my_cmd->redirection[childIdx][STDOUT_FILENO], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                        }
                        else {
                            printf("Error while redirecting output to %s\n", my_cmd->redirection[childIdx][STDOUT_FILENO]);
                        }

                        if (fdOut == -1) {
                            printf("Error when openning %s : %s\n", my_cmd->redirection[0][STDOUT_FILENO], strerror(errno));
						}

						/* Connects the last-child's STDERR with the file */
                        if (dup2(fdOut, STDOUT_FILENO) == -1) {
							perror("dup2");
							exit(errno);
						}
                        close(fdOut);
                    }
                    else if (my_cmd->redirection[childIdx][STDERR_FILENO] != NULL) {
                        if (my_cmd->redirectionType[childIdx][STDERR_FILENO] == APPEND) {
                            fdOut = open(my_cmd->redirection[childIdx][STDERR_FILENO], O_CREAT | O_WRONLY | O_APPEND, S_IRWXU);
                        }
                        else if (my_cmd->redirectionType[childIdx][STDERR_FILENO] == OVERRIDE) {
                            fdOut = open(my_cmd->redirection[childIdx][STDERR_FILENO], O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
                        }
                        else {
                            printf("Error while redirecting output to %s\n", my_cmd->redirection[childIdx][STDERR_FILENO]);
                        }

                        if (fdOut == -1) {
                            printf("Error when openning %s : %s\n", my_cmd->redirection[0][STDERR_FILENO], strerror(errno));
						}

						/* Connects the last-child's STDERR with the file */
                        if (dup2(fdOut, STDERR_FILENO) == -1) {
							perror("dup2");
							exit(errno);
						}
                        close(fdOut);
                    }
					else if (childIdx > 0) {
						/* Connects the last-child's STDOUT with the current tube's output */
		                close(pipes[childIdx][0]);
		                if (dup2(pipes[childIdx][1], STDOUT_FILENO) == -1) {
							perror("dup2");
							exit(errno);
						}
		                close(pipes[childIdx][1]);
					}
                }

				/* Executes the command */ 
                if (execvp(my_cmd->cmdMembersArgs[childIdx][0], my_cmd->cmdMembersArgs[childIdx]) == -1) {
                    perror("execvp");
                    exit(errno);
                }

                exit(EXIT_SUCCESS);
            }
            else {
                if (childIdx > 0) {
					// Close the parent's pipes no longer used
                    close(pipes[childIdx - 1][0]);
                    close(pipes[childIdx - 1][1]);
                }
            }
        }

		/* Once all the children processes are launched */
        savedStdIn = dup(STDIN_FILENO); // Stores the current STDIN_FILENO of the parent

        close(pipes[childIdx - 1][1]);
        dup2(pipes[childIdx - 1][0], STDIN_FILENO);	// Connects the parent's STDIN with the last tube's input
        close(pipes[childIdx - 1][0]);

        // If the process didn't respond for 5 seconds, triggers the alarm
        signal(SIGALRM, alarmHandler);
        alarm(5);

        for (childIdx = 0 ; childIdx < nbChildren ; childIdx++) { // Waits all the children to finish
            waitpid(pids[childIdx], &status[childIdx], 0);
            if (!WIFEXITED(status[childIdx]))
                printf("Process %d didn't exit normally !\n", pids[childIdx]);
        }

        alarm(0); // Disabling the alarm if everything's ok

        while (fgets(buffer, 24, stdin) != NULL)
            printf("%s", buffer);

        free(pids);

        dup2(savedStdIn, STDIN_FILENO); // Sets back the correct STDIN_FILENO for the parent
        close(savedStdIn);
    }

    return 0; // SUCCESS
}
