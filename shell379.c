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

    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);

    printf("cmd: ");
    fflush(stdout);
    total_args = prompt_cmd(cmd, args);


    while (true) {

        if (strcmp(cmd, "exit") == 0) {
            break;
        }

        if (is_shell_cmd(cmd)) {
            ;

        } else {

            char *rin_fname = get_input_redirection_fname(args);
            char *rout_fname = get_output_redirection_fname(args);


            // NULL args where rin_fname and rout_fname are positioned
            char *filtered_args[MAX_ARGS+1];
            int i, j;
            for (i=0, j=0; i<total_args; i++) {
                if ( !(args[i][0] == '<' || args[i][0] == '>') ) {
                    filtered_args[j++] = args[i];
                } 
            }
            filtered_args[j] = NULL;
            

            int pipe_fd[2];
            if (pipe(pipe_fd) < 0)      // create pipe before forking a child
                perror("pipe error");

            if ((pid = fork()) < 0)     // fork a child
                perror("fork error!");
            else if (pid == 0) {       // child 

                /*FILE *rin_f = fopen(rin_fname, "r");*/

                if (rin_fname != NULL) {
                    int rin_fd = open(rin_fname, O_RDONLY);
                    if (rin_fd < 0)
                        perror("open error!");
                    if (dup2(rin_fd, 0) < 0)
                        perror("dup2 error!");
                    close(rin_fd);
                }

                if (rout_fname != NULL) {
                    if (dup2(pipe_fd[1], STDOUT_FILENO) < 0)    // stdout = pipe write end
                        perror("dup2 error!");
                    /*redirect_output_to(rout_fname, STDOUT_FILENO);*/
                }

                if (close(pipe_fd[0]) < 0)
                    perror("Pipe close error");
                if (close(pipe_fd[1] < 0))          
                    perror("Pipe close error");

                if (execvp(cmd, filtered_args) < 0) {  // child done
                    perror("execvp error!");
                    _exit(1);
                }

            } else {  // parent

                num_active_processes++;

                if (rout_fname != NULL)
                    redirect_output_to(rout_fname, pipe_fd[0]);

                waitpid(pid, NULL, 0);      

                if (close(pipe_fd[0]) < 0)
                    perror("Pipe close error");
                if (close(pipe_fd[1] < 0))          
                    perror("Pipe close error");

                num_active_processes--;

            }
        }

        // freeing allocated strs in args
        if (total_args > 1) {
            free_args(args, total_args);
        }

        dup2(stdin_copy, STDIN_FILENO);     // restore stdin
        dup2(stdout_copy, STDOUT_FILENO);   // restore stdout
        printf("cmd: ");
        fflush(stdout);
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


void free_args(char *args[MAX_ARGS+1], int total_args) 
{
    // does not need to free the first arg
    for (int i=1; i<total_args; i++) {
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

void redirect_output_to(char *rout_fname, int read_fd) {

    char buf[MAX_BUF+1];
    int rout_fd;        // redirected output file description
    ssize_t nbytes;

    // Write only, set read/write mode, and create if the file does not exist
    rout_fd = open(rout_fname, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR); 
    if (rout_fd < 0)
        perror("open failed");

    /*while ((nbytes = read(read_fd, buf, sizeof(buf))) > 0) {*/
        /*if (nbytes < 0) {*/
            /*perror("read error!");*/
            /*break;*/
        /*}*/
        /*if (write(rout_fd, buf, nbytes) < 0 && errno != EINTR) {*/
            /*perror("write error!");*/
            /*break;*/
        /*}*/
    /*}*/

    // TODO use while loop to read
    nbytes = read(read_fd, buf, sizeof(buf));
    if (nbytes < 0) {
        perror("read error!");
    }
    if (write(rout_fd, buf, nbytes) < 0 && errno != EINTR) 
        perror("write error!");

    if (close(rout_fd) < 0)
        perror("Redirected output fd close error");
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
