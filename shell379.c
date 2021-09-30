#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#include "shell379.h"

int main(int argc, char *argv[])
{
    // TODO for now assume the args are valid

    char cmd[MAX_LENGTH];
    char *args[MAX_ARGS+1];
    int total_args;

    int num_active_processes = 0;

    /*int user_time = 0;*/
    /*int sys_time = 0;*/
    pid_t pid;

    int stdin_copy = dup(0);

    total_args = prompt_cmd(cmd, args);


    while (strcmp(cmd, "exit") != 0) {

        if (is_shell_cmd(cmd)) {
            ;

        } else {

            int pipe_fd[2];
            if (pipe(pipe_fd) < 0)      // create pipe before forking a child
                perror("pipe error");

            if ((pid = fork()) < 0)     // fork a child
                perror("fork error!");
            else if (pid == 0) {       // child 
                dup2(pipe_fd[1], STDOUT_FILENO);    // stdout = pipe write end
                close(pipe_fd[0]);          // TODO for now, child won't read
                close(pipe_fd[1]);          // stdout is still open
                execvp(cmd, args);
                // child done
                
                fprintf(stderr, "Failed to execute the given command\n");
                _exit(1);
            } else {  // parent
                char buf[MAX_BUF+1];
                int n;
                int rout_fd;        // redirected output file description

                num_active_processes++;
                close(pipe_fd[1]);          // TODO for now, parent won't write

                waitpid(pid, NULL, 0);

                if ((rout_fd = open("redirected_output", O_CREAT | O_WRONLY, S_IRUSR | S_IWUSR ) < 0))
                    perror("open failed");

                if (close(pipe_fd[0]) < 0)
                    perror("Pipe fd close error");
                if (close(rout_fd) < 0 )
                    perror("Redirected output fd close error");

                num_active_processes--;
            }
        }

        // freeing strs in args
        if (total_args > 1) {
            free_args(args);
        }

        dup2(stdin_copy, STDIN_FILENO);     // restore stdin
        total_args = prompt_cmd(cmd, args);
    }

    

    return 0;
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
int prompt_cmd(char cmd[MAX_LENGTH], char *args[MAX_ARGS+1])
{
    char ch;
    int arg_i = 0;
    int ch_i = 0;

    scanf("%s", cmd);
    args[arg_i++] = cmd;

    if ((ch = getchar()) == '\n') {
        args[arg_i] = NULL;
        return arg_i;       // only the cmd was passed in
    }

    char *arg = calloc(MAX_LENGTH + 1, sizeof(*arg));

    // TODO use (ch = getchar()) != EOF
    while ((ch = getchar()) != '\n') {

        // the rest are args
        if (ch != ' ') {
            arg[ch_i++] = ch;
        } else {
            args[arg_i++] = arg;

            arg = calloc(MAX_LENGTH + 1, sizeof(*arg));       // reset arg
            ch_i = 0;
        }
    }
    args[arg_i++] = arg;
    args[arg_i] = NULL;

    return arg_i;       // cmd + args
}


void free_args(char *args[MAX_ARGS+1]) 
{
    // does not need to free the first arg
    for (int i=1; (i<MAX_ARGS+1) && (args[i] != NULL); i++) {
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
