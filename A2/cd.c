#include "headers.h"
#include "functions.h"

void cd(char *token, char *home, char *prev)
{
    token = strtok(NULL, " ");
    if (token == NULL)
        return;
    char inp[sz], dir[sz], cur_dir[sz];
    getcwd(cur_dir, sz);
    strcpy(inp, token);
    int end, l = strlen(cur_dir);

    if (inp[0] == '.' && inp[1] != '.')
    {
        if (chdir(inp) != 0)
        {
            perror("Chdir error");
            return;
        }
    }
    else if (inp[0] == '.' && inp[1] == '.')
    {
        if (chdir(inp) != 0)
        {
            perror("Chdir error");
            return;
        }
    }
    else if (inp[0] == '-')
    {
        if (chdir(prev) != 0)
        {
            perror("Chdir error");
            return;
        }
    }
    else if (inp[0] == '~')
    {
        if (strlen(inp) < 2)
        {
            if (chdir(home) != 0)
            {
                perror("Chdir error");
                return;
            }
            return;
        }
        char *ptr = inp;
        ptr++;
        strcpy(dir, home);
        strcat(dir, ptr);
        if (chdir(dir) != 0)
        {
            perror("Chdir error");
            return;
        }
    }
    else
    {
        strcpy(dir, cur_dir);
        strcat(dir, "/");
        strcat(dir, inp);
        if (chdir(dir) != 0)
        {
            perror("Relative path to (our) home failed: ");
            if (chdir(inp) != 0)
            {
                perror("Absolute path failed too: ");
                return;
            }
        }
    }
}

void pwd()
{
    char cwd[sz];
    if (getcwd(cwd, sz) == NULL)
        perror("Get cwd: ");
    printf("%s\n", cwd);
}