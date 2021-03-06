#if !(defined(__APPLE__) && defined(__MACH__))
    #define _POSIX_SOURCE
#endif
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <ctype.h>


#include "shell379.h"


/*
 * Function: is_shell_cmd 
 * -----------------------------------
 *      Return true if cmd is shell379 cmd and false if not.
 *
 *  Inputs:
 *      cmd: char *
 *
 *  Returns:
 *      bool
 *
 */
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


/*
 * Function: jobs_cmd 
 * -----------------------------------
 *      Displays currently active children processes and 
 *      cumulative used user and sys times by all children processes.
 *
 *  Inputs:
 *      ptable: process table
 *      num_active_p: total # of processes in process table
 *
 */
void jobs_cmd(struct process ptable[MAX_PT_ENTRIES], int num_active_p) 
{
    pid_t pids[MAX_PT_ENTRIES];
    char states[MAX_PT_ENTRIES][5];
    int times[MAX_PT_ENTRIES];

    int ps_num_active_p = run_ps(pids, states, times);

    printf("\n");
    printf("Running processes:\n");
    if (ps_num_active_p > 0) {
        printf(" #       PID S SEC COMMAND\n");
        for (int i=0; i<ps_num_active_p; i++) {
            printf("%2d: %8d %s %3d", i, pids[i], states[i], times[i]);        

            for (int j=0; j<ptable[i].total_args; j++)
                printf(" %s", ptable[i].args[j]);
            printf("\n");
        }
    }
    printf("Processes = %6d active\n", ps_num_active_p);

    printf("Completed processes:\n");
    print_children_cputimes();
    printf("\n");
}


/*
 * Function: run_ps 
 * -----------------------------------
 *      Runs "ps" command in child process and stores information about 
 *      all children processes forked by the parent.
 *      Returns the number of children processes found in "ps".
 *
 *  Inputs:
 *      pids: pid_t array
 *      states: char *array
 *      times: int array
 *
 *  Return:
 *      # of children processes found in ps command in int.
 *
 */
int run_ps(pid_t pids[MAX_PT_ENTRIES], char states[MAX_PT_ENTRIES][5], int times[MAX_PT_ENTRIES])
{
    pid_t ppid = getpid();
    FILE *p_fp;

    int buf_size = 100;
    char buf[buf_size];
    char *trimmed_buf;
    char *token;

    int i = 0;

    char args_str[100];
    sprintf(args_str, "ps --ppid %d -o pid,state,cputimes --sort start", ppid);

    p_fp = popen(args_str, "r");

    fgets(buf, buf_size, p_fp);      // don't need first row

    while (fgets(buf, buf_size, p_fp) != NULL) {

        trimmed_buf = rtrim(ltrim(buf));

        token = trim(strtok_r(trimmed_buf, " ", &trimmed_buf));
        if (atoi(token) == ppid)
            continue;       // don't need to store parent info

        pids[i] = atoi(token);      // store pid

        token = trim(strtok_r(trimmed_buf, " ", &trimmed_buf));
        strcpy(states[i], token);

        token = trim(strtok_r(trimmed_buf, " ", &trimmed_buf));
        times[i] = atoi(token);     // store time in seconds

        i++;
    }

    pclose(p_fp);

    return i - 1;       // don't need ps process info
}


char *trim(char *s)
{
    return rtrim(ltrim(s));
}

char *ltrim(char *s)
{
    while (isspace(*s)) s++;
    return s;
}

char *rtrim(char *s)
{
    char *back = s + strlen(s);
    while (isspace(*--back)) ;
    *(back+1) = '\0';

    return s;
}
