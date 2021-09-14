#include "headers.h"
#define sz 1024

void pwd()
{
    char cwd[sz];
    getcwd(cwd, sz);
    printf("%s\n", cwd);
}