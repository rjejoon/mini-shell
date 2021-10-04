#ifndef SHELL379_H
#define SHELL379_H

#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>

#define LINE_LENGTH 100     // Max # of characters in an input file
#define MAX_ARGS 7          // Max # of args to a command
#define MAX_LENGTH 20       // Max # of characters in an argument
#define MAX_PT_ENTRIES 32   // Max entries in the Process Table
#define MAX_BUF 100000


struct process {
    pid_t pid;
    char args[MAX_ARGS+1][MAX_LENGTH+1];
    int total_args;
};


// shell379 functions
int prompt_cmd(char **args);
void free_args(char **args, int total_args);
void copy_args(char dest[MAX_ARGS+1][MAX_LENGTH+1], char **args);
bool remove_redirection_and_bg_args(char *filtered_args[MAX_ARGS+1], char **args, int total_args);
void reap_possible_children();
void print_children_cputimes(void);

char *get_input_redirection_fname(char **args);
char *get_output_redirection_fname(char **args);
void redirect_input(char *rin_fname);
void redirect_output(char *rout_fname);


// Process table functions
void proc_exit_handler(int signum);
int find_proc_index(struct process ptable[MAX_PT_ENTRIES], unsigned int num_active_p, pid_t pid); 

// shell cmds
bool is_shell_cmd(char *cmd);
void jobs_cmd(struct process ptable[MAX_PT_ENTRIES], int num_active_p);
int run_ps(pid_t pids[MAX_PT_ENTRIES], char states[MAX_PT_ENTRIES][5], int times[MAX_PT_ENTRIES]);
char *trim(char *s);
char *ltrim(char *s);
char *rtrim(char *s);


#endif
