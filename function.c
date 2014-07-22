#include "function.h"

void execute_command(char * commandLine, char ** parameters){
	int status;
	int flag=0;
	int need_pipe=0;
	char * file_name;
	char * output_file="default";
	
	flag=get_filename(commandLine,file_name);
	if (flag!=0){
		if(strlen(file_name)>0) output_file=file_name;
	}

	if(strlen(commandLine)<=0) return;

	if(strstr(commandLine,"$(")) need_pipe=1;
	
	if(need_pipe==1){
		execute_pipe(commandLine);
		return;
	}
    
    	check_dollar(commandLine);

	get_parameters(commandLine,parameters);
	
	if(strcmp(commandLine,"cd")==0){
		change_directory(commandLine,parameters);
		return;
	}
	
	if(strcmp(commandLine,"calc")==0){
        	printf("%f\n",calculator(commandLine,parameters[1]));
        	return;
	}
    	
	if (strcmp(parameters[0],"var")==0){
        	set_one_variable(commandLine,parameters);
        	return;
    	}
    
	if(fork()!=0){
		waitpid(-1,&status,0);
	}else{
		if(flag==1){
			int fd = open(output_file, O_RDWR | O_CREAT,S_IRUSR | S_IWUSR);
			dup2(fd,1);
			close(fd);
		}
		if(execvp(commandLine,parameters)==-1){
			printf("command not found\n");
			exit(0);
		}
	}
}



void execute_pipe(char * commandLine){
	char * commands[256];
	char * params[256];
	int n=0,i=0,t=0;

	get_commands_pipe(commandLine,commands);
	while(commands[i]){
		if(i>0)commands[i]=commands[i]+1;
		if(commands[i][strlen(commands[i])-1]==' ') commands[i][strlen(commands[i])-1]='\0';
		i++;
	}
	n=i;
	i=0;

	int fdin=-1,fdout;
	for(i=n-1;i>=0;i--){
		int fd[2];

		if(i>0){
			pipe(fd);
			fdout=fd[1];
		}else{
			fdout=-1;
		}
		get_parameters(commands[i],params);
		exec_one_command(commands[i],params,fdin,fdout);

		close(fdin);
		close(fdout);

		fdin=fd[0];
	}
}

void exec_one_command(char * command, char ** parameters, int fdin, int fdout){
	pid_t child=fork();
	
	if(child!=0){
	}else{
		if(fdin!=-1&&fdin!=1){
			dup2(fdin,0);
			close(fdin);
		}
		if(fdout!=-1&&fdin!=1){
			dup2(fdout,1);
			close(fdout);
		}
		execvp(command,parameters);
	}
}

void change_directory(char * commandLine, char ** parameters){
	if(parameters[1]==NULL){
		chdir(getenv("HOME"));
	}else{
		chdir(parameters[1]);
	}
}

void check_dollar(char * commandLine){
	int i=0;
	int j=0;
	int ref=0;
	int founded=0;
	int tmp=0;
	int endref=0;
	char tmpVar[256];
	char variable[256];
    
	while (commandLine[i]!='\0'){
		founded=0;
		j=0;
		if (commandLine[i]=='$'){
			ref = i-1;
			tmp = i+1;
			while (commandLine[tmp]!='\0'){
				tmpVar[j]=commandLine[tmp];
				tmpVar[j+1]='\0';
				if (!(!(getenv(tmpVar)))){
					strcpy(variable,tmpVar);
					founded=1;
					endref=tmp+1;
				}
				j++;
				tmp++;
			}
			if ((founded==0)&&(commandLine[tmp]=='\0')){
				printf("Variable not correct\n");
				break;
			}else{
				i=transform_commandLine(commandLine, variable, ref, endref);
			}
		}
		i++;
	}
}

int transform_commandLine(char * commandLine, char * variable, int ref, int endref){
	char beginCommandLine[256];
	char endCommandLine[256];
	char Test[512];
	int result=0;
	int i=0;
    
	for (i=0;i<ref;i++){
		beginCommandLine[i]=commandLine[i];
	}
	beginCommandLine[i]='\0';
    
	for (i=endref;i<strlen(commandLine);i++){
		endCommandLine[i-endref]=commandLine[i];
	}
	endCommandLine[i-endref]='\0';
	
	strncpy(commandLine,strcat(beginCommandLine," "),strlen(beginCommandLine)+1);
	strcat(commandLine,getenv(variable));
	result=strlen(commandLine);
	strcat(commandLine,endCommandLine);
	
	return (result);
}

