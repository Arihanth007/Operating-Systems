#include "stdio.h"
#include "stdlib.h"

int testme(int *n)
{
    *n = 5;
    return 1;
}

int main(int argc, char *argv[])
{
    // int a = atoi(argv[1]);
    // int num = atoi(argv[1]);
    int t = (int)1 << num;
    // int ans = a & t;

    printf("2^%d: %d\n", num, t);
    // printf("1 << num: %d\na & t: %d\n", t, ans);

    return 0;
}