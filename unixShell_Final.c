#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/wait.h>



#define MAX_INPUT 1024

/*The function tokenizeLine() takes as  parameters
 an user input and an array of pointers to get every 
 token from the string*/
void tokenizeLine(char* line, int length, char** args, _Bool* run_background){
    int i = 0;

    //Detect enter from the user
    line[length] = '\0'; //set the end of string by assigning \0
    if(line[strlen(line) - 1] == '\n') { //find where the string finishes by the user enter in case there is nt 
        line[strlen(line) - 1] = '\0'; 
    }

    char* token = strtok(line, " "); //strtok() tokenize the input string using space as delimeter 

    while(token != NULL){
        args[i] = token; 
        i++;
        token = strtok(NULL, " "); //set every token with a null value when it finds a space 
    }                               
    args[i] = NULL;  // Example, "helloNULLworldNULL"

    if(i > 0 && strcmp(args[i - 1], "&") == 0){ //find Background task in user input
        *run_background = true; // change state to swap parent process and print background tasks
        args[i - 1] = 0;
    }
}

/*The function readCommand() takes as  parameters
 an user input and an array of pointers to get every 
 token from the string. It uses the system call "read()"
 to call the command typed by the user and determine
 if the command is a valid command or not*/
int readCommand(char* line, char** args, _Bool* run_background){
    // Read input
    int length = read(0, line, 1024);
    if((length < 0) && (errno !=EINTR)){ //report the EINTR error code if a signal occurred while the system call was in progress
    
        perror("Command not found");
        exit(-1);
    }

    return length;
}



/*The function executeCommands() takes as  parameters
 an array of pointers. This fuction provide the 
 built-in commands exit & cd also redirectioning inputs and outputs*/
void executeCommands(char** args, _Bool run_background){
    int fd0,fd1,i, in=0, out=0, pip=0, background=0;
    char input[64],output[64];

    // internal built-in commands
    if(strcmp(args[0], "exit") == 0) { //takes the args[0] which is the token "exit" to finish the program
        exit(0);
    }
    else if(strcmp(args[0], "cd") == 0) {//takes the args[0] which is the token "cd" to access a directory
        if(chdir(args[1]) != 0) { //chdir() is in charge of accessing to the directory assigned by args[1]
            write(1, "Invalid Directory\n", strlen("Invalid Directory\n")); //a success value is 0
        }
        return;
    }

    pid_t pid = fork(); //Starts fork to create two processes,parent & child
    if(pid < 0) {
        perror("Shell");
    }

    /*all the internal proccess are created in the child process while the parent waits

        "Hello from Child! */
    else if(pid == 0) {
        /*Redirection of input and output (e.g. $ ls > /tmp/files.txt).*/
         for(i=0;args[i]!='\0';i++){ // go through the while array args[] to find the char > which redirections the output
            if(strcmp(args[i],">")==0){  
                args[i]=NULL;
                strcpy(output,args[i+1]); //creates a copy of the destionation 
                out=2;  //asigned a value to the variable out to let the child process that a redirection has being asked.
            }else if(strcmp(args[i],"<")==0){      
                args[i]=NULL;
                strcpy(output,args[i+1]);
                in=2;
            }    
            else if(strcmp(args[i],"|")==0){ // find the char | character which redirections the output as an input to the next command.   
                args[i]=NULL;
                strcpy(input,args[i-1]);
                strcpy(output,args[i+1]);
                pip=2;
            }

        }

        if(out){
            if ((fd1 = creat(output , 0644)) < 0) { //creates a file which contains the output os the first command.
                perror("Couldn't open the output file");
                exit(0);
            }           
            
            dup2(fd1, STDOUT_FILENO); // it duplicates the value of fd1 and assigned to the first position in the table.
            close(fd1);

        }else if(in){
            if((fd0 = open(output, O_RDONLY, 0644)) < 0){ //prints a file which contains the output os the second command.
                perror("Couldn't read from the input file");
                exit(0);
            }

            dup2(fd0, 0);
            close(fd0);

        /*Command1 | Command2 
           Command1 > tempfile
           Command2 < tempfile*/
        }else if(pip){      
            if((fd1 = creat(input, 0644)) > 0) {    //Command1 | Command2 -> creates a file to store the content of the first command as input to the next commnad
                if((fd0 = open(input, O_RDONLY, 0644)) < 0){      //opens the file to get the content as an input
                    perror("Couldn't read from the input file"); //
                    exit(0);
                }

            }else{
                 perror("Couldn't open the output file");
                exit(0);
            }
        }

        

        if(execvp(args[0], args) == -1) { // show an error in case it returns a -1
            write(1, args[0], strlen(args[0]));
            write(1, "\nUnknown command.\n", strlen("\nUnknown command.\n"));
            exit(-1);
        }else{
            execvp(args[0], args); // runs the first command otherwise
            _exit(1);
        }

    }
    /*"Hello from Parent!
    parent waits meanwhile*/
    else if(!run_background){ //print bakcground tasks in the parent process
        wait(NULL);
    }
}



/*Simple funciton to print the shel environment*/
void printPrompt(){
    char cwd[MAX_INPUT];

    char* username = getenv("USER");
    if(getcwd(cwd, sizeof(cwd)) != NULL) {
        write(1, strcat(username, cwd), strlen(cwd));

    }else{
        perror("An error has ocurred");
    }
}


/*Function to set a new PS1 value as shell name*/
void printPS1(){
     char *PS1_user = getenv("PS1");

    char cwd[200];
    if(PS1_user == NULL){
         write(STDOUT_FILENO, "$", strlen("$ "));
            
    }else{
        write(STDOUT_FILENO, PS1_user, strlen(PS1_user));
            
    }
}


/* A linux shell is just a loop that 
reads commands from the user to the parse
the input and execute the correspont commands. 
I used the funciton loop to show this
 process used by the OS*/
void loop(){
    char input_line[1024];
    char* args[1024];
    int len;

    //everything runs on a loop until user types exit
    while(1){
        
        //read the commands entered by the user
        printPS1();
        _Bool run_background = false;

        //readCommand(input_line, args, &run_background);
        len = readCommand(input_line, args, &run_background);

        //in case a enter is clicked 
        if(strlen(input_line) == 0){
            continue;
        }

        //Parce the commands entered by the user
        tokenizeLine(input_line, len, args, &run_background);

        //execute user commands
        executeCommands(args, run_background);
    }

}

int main(int argc, char* args[]){
    system("clear");
    printf("------------------------------CS 4375 Lab 2: Unix Shell-------------------------\n");
    printf("\t\t\tAuthors: Jorge Quiñonez & Dilan Ramirez\n\n");

    printPrompt();
    sleep(2);
    loop();

    return 0;
}
