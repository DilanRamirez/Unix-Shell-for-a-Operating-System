#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include<readline/readline.h> 
#include<readline/history.h> 

#define MAX_INPUT 1024


void startShell(){
	system("clear");
	printf("---------CS 4375 Lab 2: Unix Shell-----------\n");
	char* username = getenv("USER"); // Prints a prompt string specified by shell variable PS1
	printf("\n@%s:", username);	
	
	
}

int readInput(char* userInput){
	char* copy;

  	copy = readline ("$"); //Get a line from the user.

  	if(copy && *copy){//If the line has any text in it, save it on the history. 
    	add_history(copy);
    	strcpy(userInput, copy);
    	return 0;
    }else{
    	return 1;
    }
}

void getDirectory(){
	char cwd[MAX_INPUT];
	getcwd(cwd, sizeof(cwd));
	printf("%s", cwd);

}

void creatingFork(){
	int rc = fork();
    if (rc < 0) {// fork failed; exit
        fprintf(stderr, "fork failed\n");
        exit(1);

    } else if (rc == 0) { // child (new process)
        printf("hello, I am child (pid:%d)\n", (int) getpid());

    } else {// parent goes down this path (original process
        printf("hello, I am parent of %d (pid:%d)\n",rc, (int) getpid());
    }
}

int findPipe(char *str, char **strpiped){
	for(int i=0; i<2;i++){
		strpiped[i] = strsep(&str,"|");//Looking for pipes in user input
	}
	if(strpiped[1] == NULL){
		return 0; 
	}else{
		return 1;
	}
}

int processInput(char* str, char** parsed, char** pipefound){
	char* strpiped[2];
	int isPipe;

	isPipe = findPipe(str, strpiped);
	printf("isPipe: %d\n", isPipe);
}


int main(int argc, char *argv[]){
	int len, pipeFound;
	char userInput[MAX_INPUT], *parsed[MAX_INPUT], *parsedPipe[MAX_INPUT];

	
		startShell();
		getDirectory();
		//fgets(input,50,stdin);
		//standardUnixCommands(input);
		readInput(userInput);
		pipeFound = processInput(userInput, parsed, parsedPipe);
		

	return 0;
}