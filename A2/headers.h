#ifndef HEADERS
#define HEADERS

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdint.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <signal.h>

#define sz 1024
#define pid_sz 1000000
// #define intro "\n\
// ███████╗██╗      █████╗ ███████╗██╗  ██╗    \n\
// ██╔════╝██║     ██╔══██╗██╔════╝██║  ██║    \n\
// █████╗  ██║     ███████║███████╗███████║    \n\
// ██╔══╝  ██║     ██╔══██║╚════██║██╔══██║    \n\
// ██║     ███████╗██║  ██║███████║██║  ██║    \n\
// ╚═╝     ╚══════╝╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝    \n\
// Functional Limited Applicability SHell\n"
#define intro "\n\
 █████╗ ███████╗██╗  ██╗    \n\
██╔══██╗██╔════╝██║  ██║    \n\
███████║███████╗███████║    \n\
██╔══██║╚════██║██╔══██║    \n\
██║  ██║███████║██║  ██║    \n\
╚═╝  ╚═╝╚══════╝╚═╝  ╚═╝    \n\
Arihanth SHell\n"

#endif