#include "shell379.h"


/*
 * Function: find_proc_index 
 * -----------------------------------
 *      Return the index of process in ptable, given a pid.
 *      Return -1 if there is no such process with the given pid.
 *
 *  Inputs:
 *      ptable: process table to be searched
 *      num_active_p: total # of processes in ptable
 *      pid: pid to search
 *
 *  Returns:
 *      index or -1
 *
 */
int find_proc_index(struct process ptable[MAX_PT_ENTRIES], unsigned int num_active_p, pid_t pid)
{
    for (int i=0; i<num_active_p; i++) {
        if (ptable[i].pid == pid)
            return i;
    }
    return -1;
}


