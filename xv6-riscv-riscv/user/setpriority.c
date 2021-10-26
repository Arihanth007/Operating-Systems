#include "kernel/param.h"
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main(int argc, char *argv[])
{
    int EXIT_FAILURE = 1, EXIT_SUCCESS = 0, STDERR = 2;
    // check for correct command from user
    if (argc <= 2 || (atoi(argv[1]) < 0 || atoi(argv[1]) > 100))
    {
        fprintf(STDERR, "setpriority usage: setpriority <priority(int[0,100])> <pid(int)>\n");
        exit(EXIT_FAILURE);
    }

    setpriority(atoi(argv[1]), atoi(argv[2]));

    exit(EXIT_SUCCESS);
}