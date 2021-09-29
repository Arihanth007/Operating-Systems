#include "headers.h"
#include "functions.h"
#include "echo.h"
#include "cd.h"
#include "ls.h"
#include "processes.h"
#include "pinfo.h"

char hostname[sz],
    username[sz], home[sz], prevdir[2][sz], currentdir[sz], dirprint[sz], hist[25][sz];
int hist_sz = 0, process_num_added = 0;
struct Process *BG_Process[MAX_BG_PCS];

void scam()
{
    int forkReturn = fork();
    if (forkReturn == 0)
    {
        char command[] = "cp", *args[] = {"cp", ".pipe_write", ".pipe_read", NULL};
        if (execvp(command, args) < 0)
        {
            perror("Execvp");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        wait(NULL);
        return;
    }
}

void refresh()
{
    for (int i = 0; i < MAX_BG_PCS; i++)
    {
        if (BG_Process[i]->pid != 0)
        {
            char to_pinfo[sz];
            sprintf(to_pinfo, "%d", BG_Process[i]->pid);
            prcs_stat(to_pinfo, BG_Process[i]->process_status);
        }
    }
}

void free_allocated_mem()
{
    for (int i = 0; i < MAX_BG_PCS; i++)
        free(BG_Process[i]);
}

void handler(int num)
{
    // write(STDOUT_FILENO, "Encountered CTRL+C\n", 20);
    return;
}

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
    for (int i = 0; i < MAX_BG_PCS; i++)
    {
        BG_Process[i] = (struct Process *)malloc(sizeof(struct Process));
        BG_Process[i]->pid = 0;
    }
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

int check_pipes(char a[][sz], int t)
{
    int pipes = 0;
    for (int i = 0; i < t; i++)
    {
        if (strcmp(a[i], "|") == 0)
            pipes++;
    }
    return pipes;
}

void call_fn(char a[][sz], int t)
{
    refresh(); // Refresh process status

    // Check for which command
    if (strcmp(a[0], "echo") == 0)
        echo(a, t);
    else if (strcmp(a[0], "pwd") == 0)
        pwd();
    else if (strcmp(a[0], "cd") == 0)
    {
        if (strcmp(prevdir[0], currentdir) != 0)
        {
            memset(prevdir[1], 0, sizeof(prevdir[1]));
            strcpy(prevdir[1], prevdir[0]);
            memset(prevdir[0], 0, sizeof(prevdir[0]));
            strcpy(prevdir[0], currentdir);
        }
        cd(a, t, home, prevdir[1]);
    }
    else if (strcmp(a[0], "ls") == 0)
        ls(a, t, home);
    else if (strcmp(a[0], "pinfo") == 0)
        pinfo(a, t);
    else if (strcmp(a[0], "jobs") == 0)
        jobs(a, t);
    else if (strcmp(a[0], "sig") == 0)
        sig(a, t);
    else if (strcmp(a[0], "bg") == 0)
        run_bg(a, t);
    else if (strcmp(a[0], "history") == 0)
        query_history(a, t);
    else if (strcmp(a[0], "exit") == 0)
    {
        free_allocated_mem();
        exit(0);
    }
    else //system commands
    {
        process(a, t, home, prevdir[1]);
    }
}

void print_piping()
{
    int fdw, fdr;
    char pipe_write[] = ".pipe_write", pipe_read[] = ".pipe_read";
    if ((fdw = open(pipe_write, O_RDONLY | O_CREAT, 0644)) < 0)
    {
        perror("Opening file");
        return;
    }

    char buff[sz] = "";
    while (read(fdw, buff, sz) > 0)
        printf("%s", buff);

    if (close(fdw) < 0)
    {
        perror("Close");
        return;
    }
}

void perform_piping(char b[][sz], int l, int cur_pipe, int pipe_arr[2])
{
    int fdw, fdr, copy_stdout, copy_stdin;
    char pipe_write[] = ".pipe_write", pipe_read[] = ".pipe_read";
    FILE *f = fopen(pipe_write, "w+");
    fclose(f);
    if ((fdw = open(pipe_write, O_WRONLY | O_CREAT, 0644)) < 0)
    {
        perror("Opening file");
        return;
    }
    if ((fdr = open(pipe_read, O_RDONLY | O_CREAT, 0644)) < 0)
    {
        perror("Open");
        return;
    }

    if (cur_pipe != 1)
    {
        copy_stdin = dup(STDIN_FILENO);
        dup2(fdr, STDIN_FILENO);
    }
    copy_stdout = dup(STDOUT_FILENO);
    dup2(fdw, STDOUT_FILENO);

    call_fn(b, l);

    dup2(copy_stdout, STDOUT_FILENO);
    if (cur_pipe != 1)
    {
        dup2(copy_stdin, STDIN_FILENO);
    }
    if (close(fdw) < 0)
    {
        perror("Close");
        return;
    }
    if (close(fdr) < 0)
    {
        perror("Close");
        return;
    }
}

void perform_redirection(char b[][sz], int k, int isWrite, int isAppend, int isRead, char *write_file, char *read_file)
{
    int fdw, fdr, copy_stdout, copy_stdin;
    if (isWrite)
    {
        FILE *f = fopen(write_file, "w+");
        fclose(f);
        fdw = open(write_file, O_WRONLY | O_CREAT, 0644);
    }
    else if (isAppend)
        fdw = open(write_file, O_APPEND | O_WRONLY | O_CREAT, 0644);
    if (isAppend || isWrite)
    {
        if (fdw < 0)
        {
            perror("Opening file");
            return;
        }
        copy_stdout = dup(STDOUT_FILENO);
        dup2(fdw, STDOUT_FILENO);
    }
    if (isRead)
    {
        if ((fdr = open(read_file, O_RDONLY)) < 0)
        {
            perror("Open");
            return;
        }
        copy_stdin = dup(STDIN_FILENO);
        dup2(fdr, STDIN_FILENO);
    }

    call_fn(b, k);

    if (isAppend || isWrite)
    {
        dup2(copy_stdout, STDOUT_FILENO);
        if (close(fdw) < 0)
        {
            perror("Close");
            return;
        }
    }
    if (isRead)
    {
        dup2(copy_stdin, STDIN_FILENO);
        if (close(fdr) < 0)
        {
            perror("Close");
            return;
        }
    }
}

int main(int argc, char **argv)
{
    printf("%s\n\n", intro);
    initialise();
    // signal(SIGINT, handler);

    while (1)
    {
        // Additional flush conditions
        fseek(stdin, 0, SEEK_END);
        fseek(stdout, 0, SEEK_END);
        print_prompt();

        char string[sz], copy2[sz] = "";
        get_input(string);
        strcpy(copy2, string);
        add_history(string);

        int cnt = 0;
        char *cmd = strtok(string, ";"), cmd_arr[ARR_LEN][sz];
        if (cmd == NULL)
            continue;
        while (cmd != NULL) //separates by ;
        {
            strcpy(cmd_arr[cnt++], cmd);
            cmd = strtok(NULL, ";");
        }

        for (int i = 0; i < cnt; i++)
        {
            int t = 0;
            char *token = strtok(cmd_arr[i], " "), a[ARR_LEN][sz];
            while (token != NULL)
            {
                strcpy(a[t++], token);
                token = strtok(NULL, " ");
            }

            if (strcmp(a[0], "repeat") == 0) // checks repeats
            {
                int repeat = atoi(a[1]);
                char repeating_arr[ARR_LEN][sz];
                for (int i = 2; i < t; i++)
                    strcpy(repeating_arr[i - 2], a[i]);
                for (int i = 0; i < repeat; i++)
                    call_fn(repeating_arr, t - 2);
            }
            else
            {
                int isAppend = 0, isWrite = 0, isRead = 0, k = 0, pipes = check_pipes(a, t);
                char write_file[sz] = "", read_file[sz] = "", b[ARR_LEN][sz];
                for (int i = 0; i < t; i++)
                {
                    if (strcmp(a[i], ">") == 0) // handles redirection files and flags
                    {
                        isWrite = 1;
                        strcpy(write_file, a[i + 1]);
                        i++;
                    }
                    else if (strcmp(a[i], ">>") == 0)
                    {
                        isAppend = 1;
                        strcpy(write_file, a[i + 1]);
                        i++;
                    }
                    else if (strcmp(a[i], "<") == 0)
                    {
                        isRead = 1;
                        strcpy(read_file, a[i + 1]);
                        i++;
                    }
                    else
                        strcpy(b[k++], a[i]);
                }
                if (pipes != 0)
                    strcpy(a[t++], "|");

                if ((isAppend || isWrite || isRead) && pipes == 0) // Redirection without piping
                {
                    perform_redirection(b, k, isWrite, isAppend, isRead, write_file, read_file);
                }
                else if ((isAppend || isWrite || isRead) && pipes != 0) // Redirection with piping
                {
                    char b[ARR_LEN][sz], l = 0;
                    int pipe_arr[2], cur_pipe = 0, last_pipe = 0, isPrint = 1;
                    for (int i = 0; i < t; i++)
                    {
                        if (strcmp(a[i], ">") == 0 || strcmp(a[i], ">>") == 0 || strcmp(a[i], "<") == 0)
                        {
                            i++;
                            continue;
                        }
                        else if (strcmp(a[i], "|") == 0)
                        {
                            cur_pipe++;
                            isWrite = 0;
                            isAppend = 0;
                            isRead = 0;
                            int temp_pipe_cnt = 0, x = 0;
                            char c[ARR_LEN][sz];
                            for (int j = last_pipe; j < i; j++)
                            {
                                if (strcmp(a[j], ">") == 0)
                                {
                                    isWrite = 1;
                                    strcpy(write_file, a[j + 1]);
                                    j++;
                                }
                                else if (strcmp(a[j], ">>") == 0)
                                {
                                    isAppend = 1;
                                    strcpy(write_file, a[j + 1]);
                                    j++;
                                }
                                else if (strcmp(a[j], "<") == 0)
                                {
                                    isRead = 1;
                                    strcpy(read_file, a[j + 1]);
                                    j++;
                                }
                                else
                                    strcpy(c[x++], a[j]);
                            }

                            // debug
                            int isDebug = 0;
                            if (isDebug)
                            {
                                printf("\nCommand at pipe(%d/%d):", cur_pipe, pipes);
                                for (int z = 0; z < x; z++)
                                    printf(" %s", c[z]);
                                if (isWrite)
                                    printf("\nWriting to: %s", write_file);
                                if (isAppend)
                                    printf("\nAppending to: %s", write_file);
                                if (isRead)
                                    printf("\nReading from: %s", read_file);
                                printf("\n");
                            }

                            if (isWrite || isAppend)
                            {
                                if (isRead)
                                    perform_redirection(c, x, isWrite, isAppend, isRead, write_file, read_file);
                                else
                                    perform_redirection(c, x, isWrite, isAppend, 1, write_file, ".pipe_read");
                            }
                            if (cur_pipe == 1)
                                perform_redirection(c, x, 1, 0, isRead, ".pipe_write", read_file);
                            else
                                perform_redirection(c, x, 1, 0, 1, ".pipe_write", ".pipe_read");

                            last_pipe = i + 1;
                            if (cur_pipe - 1 == pipes && (isWrite || isAppend))
                                isPrint = 0;
                            scam();
                        }
                    }
                    if (isPrint)
                        print_piping();
                }
                else if (pipes != 0) // Piping without redirection
                {
                    char b[ARR_LEN][sz], l = 0;
                    int pipe_arr[2], cur_pipe = 0;
                    for (int i = 0; i < t; i++)
                    {
                        if (strcmp(a[i], "|") == 0)
                        {
                            cur_pipe++;
                            perform_piping(b, l, cur_pipe, pipe_arr);

                            for (int j = 0; j < l; j++)
                                memset(b[j], 0, sz);
                            l = 0;
                            scam();
                        }
                        else
                            strcpy(b[l++], a[i]);
                    }
                    print_piping();
                }
                else // No piping or redirection
                    call_fn(a, t);
            }
        }
    }

    return 0;
}