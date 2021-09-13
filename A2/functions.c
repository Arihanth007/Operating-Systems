#include "functions.h"
#include "headers.h"

char *get_input()
{
    size_t size = 1024; // initial size of char array
    char *string = malloc(size), *output = malloc(size);
    char **string_pointer = &string; // double pointer to char array

    size_t characters = getline(string_pointer, &size, stdin);

    const char s[10] = " ";
    char *token;

    token = strtok(string, s); // get the first token

    /* walk through other tokens */
    while (token != NULL)
    {
        strcat(output, token);
        strcat(output, s);
        token = strtok(NULL, s);
    }

    return output;
}