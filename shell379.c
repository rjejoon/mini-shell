#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>


/*#include <sys/times.h>*/

#include "shell379.h"



struct process *ptable[MAX_PT_ENTRIES];
unsigned int num_active_p = 0;


int main(int argc, char *argv[])
{
    // TODO for now assume the args are valid

    /*char cmd[MAX_LENGTH];*/
    char **args;
    int total_args;


    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);

    struct rusage usage;    // struct for storing resources used by child process
    struct timeval user_t;
    struct timeval sys_t;

    double total_user_time = 0;
    double total_sys_time = 0;


    pid_t pid;


    while (true) {

        dup2(stdin_copy, STDIN_FILENO);     // restore stdin
        dup2(stdout_copy, STDOUT_FILENO);   // restore stdout

        args = malloc( (MAX_ARGS + 1) * sizeof(*args) );
        total_args = prompt_cmd(args);

        if (is_shell_cmd(args[0])) {

            if (strcmp(args[0], "exit") == 0) {
                free_args(args, total_args);
                return EXIT_SUCCESS;
            }

            if (strcmp(args[0], "jobs") == 0) {
                printf("\n");
                printf("Running processes:\n");
                if (num_active_p > 0) {
                    printf(" #    PID S SEC COMMAND\n");
                    for (int i=0; i<num_active_p; i++) {
                        printf(" %d: %5d R %3d ", i, ptable[i]->pid, 0);        // TODO SEC

                        // print args
                        for (int j=0; j<ptable[i]->total_args; j++) 
                            printf(" %s", ptable[i]->args[j]);
                        printf("\n");
                    }
                }
                printf("Processes = %6d active\n", num_active_p);
                printf("Completed processes:\n");
                printf("User time = %6d seconds\n", (int) total_user_time);
                printf("Sys  time = %6d seconds\n", (int) total_sys_time);
                printf("\n");
            }

            if (strcmp(args[0], "sleep") == 0) {
                long int seconds = strtol(args[1], NULL, 10);
                sleep(seconds);
            }


        } else {
            char *rin_fname = get_input_redirection_fname(args);
            char *rout_fname = get_output_redirection_fname(args);
            bool is_bg_process;

            char *filtered_args[MAX_ARGS+1];
            is_bg_process = remove_redirection_and_bg_args(filtered_args, args, total_args);

            signal(SIGCHLD, proc_exit);

            int pipe_fd[2];
            if (pipe(pipe_fd) < 0)      // create pipe before forking a child
                perror("pipe error");

            if ((pid = fork()) < 0) {
                perror("fork error");

            } else if (pid == 0) {          // child

                if (rin_fname != NULL)
                    redirect_input(rin_fname);

                if (rout_fname != NULL)
                    redirect_output(rout_fname);

                close(pipe_fd[0]);
                close(pipe_fd[1]);

                execvp(filtered_args[0], filtered_args);

            } else {        // parent

                struct process *proc = malloc(sizeof(*proc));
                proc->pid = pid;
                proc->args = args;
                proc->total_args = total_args;

                ptable[num_active_p++] = proc;      // add proc to process table

                printf("pid: %d start\n", pid);


                if (!is_bg_process)
                    wait(NULL);         

                getrusage(RUSAGE_CHILDREN, &usage);
                user_t = usage.ru_utime;
                sys_t = usage.ru_stime;

                // TODO Corretly update total times
                total_user_time += user_t.tv_sec + ((double) user_t.tv_usec)/1000000;
                total_sys_time += sys_t.tv_sec + ((double) sys_t.tv_usec)/1000000;

                printf("\n");
                printf("User time: %f seconds\n", total_user_time);
                printf("Sys time: %f seconds\n", total_sys_time);
                printf("\n");



                if (close(pipe_fd[0]) < 0)
                    perror("Pipe close error");
                if (close(pipe_fd[1] < 0))          
                    perror("Pipe close error");
            }

        }

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
int prompt_cmd(char **args)
{
    int ch;
    int arg_i = 0;

    printf("SHELL379: "); 
    fflush(stdout);

    char *arg;
    do {
        arg = calloc(MAX_LENGTH + 1, sizeof(*arg));
        scanf("%s", arg);
        args[arg_i++] = arg;

    } while ((ch = getchar()) != '\n');

    args[arg_i] = NULL;



    return arg_i;
}


void free_args(char **args, int total_args) 
{
    for (int i=0; i<total_args; i++) {
        free(args[i]);
    }
    free(args);
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

char *get_input_redirection_fname(char **args)
{
    for (int i=1; (i<MAX_ARGS+1) && (args[i] != NULL); i++) {
        if (args[i][0] == '<') {
            return args[i] + 1;
        }
    }

    return NULL;
}

char *get_output_redirection_fname(char **args)
{
    for (int i=1; (i<MAX_ARGS+1) && (args[i] != NULL); i++) {
        if (args[i][0] == '>') {
            return args[i] + 1;
        }
    }

    return NULL;
}


bool remove_redirection_and_bg_args(char *filtered_args[MAX_ARGS+1], char **args, int total_args)
{
    int i, j;
    bool is_bg = false;
    char arg_type;
    for (i=0, j=0; i<total_args; i++) {
        arg_type = args[i][0];
        if ( !(arg_type == '<' || arg_type == '>' || arg_type == '&') )
            filtered_args[j++] = args[i];
        if (arg_type == '&')
            is_bg = true;
    }
    filtered_args[j] = NULL;

    return is_bg;
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


void proc_exit(int signum)
{
    pid_t pid;

    while (true) {
        pid = wait3(NULL, WNOHANG, (struct rusage *) NULL);
        if (pid == 0)
            return;
        else if (pid == -1)
            return;
        else {
            int pindex = find_proc_index(ptable, num_active_p, pid);
            free_proc(ptable, pindex);
            printf ("pid: %d finished \n", pid);
        }
    }

}

int find_proc_index(struct process *ptable[MAX_PT_ENTRIES], unsigned int num_active_p, pid_t pid)
{
    for (int i=0; i<num_active_p; i++) {
        if (ptable[i]->pid == pid)
            return i;
    }

    return -1;
}


void free_proc(struct process *ptable[MAX_PT_ENTRIES], int pindex)
{
    free_args(ptable[pindex]->args, ptable[pindex]->total_args);
    printf("Freed args\n");
    free(ptable[pindex]);
    printf("Freed proc\n");

    // shift existing proc
    int i;
    for (i=pindex; i<num_active_p-1; i++) {
        ptable[i] = ptable[i+1];
    }
    ptable[i] = NULL;       // remove the last one 

    num_active_p--;
}


void args_tostr(char **args, char *args_str, int total_args)
{
    strcpy(args_str, args[0]);
    for (int i=1; i<total_args; i++) {
        strcat(args_str, args[i]);
    }
}






