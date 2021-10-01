#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include "shell379.h"

int main(int argc, char *argv[])
{
    // TODO for now assume the args are valid

    /*char cmd[MAX_LENGTH];*/
    char *args[MAX_ARGS+1];
    int total_args;

    int num_active_processes = 0;

    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);

    /*int user_time = 0;*/
    /*int sys_time = 0;*/

    pid_t pid;

    while (true) {

        dup2(stdin_copy, STDIN_FILENO);     // restore stdin
        dup2(stdout_copy, STDOUT_FILENO);   // restore stdout

        total_args = prompt_cmd(args);

        if (is_shell_cmd(args[0])) {
            if (strcmp(args[0], "exit") == 0) {
                break;
            }


        } else {
            char *rin_fname = get_input_redirection_fname(args);
            char *rout_fname = get_output_redirection_fname(args);

            char *filtered_args[MAX_ARGS+1];
            remove_input_output_args(filtered_args, args, total_args);

            int pipe_fd[2];
            if (pipe(pipe_fd) < 0)      // create pipe before forking a child
                perror("pipe error");

            if ((pid = fork()) < 0) {
                perror("fork error");

            } else if (pid == 0) {

                if (rin_fname != NULL)
                    redirect_input(rin_fname);

                if (rout_fname != NULL)
                    redirect_output(rout_fname);

                close(pipe_fd[0]);
                close(pipe_fd[1]);

                execvp(filtered_args[0], filtered_args);
            }
            else {
                wait(NULL);
                if (close(pipe_fd[0]) < 0)
                    perror("Pipe close error");
                if (close(pipe_fd[1] < 0))          
                    perror("Pipe close error");
            }

        }

        free_args(args, total_args);
    }



    return EXIT_SUCCESS;

    

}

/*
 * Function: prompt_cmd 
 * -----------------------------------
 *      Prompts the user for the command and returns the total number of arguments.
 *
 *  Inputs:
 *      cmd: main command to be stored
 *      args: the following arguments for the command to be stored
 *
 *  Returns:
 *      total_args: the number of arguments 
 *
 */
int prompt_cmd(char *args[MAX_ARGS+1])
{
    int ch;
    int arg_i = 0;

    printf("SHELL379: "); 
    fflush(stdout);

    char *arg = calloc(MAX_LENGTH + 1, sizeof(*arg));
    do {
        scanf("%s", arg);
        args[arg_i++] = arg;
        arg = calloc(MAX_LENGTH + 1, sizeof(*arg));

        ch = getchar();

    } while (ch != '\n');

    args[arg_i] = NULL;

    /*rewind(stdin);*/

    fseek(stdin, 0L, SEEK_SET);
    clearerr(stdin);



    return arg_i;
}


void free_args(char *args[MAX_ARGS+1], int total_args) 
{
    for (int i=0; i<total_args; i++) {
        free(args[i]);
    }
}
    

bool is_shell_cmd(char *cmd)
{
    char *shell_cmds[] = {"exit", "jobs", "kill", "resume", "sleep", "suspend", "wait"};
    int len = sizeof(shell_cmds) / sizeof(shell_cmds[0]);

    for (int i=0; i<len; i++) {
        if (strcmp(cmd, shell_cmds[i]) == 0)
            return true;
    }
    return false;

}

char *get_input_redirection_fname(char *args[MAX_ARGS+1])
{
    for (int i=1; (i<MAX_ARGS+1) && (args[i] != NULL); i++) {
        if (args[i][0] == '<') {
            return args[i] + 1;
        }
    }

    return NULL;
}

char *get_output_redirection_fname(char *args[MAX_ARGS+1])
{
    for (int i=1; (i<MAX_ARGS+1) && (args[i] != NULL); i++) {
        if (args[i][0] == '>') {
            return args[i] + 1;
        }
    }

    return NULL;
}


void remove_input_output_args(char *filtered_args[MAX_ARGS+1], char *args[MAX_ARGS+1], int total_args)
{
    int i, j;
    for (i=0, j=0; i<total_args; i++) {
        if ( !(args[i][0] == '<' || args[i][0] == '>') ) {
            filtered_args[j++] = args[i];
        }
    }
    filtered_args[j] = NULL;
}

void redirect_input(char *rin_fname)
{
    int input_fds;
    input_fds = open(rin_fname, O_RDONLY);
    if (dup2(input_fds, STDIN_FILENO) < 0) {
        perror("input dup2 error");
        _exit(EXIT_FAILURE);
    }
    close(input_fds);
}

void redirect_output(char *rout_fname)
{
    int output_fds;
    int mode = S_IRUSR | S_IWUSR;
    output_fds = creat(rout_fname, mode);
    if (dup2(output_fds, STDOUT_FILENO) < 0) {
        perror("output dup2 error");
        _exit(EXIT_FAILURE);
    }
    close(output_fds);
}
