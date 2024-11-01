#include <stdio.h>		// std library for printf and getc
#include <string.h>		// library for string fucntions
#include <stdlib.h>		// library for memory allocation and string fucntions 
#include <unistd.h>		// unix library for fork exec etc 
#include <sys/types.h>		// types used in system calls
#include <sys/wait.h>		// wait function for process control

#define MAX_LEN 512		// Maximum length of a command line input
#define MAXARGS 10		// Maximum number of arguments a command can have
#define ARGLEN 30		// Maximum length of each argument
#define PROMPT "PUCITshell:- "	// Custom shell prompt


//The main function sets up a loop that continually reads user input,
// tokenizes it into command-line arguments, and executes the command until the user signals the end of input (like pressing Ctrl+D).
//It handles memory management by freeing any dynamically allocated memory for the command line and argument list to prevent memory leaks.

int execute(char* arglist[]);		//the execute function is responsible for running the command that the user entered
char** tokenize(char* cmdline);		//The tokenize function breaks down the full command line into individual components (arguments) that can be processed separately.
char* read_cmd(char*, FILE*);		//The read_cmd function shows a prompt to the user and reads their input, returning the complete command line as a string.
int main(){
   char *cmdline;			
   char** arglist;			 
   char* prompt = PROMPT;   		
   while((cmdline = read_cmd(prompt,stdin)) != NULL){			//This line begins a loop that continues to run as long as the read_cmd function returns a non-null pointer
      if((arglist = tokenize(cmdline)) != NULL){			//Inside the loop, this line calls the tokenize function, passing the cmdline string as an argument.
            execute(arglist);						//This line calls the execute function with arglist, which contains the command and its arguments. The execute function attempts to run the specified command.
       //  need to free arglist						//This comment suggests that the programmer should free the memory allocated for arglist.
         for(int j=0; j < MAXARGS+1; j++)
	         free(arglist[j]);
         free(arglist);
         free(cmdline);
      }
  }//end of while loop
   printf("\n");
   return 0;
}

//This function is responsible for creating a new process to execute a command specified by the user
int execute(char* arglist[]){
   int status;						//Declares an integer variable status to store the exit status of the child process after it finishes executing.
   int cpid = fork();
   switch(cpid){						//This line begins a switch statement to handle the different cases based on the return value of fork() (stored in cpid).

      case -1:
         perror("fork failed");			//fork failure
	      exit(1);
      case 0:
	      execvp(arglist[0], arglist);				//child process
 	      perror("Command not found...");
	      exit(1);
      default:
	      waitpid(cpid, &status, 0);
         printf("child exited with status %d \n", status >> 8);			//parent process 
         return 0;
   }
}

//tokenize function, which is responsible for parsing a command line input into individual arguments (tokens) and storing them in an array.
char** tokenize(char* cmdline){
//allocate memory
   char** arglist = (char**)malloc(sizeof(char*)* (MAXARGS+1));
   for(int j=0; j < MAXARGS+1; j++){
	   arglist[j] = (char*)malloc(sizeof(char)* ARGLEN);
      bzero(arglist[j],ARGLEN);
    }
   if(cmdline[0] == '\0')//if user has entered nothing and pressed enter key
      return NULL;
   int argnum = 0; //slots used
   char*cp = cmdline; // pos in string
   char*start;
   int len;
   while(*cp != '\0'){
      while(*cp == ' ' || *cp == '\t') //skip leading spaces
          cp++;
      start = cp; //start of the word
      len = 1;
      //find the end of the word
      while(*++cp != '\0' && !(*cp ==' ' || *cp == '\t'))
         len++;
      strncpy(arglist[argnum], start, len);
      arglist[argnum][len] = '\0';
      argnum++;
   }
   arglist[argnum] = NULL;
   return arglist;
}      
//display prompt 
char* read_cmd(char* prompt, FILE* fp){
   printf("%s", prompt);
  int c; //input character
   int pos = 0; //position of character in cmdline
   char* cmdline = (char*) malloc(sizeof(char)*MAX_LEN);
   while((c = getc(fp)) != EOF){
       if(c == '\n')
	  break;
       cmdline[pos++] = c;
   }
//these two lines are added, in case user press ctrl+d to exit the shell
   if(c == EOF && pos == 0) 
      return NULL;
   cmdline[pos] = '\0';
   return cmdline;
}
