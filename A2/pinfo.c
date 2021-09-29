#include "headers.h"
#include "functions.h"

int readfile(char *filename, char *content)
{
    FILE *fptr;
    char c;

    fptr = fopen(filename, "r");
    if (fptr == NULL)
    {
        perror("Cannot open file");
        return 0;
    }

    int cnt = 0;
    while ((c = fgetc(fptr)) != EOF)
        content[cnt++] = c;

    fclose(fptr);
    return cnt;
}

void pinfo(char a[][sz], int t)
{
    int pid = getpid();
    if (t > 1)
        pid = atoi(a[1]);

    FILE *fptr;
    char f1[20], c1[sz] = "", f2[20], c2[sz] = "";
    sprintf(f1, "/proc/%d/stat", pid);
    sprintf(f2, "/proc/%d/exe", pid);

    int cnt1 = readfile(f1, c1), cnt2 = readlink(f2, c2, sz), cnt = 0;
    if (cnt1 == 0 || cnt2 == 0)
    {
        printf("Process isn't present in /proc\n");
        return;
    }

    printf("pid -- %d\n", pid);

    char *stat_vals = strtok(c1, " ");
    while (stat_vals != NULL)
    {
        if (cnt == 2)
            printf("Process Status -- %s%s\n", stat_vals, pid == getpid() ? "+" : "");
        if (cnt == 22)
            printf("memory -- %s {Virtual Memory}\n", stat_vals);
        stat_vals = strtok(NULL, " ");
        cnt++;
    }

    printf("Executable Path -- %s\n", c2);
}

void prcs_stat(char a[], char *status_arr)
{
    int pid = atoi(a);

    FILE *fptr;
    char f1[20], c1[sz] = "", f2[20], c2[sz] = "";
    sprintf(f1, "/proc/%d/stat", pid);
    sprintf(f2, "/proc/%d/exe", pid);

    int cnt1 = readfile(f1, c1), cnt2 = readlink(f2, c2, sz), cnt = 0;
    if (cnt1 == 0 || cnt2 == 0)
    {
        printf("Process isn't present in /proc\n");
        return;
    }

    char *stat_vals = strtok(c1, " ");
    while (stat_vals != NULL)
    {
        if (cnt == 2)
        {
            // printf("Process Status -- %s%s\n", stat_vals, pid == getpid() ? "+" : "");
            strcpy(status_arr, stat_vals);
            break;
        }
        stat_vals = strtok(NULL, " ");
        cnt++;
    }
}