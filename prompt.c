// dev script for prompt used in smallsh

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void cd(char* pathName);

int MAXARGS = 512;
int MAXARGSIZE = 100;

int main(){

	char* input = NULL;
	size_t cmdLength = 2049;
	input = (char*) malloc(cmdLength * sizeof(char));
	memset(input, '\0', cmdLength);

	ssize_t nread;

	pid_t smallshPID = getpid();

	char* token = NULL;

	char inputArgs[MAXARGS][MAXARGSIZE];
	int numArgs = 0;

	int i = 0;		

	while(1){
		
		printf(": ");
		fflush(stdout);	
	
		// user input
		nread = getline(&input, &cmdLength, stdin);	

		input[nread - 1] = '\0';

		if(input[0] == '#'){
			continue;
		}
		
		// TODO: this is not working right. need to incorporate
		// into the tokenize operation and use strstr()
		if(input[0] == '$' && input[1] == '$'){
			printf("%d", smallshPID);
		}

		// parse out input into array
		token = strtok(input, " ");

		while(token != NULL){
		
			// check num of arguments
			numArgs++;
			if(numArgs > MAXARGS){
				printf("Error! Max num. of arguments is %d", MAXARGS);
				break;
			}
			else{
				strncpy(inputArgs[numArgs - 1], token, (size_t) MAXARGSIZE);
			}

			token = strtok(NULL, " ");
		}	

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

		// reset arg counter for next command
		numArgs = 0;

		// reset args
		for(int i = 0; i < numArgs; i++){
			memset(inputArgs[i], '\0', sizeof(*inputArgs));
		}
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
