#include <stdbool.h>

#define LINE_LENGTH 100     // Max # of characters in an input file
#define MAX_ARGS 7          // Max # of args to a command
#define MAX_LENGTH 20       // Max # of characters in an argument
#define MAX_PT_ENTRIES 32   // Max entries in the Process Table
#define MAX_BUF 100000


int prompt_cmd(char cmd[MAX_LENGTH], char *args[MAX_ARGS+1]);
void free_args(char *args[MAX_ARGS+1], int total_args);
bool is_shell_cmd(char *cmd);
void redirect_output_to(char *rout_fname, int pipe_read_fd);
char *get_input_redirection_fname(char *args[MAX_ARGS+1]);
char *get_output_redirection_fname(char *args[MAX_ARGS+1]);

