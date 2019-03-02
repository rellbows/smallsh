// dev script for prompt used in smallsh

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cd(char* pathName);

int MAXARGS = 513; // 512 args + NULL
int MAXARGSIZE = 100;

int main(){

	// testing
	int loopGuard = 0;

	char* input = NULL;
	size_t cmdLength = 2049;
	input = (char*) malloc(cmdLength * sizeof(char));
	memset(input, '\0', cmdLength);

	ssize_t nread;

	pid_t smallshPID = getpid();

	char* token = NULL;

	char command[MAXARGSIZE];

	char inputArgs[MAXARGS][MAXARGSIZE];
	int numArgs = 0;
	
	int commandFlag = 0;

	char inFile[MAXARGSIZE];
	char outFile[MAXARGSIZE];
	
	int backgroundFlag = 0;

	int i = 0;		

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
	
			// load command
			if(commandFlag == 0){
				strncpy(command, token, (size_t) MAXARGSIZE);
				commandFlag = 1;
			}
			// input file
			else if(token[0] == '<'){
				token = strtok(NULL, " ");
				strncpy(inFile, token, (size_t) MAXARGSIZE);
			}
			// output file
			else if(token[0] == '>'){
				token = strtok(NULL, " ");
				strncpy(outFile, token, (size_t) MAXARGSIZE);
			}
			else if(token[0] == '&'){
				backgroundFlag = 1;
			}
			// argument
			else{
		
				// check num of arguments
				numArgs++;
				if(numArgs > (MAXARGS - 1)){
					printf("Error! Max num. of arguments is %d", MAXARGS);
					break;
				}
				else{
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
		}
		else if(strcmp(inputArgs[0], "cd") == 0){
			cd(inputArgs[1]);
		}
		else if(strcmp(inputArgs[0], "status") == 0){
			// testing
			printf("status\n");
		}
		else if(strcmp(inputArgs[0], "") != 0){
			printf("Not built-in cmd\n");
		}

		/*
		// testing
		printf("CMD: %s\n", command);
		printf("ARGS:\n");
		for(int j = 0; j < numArgs; j++){
			printf("%s\n", inputArgs[j]);
		}
		printf("IN: %s\n", inFile);
		printf("OUT: %s\n", outFile);
		printf("BACKGROUND: %d\n", backgroundFlag);
		*/

		// reset command and in/out file
		memset(command, '\0', sizeof(command));
		memset(inFile, '\0', sizeof(inFile));
		memset(outFile, '\0', sizeof(outFile));

		// reset command flag, background flag, and arg counter for next command
		commandFlag = 0;
		backgroundFlag = 0;
		numArgs = 0;

		// reset args
		for(int i = 0; i < numArgs; i++){
			memset(inputArgs[i], '\0', sizeof(*inputArgs));
		}

		loopGuard++;
	}

	return 0;
}

void cd(char* pathName){

	int chdirStatus;
	char cwd[100]; 

	// No destination dir given = go home!
	if(pathName == NULL){
		pathName = (getenv("HOME"));
	}
	
	if(chdir(pathName) != 0){
		perror("chdir");
	}

	// testing
	printf("%s\n", getcwd(cwd, 100));
}
