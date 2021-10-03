#define _POSIX_SOURCE
#define _DEFAULT_SOURCE

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>


#include "shell379.h"



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


void jobs_cmd(struct process *ptable[MAX_PT_ENTRIES], int num_active_p) 
{
    pid_t pids[MAX_PT_ENTRIES];
    char *states[MAX_PT_ENTRIES];
    int times[MAX_PT_ENTRIES];

    int ps_num_active_p = run_ps(pids, states, times);

    printf("\n");
    printf("Running processes:\n");
    if (ps_num_active_p > 0) {
        printf(" #       PID S SEC COMMAND\n");
        for (int i=0; i<ps_num_active_p; i++) {
            printf("%2d: %8d %s %3d \n", i, pids[i], states[i], times[i]);        

            // TODO  print args
            /*
            for (int j=0; j<ptable[i]->total_args; j++)
                printf(" %s", ptable[i]->args[j]);
            printf("\n");
            */
        }
    }
    printf("Processes = %6d active\n", ps_num_active_p);
    printf("Completed processes:\n");
    printf("\n");


}

int run_ps(pid_t pids[MAX_PT_ENTRIES], char *states[MAX_PT_ENTRIES], int times[MAX_PT_ENTRIES])
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
        states[i] = token;          // store state

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
