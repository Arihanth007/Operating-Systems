#include "headers.h"
#include "functions.h"
#include "echo.h"
#include "cd.h"
#include "ls.h"
#include "processes.h"
#include "pinfo.h"

char hostname[sz],
    username[sz], home[sz], prevdir[2][sz], currentdir[sz], dirprint[sz], hist[25][sz], *process_name[pid_sz];
int hist_sz = 0;

void load_history()
{
    FILE *fp;
    char line[sz];

    fp = fopen(".history", "r");
    if (fp == NULL)
    {
        // perror("Opening file");
        return;
    }

    while (fgets(line, sz, fp) != NULL)
    {
        char *token = strtok(line, "\n");
        if (token == NULL)
            continue;
        // printf("%s\n", token);
        strcpy(hist[hist_sz++], token);
    }

    fclose(fp);

    return;
}

void add_history(char *command)
{
    if (strcmp(command, "") == 0)
        return;

    FILE *fp;

    fp = fopen(".history", "w+");
    if (fp == NULL)
    {
        perror("Opening file");
        return;
    }

    if (hist_sz < 20)
    {
        strcpy(hist[hist_sz++], command);
        for (int i = 0; i < hist_sz; i++)
            fprintf(fp, "%s\n", hist[i]);
    }
    else
    {
        for (int i = 0; i < 20; i++)
        {
            memset(hist[i], 0, sz);
            strcpy(hist[i], hist[i + 1]);
        }
        strcpy(hist[19], command);
        for (int i = 0; i < 20; i++)
            fprintf(fp, "%s\n", hist[i]);
    }

    fclose(fp);
}

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
    load_history();
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
    printf("<\033[0;31m%s\033[0;34m@%s\033[0;32m:%s\033[0m> ", username, hostname, dirprint);
}

int check_repreat(char *str)
{
    int repeat = 1;
    char *cmd = strtok(str, " ");
    if (strcmp(cmd, "repeat") == 0)
    {
        cmd = strtok(NULL, " ");
        repeat = atoi(cmd);
    }
    while (cmd != NULL)
        cmd = strtok(NULL, " ");

    return repeat;
}

void call_fn(char *str)
{
    char *cmd = strtok(str, " ");
    if (cmd == NULL)
        return;

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
        ls(cmd, home);
    }
    else if (strcmp(cmd, "pinfo") == 0)
    {
        pinfo(cmd);
    }
    else if (strcmp(cmd, "history") == 0)
    {
        query_history(cmd);
    }
    else
    {
        process(cmd, home, prevdir[1]);
    }
}

int main(int argc, char **argv)
{
    printf("%s\n\n", intro);
    initialise();

    while (1)
    {
        print_prompt();

        char *string = malloc(sz), copy2[sz] = "";
        get_input(string);
        strcpy(copy2, string);
        add_history(string);

        int cnt = 0;
        char *cmd = strtok(string, ";"), cmd_arr[100][sz];
        if (cmd == NULL)
            continue;
        while (cmd != NULL)
        {
            strcpy(cmd_arr[cnt++], cmd);
            cmd = strtok(NULL, ";");
        }

        for (int i = 0; i < cnt; i++)
        {
            char simply_temp[sz] = "";
            strcpy(simply_temp, cmd_arr[i]);
            char *token = strtok(simply_temp, " ");
            if (strcmp(token, "repeat") == 0)
            {
                token = strtok(NULL, " ");
                int repeat_num = atoi(token);
                char repeat_cmd[sz] = "";
                while ((token = strtok(NULL, " ")) != NULL)
                {
                    strcat(repeat_cmd, token);
                    strcat(repeat_cmd, " ");
                }
                while (repeat_num--)
                {
                    char tmp[sz] = "";
                    strcpy(tmp, repeat_cmd);
                    call_fn(tmp);
                }
            }
            else
                call_fn(cmd_arr[i]);
        }

        free(string);
    }

    return 0;
}