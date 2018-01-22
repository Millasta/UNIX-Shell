#include "cmd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

//Prints the command
void printCmd(Cmd *cmd){
	//your implementation comes here
}

//Initializes the initial_cmd, membres_cmd et nb_membres fields
void parseMembers(char *inputString, Cmd *cmd){
    //Your implementation comes here
    cmd->initCmd = strdup(inputString);
    cmd->cmdMembers  = (char**)malloc(sizeof(char*));
    cmd->nbCmdMembers = 0;
    char *currentChar = &inputString[0];
    while(*currentChar == ' ')
        currentChar++;


    char strBuff[255];
    memset(strBuff, '\0', 255);
    int memberLength;
    while(*currentChar != '\0') {
        memberLength = 0;
        while(*currentChar != '|' && *currentChar != '\0') {
            strBuff[memberLength] = *currentChar;
            memberLength++;
            currentChar++;
        }
        int i = 1;
        while(*(currentChar - i) == ' ') {
            strBuff[memberLength - 1] = '\0';
            memberLength--;
            i++;
        }
        cmd->nbCmdMembers++;
        cmd->cmdMembers = (char**)realloc(cmd->cmdMembers, sizeof(char*) * cmd->nbCmdMembers);
        cmd->cmdMembers[cmd->nbCmdMembers - 1] = strdup(strBuff);
        cmd->cmdMembers[cmd->nbCmdMembers - 1][memberLength] = '\0';
        cmd->redirectionType = (int**)malloc(sizeof(int*) * cmd->nbCmdMembers);

        memset(strBuff, '\0', 255);

        while(*currentChar == ' ' || *currentChar == '|')
            currentChar++;
    }

    int j;
    cmd->cmdMembersArgs = (char***)malloc(sizeof(char**) * cmd->nbCmdMembers);
    cmd->nbMembersArgs = (unsigned int*)malloc(sizeof(unsigned int) * cmd->nbCmdMembers);
    cmd->redirection = (char***)malloc(sizeof(char**) * cmd->nbCmdMembers);

    for(j = 0 ; j < cmd->nbCmdMembers ; j++) {
        cmd->cmdMembersArgs[j] = (char**)malloc(sizeof(char*));
        cmd->redirection[j] = (char**)malloc(sizeof(char*) * 3);
        cmd->redirection[j][STDIN_FILENO] = NULL;
        cmd->redirection[j][STDOUT_FILENO] = NULL;
        cmd->redirection[j][STDERR_FILENO] = NULL;
        cmd->redirectionType[j] = (int*)malloc(sizeof(int) * 3);
        cmd->nbMembersArgs[j] = 0;
        // DEBUG
        //printf("\nMember : %s\n", cmd->cmdMembers[j]);
        int index = 0;
        int indexBuff = 0;
        int redirectionType = 0;
        char buffer[255];
        while(cmd->cmdMembers[j][index] != '\0') {
            cmd->nbMembersArgs[j]++;
            indexBuff = 0;

            // Redirection
            if((cmd->cmdMembers[j][index] == '2' && cmd->cmdMembers[j][index + 1] == '>') || cmd->cmdMembers[j][index] == '>') {
                int error = 0;
                if(cmd->cmdMembers[j][index] == '2') {
                    index++;
                    error = 1;
                }
                redirectionType = OVERRIDE;
                index++;

                if(cmd->cmdMembers[j][index] == '>')
                    redirectionType = APPEND;
                index++;

                // Reaching next non-space char
                while(cmd->cmdMembers[j][index] == ' ')
                    index++;

                while(cmd->cmdMembers[j][index] != '|' && cmd->cmdMembers[j][index] != '\0') {
                    buffer[indexBuff] = cmd->cmdMembers[j][index];
                    indexBuff++;
                    index++;
                }
                buffer[indexBuff] = '\0';

                //Clearing spaces at the end of the buff
                while(buffer[indexBuff - 1] == ' ') {
                    buffer[indexBuff - 1] = '\0';
                    indexBuff--;
                }

                if(!error) {
                    cmd->redirection[j][STDOUT_FILENO] = (char*)malloc(sizeof(char) * strlen(buffer));
                    cmd->redirection[j][STDOUT_FILENO] = strdup(buffer);
                    cmd->redirectionType[j][STDOUT_FILENO] = redirectionType;
                }
                else {
                    cmd->redirection[j][STDERR_FILENO] = (char*)malloc(sizeof(char) * strlen(buffer));
                    cmd->redirection[j][STDERR_FILENO] = strdup(buffer);
                    cmd->redirectionType[j][STDERR_FILENO] = redirectionType;
                }
                cmd->nbMembersArgs[j]--;

                // Reaching the next non-space char
                while(cmd->cmdMembers[j][index] == ' ')
                    index++;
            }
            else if(cmd->cmdMembers[j][index] == '<') {
                redirectionType = OVERRIDE;
                index++;
                if(cmd->cmdMembers[j][index] == '<')
                    redirectionType = APPEND;
                index++;

                // Reaching the next non-space char
                while(cmd->cmdMembers[j][index] == ' ')
                    index++;

                while(cmd->cmdMembers[j][index] != '|' && cmd->cmdMembers[j][index] != '\0' && cmd->cmdMembers[j][index] != '>') {
                    buffer[indexBuff] = cmd->cmdMembers[j][index];
                    indexBuff++;
                    index++;
                }
                buffer[indexBuff] = '\0';

                //Clearing spaces at the end of the buff
                while(buffer[indexBuff - 1] == ' ') {
                    buffer[indexBuff - 1] = '\0';
                    indexBuff--;
                }

                cmd->redirection[j][STDIN_FILENO] = (char*)malloc(sizeof(char) * strlen(buffer));
                cmd->redirection[j][STDIN_FILENO] = strdup(buffer);
                cmd->redirectionType[j][STDIN_FILENO] = redirectionType;
                cmd->nbMembersArgs[j]--;

                // Reaching the next non-space char
                while(cmd->cmdMembers[j][index] == ' ')
                    index++;
            }

            else {
                while(cmd->cmdMembers[j][index] != '\0' && cmd->cmdMembers[j][index] != ' ') {
                    buffer[indexBuff] = cmd->cmdMembers[j][index];
                    index++;
                    indexBuff++;
                }

                // Adding arg
                buffer[indexBuff] = '\0';
                cmd->cmdMembersArgs[j] = (char**)realloc(cmd->cmdMembersArgs[j], sizeof(char*) * cmd->nbMembersArgs[j]);
                cmd->cmdMembersArgs[j][cmd->nbMembersArgs[j]-1] = (char*)malloc(sizeof(char) * strlen(buffer));
                cmd->cmdMembersArgs[j][cmd->nbMembersArgs[j]-1] = strdup(buffer);
                while(cmd->cmdMembers[j][index] == ' ')
                    index++;
            }
        }

        // Adding the NULL pointer for execvp()
        cmd->cmdMembersArgs[j] = (char**)realloc(cmd->cmdMembersArgs[j], sizeof(char*) * (cmd->nbMembersArgs[j] + 1));
        cmd->cmdMembersArgs[j][cmd->nbMembersArgs[j]] = NULL;

        // DEBUG
        /*printf("Nb args : %d\n", cmd->nbMembersArgs[j]);
        printf("Redirection :\n");
        if(cmd->redirection[j][STDIN_FILENO] != NULL) {
            printf("STDIN : %s Type : %d\n", cmd->redirection[j][STDIN_FILENO], cmd->redirectionType[j][STDIN_FILENO]);
        }
        if(cmd->redirection[j][STDOUT_FILENO] != NULL) {
            printf("STDOUT : %s Type : %d\n", cmd->redirection[j][STDOUT_FILENO], cmd->redirectionType[j][STDOUT_FILENO]);
        }
        if(cmd->redirection[j][STDERR_FILENO] != NULL) {
            printf("STDERR : %s Type : %d\n", cmd->redirection[j][STDERR_FILENO], cmd->redirectionType[j][STDERR_FILENO]);
        }*/
    }
}

//Frees memory associated to a cmd
void freeCmd(Cmd * cmd){
    int i;
    for(i = 0 ; i < cmd->nbCmdMembers ; i++) {
        int j;
        for(j = 0 ; j < cmd->nbMembersArgs[i] ; j++)
            free(cmd->cmdMembersArgs[i][j]);
        if(cmd->redirection[i][STDIN_FILENO] != NULL)
            free(cmd->redirection[i][STDIN_FILENO]);
        if(cmd->redirection[i][STDOUT_FILENO] != NULL)
            free(cmd->redirection[i][STDOUT_FILENO]);
        if(cmd->redirection[i][STDERR_FILENO] != NULL)
            free(cmd->redirection[i][STDERR_FILENO]);
        free(cmd->cmdMembers[i]);
        free(cmd->redirection[i]);
        free(cmd->redirectionType[i]);
        free(cmd->cmdMembersArgs[i]);
    }
    free(cmd->redirectionType);
    free(cmd->redirection);
    free(cmd->nbMembersArgs);
    free(cmd->cmdMembersArgs);
    free(cmd->cmdMembers);
    free(cmd->initCmd);
    free(cmd);
}
