#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64

// Function to parse the command and separate arguments, file redirection, and pipe handling
void parse_command(char *cmd, char **args, int *input_redirect, char **input_file, int *output_redirect, char **output_file, int *pipe_flag, char **cmd_after_pipe) {
    // Initialize flags for input/output redirection and piping
    *input_redirect = *output_redirect = *pipe_flag = 0;
    *input_file = *output_file = NULL;
    *cmd_after_pipe = NULL;

    int i = 0;  // Index for storing arguments
    char *token = strtok(cmd, " \n");  // Split the command by spaces and newlines

    // Loop through each part of the command
    while (token != NULL) {
        if (strcmp(token, "<") == 0) {  // Check if input redirection is needed
            *input_redirect = 1;
            token = strtok(NULL, " \n");  // Get the input file name
            *input_file = token;
        } else if (strcmp(token, ">") == 0) {  // Check if output redirection is needed
            *output_redirect = 1;
            token = strtok(NULL, " \n");  // Get the output file name
            *output_file = token;
        } else if (strcmp(token, "|") == 0) {  // Check if there's a pipe in the command
            *pipe_flag = 1;
            args[i] = NULL;  // Null-terminate the first command
            token = strtok(NULL, " \n");  // Move to the command after the pipe
            *cmd_after_pipe = token;
            break;  // Stop parsing for the first command
        } else {
            args[i++] = token;  // Store each argument in the args array
        }
        token = strtok(NULL, " \n");  // Move to the next part
    }
    args[i] = NULL;  // Null-terminate the args array
}

// Function to execute the command with support for input/output redirection and piping
void execute_command(char **args, int input_redirect, char *input_file, int output_redirect, char *output_file, int pipe_flag, char *cmd_after_pipe) {
    int pid1, pid2, status;

    if (pipe_flag) {
        // If there's a pipe, create a pipe file descriptor array
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe failed");
            exit(1);
        }

        // First command (before the "|")
        pid1 = fork();
        if (pid1 == 0) {
            // Redirect the standard output to the write end of the pipe
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[0]);  // Close unused read end
            close(pipe_fd[1]);  // Close write end after redirecting
            execvp(args[0], args);  // Execute the first command
            perror("execvp failed");  // Error if execvp fails
            exit(1);
        }

        // Second command (after the "|")
        pid2 = fork();
        if (pid2 == 0) {
            // Redirect the standard input to the read end of the pipe
            dup2(pipe_fd[0], STDIN_FILENO);
            close(pipe_fd[1]);  // Close unused write end
            close(pipe_fd[0]);  // Close read end after redirecting

            // Parse the command after pipe into arguments for the second command
            char *args_after_pipe[MAX_ARGS];
            args_after_pipe[0] = cmd_after_pipe;
            int j = 1;
            char *token = strtok(NULL, " \n");
            while (token != NULL) {
                args_after_pipe[j++] = token;
                token = strtok(NULL, " \n");
            }
            args_after_pipe[j] = NULL;  // Null-terminate the second command

            execvp(args_after_pipe[0], args_after_pipe);  // Execute the second command
            perror("execvp failed");  // Error if execvp fails
            exit(1);
        }

        // Close the pipe in the parent process
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        // Wait for both child processes to finish
        waitpid(pid1, &status, 0);
        waitpid(pid2, &status, 0);
    } else {
        // If there's no pipe, execute a single command
        pid1 = fork();
        if (pid1 == 0) {
            // Handle input redirection if specified
            if (input_redirect) {
                int in_fd = open(input_file, O_RDONLY);
                if (in_fd == -1) {
                    perror("Error opening input file");
                    exit(1);
                }
                dup2(in_fd, STDIN_FILENO);  // Redirect standard input
                close(in_fd);  // Close input file descriptor after redirecting
            }
            // Handle output redirection if specified
            if (output_redirect) {
                int out_fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if (out_fd == -1) {
                    perror("Error opening output file");
                    exit(1);
                }
                dup2(out_fd, STDOUT_FILENO);  // Redirect standard output
                close(out_fd);  // Close output file descriptor after redirecting
            }
            execvp(args[0], args);  // Execute the command
            perror("execvp failed");  // Error if execvp fails
            exit(1);
        }
        waitpid(pid1, &status, 0);  // Wait for the command to finish
    }
}

int main() {
    char cmd[MAX_CMD_LEN];
    char *args[MAX_ARGS];
    int input_redirect, output_redirect, pipe_flag;
    char *input_file, *output_file, *cmd_after_pipe;

    // Main loop to read and execute commands until exit
    while (1) {
        printf("PUCITshell:- ");  // Display prompt
        if (fgets(cmd, MAX_CMD_LEN, stdin) == NULL) {  // Read command from user
            printf("\n");
            break;  // Exit on CTRL+D
        }

        // Parse the command into arguments and other flags
        parse_command(cmd, args, &input_redirect, &input_file, &output_redirect, &output_file, &pipe_flag, &cmd_after_pipe);
        if (args[0] != NULL) {
            execute_command(args, input_redirect, input_file, output_redirect, output_file, pipe_flag, cmd_after_pipe);  // Run the parsed command
        }
    }

    return 0;
}
