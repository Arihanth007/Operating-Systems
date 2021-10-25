#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int main(int argc, char **argv)
{
    int pid = fork(), FAIL = 1, SUCCESS = 0;

    if (pid < 0)
    {
        fprintf(2, "fork(): failed\n");
        exit(FAIL);
    }
    else if (pid == 0)
    {
        if (argc == 1)
        {
            sleep(10);
            exit(SUCCESS);
        }
        else
        {
            exec(argv[1], argv + 1);
            fprintf(2, "exec(): failed\n");
            exit(FAIL);
        }
    }
    else
    {
        int rtime, wtime;
        waitx(0, &wtime, &rtime);
        printf("\nwaiting: %d\nrunning: %d\n", wtime, rtime);
    }

    exit(SUCCESS);
}