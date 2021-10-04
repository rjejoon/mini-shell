#if !(defined(__APPLE__) && defined(__MACH__))
    #define _POSIX_SOURCE
#endif
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


struct process ptable[MAX_PT_ENTRIES];
unsigned int num_active_p;

int main(int argc, char *argv[])
{
    // TODO for now assume the args are valid

    char **args;
    int total_args;

    // copies of stdin and stdout
    int stdin_copy = dup(STDIN_FILENO);
    int stdout_copy = dup(STDOUT_FILENO);

    pid_t pid;
    num_active_p = 0;

    while (true) {

        dup2(stdin_copy, STDIN_FILENO);     // restore stdin
        dup2(stdout_copy, STDOUT_FILENO);   // restore stdout

        args = malloc( (MAX_ARGS + 1) * sizeof(*args) );
        total_args = prompt_cmd(args);

        reap_possible_children();           // reap chlid proc right after the prompt

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
                while (true) {
                    pid_t end_pid = waitpid(target_pid, NULL, 0);
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

                if (is_bg_process)
                    setpgid(0, 0);          // make new process group and set child as a leader

                if (rin_fname)
                    redirect_input(rin_fname);

                if (rout_fname)
                    redirect_output(rout_fname);

                if (close(pipe_fd[0]) < 0)
                    perror("Pipe close error in child process");
                if (close(pipe_fd[1] < 0))          
                    perror("Pipe close error in child process");

                if (execvp(filtered_args[0], filtered_args) < 0) {
                    char err_msg[100];
                    sprintf(err_msg, "Failed to execute '%s'", filtered_args[0]);
                    perror(err_msg);
                    _exit(EXIT_FAILURE);
                }

            } else {        // parent
                signal(SIGCHLD, proc_exit_handler);     // catch SIGCHLD signal

                if (!is_bg_process) {
                    // wait for the child to finish and remove it from process table
                    waitpid(pid, NULL, 0);
                } else {
                    // add proc to process table
                    struct process proc;
                    proc.pid = pid;
                    copy_args(proc.args, args);
                    proc.total_args = total_args;
                    ptable[num_active_p++] = proc;      
                }

                free_args(args, total_args);

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


/*
 * Function: copy_args 
 * -----------------------------------
 *      Hard copy args in source to destination.
 *
 *  Inputs:
 *      dest: where the copied args will go 
 *      source_args: args to be copied
 *
 */
void copy_args(char dest[MAX_ARGS+1][MAX_LENGTH+1], char **source_args)
{
    for (int i=0; source_args[i] != NULL; i++) 
        strcpy(dest[i], source_args[i]);
}


void free_args(char **args, int total_args) 
{
    for (int i=0; i<total_args; i++)
        free(args[i]);
    free(args);
}


/*
 * Function: reap_possible_children
 * -----------------------------------
 *      Reap all possible children processes and remove corresponding processes in the process table
 *
 */
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
            
            // shift existing proc
            for (int i=pindex; i<num_active_p-1; i++) 
                ptable[i] = ptable[i+1];
            num_active_p--;
        }
    }
}


/*
 * Function: get_input_redirection_fname 
 * -----------------------------------
 *      Return the file name for input redirection 
 *      Return a null pointer if it does not exist.
 *
 *  Inputs:
 *      args: args to be searched
 *
 *  Returns:
 *      char *: file name
 *
 */
char *get_input_redirection_fname(char **args)
{
    for (int i=1; (i<MAX_ARGS+1) && (args[i] != NULL); i++) {
        if (args[i][0] == '<') {
            return args[i] + 1;
        }
    }
    return NULL;
}


/*
 * Function: get_output_redirection_fname 
 * -----------------------------------
 *      Return the file name for output redirection. 
 *      Return a null pointer if it does not exist.
 *
 *  Inputs:
 *      args: args to be searched
 *
 *  Returns:
 *      char *: file name
 *
 */
char *get_output_redirection_fname(char **args)
{
    for (int i=1; (i<MAX_ARGS+1) && (args[i] != NULL); i++) {
        if (args[i][0] == '>') {
            return args[i] + 1;
        }
    }
    return NULL;
}


/*
 * Function: remove_redirection_and_bg_args 
 * -----------------------------------
 *      Copy the pointers of all args execept for the redirection args and &. Then store them in filtered_args.
 *      Return true if the user wants to have the process running in backround.
 *
 *  Inputs:
 *      filtered_args: filtered args ptrs to be stored
 *      args: original args
 *      total_args: # of original args
 *
 *  Returns:
 *      is_bg: bool
 *
 */
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


/*
 * Function: redirect_input 
 * -----------------------------------
 *      Redirect stdin to rin_fname
 *
 *  Inputs:
 *      rin_fname: file name
 *
 */
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


/*
 * Function: redirect_output 
 * -----------------------------------
 *      Redirect stdout to rin_fname
 *
 *  Inputs:
 *      rin_fname: file name
 *
 */
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


/*
 * Function: proc_exit_handler 
 * -----------------------------------
 *      SIGCHLD signal handler.
 *      Reap a terminated child process and remove it from the process table.
 *
 *  Inputs:
 *      signum: int
 *
 */
void proc_exit_handler(int signum)
{
    pid_t pid;

    while (true) {
        pid = waitpid((pid_t) -1, NULL, WNOHANG);
        if (pid == 0)           // child still in progress
            ;
        else if (pid == -1)     // child does not exist
            return;
        else {
            // found terminated child
            int pindex = find_proc_index(ptable, num_active_p, pid);
            
            // shift existing proc
            for (int i=pindex; i<num_active_p-1; i++) 
                ptable[i] = ptable[i+1];
            num_active_p--;

            return;
        }
    }

}


/*
 * Function: print_children_cputimes
 * -----------------------------------
 *      Print cumulative user and sys times used by all children processes.
 *
 */
void print_children_cputimes(void)
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

    printf("User time: %6ld seconds\n", total_user_time);
    printf("Sys time: %6ld seconds\n", total_sys_time);
}
