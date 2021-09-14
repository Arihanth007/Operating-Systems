#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>

#define sz 1024
#define intro "\n\
███████╗██╗      █████╗ ███████╗██╗  ██╗    \n\
██╔════╝██║     ██╔══██╗██╔════╝██║  ██║    \n\
█████╗  ██║     ███████║███████╗███████║    \n\
██╔══╝  ██║     ██╔══██║╚════██║██╔══██║    \n\
██║     ███████╗██║  ██║███████║██║  ██║    \n\
╚═╝     ╚══════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝    \n\
Functional Limited Applicability SHell\n"