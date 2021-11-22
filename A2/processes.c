#include "headers.h"
#include "functions.h"
#include "pinfo.h"

extern int process_num_added, terminal_pid, all_bg_pcs[pid_sz], all_fg_pcs[pid_sz], only_fg_pcs;
extern struct Process *BG_Process[MAX_BG_PCS];
extern void print_prompt(void);
char temp_name[sz];

void sigchldHandler(int num);
void execute_bg(char *args[]);
void execute_fg(char *args[]);
void jobs(char a[][sz], int t);
void sig(char a[][sz], int t);
void run_bg(char a[][sz], int t);
void run_fg(char a[][sz], int t);
void terminate_fg(int num);
void send_fg_bg(int num);

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

void sigchldHandler(int num)
{
    pid_t pid;
    int status;
    char buf[sz] = "";

    // Checks for terminated child processes
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        status = WIFEXITED(status);
        if(pid == only_fg_pcs)
        {
            kill(pid, 20);
            BG_Process[process_num_added]->pid = pid;
            BG_Process[process_num_added]->processID = process_num_added + 1;
            strcpy(BG_Process[process_num_added]->process_name, temp_name);
            strcpy(BG_Process[process_num_added]->process_status, "T");
            process_num_added++;
            only_fg_pcs = 0;
        }
        else
        {
            for(int i = 0; i < process_num_added; i++)
            {
                if (BG_Process[i]->pid == pid)
                {
                    fprintf(stderr, "\n%s with %d exited %s", BG_Process[i]->process_name, pid, status ? "abnormally" : "normally\n");
                    BG_Process[i]->pid = 0;
                    break;
                }
            }
        }

        if (errno != ECHILD)
            perror("Child process termination");
        return;
    }
}

void execute_bg(char *args[])
{
    char *pname = malloc(sz);
    char *st_val = malloc(sz);
    strcpy(pname, args[0]);
    for (int i = 1; i < sz; i++)
    {
        if (args[i] == NULL)
            break;
        strcat(pname, " ");
        strcat(pname, args[i]);
    }
    // signal(SIGCHLD, sigchldHandler); // checks for termination
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
    memset(temp_name, 0, sizeof(temp_name));
    strcpy(temp_name, args[0]);
    for (int i = 1; i < sz; i++)
    {
        if (args[i] == NULL)
            break;
        strcat(temp_name, " ");
        strcat(temp_name, args[i]);
    }
    int forkReturn = fork();
    if (forkReturn == 0)
    {
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
        only_fg_pcs = forkReturn;
        if (waitpid(forkReturn, &status, WUNTRACED) > 0)
        {
            if (WIFSTOPPED(status))
                printf("Suspending Process %d\n", forkReturn);
            only_fg_pcs = -1;
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

void terminate_fg(int num)
{
    if(only_fg_pcs == -1)
    {
        write(STDOUT_FILENO, "\n", 1);
        print_prompt();
    }
    only_fg_pcs = -1;
}

void send_fg_bg(int num)
{
    if(only_fg_pcs != -1)
    {
        int pid = only_fg_pcs;
        only_fg_pcs = -1;
        printf("Sending Current FG process to BG: %d\n", pid);

        BG_Process[process_num_added]->pid = pid;
        BG_Process[process_num_added]->processID = process_num_added + 1;
        strcpy(BG_Process[process_num_added]->process_name, temp_name);
        strcpy(BG_Process[process_num_added]->process_status, "T");
        process_num_added++;
    }
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

    if (tcsetpgrp(STDIN_FILENO, getpgid(pid)))
    {
        perror("Unable to transfer control to foreground process");
        // Setting signals to default
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        return;
    }

    // Getting the job running from bg
    if (kill(pid, SIGCONT))
    {
        perror("Unable to get bg process to fg");
        // Setting signals to default
        signal(SIGTTOU, SIG_DFL);
        signal(SIGTTIN, SIG_DFL);
        return;
    }

    // Reached till this point
    // => process has come to fg
    // all_fg_pcs[pid] = pid;
    only_fg_pcs = pid;
    BG_Process[idx]->pid = 0;

    // check for ctrl+C and ctrl+Z
    // signal(SIGINT, terminate_fg);
    // signal(SIGTSTP, send_fg_bg);

    int status;
    if (waitpid(pid, &status, WUNTRACED) > 0)
        if (WIFSTOPPED(status) == 0)
            only_fg_pcs = 0;

    // Terminal regains control
    if (tcsetpgrp(STDIN_FILENO, getpgid(0)))
    {
        perror("Terminal failed to regain control");
        exit(EXIT_FAILURE);
    }

    // Setting signals to default
    signal(SIGTTOU, SIG_DFL);
    signal(SIGTTIN, SIG_DFL);
}