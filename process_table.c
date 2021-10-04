#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "shell379.h"


int find_proc_index(struct process ptable[MAX_PT_ENTRIES], unsigned int num_active_p, pid_t pid)
{
    for (int i=0; i<num_active_p; i++) {
        if (ptable[i].pid == pid)
            return i;
    }

    return -1;
}


void free_proc(struct process *ptable[MAX_PT_ENTRIES], int num_active_p, int pindex)
{
    // shift existing proc
    for (int i=pindex; i<num_active_p-1; i++) {
        ptable[i] = ptable[i+1];
    }
    num_active_p--;
}


