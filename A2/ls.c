#include "headers.h"
#include "functions.h"

void print_ls(char *dir, int op_a, int op_l)
{
    //Here we will list the directory
    struct dirent *d;
    DIR *dh = opendir(dir);
    if (!dh)
    {
        if (errno == ENOENT)
            perror("Directory doesn't exist");
        else
            perror("Unable to read directory");
        return;
    }
    //While the next entry is not readable we will print directory files
    while ((d = readdir(dh)) != NULL)
    {
        //If hidden files are found we continue
        if (!op_a && d->d_name[0] == '.')
            continue;
        printf("%s  ", d->d_name);
        if (op_l)
            printf("\n");
    }
    if (!op_l)
        printf("\n");
}

void ls(char *token, char *home, char *prev)
{
    char args[100][sz], flags[5], directories[sz];
    int t = 0, f = 0, d = 0, isL = 0, isA = 0;
    token = strtok(NULL, " ");
    while (token != NULL)
    {
        strcpy(args[t++], token);
        token = strtok(NULL, " ");
    }

    for (int i = 0; i < t; i++)
    {
        if (args[i][0] == '-')
        {
            flags[f++] = args[i][1];
            if (strlen(args[i]) > 2)
                flags[f++] = args[i][2];
        }
        else
            strcpy(&directories[d++], args[i]);
    }
    if (d == 0)
        strcpy(&directories[d++], ".");

    for (int i = 0; i < f; i++)
    {
        if (flags[i] == 'a')
            isA = 1;
        else if (flags[i] == 'l')
            isL = 1;
    }

    for (int i = 0; i < d; i++)
        print_ls(&directories[i], isA, isL);
}
