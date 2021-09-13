#include "headers.h"
#include "functions.h"

int main(int argc, char **argv)
{
    char *string = get_input();
    printf("You typed: %s\n", string);
    free(string);

    return 0;
}