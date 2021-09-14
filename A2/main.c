#include "headers.h"
#include "functions.h"
#include "echo.h"
#include "pwd.h"
#include "cd.h"

char hostname[sz],
    username[sz], home[sz], prevdir[sz], currentdir[sz], dirprint[sz];

int initialise()
{
    getlogin_r(username, sz);
    gethostname(hostname, sz);
    getcwd(home, sz);
    strcpy(prevdir, home);
    return 0;
}

void check_pwd()
{
    memset(prevdir, 0, sizeof(prevdir));
    strcpy(prevdir, currentdir);
    memset(dirprint, 0, sizeof(dirprint));
    memset(currentdir, 0, sizeof(currentdir));
    getcwd(currentdir, sz);
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
        else
            getcwd(dirprint, sz);
    }
    else
    {
        getcwd(dirprint, sz);
    }
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
        cd(cmd, home, prevdir);
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
            // print(cmd);
            // print("\n");
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