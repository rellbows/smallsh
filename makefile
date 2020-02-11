smallsh: smallsh.c
	gcc -o smallsh -std=c99 smallsh.c

clean:
	rm -f smallsh 
