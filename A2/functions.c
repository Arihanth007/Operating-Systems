#include "functions.h"
#include "headers.h"

char *get_input()
{
    size_t size = 1024; // initial size of char array
    char *string = malloc(size);
    char **string_pointer = &string; // double pointer to char array
    size_t characters = getline(string_pointer, &size, stdin);

    char *output = malloc(characters + 5), *temp = malloc(characters + 5);
    temp = clear_tabs(string, temp);
    output = clear_spaces(temp, output);

    free(temp);
    free(string);
    return output;
}

char *clear_spaces(char *string, char *output)
{
    const char s[2] = " ", *token;
    token = strtok(string, s);

    /* walk through other tokens */
    while (token != NULL)
    {
        strcat(output, token);
        strcat(output, s);
        token = strtok(NULL, s);
    }

    return output;
}

char *clear_tabs(char *string, char *output)
{
    const char s[2] = "\t", *token;
    token = strtok(string, s);

    /* walk through other tokens */
    while (token != NULL)
    {
        strcat(output, token);
        strcat(output, " ");
        token = strtok(NULL, s);
    }

    return output;
}

void print(char *str)
{
    char p[1024];
    sprintf(p, "%s", str);
    printf("%s", p);
    return;
}