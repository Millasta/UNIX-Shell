#include <stdlib.h>

#define MYSHELL_CMD_OK 0	// Command is well-formed
#define APPEND 1
#define OVERRIDE 2

typedef struct Cmd {
    char *initCmd;				// The command originally inputed by the user
    unsigned int nbCmdMembers;	// Number of members
    char **cmdMembers;			// Each position holds a command member
    char ***cmdMembersArgs;		// cmd_members_args[i][j] holds the jth argument of the ith member
    unsigned int *nbMembersArgs;// Number of arguments per member
    char ***redirection;		// The path to the redirection file
    int **redirectionType;		// The redirection type (append vs. override)
} Cmd;

void printCmd(Cmd *cmd);				// Prints the command
void freeCmd(Cmd  * cmd);				// Frees memory associated to a cmd
void parseMembers(char *str, Cmd *c); 	// Initializes the members and arguments of the cmd
