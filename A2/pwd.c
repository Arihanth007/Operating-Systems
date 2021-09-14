#include "headers.h"

void pwd()
{
    char cwd[sz];
    getcwd(cwd, sz);
    printf("%s\n", cwd);
}