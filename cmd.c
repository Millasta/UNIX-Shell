#include "cmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

// Prints the command
void printCmd(Cmd *cmd){
    int memberIdx = 0;
	int memberArgIdx = 0;
	char * filenos[3] = {"STDIN_FILENO", "STDOUT_FILENO", "STDERR_FILENO"};
	char * macros[2] =  {"APPEND", "OVERRIDE"};

	printf("cmd->initCmd\t\t\t\t = \t%s\n", cmd->initCmd);
	printf("cmd->nbCmdMembers\t\t\t = \t%d\n", cmd->nbCmdMembers);

	for (memberIdx = 0; memberIdx < cmd->nbCmdMembers; memberIdx++) {
		printf("cmd->cmdMembers[%d]\t\t\t = \t%s\n", memberIdx, cmd->cmdMembers[memberIdx]);
	}

	for (memberIdx = 0; memberIdx < cmd->nbCmdMembers; memberIdx++) {
		printf("cmd->nbMembersArgs[%d]\t\t\t = \t%d\n", memberIdx, cmd->nbMembersArgs[memberIdx]);
	}

	for (memberIdx = 0; memberIdx < cmd->nbCmdMembers; memberIdx++) {
		for (memberArgIdx = 0; memberArgIdx < cmd->nbMembersArgs[memberIdx]; memberArgIdx++) {
			printf("cmd->cmdMembersArgs[%d][%d]\t\t = \t%s\n", memberIdx, memberArgIdx, cmd->cmdMembersArgs[memberIdx][memberArgIdx]);
		}
	}	

	for (memberIdx = 0; memberIdx < cmd->nbCmdMembers; memberIdx++) {
		for (memberArgIdx = 0; memberArgIdx < 3; memberArgIdx++) {
			printf("cmd->redirection[%d][%s]\t = \t%s\n", memberIdx, filenos[memberArgIdx], cmd->redirection[memberIdx][memberArgIdx]);
		}
	}

	for (memberIdx = 0; memberIdx < cmd->nbCmdMembers; memberIdx++) {
		for (memberArgIdx = 0; memberArgIdx < 3; memberArgIdx++) {
			if (cmd->redirectionType[memberIdx][memberArgIdx] == APPEND || cmd->redirectionType[memberIdx][memberArgIdx] == OVERRIDE) {
				printf("cmd->redirectionType[%d][%s]\t = \t%s\n", memberIdx, filenos[memberArgIdx], macros[cmd->redirectionType[memberIdx][memberArgIdx] - 1]);
			}
		}
	}
}

