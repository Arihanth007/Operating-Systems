#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int EXIT_FAILURE = 1, EXIT_SUCCESS = 0;
    // check for correct command from user
    if (argc <= 2 || (argv[1][0] < '0' || argv[1][0] > '9'))
    {
        printf("strace usage: strace <mask(int[0,9])> <command[args]>\n");
        exit(EXIT_FAILURE);
    }

    strace(atoi(argv[1]));

    // copy the command to a new array
    char *new_args[MAXARG];
    for (int i = 0; i < argc && i < MAXARG; i++)
        new_args[i] = argv[i + 2];

    exec(new_args[0], new_args);
    exit(EXIT_SUCCESS);
}
