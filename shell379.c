#define _POSIX_SOURCE

#include <string.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>

#include "shell379.h"


// struct process *ptable[MAX_PT_ENTRIES];
struct process ptable[MAX_PT_ENTRIES];
unsigned int num_active_p;

int main(int argc, char *argv[])
{
    // TODO for now assume the args are valid

    char **args;
    int total_args;


    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);


    pid_t pid;
    num_active_p = 0;


    printf("Parent ppid = %d\n", getpid());     // TODO delete later

    while (true) {


        dup2(stdin_copy, STDIN_FILENO);     // restore stdin
        dup2(stdout_copy, STDOUT_FILENO);   // restore stdout

        args = malloc( (MAX_ARGS + 1) * sizeof(*args) );
        total_args = prompt_cmd(args);

        reap_possible_children();

        if (is_shell_cmd(args[0])) {

            if (strcmp(args[0], "exit") == 0) {
                while (wait(NULL) > 0) ;        // wait for all children to finish

                printf("\nResources used\n");
                print_children_cputimes();
                free_args(args, total_args);
                return EXIT_SUCCESS;
            }

            else if (strcmp(args[0], "jobs") == 0) {
                jobs_cmd(ptable, num_active_p);
                free_args(args, total_args);
            }

            else if (strcmp(args[0], "kill") == 0) {
                pid_t target_pid = (pid_t) strtol(args[1], NULL, 10);
                kill(target_pid, SIGKILL);
                free_args(args, total_args);
            }

            else if (strcmp(args[0], "resume") == 0) {
                pid_t target_pid = (pid_t) strtol(args[1], NULL, 10);
                kill(target_pid, SIGCONT);
                free_args(args, total_args);
            }

            else if (strcmp(args[0], "sleep") == 0) {
                long int seconds = strtol(args[1], NULL, 10);
                sleep(seconds);
                free_args(args, total_args);
            }

            else if (strcmp(args[0], "suspend") == 0) {
                pid_t target_pid = (pid_t) strtol(args[1], NULL, 10);
                kill(target_pid, SIGSTOP);
                free_args(args, total_args);
            }

            else if (strcmp(args[0], "wait") == 0) {
                pid_t target_pid = (pid_t) strtol(args[1], NULL, 10);
                pid_t end_pid;
                while (true) {
                    end_pid = waitpid(target_pid, NULL, WNOHANG);
                    if (end_pid == -1) {            // child not found
                        perror("wait error!");
                        break;
                    } else if (end_pid == 0) {      // child is in process
                        continue;
                    } else if (end_pid == target_pid) {     // child ended 
                        break;
                    }
                }
                free_args(args, total_args);
            }

        } else {
            char *rin_fname = get_input_redirection_fname(args);
            char *rout_fname = get_output_redirection_fname(args);
            bool is_bg_process;

            char *filtered_args[MAX_ARGS+1];
            is_bg_process = remove_redirection_and_bg_args(filtered_args, args, total_args);


            int pipe_fd[2];
            if (pipe(pipe_fd) < 0)      // create pipe before forking a child
                perror("pipe error");

            if ((pid = fork()) < 0) {
                perror("fork error");

            } else if (pid == 0) {          // child

                if (is_bg_process) {
                    setpgid(0, 0);  
                }

                if (rin_fname != NULL)
                    redirect_input(rin_fname);

                if (rout_fname != NULL)
                    redirect_output(rout_fname);

                close(pipe_fd[0]);
                close(pipe_fd[1]);

                if (execvp(filtered_args[0], filtered_args) < 0) {
                    perror("exec error!");
                    _exit(EXIT_FAILURE);
                }

            } else {        // parent
                // TODO handle SIGCHLD
                signal(SIGCHLD, proc_exit_handler);

                struct process proc;
                proc.pid = pid;
                copy_args(proc.args, args);
                proc.total_args = total_args;

                /*
                printf("#: %d\n", num_active_p);
                printf("cmd:");
                for (int i=0; i<proc.total_args; i++)
                    printf(" %s", proc.args[i]);
                printf("\n");
                */

                ptable[num_active_p++] = proc;      // add proc to process table

                free_args(args, total_args);

                if (!is_bg_process) {
                    // wait for the child to finish
                    while (true) {
                        pid_t pid = waitpid(-1, NULL, 0);
                        if (pid == 0)
                            break;
                        else if (pid == -1)
                            break;
                        else {
                            int pindex = find_proc_index(ptable, num_active_p, pid);
                            // free_proc(ptable, pindex, num_active_p);
                            
                            // shift existing proc
                            for (int i=pindex; i<num_active_p-1; i++) 
                                ptable[i] = ptable[i+1];
                            num_active_p--;

                            printf ("pid: %d finished \n", pid);
                            break;
                        }

                    }
                }

                if (close(pipe_fd[0]) < 0)
                    perror("Pipe close error");
                if (close(pipe_fd[1] < 0))          
                    perror("Pipe close error");
            }
        }

        reap_possible_children();
    }

    return EXIT_SUCCESS;
}

