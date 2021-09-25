#include "headers.h"
#include "functions.h"

void echo(char a[][sz], int t)
{
    for (int i = 1; i < t - 1; i++)
        printf("%s ", a[i]);
    printf("%s\n", a[t - 1]);
    return;
}
// void echo(char *token)
// {
//     token = strtok(NULL, " ");
//     if (token == NULL)
//         return;
//     while (1)
//     {
//         print(token);
//         token = strtok(NULL, " ");
//         if (token == NULL)
//         {
//             print("\n");
//             break;
//         }
//         print(" ");
//     }
// }