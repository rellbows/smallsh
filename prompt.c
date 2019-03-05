// dev script for prompt used in smallsh

#define _GNU_SOURCE

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

int MAXARGS = 513; // 512 args + NULL
int MAXARGSIZE = 100;

void cd(char* pathName);
pid_t execute(char* inputArgs[MAXARGS], char* inFile, char* outFile, int backgroundFlag, int* fgChildExitMethod);
void killBgPIDs(pid_t bgPIDs[50], int bgPIDsStat[50], int numBgPIDs);
void status(int childExitMethod);

int main(){

	// testing
	int loopGuard = 0;

	char* input = NULL;
	size_t cmdLength = 2049;

	ssize_t nread;

	pid_t smallshPID = getpid();

	char* token = NULL;

	char command[MAXARGSIZE];

	char* inputArgs[MAXARGS];
	int numArgs = 0;
	
	int commandFlag = 0;

	char inFile[MAXARGSIZE];
	memset(inFile, '\0', sizeof(inFile));
	char outFile[MAXARGSIZE];
	memset(outFile, '\0', sizeof(outFile));
	
	int backgroundFlag = 0;

	// vars for command execution
	int fgChildExitMethod = -5;
	pid_t childPID = -5;
	pid_t fgChildPID = -5;
	pid_t bgPIDs[50];
	int bgPIDsStat[50];
	int numBgPIDs = 0;

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
				//printf("Double Doller Sign!\n");
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
		
			killBgPIDs(bgPIDs, bgPIDsStat, numBgPIDs);	
			break;
		}
		else if(strcmp(inputArgs[0], "cd") == 0){
			
			cd(inputArgs[1]);
		}
		else if(strcmp(inputArgs[0], "status") == 0){
			
			status(fgChildExitMethod);
		}
		else if(strcmp(inputArgs[0], "") != 0){
		
			childPID = execute(inputArgs, inFile, outFile, backgroundFlag, &fgChildExitMethod);
			if(backgroundFlag == 0){
				fgChildPID = childPID;
			}
			else{
				bgPIDs[numBgPIDs] = childPID;
				bgPIDsStat[numBgPIDs] = 0;
				numBgPIDs++;
			}
		}

		// reset command and in/out file
		memset(command, '\0', sizeof(command));
		memset(inFile, '\0', sizeof(inFile));
		memset(outFile, '\0', sizeof(outFile));

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
}

// handles the execution of commands
// CITATION: used below link as ref. for setting up fork/execvp/wait
// https://web.mst.edu/~ercal/284/UNIX-fork-exec/Fork-Exec-2.cpp
pid_t execute(char* inputArgs[MAXARGS], char* inFile, char* outFile, int backgroundFlag, int* fgChildExitMethod){

	int inFD, outFD, result;
	pid_t childPID;

	childPID = fork();

	// child thread 
	if(childPID == 0){
	
		// input file specified
		if(strlen(inFile) != 0){
		
			inFD = open(inFile, O_RDONLY);
			
			// check open
			if(inFD == -1){
				perror("input file open()");
				fflush(stdout);
				exit(1);
			}

			// set stdin to input file
			result = dup2(inFD, 0);
			
			// check redirection
			if(result == -1){
				perror("input dup2()");
				fflush(stdout);
				exit(1);
			}

			// close on exec call
			fcntl(inFD, F_SETFD, FD_CLOEXEC);
		}
		else if(backgroundFlag == 1){
			
			inFD = open("/dev/null", O_RDONLY);

			// check open
			if(inFD == -1){
				perror("background input file open()");
				fflush(stdout);
				exit(1);
			}

			result = dup2(inFD, 0);

			// check redirection
			if(result == -1){
				perror("background input dup2()");
				fflush(stdout);
				exit(1);
			}

			// close on exec call
			fcntl(inFD, F_SETFD, FD_CLOEXEC);
		}

		// output file specified
		if(strlen(outFile) != 0){

			outFD = open(outFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
			
			// check open
			if(outFD == -1){
				perror("output file open()");
				fflush(stdout);
				exit(1);
			}

			// set stdout to output file
			result = dup2(outFD, 1);

			// check redirection
			if(result == -1){
				perror("output dup2()");
				fflush(stdout);
				exit(1);
			}

			// close on exec call
			fcntl(outFD, F_SETFD, FD_CLOEXEC);
		}
		else if(backgroundFlag == 1){
			
			outFD = open("/dev/null", O_WRONLY | O_CREAT | O_TRUNC, 0644);

			// check open
			if(outFD == -1){
				perror("background output file open()");
				fflush(stdout);
				exit(1);
			}

			result = dup2(outFD, 1);

			// check redirection
			if(result = -1){
				perror("background output dup2()");
				fflush(stdout);
				exit(1);
			}

			// close on exec call
			fcntl(outFD, F_SETFD, FD_CLOEXEC);
		}

		execvp(inputArgs[0], inputArgs);

		// if command successful, below will not run
		printf("%s is not a valid command\n", inputArgs[0]);
		exit(1);
	}
	// parent thread
	else if(childPID > 0){
		
		// foreground process
		if(backgroundFlag == 0){

			waitpid(childPID, fgChildExitMethod, 0);
		}
		else if(backgroundFlag == 1){

			printf("background pid is %d\n", childPID);
		}
	}

	return childPID;
}

// kills all background processes
void killBgPIDs(pid_t bgPIDs[50], int bgPIDsStat[50], int numBgPIDs){

	// loop thru all bg PID's - kill and wait each
	int i = 0;
	for(i; i < numBgPIDs; i++){
		if(bgPIDsStat[i] == 0){
			kill(bgPIDs[i], SIGKILL);
			wait(NULL);
			
			// testing
			printf("Killing bg: %d\n", bgPIDs[i]);
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
