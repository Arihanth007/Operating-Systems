#include "headers.h"

void pwd()
{
    char cwd[sz];
    if (getcwd(cwd, sz) == NULL)
        perror("Get cwd: ");
    printf("%s\n", cwd);
}