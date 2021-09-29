#include "headers.h"

int my_compare(const void *a, const void *b)
{
    // a is a pointer into the array of pointers
    struct Process *l = *(struct Process **)a;
    struct Process *r = *(struct Process **)b;

    return strcmp(l->process_name, r->process_name);
}

int main()
{
    // char a[] = "HI", b[] = "HHCEFGHIJKP";
    // printf("%d\n", strcmp((const char *)b, (const char *)a));

    struct Process *BG_Process[MAX_BG_PCS];
    for (int i = 0; i < MAX_BG_PCS; i++)
    {
        BG_Process[i] = (struct Process *)malloc(sizeof(struct Process));
        BG_Process[i]->pid = 0;
        // memset(BG_Process[i]->process_name, 0, sizeof(BG_Process[i]->process_name));
        // memset(BG_Process[i]->process_status, 0, sizeof(BG_Process[i]->process_status));
    }

    BG_Process[0]->pid = 100;
    strcpy(BG_Process[0]->process_name, "abcde");
    BG_Process[1]->pid = 200;
    strcpy(BG_Process[1]->process_name, "cde");
    BG_Process[2]->pid = 150;
    strcpy(BG_Process[2]->process_name, "abcd");

    printf("Before Sorting\n");
    for (int i = 0; i < MAX_BG_PCS; i++)
    {
        if (BG_Process[i]->pid != 0)
            printf("%s\n", BG_Process[i]->process_name);
    }
    printf("\n");

    qsort(BG_Process, 3, sizeof(struct Process *), my_compare);

    printf("\nAfter Sorting\n");
    for (int i = 0; i < MAX_BG_PCS; i++)
    {
        if (BG_Process[i]->pid != 0)
            printf("%d: %s\n", BG_Process[i]->pid, BG_Process[i]->process_name);
    }
    for (int i = 0; i < MAX_BG_PCS; i++)
        free(BG_Process[i]);

    return 0;
}