// Initializes the initial_cmd, membres_cmd et nb_membres fields
void parseMembers(char *inputString, Cmd *cmd){
	int memberLength;
	int buffIdx = 1;
	int memberIdx = 0;
 	char strBuff[255];
    char *currentChar;

    cmd->initCmd = strdup(inputString);
    cmd->cmdMembers  = (char**)malloc(sizeof(char*));
    cmd->nbCmdMembers = 0;

	memset(strBuff, '\0', 255);

	currentChar = &inputString[0];
    while (*currentChar == ' ')
        currentChar++;

	/* Parses the string a first time and store the members */
    while (*currentChar != '\0') {
        memberLength = 0;

		/* Stores the member in the buffer */
	    while (*currentChar != '|' && *currentChar != '\0') { 
            strBuff[memberLength] = *currentChar;
            memberLength++;
            currentChar++;
        }

		/* Removes the blanks at the end of the buffer */
        while (*(currentChar - buffIdx) == ' ') {
            strBuff[memberLength - 1] = '\0';
            memberLength--;
            buffIdx++;
        }

		/* Add the member to the cmd object */
        cmd->nbCmdMembers++;
        cmd->cmdMembers = (char**)realloc(cmd->cmdMembers, sizeof(char*) * cmd->nbCmdMembers);
        cmd->cmdMembers[cmd->nbCmdMembers - 1] = strdup(strBuff);
        cmd->cmdMembers[cmd->nbCmdMembers - 1][memberLength] = '\0';
        cmd->redirectionType = (int**)malloc(sizeof(int*) * cmd->nbCmdMembers);

        memset(strBuff, '\0', 255);

		
        while (*currentChar == ' ' || *currentChar == '|')
            currentChar++;
    }

    cmd->cmdMembersArgs = (char***)malloc(sizeof(char**) * cmd->nbCmdMembers);
    cmd->nbMembersArgs = (unsigned int*)malloc(sizeof(unsigned int) * cmd->nbCmdMembers);
    cmd->redirection = (char***)malloc(sizeof(char**) * cmd->nbCmdMembers);

	/* Parses the members and store the args */
    for (memberIdx = 0 ; memberIdx < cmd->nbCmdMembers ; memberIdx++) {

        int memberArgIdx = 0;
        int redirectionType = 0;
        char buffer[255];

		buffIdx = 0;

        cmd->cmdMembersArgs[memberIdx] = (char**)malloc(sizeof(char*));
        cmd->redirection[memberIdx] = (char**)malloc(sizeof(char*) * 3);
        cmd->redirection[memberIdx][STDIN_FILENO] = NULL;
        cmd->redirection[memberIdx][STDOUT_FILENO] = NULL;
        cmd->redirection[memberIdx][STDERR_FILENO] = NULL;
        cmd->redirectionType[memberIdx] = (int*)malloc(sizeof(int) * 3);
        cmd->nbMembersArgs[memberIdx] = 0;

        while (cmd->cmdMembers[memberIdx][memberArgIdx] != '\0') {
            cmd->nbMembersArgs[memberIdx]++;
            buffIdx = 0;

            // Redirection
            if ((cmd->cmdMembers[memberIdx][memberArgIdx] == '2' && cmd->cmdMembers[memberIdx][memberArgIdx + 1] == '>') || cmd->cmdMembers[memberIdx][memberArgIdx] == '>') {
                int error = 0;
                if (cmd->cmdMembers[memberIdx][memberArgIdx] == '2') {
                    memberArgIdx++;
                    error = 1;
                }
                redirectionType = OVERRIDE;
                memberArgIdx++;

                if (cmd->cmdMembers[memberIdx][memberArgIdx] == '>')
                    redirectionType = APPEND;
                memberArgIdx++;

                // Reaching next non-space char
                while (cmd->cmdMembers[memberIdx][memberArgIdx] == ' ')
                    memberArgIdx++;

                while (cmd->cmdMembers[memberIdx][memberArgIdx] != '|' && cmd->cmdMembers[memberIdx][memberArgIdx] != '\0') {
                    buffer[buffIdx] = cmd->cmdMembers[memberIdx][memberArgIdx];
                    buffIdx++;
                    memberArgIdx++;
                }
                buffer[buffIdx] = '\0';

                //Clearing spaces at the end of the buff
                while (buffer[buffIdx - 1] == ' ') {
                    buffer[buffIdx - 1] = '\0';
                    buffIdx--;
                }

                if (!error) {
                    cmd->redirection[memberIdx][STDOUT_FILENO] = (char*)malloc(sizeof(char) * strlen(buffer));
                    cmd->redirection[memberIdx][STDOUT_FILENO] = strdup(buffer);
                    cmd->redirectionType[memberIdx][STDOUT_FILENO] = redirectionType;
                }
                else {
                    cmd->redirection[memberIdx][STDERR_FILENO] = (char*)malloc(sizeof(char) * strlen(buffer));
                    cmd->redirection[memberIdx][STDERR_FILENO] = strdup(buffer);
                    cmd->redirectionType[memberIdx][STDERR_FILENO] = redirectionType;
                }
                cmd->nbMembersArgs[memberIdx]--;

                // Reaching the next non-space char
                while (cmd->cmdMembers[memberIdx][memberArgIdx] == ' ')
                    memberArgIdx++;
            }
            else if (cmd->cmdMembers[memberIdx][memberArgIdx] == '<') {
                redirectionType = OVERRIDE;
                memberArgIdx++;
                if (cmd->cmdMembers[memberIdx][memberArgIdx] == '<')
                    redirectionType = APPEND;
                memberArgIdx++;

                // Reaching the next non-space char
                while (cmd->cmdMembers[memberIdx][memberArgIdx] == ' ')
                    memberArgIdx++;

                while (cmd->cmdMembers[memberIdx][memberArgIdx] != '|' && cmd->cmdMembers[memberIdx][memberArgIdx] != '\0' && cmd->cmdMembers[memberIdx][memberArgIdx] != '>') {
                    buffer[buffIdx] = cmd->cmdMembers[memberIdx][memberArgIdx];
                    buffIdx++;
                    memberArgIdx++;
                }
                buffer[buffIdx] = '\0';

                // Clearing spaces at the end of the buff
                while (buffer[buffIdx - 1] == ' ') {
                    buffer[buffIdx - 1] = '\0';
                    buffIdx--;
                }

                cmd->redirection[memberIdx][STDIN_FILENO] = (char*)malloc(sizeof(char) * strlen(buffer));
                cmd->redirection[memberIdx][STDIN_FILENO] = strdup(buffer);
                cmd->redirectionType[memberIdx][STDIN_FILENO] = redirectionType;
                cmd->nbMembersArgs[memberIdx]--;

                // Reaching the next non-space char
                while (cmd->cmdMembers[memberIdx][memberArgIdx] == ' ')
                    memberArgIdx++;
            }
            else {
                while (cmd->cmdMembers[memberIdx][memberArgIdx] != '\0' && cmd->cmdMembers[memberIdx][memberArgIdx] != ' ') {
                    buffer[buffIdx] = cmd->cmdMembers[memberIdx][memberArgIdx];
                    memberArgIdx++;
                    buffIdx++;
                }

                // Adding the argument
                buffer[buffIdx] = '\0';
                cmd->cmdMembersArgs[memberIdx] = (char**)realloc(cmd->cmdMembersArgs[memberIdx], sizeof(char*) * cmd->nbMembersArgs[memberIdx]);
                cmd->cmdMembersArgs[memberIdx][cmd->nbMembersArgs[memberIdx] - 1] = (char*)malloc(sizeof(char) * strlen(buffer));
                cmd->cmdMembersArgs[memberIdx][cmd->nbMembersArgs[memberIdx] - 1] = strdup(buffer);
                while (cmd->cmdMembers[memberIdx][memberArgIdx] == ' ')
                    memberArgIdx++;
            }
        }

        // Adding the NULL pointer for execvp()
        cmd->cmdMembersArgs[memberIdx] = (char**)realloc(cmd->cmdMembersArgs[memberIdx], sizeof(char*) * (cmd->nbMembersArgs[memberIdx] + 1));
        cmd->cmdMembersArgs[memberIdx][cmd->nbMembersArgs[memberIdx]] = NULL;
    }

	//printCmd(cmd);
}

