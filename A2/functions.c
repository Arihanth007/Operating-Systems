#include "functions.h"
#include "headers.h"

void get_input(char *ostr)
{
    size_t size = 1024; // initial size of char array
    fflush(stdin);
    fflush(stdout);
    char string[sz];
    memset(string, 0, size);
    char *tmp = &string[0], **string_pointer = &tmp; // double pointer to char array
    size_t characters = getline(string_pointer, &size, stdin);
    if (characters == -1)
    {
        perror("Taking input: ");
        return;
    }

    char output[sz] = "";
    my_tokenizer(string, output);

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

// void my_sort(struct Process *BG_Process[])
// {
//     qsort(BG_Process, MAX_BG_PCS, sizeof(BG_Process), &my_compare);
// }