// dev script for prompt used in smallsh

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

int MAXARGS = 513; // 512 args + NULL
int MAXARGSIZE = 100;

void cd(char* pathName);
void execute(char* inputArgs[MAXARGS], char* inFile, char* outFile, int backgroundFlag, int childPID, int* childExitMethod);
void status(int childExitMethod);

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
	int backgroundPIDs[50];
	int numBackgroundPIDs = 0;

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

			/* TODO: don't think this is needed
			// load command
			if(commandFlag == 0){
				// testing
				//printf("COMMAND\n");

				strncpy(command, token, (size_t) MAXARGSIZE);
				commandFlag = 1;
			}
			*/
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
		if(strcmp(inputArgs[0], "exit") == 0){
			// testing
			printf("exit\n");
			
			break;
		}
		else if(strcmp(inputArgs[0], "cd") == 0){
			cd(inputArgs[1]);
		}
		else if(strcmp(inputArgs[0], "status") == 0){
			status(childExitMethod);
		}
		else if(strcmp(inputArgs[0], "") != 0){
			// testing
			printf("Not built-in cmd\n");
		
			childPID = fork();
		
			execute(inputArgs, inFile, outFile, backgroundFlag, childPID, &childExitMethod);
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
		exit(1);
	}

	// testing
	//printf("%s\n", getcwd(cwd, 100));
	//fflush(stdout);
}

// handles the execution of commands
// CITATION: used below link as ref. for setting up fork/execvp/wait
// https://web.mst.edu/~ercal/284/UNIX-fork-exec/Fork-Exec-2.cpp
void execute(char* inputArgs[MAXARGS], char* inFile, char* outFile, int backgroundFlag, int childPID, int* childExitMethod){
	
	// check whether 
	if(childPID == 0){
		// testing
		printf("Child executing... %s\n", inputArgs[0]);
		fflush(stdout);

		execvp(inputArgs[0], inputArgs);

		// if command successful, below will not run
		printf("%s is not a valid command\n", inputArgs[0]);
		exit(1);
	}
	else if(childPID > 0){
		if(backgroundFlag == 0){
			// testing
			printf("Parent waiting...\n");
			fflush(stdout);

			waitpid(childPID, childExitMethod, 0);
		}
		else{
			// testing
			printf("background process...\n");
			fflush(stdout);
		}
	}
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
			printf("exit value %d\n", exitStatus);
			fflush(stdout);
		}
		else if(WIFSIGNALED(childExitMethod)){
			termSig = WTERMSIG(childExitMethod);
			printf("terminated by signal %d\n", termSig);
			fflush(stdout);
		}
	}
}
