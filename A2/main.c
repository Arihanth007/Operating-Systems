#include "headers.h"
#include "functions.h"
#include "echo.h"
#include "pwd.h"
#define sz 1024

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

void print_prompt()
{
    memset(dirprint, 0, sizeof(dirprint));
    memset(currentdir, 0, sizeof(currentdir));
    getcwd(currentdir, sz);
    if (strcmp(currentdir, home) == 0)
        dirprint[0] = '~';

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
}

int main(int argc, char **argv)
{
    printf("%s\n\n", intro);
    initialise();

    while (1)
    {
        print_prompt();

        char *string = get_input();
        char *cmd = strtok(string, ";"), cmd_arr[100][sz];
        int cnt = 0;
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