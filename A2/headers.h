#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdint.h>
#include <time.h>

#define sz 1024
#define intro "\n\
███████╗██╗      █████╗ ███████╗██╗  ██╗    \n\
██╔════╝██║     ██╔══██╗██╔════╝██║  ██║    \n\
█████╗  ██║     ███████║███████╗███████║    \n\
██╔══╝  ██║     ██╔══██║╚════██║██╔══██║    \n\
██║     ███████╗██║  ██║███████║██║  ██║    \n\
╚═╝     ╚══════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝    \n\
Functional Limited Applicability SHell\n"