#include "headers.h"
#include "functions.h"
#include "echo.h"
#include "pwd.h"
#include "cd.h"
#include "ls.h"

char hostname[sz],
    username[sz], home[sz], prevdir[2][sz], currentdir[sz], dirprint[sz];

void initialise()
{
    if (getlogin_r(username, sz) == -1)
        perror("Get username: ");
    if (gethostname(hostname, sz) == -1)
        perror("Get hostname: ");
    if (getcwd(home, sz) == NULL)
        perror("Get cwd: ");
    strcpy(prevdir[0], home);
    strcpy(prevdir[1], home);
}

void check_pwd()
{
    memset(dirprint, 0, sizeof(dirprint));
    memset(currentdir, 0, sizeof(currentdir));
    if (getcwd(currentdir, sz) == NULL)
        perror("Get cwd: ");
    int cmp = strcmp(currentdir, home);
    if (cmp == 0)
        dirprint[0] = '~';
    else if (cmp > 0)
    {
        int a = strlen(home), b = strlen(currentdir), isSame = 1;
        for (int i = 0; i < a; i++)
        {
            if (home[i] != currentdir[i])
                isSame = 0;
        }
        if (isSame)
        {
            dirprint[0] = '~';
            for (int i = a; i < b; i++)
                dirprint[i - a + 1] = currentdir[i];
        }
        else if (getcwd(dirprint, sz) == NULL)
            perror("Get cwd: ");
    }
    else if (getcwd(dirprint, sz) == NULL)
        perror("Get cwd: ");
}

void print_prompt()
{
    check_pwd();
    printf("<%s@%s:%s> ", username, hostname, dirprint);
}

void call_fn(char *str)
{
    char *cmd = strtok(str, " ");
    if (strcmp(cmd, "echo") == 0)
    {
        echo(cmd);
    }
    else if (strcmp(cmd, "pwd") == 0)
    {
        pwd();
    }
    else if (strcmp(cmd, "cd") == 0)
    {
        if (strcmp(prevdir[0], currentdir) != 0)
        {
            memset(prevdir[1], 0, sizeof(prevdir[1]));
            strcpy(prevdir[1], prevdir[0]);
            memset(prevdir[0], 0, sizeof(prevdir[0]));
            strcpy(prevdir[0], currentdir);
        }
        cd(cmd, home, prevdir[1]);
    }
    else if (strcmp(cmd, "ls") == 0)
    {
        ls(cmd, home, prevdir[1]);
    }
}

int main(int argc, char **argv)
{
    printf("%s\n\n", intro);
    initialise();

    while (1)
    {
        print_prompt();

        char *string = malloc(sz);
        get_input(string);
        int cnt = 0;
        char *cmd = strtok(string, ";"), cmd_arr[100][sz];
        while (cmd != NULL)
        {
            strcpy(cmd_arr[cnt++], cmd);
            cmd = strtok(NULL, ";");
        }

        for (int i = 0; i < cnt; i++)
        {
            call_fn(cmd_arr[i]);
        }

        free(string);
    }

    return 0;
}