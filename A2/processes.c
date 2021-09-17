#include "headers.h"
#include "functions.h"

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

    fprintf(stderr, "\nProcess with %d exited %s", pid, status ? "abnormally" : "normally\n");
    return;
}

void process(char *token, char *home, char *prev)
{
    int forkReturn, i = 0, loopsize, isBG = 0;
    char *args[sz], vals[100][sz];

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
        }
    }
    return;
}