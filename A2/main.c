#include "headers.h"
#include "functions.h"
#include "echo.h"
#define sz 100

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

void call_fn(char *cmd)
{
    if (strcmp(cmd, "echo") == 0)
    {
        echo(cmd);
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
        char *token = strtok(string, " ");
        call_fn(token);

        free(string);
    }

    return 0;
}