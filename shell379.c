#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

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

    total_args = prompt_cmd(cmd, args);

    while (strcmp(cmd, "exit") != 0) {

        if (is_shell_cmd(cmd)) {




        } else {
            if ((pid = fork()) < 0)
                perror("fork error!");
            if (pid == 0) {       // child 
                execvp(cmd, args);

                // child done
            } else {
                num_active_processes++;
                waitpid(pid, NULL, 0);

                num_active_processes--;
            }
        }

        // freeing strs in args
        if (total_args > 1) {
            printf("Freeing\n");
            free_args(args);
        }

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
    for (int i=1; args[i] != NULL; i++) {
        free(args[i]);
    }
}
    

bool is_shell_cmd(char *cmd)
{
    char *shell_cmds[] = {"exit", "jobs", "kill", "resume", "sleep", "suspend", "wait"};

    for (

}
