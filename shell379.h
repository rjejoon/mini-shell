#include <stdbool.h>
#include <unistd.h>

#define LINE_LENGTH 100     // Max # of characters in an input file
#define MAX_ARGS 7          // Max # of args to a command
#define MAX_LENGTH 20       // Max # of characters in an argument
#define MAX_PT_ENTRIES 32   // Max entries in the Process Table
#define MAX_BUF 100000

struct process {
    pid_t pid;
    char **args;
    int total_args;
};

int prompt_cmd(char **args);
void free_args(char **args, int total_args);
bool is_shell_cmd(char *cmd);
void redirect_output_to(char *rout_fname, int pipe_read_fd);
char *get_input_redirection_fname(char **args);
char *get_output_redirection_fname(char **args);
bool remove_redirection_and_bg_args(char *filtered_args[MAX_ARGS+1], char **args, int total_args);
void redirect_input(char *rin_fname);
void redirect_output(char *rout_fname);
void proc_exit(int signum);
int find_proc_index(struct process *ptable[MAX_PT_ENTRIES], unsigned int num_active_p, pid_t pid); 
void free_proc(struct process *ptable[MAX_PT_ENTRIES], int pindex);
void args_tostr(char **args, char *args_str, int total_args);
