#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>


//global variables

#define MAX_INPUT 1024


//split the line into args
int tokenizeLine(char* line, char** args){
    int i = 0;
    char* token = strtok(line, " ");

    while(token != NULL){
        args[i] = token;
        i++;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    return i;
}

//parse the input command
void parceCommand(char* line, int length, char** args){
    int token_count, i;
    char* copy[strlen(line)];
    char* p;
  
    //Detect enter from the user
    line[length] = '\0';
    if(line[strlen(line) - 1] == '\n') {
        line[strlen(line) - 1] = '\0';
    }
    
    token_count = tokenizeLine(line, args);

}

/**
 * Read a command from the keyboard into the line 'line' and tokenize it
 * such that 'args[i]' points into 'line' to the i'th token in the command.
 * line: line allocated by the calling code. Must be at least
 * COMMAND_LENGTH bytes long.
 * args[]: Array of character pointers which point into 'line'. Must be at
 * least NUM_args long. Will strip out up to one final '&' token.
 * 'args' will be NULL terminated.
 * in_background: pointer to a boolean variable. Set to true if user entered
 * an & as their last token; otherwise set to false.
 */
int readCommand(char* line, char** args){
    // Read input
    int length = read(0, line, 1024-1);
    if( (length < 0) && (errno !=EINTR) )
    {
        perror("Command not found");
        exit(-1);
    }

    return length;
}

//function to execute user commands
void executeCommands(char** args){
    // internal commands
    if(strcmp(args[0], "exit") == 0) {
        exit(0);
    }
    else if(strcmp(args[0], "cd") == 0) {
        if(chdir(args[1]) != 0) {
            write(1, "Invalid Directory", strlen("Invalid Directory"));
        }
        return;
    }

    pid_t pid = fork();
    if(pid < 0) {
        perror("Shell");
    }
    else if(pid == 0) {
        if(execvp(args[0], args) == -1) {
            write(1, args[0], strlen(args[0]));
            write(1, "\nUnknown command.\n", strlen("\nUnknown command.\n"));
            exit(-1);
        }
    }
    else {
        wait(NULL);
    }
}


void printPrompt(){
    char cwd[MAX_INPUT];

    char* username = getenv("USER");
    if(getcwd(cwd, sizeof(cwd)) != NULL) {
        write(1, strcat(username, cwd), strlen(cwd));

    }else{
        perror("An error has ocurred");
    }


    write(1, "$ ", strlen( "$ ")); //read the commands entered by the user
}

void printPS1(){
     char *PS1_user = getenv("PS1");

        char cwd[200];
        if(PS1_user == NULL){
            write(STDOUT_FILENO, "$", strlen("$ "));
            
        }else{
            write(STDOUT_FILENO, PS1_user, strlen(PS1_user));
            
        }
}

void loop(){
    char input_line[1024];
    char* args[1024];
    int len;


    while(true){
        // Get command
        // Use write because we need to use read()/write() to work with
        // signals, and they are incompatible with printf().
        
        //read the commands entered by the user
        //printPrompt();
        printPS1();

        len = readCommand(input_line, args);

        //Parce the commands entered by the user
        parceCommand(input_line, len, args);

        //execute user commands
        executeCommands(args);
    }

}

int main(int argc, char* argv[]){
    system("clear");
    printf("---------CS 4375 Lab 2: Unix Shell-----------\n\n\n");
    loop();
    return 0;
}