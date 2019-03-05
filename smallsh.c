// Prog. Name: smallsh
// Author: Ryan Ellis
// Class: CS344 - Operating Systems
// Description: Simple C script that creates a shell that can be used to 
// make system calls to the OS.

#define _GNU_SOURCE

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>

int MAXARGS = 513; // 512 args + NULL
int MAXARGSIZE = 100;

int bgMuteFlag = 0; // used with SIGSTP signal cntl if bg processes can be run

void bgMute(int signo);
void cd(char* pathName);
pid_t execute(char* inputArgs[MAXARGS], char* inFile, char* outFile, int backgroundFlag, int* fgChildExitMethod);
void expandPID(char* fullString, char* pidLoc, char* newString, pid_t currentPID);
void killBgPIDs(pid_t bgPIDs[50], int bgPIDsStat[50], int numBgPIDs);
void status(int childExitMethod);
void statusBgPID(pid_t bgPID, int bgChildExitMethod, pid_t bgPIDs[50], int bgPIDsStat[50], int numBgPIDs);

int main(){

	char* input = NULL;
	size_t cmdLength = 2049;

	ssize_t nread;

	pid_t smallshPID = getpid();
	char* pidSymResult;

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
	pid_t bgChildPID = -5;
	int bgChildExitMethod = -5;
	pid_t bgPIDs[50];
	int bgPIDsStat[50];
	int numBgPIDs = 0;
	

	// structs for signals
	struct sigaction ignore_SIGINT = {0}, SIGTSTP_bgMute = {0};
	
	// setup how to deal with SIGSTP
	SIGTSTP_bgMute.sa_handler = bgMute;
	sigfillset(&SIGTSTP_bgMute.sa_mask);
	SIGTSTP_bgMute.sa_flags = SA_RESTART;
	
	// setup how parent will deal with SIGINT
	ignore_SIGINT.sa_handler = SIG_IGN;

	// setup sig handler for SIGSTP
	sigaction(SIGTSTP, &SIGTSTP_bgMute, NULL);
	// tell parent to ignore SIGINT
	sigaction(SIGINT, &ignore_SIGINT, NULL);


	while(1){
	
		while(1){
			printf(": ");
			fflush(stdout);	
	
			// user input
			nread = getline(&input, &cmdLength, stdin);
			
			if(nread == -1){
				clearerr(stdin);
			}
			else{
				break;
			}
		}
		
		input[nread - 1] = '\0';

		// check for terminated bg processes
		bgChildPID = waitpid(-1, &bgChildExitMethod, WNOHANG);
		if(bgChildPID > 0){
			statusBgPID(bgChildPID, bgChildExitMethod, bgPIDs, bgPIDsStat, numBgPIDs);
		}

		if(input[0] == '#' || input[0] == '\0'){
			continue;
		}
		
		// parse out input into array
		token = strtok(input, " ");

		while(token != NULL){
		
		
					// input file
			if(token[0] == '<'){

				token = strtok(NULL, " ");
				strncpy(inFile, token, (size_t) MAXARGSIZE);
			}
			// output file
			else if(token[0] == '>'){

				token = strtok(NULL, " ");
				strncpy(outFile, token, (size_t) MAXARGSIZE);
			}
			else if(token[0] == '&'){

				// check to see if bg mute flag is set
				if(bgMuteFlag == 0){
					backgroundFlag = 1;
				}
				else{
					backgroundFlag = 0;
				}	
				
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

					// expand $$ to PID if found
					pidSymResult = strstr(token, "$$");

					if(pidSymResult != NULL){
						expandPID(token, pidSymResult, inputArgs[numArgs - 1], smallshPID);
					}
					else{

						strncpy(inputArgs[numArgs - 1], token, (size_t) MAXARGSIZE);
					}
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
			free(inputArgs[i]); }

		// reset command flag, background flag, and arg counter for next command
		commandFlag = 0;
		backgroundFlag = 0;
		numArgs = 0;
	}

	free(input);

	return 0;
}

// expands the double dollar sign to the current PID
void expandPID(char* fullString, char* pidLoc, char* newString, pid_t currentPID){

	int length = strlen(fullString);

	if(length >= (MAXARGSIZE - 5)){
		printf("Error, not enough room to safetly expand PID");
		exit(1);
	}
	else{
		strncpy(newString, fullString, pidLoc - fullString);
		newString[pidLoc - fullString] = '\0';
		sprintf(newString + (pidLoc - fullString), "%d%s", currentPID, fullString + ( pidLoc - fullString + 2), MAXARGSIZE);
	}
}

// sig handler for muting whether bg processes can be implemented - used
// with SIGSTP
void bgMute(int signo){

	char* muteOn = "Entering foreground-only mode (& is now ignored)\n";
	char* muteOff = "Exiting foreground-only mode\n";
	if(bgMuteFlag == 0){
		write(STDOUT_FILENO, muteOn, 49);
		bgMuteFlag = 1;
	}
	else{
		write(STDOUT_FILENO, muteOff, 29);
		bgMuteFlag = 0;
	}
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

	int inFD, outFD, result, termSig;
	pid_t childPID;

	// struct for fg child signal
	struct sigaction SIGINT_child = {0}, ignore_SIGTSTP = {0};
	
	// setup how child will deal with SIGINT
	SIGINT_child.sa_handler = SIG_DFL;
	sigfillset(&SIGINT_child.sa_mask);
	SIGINT_child.sa_flags = 0;

	// setup how child will deal with SIGTSP 
	ignore_SIGTSTP.sa_handler = SIGTSTP;

	childPID = fork();

	// child thread 
	if(childPID == 0){
		
		if(backgroundFlag == 0){
			// tell child not to ignore SIGINT
			sigaction(SIGINT, &SIGINT_child, NULL);
		}
		
		// tell all children to ignore SIGTSTP
		sigaction(SIGTSTP, &ignore_SIGTSTP, NULL);


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
			if(result == -1){
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

			// check to see if fg child killed by signal
			if(WIFSIGNALED(*fgChildExitMethod)){
				termSig = WTERMSIG(*fgChildExitMethod);
				printf("terminated by signal %d\n", termSig);
				fflush(stdout);
			}
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

// kills a specified background process
void statusBgPID(pid_t bgPID, int bgChildExitMethod, pid_t bgPIDs[50], int bgPIDsStat[50], int numBgPIDs){
	
	int exitStatus;
	int termSig;

	printf("background PID %d is done: ", bgPID);

	// get status of terminated background process
	// check to see if exited
	if(WIFEXITED(bgChildExitMethod)){
		exitStatus = WEXITSTATUS(bgChildExitMethod);
		printf("exit value %d\n", exitStatus);
		fflush(stdout);
	}
	else if(WIFSIGNALED(bgChildExitMethod)){
		termSig = WTERMSIG(bgChildExitMethod);
		printf("terminated by signal %d\n", termSig);
		fflush(stdout);
	}
	
	// update bg list
	int i = 0;
	for(i; i < numBgPIDs; i++){
		if(bgPIDsStat[i] == 0){
			if(bgPID == bgPIDs[i]){
				bgPIDsStat[i] = 1;
			}
		}
	}
}

