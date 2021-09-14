#include "headers.h"
#include "functions.h"

void cd(char *token, char *home, char *prev)
{
    token = strtok(NULL, " ");
    char inp[sz], dir[sz], cur_dir[sz];
    getcwd(cur_dir, sz);
    strcpy(inp, token);
    int end, l = strlen(cur_dir);

    if (inp[0] == '.' && inp[1] != '.')
    {
        chdir(inp);
    }
    else if (inp[0] == '.' && inp[1] == '.')
    {
        chdir(inp);
    }
    else if (inp[0] == '-')
    {
        chdir(prev);
    }
    else if (inp[0] == '~')
    {
        if (strlen(inp) < 2)
            chdir(home);
        char *ptr = inp;
        ptr++;
        strcat(dir, home);
        strcat(dir, ptr);
        chdir(dir);
    }
    else
    {
        strcpy(dir, cur_dir);
        strcat(dir, "/");
        strcat(dir, inp);
        chdir(dir);
    }
}