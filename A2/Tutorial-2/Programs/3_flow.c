#include <stdio.h>
#include <unistd.h>
int main()
{

    // The child wasn't created yet. So only the parent will print this line
    printf("This is before fork\t\t%d\n\n", getpid());
    int forkReturn = fork();

    // Both child and parent run from this point after the fork.

    printf("This will be printed twice\t%d\n\n", getpid());

    if (forkReturn == 0)
    {
        printf("Only child will print this\t%d\n\n", getpid());
    }
    else
    {
        printf("Only parent will print this\t%d\n\n", getpid());
    }

    printf("Both will execute this\t\t%d\n\n", getpid());

    return 0;
}
