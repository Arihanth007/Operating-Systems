#include "headers.h"
#include "functions.h"
#include "pinfo.h"

extern int process_num_added, terminal_pid;
extern struct Process *BG_Process[MAX_BG_PCS];

void exit_bg_process(int num);
void execute_bg(char *args[]);
void execute_fg(char *args[]);
void jobs(char a[][sz], int t);
void sig(char a[][sz], int t);
void run_bg(char a[][sz], int t);
void run_fg(char a[][sz], int t);
void handler(int num);

void process(char a[][sz], int t, char *home, char *prev)
{
    int forkReturn, isBG = 0;
    char *args[t];

    // checks for bg process
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

    // Checks for terminated child processes
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
            // ser pid to 0 to indiacte that process is not active
            BG_Process[i]->pid = 0;
            // BG_Process[i]->processID = 0;
            // memset(BG_Process[i]->process_name, 0, sizeof(BG_Process[i]->process_name));
            // memset(BG_Process[i]->process_status, 0, sizeof(BG_Process[i]->process_status));
        }
    }
}

void execute_bg(char *args[])
{
    char *pname = malloc(sz);
    char *st_val = malloc(sz);
    strcpy(pname, args[0]);
    signal(SIGCHLD, exit_bg_process); // checks for termination
    int forkReturn = fork();
    if (forkReturn == 0) // child
    {
        setpgid(0, 0);
        char *command = args[0];
        if (execvp(command, args) < 0)
        {
            perror("Execvp");
            exit(EXIT_FAILURE);
        }
    }
    else // parent
    {
        printf("%d\n", forkReturn);

        char to_pinfo[sz];
        sprintf(to_pinfo, "%d", forkReturn);
        prcs_stat(to_pinfo, st_val);
        // store new process info
        BG_Process[process_num_added]->pid = forkReturn;
        BG_Process[process_num_added]->processID = process_num_added + 1;
        strcpy(BG_Process[process_num_added]->process_name, pname);
        strcpy(BG_Process[process_num_added]->process_status, st_val);
        process_num_added++;
        // free memory
        free(pname);
        free(st_val);
    }
}

void execute_fg(char *args[])
{
    int forkReturn = fork();
    if (forkReturn == 0)
    {
        setpgid(0, 0); // set new group for child
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
        { // Wait for process to complete
        }
    }
}

void jobs(char a[][sz], int t)
{
    int isRunning = 1, isStopped = 1;
    // set flags
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

    // sort array by process name
    qsort(BG_Process, process_num_added, sizeof(struct Process *), my_compare);

    // print all bg processes with details
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

    // Checking the existence of the process
    for (int i = 0; i < MAX_BG_PCS; i++)
    {
        if (BG_Process[i]->processID == pcs_id)
        {
            isPcs = 1;
            if (kill(BG_Process[i]->pid, sg_num) < 0) // send signal to process
                perror("Process not present");
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

    // Checking the existence of the process
    for (int i = 0; i < MAX_BG_PCS; i++)
    {
        if (BG_Process[i]->processID == pcs_id)
        {
            isPcs = 1;
            if (strcmp(BG_Process[i]->process_status, "T") == 0)
                if (kill(BG_Process[i]->pid, SIGCONT) < 0) // go from stopped -> running
                    perror("Process not present");
        }
    }
    if (!isPcs)
        printf("Process with ID = %d not present\n", pcs_id);
}

void handler(int num)
{
    char cur_status[sz], pid[sz];
    sprintf(pid, "%d", getpid());
    prcs_stat(pid, cur_status);
    // fprintf(stderr, "Status of %d: %s\n", getpid(), cur_status);
    if (getpid() != terminal_pid)
    {
        kill(getpid(), 9);
    }
    return;
}

void run_fg(char a[][sz], int t)
{
    if (t < 2)
    {
        printf("Not Enough Arguments Specified\n");
        return;
    }
    int pcs_id = atoi(a[1]), isPcs = 0, idx = -1, pid;

    // Checking the existence of the process
    for (int i = 0; i < MAX_BG_PCS; i++)
    {
        if (BG_Process[i]->processID == pcs_id)
        {
            isPcs = 1;
            idx = i;
            pid = BG_Process[i]->pid;
        }
    }
    if (!isPcs)
    {
        printf("Process with ID = %d not present\n", pcs_id);
        return;
    }

    // Shell ignores any signals
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    if (tcsetpgrp(STDIN_FILENO, getpgid(pid)) < 0)
    {
        perror("Unable to transfer control to foreground process");
        // Setting signals to default
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        return;
    }

    // Getting the job running from bg
    if (kill(pid, SIGCONT) < 0)
    {
        perror("Unable to get bg process to fg");
        // Setting signals to default
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        return;
    }

    // Reached till this point
    // => process has come to fg
    signal(SIGINT, handler);
    int status;
    if (waitpid(pid, &status, WUNTRACED) > 0 && WIFSTOPPED(status) != 0)
    {
    }

    // Terminal regains control
    if (tcsetpgrp(STDIN_FILENO, getpgid(0)) < 0)
    {
        perror("Terminal failed to regain control");
        exit(EXIT_FAILURE);
    }

    // Setting signals to default
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);

    // Assumed that process is no longer in bg
    if (idx != -1)
        BG_Process[idx]->pid = 0;
}