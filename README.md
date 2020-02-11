# smallsh

## Description:

C based script to create a simple shell that can be used to interact with directories and files.

## Compile Instructions:

The included 'makefile' can be used to compile the 'smallsh.c' source code by entering 'make smallsh' into the command line. Alternatively, the source code can be compiled using 'gcc -o smallsh -std=c99 smallsh.c'.

## Executiong Instructions:

Once compiled, the program can be run by typing the following command into the command line './smallsh'.

## Usage Instructions:

The shell can handle standard commands that a typical bash shell could run (ex. 'cat testfile').

The shell can also handle redirection of stdin and stdout using the '<' and '>' symbols (ex. 'cat > testfile').

The shell can run processes in the background using the '&' symbol (ex. 'sleep 5 &').

There are 3 built-in commands the shell recognizes - 'exit', 'cd', and 'status'.

'exit' - Exits the shell. It takes no arguments. When this command is run, the shell kills any other processes or jobs it has started before terminating.

'cd' - Changes the working directory of the shell. By itself - with no arguments, it will change to the directory specified in the HOME environment variable. The command can also take one argument - the path of a directory to change to.

'status' - Prints out either the exit status or the terminating signal of the last foreground process ran by the shell.

Basic rundown of syntax for the command line below...

`command [arg1 arg2] [< input_file] [> output_file] [&]`

Blank lines and lines beginning with a '#' are ignored by the shell.

## Bugs/TODO's:

1. A static array is used to keep track of background processes. This causes the potential for the array to be overrun if more than 50 background processes are run. Need to implement some sort of advanced data type to contain this list of background processes.

## Testing Details:

Scripts were tested on a MacBook Pro running macOS Sierra ver. 10.12.6.