// Frees memory associated to a cmd
void freeCmd(Cmd * cmd){
    int memberIdx = 0;
	int memberArgIdx = 0;

    for (memberIdx = 0 ; memberIdx < cmd->nbCmdMembers ; memberIdx++) {
        for (memberArgIdx = 0 ; memberArgIdx < cmd->nbMembersArgs[memberIdx] ; memberArgIdx++) {
            free(cmd->cmdMembersArgs[memberIdx][memberArgIdx]);
	}
		    if (cmd->redirection[memberIdx][STDIN_FILENO] != NULL) {
		        free(cmd->redirection[memberIdx][STDIN_FILENO]);
			}
		    if (cmd->redirection[memberIdx][STDOUT_FILENO] != NULL) {
		        free(cmd->redirection[memberIdx][STDOUT_FILENO]);
			}
		    if (cmd->redirection[memberIdx][STDERR_FILENO] != NULL) {
		        free(cmd->redirection[memberIdx][STDERR_FILENO]);
			}		

        free(cmd->cmdMembers[memberIdx]);
        free(cmd->redirection[memberIdx]);
        free(cmd->redirectionType[memberIdx]);
        free(cmd->cmdMembersArgs[memberIdx]);
    }

    free(cmd->redirectionType);
    free(cmd->redirection);
    free(cmd->nbMembersArgs);
    free(cmd->cmdMembersArgs);
    free(cmd->cmdMembers);
    free(cmd->initCmd);
    free(cmd);
}
