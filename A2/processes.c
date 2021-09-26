#include "headers.h"
#include "functions.h"

extern char *process_name[1000000];

void exit_bg_process(int num)
{
    int status, pid;
    char buf[sz] = "";

    if ((pid = waitpid(-1, &status, WNOHANG)) < 0)
    {
        if (errno != ECHILD)
            perror("Child process termination");
        return;
    }

    if (pid < pid_sz)
    {
        if (process_name[pid] != NULL)
            fprintf(stderr, "\n%s with %d exited %s", process_name[pid], pid, status ? "abnormally" : "normally\n");
    }
    else
        fprintf(stderr, "\nProcess with %d exited %s", pid, status ? "abnormally" : "normally\n");
    free(process_name[pid]);
    return;
}

void execute_bg(char *args[])
{
    char *pname = malloc(sz);
    strcpy(pname, args[0]);
    signal(SIGCHLD, exit_bg_process);
    int forkReturn = fork();
    if (forkReturn == 0)
    {
        if (execvp(args[0], args) < 0)
        {
            perror("Execvp");
            exit(EXIT_FAILURE);
        }
        exit(0);
    }
    else
    {
        if (setpgid(forkReturn, 0) != 0)
            perror("setpgid() error");
        printf("%d\n", forkReturn);
        if (forkReturn < pid_sz)
            process_name[forkReturn] = pname;
        return;
    }
}

void execute_fg(char *args[])
{
    int forkReturn = fork();
    if (forkReturn == 0)
    {
        if (execvp(args[0], args) < 0)
        {
            perror("Execvp");
            exit(EXIT_FAILURE);
        }
        exit(0);
    }
    else
    {
        if (setpgid(forkReturn, 0) != 0)
            perror("setpgid() error");
        wait(NULL);
        return;
    }
}

void process(char a[][sz], int t, char *home, char *prev)
{
    int forkReturn, isBG = 0;
    char *args[sz], *pname = malloc(sz);

    if (strcmp(a[t - 1], "&") == 0)
    {
        isBG = 1;
        t--;
    }

    for (int i = 0; i < t; i++)
        args[i] = a[i];

    if (isBG)
        execute_bg(args);
    else
        execute_fg(args);

    return;
}