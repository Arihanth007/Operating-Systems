#include "headers.h"
#include "functions.h"
#include "pinfo.h"

extern int process_num_added;
extern struct Process *BG_Process[MAX_BG_PCS];

void exit_bg_process(int num);
void execute_bg(char *args[]);
void execute_fg(char *args[]);
void jobs(char a[][sz], int t);
void sig(char a[][sz], int t);
void run_bg(char a[][sz], int t);

void process(char a[][sz], int t, char *home, char *prev)
{
    int forkReturn, isBG = 0;
    char *args[t];

    if (strcmp(a[t - 1], "&") == 0)
    {
        isBG = 1;
        t--;
    }
    args[t] = NULL;

    for (int i = 0; i < t; i++)
        args[i] = a[i];

    if (isBG)
        execute_bg(args);
    else
        execute_fg(args);

    return;
}

void exit_bg_process(int num)
{
    int status, pid;
    char buf[sz] = "";

    if ((pid = waitpid(-1, &status, WNOHANG)) < 0)
    {
        if (errno != ECHILD)
            perror("Child process termination");
        return;
    }

    for (int i = 0; i < MAX_BG_PCS; i++)
    {
        if (pid != 0 && BG_Process[i]->pid == pid)
        {
            fprintf(stderr, "\n%s with %d exited %s", BG_Process[i]->process_name, pid, status ? "abnormally" : "normally\n");
            BG_Process[i]->pid = 0;
            BG_Process[i]->processID = 0;
            memset(BG_Process[i]->process_name, 0, sizeof(BG_Process[i]->process_name));
            memset(BG_Process[i]->process_status, 0, sizeof(BG_Process[i]->process_status));
        }
    }
}

void execute_bg(char *args[])
{
    char *pname = malloc(sz);
    char *st_val = malloc(sz);
    strcpy(pname, args[0]);
    signal(SIGCHLD, exit_bg_process);
    int forkReturn = fork();
    if (forkReturn == 0)
    {
        setpgid(0, 0);
        char *command = args[0];
        if (execvp(command, args) < 0)
        {
            perror("Execvp");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        printf("%d\n", forkReturn);

        char to_pinfo[sz];
        sprintf(to_pinfo, "%d", forkReturn);
        process_num_added++;
        prcs_stat(to_pinfo, st_val);
        BG_Process[process_num_added]->pid = forkReturn;
        BG_Process[process_num_added]->processID = process_num_added;
        strcpy(BG_Process[process_num_added]->process_name, pname);
        strcpy(BG_Process[process_num_added]->process_status, st_val);
    }
}

void execute_fg(char *args[])
{
    int forkReturn = fork();
    if (forkReturn == 0)
    {
        setpgid(0, 0);
        char *command = args[0];
        if (execvp(command, args) < 0)
        {
            perror("Execvp");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        int status;
        if (waitpid(forkReturn, &status, WUNTRACED) > 0 && WIFSTOPPED(status) != 0)
        {
        }
    }
}

void jobs(char a[][sz], int t)
{
    int isRunning = 1, isStopped = 1;
    if (t > 1)
    {
        for (int i = 1; i < t; i++)
        {
            if (strcmp(a[i], "-r") == 0)
                isStopped = 0;
            else if (strcmp(a[i], "-s") == 0)
                isRunning = 0;
        }
    }
    for (int i = 0; i < MAX_BG_PCS; i++)
    {
        if (BG_Process[i]->pid != 0)
        {
            if (strcmp(BG_Process[i]->process_status, "S") == 0)
            {
                if (isRunning)
                    printf("[%d] %s %s [%d]\n", BG_Process[i]->processID, "Running", BG_Process[i]->process_name, BG_Process[i]->pid);
            }
            else if (strcmp(BG_Process[i]->process_status, "T") == 0)
            {
                if (isStopped)
                    printf("[%d] %s %s [%d]\n", BG_Process[i]->processID, "Stopped", BG_Process[i]->process_name, BG_Process[i]->pid);
            }
        }
    }
}

void sig(char a[][sz], int t)
{
    if (t < 3)
    {
        printf("Not Enough Arguments Specified\n");
        return;
    }
    int pcs_id = atoi(a[1]), sg_num = atoi(a[2]), isPcs = 0;
    for (int i = 0; i < MAX_BG_PCS; i++)
    {
        if (BG_Process[i]->processID == pcs_id)
        {
            isPcs = 1;
            kill(BG_Process[i]->pid, sg_num);
        }
    }
    if (!isPcs)
        printf("Process with ID = %d not present\n", pcs_id);
}

void run_bg(char a[][sz], int t)
{
    if (t < 2)
    {
        printf("Not Enough Arguments Specified\n");
        return;
    }
    int pcs_id = atoi(a[1]), isPcs = 0;
    for (int i = 0; i < MAX_BG_PCS; i++)
    {
        if (BG_Process[i]->processID == pcs_id)
        {
            isPcs = 1;
            if (strcmp(BG_Process[i]->process_status, "T") == 0)
                kill(BG_Process[i]->pid, SIGCONT);
        }
    }
    if (!isPcs)
        printf("Process with ID = %d not present\n", pcs_id);
}