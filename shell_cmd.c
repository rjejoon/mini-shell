#include <stdio.h>
#include <string.h>


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
    printf("\n");


}