void copy_args(char dest[MAX_ARGS+1][MAX_LENGTH+1], char **args)
{
    for (int i=0; args[i] != NULL; i++) 
        strcpy(dest[i], args[i]);
}

void reap_possible_children() 
{
    while (true) {
        pid_t pid = waitpid(-1, NULL, WNOHANG);
        if (pid == 0)
            break;
        else if (pid == -1)
            break;
        else {
            int pindex = find_proc_index(ptable, num_active_p, pid);
            // free_proc(ptable, pindex, num_active_p);
            
            // shift existing proc
            for (int i=pindex; i<num_active_p-1; i++) 
                ptable[i] = ptable[i+1];
            num_active_p--;

            printf ("pid: %d finished \n", pid);
        }

    }

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


void proc_exit_handler(int signum)
{
    /*
    pid_t pids[MAX_PT_ENTRIES];
    char *states[MAX_PT_ENTRIES];
    int times[MAX_PT_ENTRIES];

    int ps_num_active_p = run_ps(pids, states, times);

    printf("%d %d\n", num_active_p, ps_num_active_p);
    for (int i=0; i<num_active_p; i++) {
        bool found_pid = false;

        // find pid in ptable from pids in ps
        for (int j=0; j<ps_num_active_p; j++) {
            if (ptable[i] == pids[j]) {
                found_pid = true;
                break;
            }
        }
        if (!found_pid) {
            // shift existing proc
            for (int pindex=i; pindex<num_active_p-1; pindex++) 
                ptable[pindex] = ptable[pindex+1];
            num_active_p--;

            printf ("pid: %d finished \n", ptable[i]);
            return;
        }
    }
    */

    pid_t pid;

    while (true) {
        pid = waitpid((pid_t) -1, NULL, WNOHANG);
        if (pid == 0)
            return;
        else if (pid == -1)
            return;
        else {
            int pindex = find_proc_index(ptable, num_active_p, pid);
            // free_proc(ptable, pindex, num_active_p);
            
            // shift existing proc
            for (int i=pindex; i<num_active_p-1; i++) 
                ptable[i] = ptable[i+1];
            num_active_p--;

            // printf("pid: %d finished \n", pid);
            return;
        }
    }

}


void print_children_cputimes()
{
    struct rusage usage;    // struct for storing resources used by child process
    struct timeval user_t;
    struct timeval sys_t;

    long total_user_time;
    long total_sys_time;

    // print total user & sys time used by all children
    getrusage(RUSAGE_CHILDREN, &usage);
    user_t = usage.ru_utime;
    sys_t = usage.ru_stime;

    total_user_time = user_t.tv_sec;
    if ((double) user_t.tv_usec/1000000 >= 0.5)
        total_user_time++;

    total_sys_time = sys_t.tv_sec;
    if ((double) sys_t.tv_usec/1000000 >= 0.5)
        total_sys_time++;

    printf("User time: %ld seconds\n", total_user_time);
    printf("Sys time: %ld seconds\n", total_sys_time);
}
