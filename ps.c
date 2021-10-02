#define _POSIX_SOURCE

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/resource.h>

#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <stdio.h>


int main(void) 
{
    pid_t pid;
    pid_t ppid = getpid();
    char ppid_str[33];
    sprintf(ppid_str, "%d", ppid);

    int pipe_fd[2];
    if (pipe(pipe_fd) < 0)      // create pipe before forking a child
        perror("pipe error");

    if ((pid = fork()) < 0) {
        perror("Fork error!");
    } else if (pid == 0) {       // child
        // char *args[] = {"ps", "--ppid", ppid_str, "-o", "pid,state,cputimes"};
        char *args[] = {"ls", "-la"};

        if (close(pipe_fd[0]) < 0)
            perror("Pipe close error");
        if (close(pipe_fd[1] < 0))          
            perror("Pipe close error");

        execvp(args[0], args);

    } else {        // parent

        if (close(pipe_fd[0]) < 0)
            perror("Pipe close error");
        if (close(pipe_fd[1] < 0))          
            perror("Pipe close error");

    }

    
}
