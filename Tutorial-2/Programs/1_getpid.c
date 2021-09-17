#include <stdio.h>
#include <unistd.h>

int main()
{
    int pid;
    pid = getpid();
    printf("My process ID is %d\n", pid);

    // PID remains the same through the process
    for (int i = 0; i < 1000; ++i)
    {
       printf("%d\n", getpid());
    }
    return 0;
}
