#include "headers.h"
#include "functions.h"

void echo(char *token)
{
    token = strtok(NULL, " ");
    while (1)
    {
        print(token);
        token = strtok(NULL, " ");
        if (token == NULL)
        {
            print("\n");
            break;
        }
        print(" ");
    }
}