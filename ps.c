#define _POSIX_SOURCE
#define _DEFAULT_SOURCE

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
#include <ctype.h>

#define LINE_LENGTH 100     // Max # of characters in an input file
#define MAX_ARGS 7          // Max # of args to a command
#define MAX_LENGTH 20       // Max # of characters in an argument
#define MAX_PT_ENTRIES 100000   // Max entries in the Process Table
#define MAX_BUF 100000


char *trim(char *s);
char *ltrim(char *s);
char *rtrim(char *s);


int main(void) 
{
    pid_t pid;
    pid_t ppid = getpid();
    FILE *p_fp;

    char args_str[] = "ps -eo pid,state,cputimes";
    //sprintf(args_str, "ps --ppid %d -o pid,state,cputimes", ppid);

    p_fp = popen(args_str, "r");

    pid_t pids[MAX_PT_ENTRIES];
    char *states[MAX_PT_ENTRIES];
    int times[MAX_PT_ENTRIES];
    int i = 0;

    int buf_size = 100;
    char buf[buf_size];
    char *trimmed_buf;
    char *token;

    fgets(buf, buf_size, p_fp);      // don't need first row
    while (fgets(buf, buf_size, p_fp) != NULL) {

        trimmed_buf = rtrim(ltrim(buf));

        token = trim(strtok_r(trimmed_buf, " ", &trimmed_buf));
        pids[i] = atoi(token);

        token = trim(strtok_r(trimmed_buf, " ", &trimmed_buf));
        states[i] = token;

        token = trim(strtok_r(trimmed_buf, " ", &trimmed_buf));
        times[i] = atoi(token);

        i++;
    }

    pclose(p_fp);


    return 0;
    
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
