#include "functions.h"
#include "headers.h"

void get_input(char *ostr)
{
    size_t size = 1024; // initial size of char array
    fflush(stdin);
    char *string = malloc(size);
    char **string_pointer = &string; // double pointer to char array
    size_t characters = getline(string_pointer, &size, stdin);
    if (characters == -1)
    {
        perror("Taking input: ");
        return;
    }

    char output[sz] = "";
    my_tokenizer(string, output);
    free(string);

    strcpy(ostr, output);
}

void my_tokenizer(char *string, char *output)
{
    char delim[] = "\t\n ";
    char *token = strtok(string, delim);

    while (1)
    {
        strcat(output, token);
        token = strtok(NULL, delim);
        if (token == NULL)
            break;
        strcat(output, " ");
    }
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