void read_command(char * commandLine){
	signal(SIGINT,ctrl_c);
	setjmp(jbuffer);

	char c;
	int command_length=0;

	while((c=getchar())!='\n'){
		commandLine[command_length++]=c;
	}
	commandLine[command_length]='\0';
}


void get_parameters(char * commandLine, char ** parameters){
	while(*commandLine!='\0'){
		while(*commandLine==' ') *commandLine++='\0';
		*parameters++=commandLine;
		while(*commandLine!='\0' && *commandLine!=' ') commandLine++;
	}
	*parameters='\0';
}


void get_commands_pipe(char * commandLine, char ** commands){
	while(*commandLine!='\0'){
		while(*commandLine=='$'){
			*commandLine++='\0';
		}
		*commands++=commandLine;
		while(*commandLine!='\0' && *commandLine!='$') commandLine++;
	}
	*commands='\0';
}

void get_profile(){
	FILE * file=fopen("PROFILE","r");
	if(file==NULL){
		printf("PROFILE file not found, default home directory\n");
		setenv("HOME","/root",1);
		chdir("/root");
		return;
	}
	char tmp1[256];
	char tmp2[256];

	while(fgets(tmp1,sizeof(tmp1),file)){

	int j=0;
	int i=0;
	while(tmp1[i]!='='){
		tmp2[i]=tmp1[i];
		i++;
	}
	tmp2[i]='\0';

	if(strcmp(tmp2,"PATH")==0){
		i++;
		while(tmp1[i]!='\0'&&tmp1[i]!='\n'){
			tmp2[j]=tmp1[i];
			j++;
			i++;
		}
		tmp2[j]='\0';
		setenv("PATH",tmp2,1);
	}

	if(strcmp(tmp2,"PROMPT")==0){
		i++;
		while(tmp1[i]!='\0'&&tmp1[i]!='\n'){
			tmp2[j]=tmp1[i];
			j++;
			i++;
		}
		tmp2[j]='\0';
		setenv("PROMPT",tmp2,1);
	}

	if(strcmp(tmp2,"HOME")==0){
		i++;
		while(tmp1[i]!='\0'&&tmp1[i]!='\n'){
			tmp2[j]=tmp1[i];
			j++;
			i++;
		}
		tmp2[j]='\0';
		setenv("HOME",tmp2,1);
		chdir(tmp2);
	}
	}
	fclose(file);
}

void get_variables(){
	FILE * file=fopen("VARIABLES","r");
	int count = 0;
	if (file == NULL){
		printf("No existing variables\n");
	}else{
		char tmpChar[1024];
		char tmpName[256];
		char tmpValue[256];
        
		while (fgets(tmpChar, sizeof(tmpChar),file)){
			int i =	0;
			int j = 0;
			while(tmpChar[i] != '='){
				tmpName[j]=tmpChar[i];
				i++;
				j++;
			}
			tmpName[j]='\0';
			i++;
			j = 0;
			while((tmpChar[i] != '0') && (tmpChar[i] != '\n')){
				tmpValue[j]=tmpChar[i];
				i++;
				j++;
			}
			tmpValue[j]='\0';
			strncpy(myVariables[count].name, tmpName, strlen(tmpName));
			myVariables[count].value = atoi(tmpValue);
			setenv(myVariables[count].name, tmpValue, 1);
			count++;
		}
		fclose(file);
	}
	char tmpCount[256];
	sprintf(tmpCount, "%d",count);
	setenv("nbVariables",tmpCount,1);
}

void set_variables(){
	FILE * file=fopen("VARIABLES","w");
	int i = 0;
	for(i = 0; i < atoi(getenv("nbVariables"))-1; i++){
		fprintf(file,"%s=%d\n",myVariables[i].name, myVariables[i].value);
	}
	fprintf(file,"%s=%d",myVariables[i].name, myVariables[i].value);
	fclose(file);
}

