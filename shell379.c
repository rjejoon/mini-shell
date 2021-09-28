#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "shell379.h"

int main(int argc, char *argv[])
{
    // TODO for now assume the args are valid

    char cmd[MAX_LENGTH];
    char args[MAX_ARGS][MAX_LENGTH];
    int total_args;
    total_args = prompt_cmd(cmd, args);

    while (strcmp(cmd, "exit") != 0) {



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
int prompt_cmd(char cmd[MAX_LENGTH], char args[MAX_ARGS][MAX_LENGTH])
{
    char ch;
    bool is_first_word = true;
    int arg_i = 0;
    int ch_i = 0;
    int total_args = 0;

    while ((ch = getchar()) != '\n') {
        if (is_first_word) {
            // first word is cmd
            if (ch != ' ') {
                cmd[ch_i++] = ch;
            } else {
                cmd[ch_i] = '\0';
                ch_i = 0;
                is_first_word = false;
                total_args++;
            }
        } else {
            // the rest are args
            if (ch != ' ') {
                args[arg_i][ch_i++] = ch;
            } else {
                args[arg_i++][ch_i] = '\0';
                ch_i = 0;
                total_args++;
            }
        }
    }
    // append \0 at the end of str
    if (is_first_word)
        cmd[ch_i] = '\0';
    else
        args[arg_i][ch_i] = '\0';
    total_args++;

    return total_args;
}


    

