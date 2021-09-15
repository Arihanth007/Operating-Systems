#include "headers.h"
#include "functions.h"

void process(char *token, char *home, char *prev)
{
    int forkReturn, i = 0, loopsize, isBG = 0;
    forkReturn = fork();
    char *args[sz], vals[100][sz];

    while (token != NULL)
    {
        strcpy(vals[i], token);
        args[i] = vals[i];
        i++;
        token = strtok(NULL, " ");
    }
    if (strcmp(vals[i - 1], "&") == 0)
    {
        isBG = 1;
        i--;
    }
    strcpy(vals[i++], "NULL");

    if (!isBG)
    {
        if (forkReturn == 0)
        {
            execvp(vals[0], args);
        }
        else
        {
            wait(NULL);
        }
    }
    else
    {
        if (forkReturn == 0)
        {
            execvp(vals[0], args);
        }
        else
        {
            printf("%d\n", forkReturn);
            // wait(NULL);
        }
    }
}