void set_one_variable(char * commandLine,char ** parameters){
	int count = 0;
	while(parameters[count]!='\0'){
		count++;
	}
	if (count!=2)printf("Invalid input\nThe valid input is: var \"variable\"=\"value\"\n");
	else{
		char tmp[256];
		strcat(parameters[1],"\0");
		strncpy(tmp,parameters[1],256);
		int nbOfEgals=0;
		int i=0;
		for (i=0;i<strlen(tmp);i++){
			if (tmp[i]=='=') nbOfEgals++;
		}
		if (nbOfEgals!=1)printf("Invalid input\nThe valid input is: var \"variable\"=\"value\"\n");
		else{
			char tmpName[256];
			char tmpValue[256];
			int error=0;
			i=0;
			int j=0;
			while(tmp[i]!='='){
				tmpName[j]=tmp[i];
				i++;
				j++;
			}
			tmpName[j]='\0';
			if (strlen(tmpName)==0)printf("Invalid input\nThe valid input is: var \"variable\"=\"value\"\n");
			else {
				i++;
				j=0;
				while (tmp[i]!='\0'){
					tmpValue[j]=tmp[i];
					i++;
					j++;
				}
				tmpValue[j]='\0';
				if (strlen(tmpValue)==0)printf("Invalid input\nThe valid input is: var \"variable\"=\"value\"\n");
				else{
					i=0;
					int error=0;
					while ((tmpValue[i]!='\0')&&(error==0)){
						if (isdigit(tmpValue[i])==0)error=1;
						i++;
					}
					if (error == 1) printf("Invalid input\nThe valide input is: var \"variable\"=\"value\"\nValue must be a integer\n");
					else{
						int count = atoi(getenv("nbVariables"));
						if (!getenv(tmpName)){
							strncpy(myVariables[count].name,tmpName,256);
							myVariables[count].value = atoi(tmpValue);
							setenv(myVariables[count].name,tmpValue,1);
							count++;
							char tmpCount[256];
							sprintf(tmpCount,"%d",count);
							setenv("nbVariables",tmpCount,1);
						}else{
							setenv(tmpName,tmpValue,1);
							for(i=0;i<count;i++){
								if (strcmp(myVariables[i].name,tmpName)==0){
									myVariables[i].value = atoi(tmpValue);
									break;
								}
							}
						}
					}
				}
			}
		}
	}
}

void print_prompt(){
	char * prompt=getenv("PROMPT");
	char buffer[256];
	printf("%s%s",getcwd(buffer,256),prompt);
}


void ctrl_c(int sig){
	char c[16];
	printf("\nExit ? [y/n]: ");
	scanf("%s",c);

	if(strcmp(c,"y")==0) exit(0);
	else longjmp(jbuffer,0);
}


float calculator(char * commandLine, char * parameters){

    int n=0,o=0,i=0,t=0;
    char operation[256];
    float numbers[256];
    char operators[256];
    
    if(parameters!=NULL)
    	strcpy(operation,parameters);
    else
	return 0;

    if(strchr(operation,'(')!=NULL){
	int x=strchr(operation,'(')-operation;
	char result[256];
	int t=sprintf(result,"%f",calculator(commandLine,parameters+x+1));

	operation[x]='\0';
	strcat(operation,result);
    }
   
    
    while(i<strlen(operation)){
        t=0;
	char tmp[256]={};
        while(operation[i]!='+' && operation[i]!='-' &&operation[i]!='*' && operation[i]!='/' && operation[i]!='\0'){
            tmp[t]=operation[i];
            i++;
            t++;
        }
	
       	numbers[n]=atof(tmp);
	n++;

        operators[o]=operation[i];
        o++;
        i++;
    }
    operators[o-1]='\0';
    
    // On execute les * et /
    o=0;
    while(o<strlen(operators)){
        if(operators[o]=='/') numbers[o]=numbers[o]/numbers[o+1];

        if(operators[o]=='*') numbers[o]=numbers[o]*numbers[o+1];

        if(operators[o]=='/' || operators[o]=='*'){
            for(i=o+1;i<n-1;i++){
                numbers[i]=numbers[i+1];
            }

	    for(i=o;i<strlen(operators)-1;i++){
                operators[i]=operators[i+1];
            }
            operators[strlen(operators)-1]='\0';
            o--;
        }
        o++;
    }

    //on execute + et -
    o=0;
    while(o<strlen(operators)){
        if(operators[o]=='-') numbers[o]=numbers[o]-numbers[o+1];

        if(operators[o]=='+') numbers[o]=numbers[o]+numbers[o+1];

        if(operators[o]=='-' || operators[o]=='+'){
            for(i=o+1;i<n-1;i++){
                numbers[i]=numbers[i+1];
            }
            for(i=o;i<strlen(operators)-1;i++){
                operators[i]=operators[i+1];
            }
            operators[strlen(operators)-1]='\0';
            o--;
        }
        o++;
    }

   return(numbers[0]); 

}



int get_filename(char * commandLine, char  * file_name){
	char * out_file;
	int flag=0;
	int pos=0;
	int pos_name=0;
	int i=0,j=0;

	out_file=strstr(commandLine, "=>");

	if(out_file!=NULL){
		i=2;
		j=0;

		while(out_file[i]!='\0'){
			if(out_file[i]!=' '){
				file_name[j]=out_file[i];
				j++;
			}
			i++;
		}
		file_name[j]='\0';
		pos=(int)(out_file-commandLine);
		commandLine[pos-1]='\0';
		flag=1;
	}
	return flag;
}
