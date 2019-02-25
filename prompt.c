// dev script for prompt used in smallsh

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

	char inputArgs[MAXARGS][MAXARGSIZE];
	int numArgs = 0;		

	while(1){
		
		printf(": ");
		fflush(stdout);	
	
		// user input
		getline(&input, &cmdLength, stdin);	
		

		// parse out input into array

		// use command (1st arg) to process input


	}

	return 0;
}
