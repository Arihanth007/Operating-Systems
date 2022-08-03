#include "functions.h"
#include "headers.h"

extern int isExit;

void get_input(char *ostr)
{
    size_t size = 1024; // initial size of char array
    fflush(stdin);
    fflush(stdout);
    char inp_cmd[size];
    size_t inp_len = 0;

    if (!fgets(inp_cmd, size, stdin))
    {
        perror("Recieved EOF Character\n");
        isExit = 1;
        return;
    }

    char output[sz] = {'\0'};
    my_tokenizer(inp_cmd, output);

    strcpy(ostr, output);
}

void my_tokenizer(char *string, char *output)
{
    char delim[] = "\t\n ";
    char *token = strtok(string, delim);
    if (token == NULL)
        return;

    while (1)
    {
        strcat(output, token);
        token = strtok(NULL, delim);
        if (token == NULL)
            break;
        strcat(output, " ");
    }
}

void query_history(char a[][sz], int t)
{
    FILE *fp;
    char line[sz], hist[100][sz];
    int id = 10, cnt = 0;

    if (t > 1)
        id = atoi(a[1]);

    fp = fopen(".history", "r");
    if (fp == NULL)
    {
        perror("Opening file");
        return;
    }

    while ((fgets(line, sz, fp) != NULL))
    {
        char *token2 = strtok(line, "\n");
        strcpy(hist[cnt++], line);
    }

    for (int i = max(cnt - id, 0); i < cnt; i++)
        printf("%s\n", hist[i]);

    fclose(fp);

    return;
}

void print(char *str)
{
    char p[1024];
    sprintf(p, "%s", str);
    printf("%s", p);
    return;
}

int min(int a, int b)
{
    return a < b ? a : b;
}

int max(int a, int b)
{
    return a > b ? a : b;
}

int my_compare(const void *a, const void *b)
{
    // a is a pointer into the array of pointers
    struct Process *l = *(struct Process **)a;
    struct Process *r = *(struct Process **)b;

    return strcmp(l->process_name, r->process_name);
}