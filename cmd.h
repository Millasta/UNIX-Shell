#include <stdlib.h>

//Command is well-formed
#define MYSHELL_CMD_OK 0
#define APPEND 1
#define OVERRIDE 2

typedef struct Cmd {
    //the command originally inputed by the user
    char *initCmd;

    //number of members
    unsigned int nbCmdMembers;

    //each position holds a command member
    char **cmdMembers;

    //cmd_members_args[i][j] holds the jth argument of the ith member
    char ***cmdMembersArgs;

    //number of arguments per member
    unsigned int *nbMembersArgs;

    //the path to the redirection file
    char ***redirection;

    //the redirection type (append vs. override)
    int **redirectionType;
} Cmd;

//Prints the command
void printCmd(Cmd *cmd);
//Frees memory associated to a cmd
void freeCmd(Cmd  * cmd);
//Initializes the initial_cmd, membres_cmd et nb_membres fields
void parseMembers(char *s,Cmd *c);
