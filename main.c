

#include "function.h"


int main (void){

	char commandLine[256];
	char * parameters[256];

	get_profile();
    	get_variables();

	while(TRUE){
		print_prompt();

		read_command(commandLine);
		if(strcmp("exit",commandLine)==0){
            	set_variables();
			printf("Hasta la vista\n");
			exit(0);
		}

		execute_command(commandLine,parameters);
	}

	return 0;
}
