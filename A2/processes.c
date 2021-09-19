#include "headers.h"
#include "functions.h"

extern char *process_name[1000000];

void exit_bg_process()
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
        fprintf(stderr, "\n%s with %d exited %s", process_name[pid], pid, status ? "abnormally" : "normally\n");
    else
        fprintf(stderr, "\nProcess with %d exited %s", pid, status ? "abnormally" : "normally\n");
    free(process_name[pid]);
    return;
}

void process(char *token, char *home, char *prev)
{
    int forkReturn, i = 0, isBG = 0;
    char *args[sz], vals[100][sz], *pname = malloc(sz);

    while (token != NULL)
    {
        if (strcmp(token, "&") == 0)
        {
            isBG = 1;
            token = strtok(NULL, " ");
            continue;
        }
        strcpy(vals[i], token);
        args[i] = vals[i];
        i++;
        token = strtok(NULL, " ");
    }
    vals[i][0] = '\0';

    if (!isBG)
    {
        forkReturn = fork();
        if (forkReturn == 0)
        {
            if (execvp(vals[0], args) < 0)
            {
                perror("Execvp");
            }
        }
        else
        {
            wait(NULL);
        }
    }
    else
    {
        strcpy(pname, vals[0]);
        signal(SIGCHLD, exit_bg_process);
        forkReturn = fork();
        if (forkReturn == 0)
        {
            if (execvp(vals[0], args) < 0)
            {
                perror("Execvp");
            }
        }
        else
        {
            printf("%d\n", forkReturn);
            if (forkReturn < pid_sz)
                process_name[forkReturn] = pname;
        }
    }
    return;
}