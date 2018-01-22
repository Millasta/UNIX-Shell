#include <pwd.h>
#include <readline/readline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include "shell_fct.h"

//To complete
int main(int argc, char** argv)
{
	//..........
	int ret = MYSHELL_CMD_OK;
	char* readlineptr;
	struct passwd* infos;
	char str[1024] = {0};
	char hostname[256];
	char workingdirectory[256];

	//..........
	while(ret != MYSHELL_FCT_EXIT)
	{
		//Get your session info
        infos=getpwuid(getuid());
		gethostname(hostname, 256);
		getcwd(workingdirectory, 256);
        //Print it to the console
		sprintf(str, "\n{myshell}%s@%s:%s$ ", infos->pw_name, hostname, workingdirectory);
		readlineptr = readline(str);

        //Your code goes here.......
        //Parse the comand
		Cmd *cmd = malloc(sizeof(Cmd));
		parseMembers(readlineptr ,cmd);

        //Execute the comand
		ret = exec_command(cmd);

		//Clean the house
		freeCmd(cmd);
		free(readlineptr);
		//Clear str
		memset(str, '\0', 1024);
	}
	return 0;
}
