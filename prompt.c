// dev script for prompt used in smallsh

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int MAXARGS = 512;
int MAXARGSIZE = 100;

int main(){

	char* input = NULL;
	size_t cmdLength = 2049;
	input = (char*) malloc(cmdLength * sizeof(char));
	memset(input, '\0', cmdLength);

	ssize_t nread;

	char* token = NULL;

	char inputArgs[MAXARGS][MAXARGSIZE];
	int numArgs = 0;		

	while(1){
		
		printf(": ");
		fflush(stdout);	
	
		// user input
		nread = getline(&input, &cmdLength, stdin);	

		input[nread - 1] = '\0';

		// parse out input into array
		token = strtok(input, " ");

		while(token != NULL){
			printf("%s\n", token);
			token = strtok(NULL, " ");
		}	

		// use command (1st arg) to process input


	}

	return 0;
}
