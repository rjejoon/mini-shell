#include <stdbool.h>

#define LINE_LENGTH 100     // Max # of characters in an input file
#define MAX_ARGS 7          // Max # of args to a command
#define MAX_LENGTH 20       // Max # of characters in an argument
#define MAX_PT_ENTRIES 32   // Max entries in the Process Table


int prompt_cmd(char cmd[MAX_LENGTH], char *args[MAX_ARGS+1]);
void free_args(char *args[MAX_ARGS+1]);
bool is_shell_cmd(char *cmd);

