#include <stdlib.h>
#include <sys/types.h>


#include "shell379.h"



void proc_exit_handler(int signum)
{
    /*pid_t pid;*/

    // TODO use ps command
    
    /*while (true) {*/
        /*[>pid = wait3(NULL, WNOHANG, (struct rusage *) NULL);<]*/
        /*pid = waitpid((pid_t) -1, NULL, WNOHANG);*/
        /*if (pid == 0)*/
            /*return;*/
        /*else if (pid == -1)*/
            /*return;*/
        /*else {*/
            /*int pindex = find_proc_index(ptable, num_active_p, pid);*/
            /*free_proc(ptable, pindex, num_active_p);*/
            /*printf ("pid: %d finished \n", pid);*/
        /*}*/
    /*}*/

}


int find_proc_index(struct process *ptable[MAX_PT_ENTRIES], unsigned int num_active_p, pid_t pid)
{
    for (int i=0; i<num_active_p; i++) {
        if (ptable[i]->pid == pid)
            return i;
    }

    return -1;
}


void free_proc(struct process *ptable[MAX_PT_ENTRIES], int num_active_p, int pindex)
{
    free_args(ptable[pindex]->args, ptable[pindex]->total_args);
    free(ptable[pindex]);

    // shift existing proc
    int i;
    for (i=pindex; i<num_active_p-1; i++) {
        ptable[i] = ptable[i+1];
    }
    ptable[i] = NULL;       // remove the last one

    num_active_p--;
}


