#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <limits.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 100

void display_prompt() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        printf("%s PUCITshell:- ", cwd);
    } else {
        perror("getcwd() error");
    }
}

void parse_input(char *input, char **args) {
    for (int i = 0; i < MAX_ARGS; i++) {
        args[i] = strsep(&input, " \t\n");
        if (args[i] == NULL) break;
        if (strlen(args[i]) == 0) i--;  // Remove empty tokens
    }
}

int main() {
    char input[MAX_INPUT_SIZE];
    char *args[MAX_ARGS];
    pid_t pid;
    int status;

    while (1) {
        display_prompt();

        // Read input from user
        if (fgets(input, MAX_INPUT_SIZE, stdin) == NULL) {
            printf("\nExiting PUCITshell.\n");
            break;  // Handle Ctrl+D (EOF) to exit
        }

        // Parse the input into command and arguments
        parse_input(input, args);

        // If no command is entered, prompt again
        if (args[0] == NULL) {
            continue;
        }

        // Fork a new process
        pid = fork();

        if (pid < 0) {
            perror("Fork failed");
        } else if (pid == 0) {  // Child process
            if (execvp(args[0], args) < 0) {
                perror("Execution failed");
            }
            exit(EXIT_FAILURE);
        } else {  // Parent process
            waitpid(pid, &status, 0);  // Wait for the child to finish
        }
    }
    return 0;
}
