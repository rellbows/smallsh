// dev script for prompt used in smallsh

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cd(char* pathName);
void status(int childExitMethod);

int MAXARGS = 513; // 512 args + NULL
int MAXARGSIZE = 100;

int main(){

	// testing
	int loopGuard = 0;

	char* input = NULL;
	size_t cmdLength = 2049;
	//input = (char*) malloc(cmdLength * sizeof(char));
	//memset(input, '\0', cmdLength);

	ssize_t nread;

	pid_t smallshPID = getpid();

	char* token = NULL;

	char command[MAXARGSIZE];

	char* inputArgs[MAXARGS];
	/*
	int j = 0;
	for(j = 0; j < MAXARGS; j++){
		inputArgs[j] = malloc(MAXARGSIZE * sizeof(char));
		memset(inputArgs[j], '\0', MAXARGSIZE);
	}
	*/
	int numArgs = 0;
	
	int commandFlag = 0;

	char inFile[MAXARGSIZE];
	memset(inFile, '\0', sizeof(inFile));
	char outFile[MAXARGSIZE];
	memset(outFile, '\0', sizeof(outFile));
	
	int backgroundFlag = 0;

	// vars for command execution
	int childExitMethod = -5;
	int childPID = -5;

	// testing: have limited # of times while can run for now...
	while(loopGuard < 75){
		
		printf(": ");
		fflush(stdout);	
	
		// user input
		nread = getline(&input, &cmdLength, stdin);	

		input[nread - 1] = '\0';

		if(input[0] == '#'){
			continue;
		}
		
		// parse out input into array
		token = strtok(input, " ");

		while(token != NULL){
			
			// check for double dollar sign 
			if(strstr(token, "$$") != NULL){
				// testing
				//printf("Double Ampersand!\n");
			}

			// load command
			if(commandFlag == 0){
				// testing
				//printf("COMMAND\n");

				strncpy(command, token, (size_t) MAXARGSIZE);
				commandFlag = 1;
			}
			// input file
			else if(token[0] == '<'){
				// testing
				//printf("INPUT\n");				

				token = strtok(NULL, " ");
				strncpy(inFile, token, (size_t) MAXARGSIZE);
			}
			// output file
			else if(token[0] == '>'){
				// testing
				//printf("OUTPUT\n");

				token = strtok(NULL, " ");
				strncpy(outFile, token, (size_t) MAXARGSIZE);
			}
			else if(token[0] == '&'){
				// testing
				//printf("AMP\n");

				backgroundFlag = 1;
			}
			// argument
			else{
				// testing
				//printf("ARG\n");	
	
				// check num of arguments
				numArgs++;
				if(numArgs > (MAXARGS - 1)){
					printf("Error! Max num. of arguments is %d", MAXARGS);
					break;
				}
				else{
					inputArgs[numArgs - 1] = malloc(MAXARGSIZE * sizeof(char));
					strncpy(inputArgs[numArgs - 1], token, (size_t) MAXARGSIZE);
				}
			}

			token = strtok(NULL, " ");
		}	

		// append NULL to end of arg list
		numArgs++;
		inputArgs[numArgs - 1] = NULL;		

		// use command (1st arg) to process input
		if(strcmp(command, "exit") == 0){
			// testing
			printf("exit\n");
			
			break;
		}
		else if(strcmp(command, "cd") == 0){
			cd(inputArgs[0]);
		}
		else if(strcmp(command, "status") == 0){
			// testing
			printf("status\n");

			status(childExitMethod);
		}
		else if(strcmp(command, "") != 0){
			// testing
			printf("Not built-in cmd\n");
			
			//execute(command, inputArgs, numArgs, inFile, outFile, backgroundFlag);
		}

		/*
		// testing
		printf("CMD: %s\n", command);
		printf("ARGS:\n");
		int l = 0;
		for(l; l < (numArgs - 1); l++){
			printf("%s\n", inputArgs[l]);
		}
		printf("IN: %s\n", inFile);
		printf("OUT: %s\n", outFile);
		printf("BACKGROUND: %d\n", backgroundFlag);

		// reset command and in/out file
		memset(command, '\0', sizeof(command));
		memset(inFile, '\0', sizeof(inFile));
		memset(outFile, '\0', sizeof(outFile));
		*/

		// reset args
		int i = 0;
		for(i; i < (numArgs - 1); i++){
			free(inputArgs[i]);
		}

		// reset command flag, background flag, and arg counter for next command
		commandFlag = 0;
		backgroundFlag = 0;
		numArgs = 0;

		loopGuard++;
	}

	free(input);

	return 0;
}

// expands the double dollar sign to the current PID
void expandPID(char* arg){
	// testing	
	printf("In expandPID");
	fflush(stdout);
}

// changes the working directory
void cd(char* pathName){

	int chdirStatus;
	char cwd[100]; 

	// No destination dir given = go home!
	if(pathName == NULL){
		pathName = getenv("HOME");
	}
	
	if(chdir(pathName) != 0){
		perror("chdir");
		fflush(stdout);
	}

	// testing
	//printf("%s\n", getcwd(cwd, 100));
	//fflush(stdout);
}

// prints the exit status or terminating signal for last foregrouond
// process
void status(int childExitMethod){
	
	int exitStatus;
	int termSig;

	// no foreground process run yet...
	if(childExitMethod == -5){
		printf("exit value 0\n");
		fflush(stdout);
	}
	else{
		// check to see if exited
		if(WIFEXITED(childExitMethod)){
			exitStatus = WEXITSTATUS(childExitMethod);
			printf("exit value %d", exitStatus);
		}
		else if(WIFSIGNALED(childExitMethod)){
			termSig = WTERMSIG(childExitMethod);
			printf("terminated by signal %d", termSig);
		}
	}
}
