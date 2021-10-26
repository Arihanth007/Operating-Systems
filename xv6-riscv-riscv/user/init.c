// init: The initial user-level program

#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/spinlock.h"
#include "kernel/sleeplock.h"
#include "kernel/fs.h"
#include "kernel/file.h"
#include "user/user.h"
#include "kernel/fcntl.h"

char *argv[] = {"sh", 0};

int main(void)
{
  int pid, wpid;

  if (open("console", O_RDWR) < 0)
  {
    mknod("console", CONSOLE, 0);
    open("console", O_RDWR);
  }
  dup(0); // stdout
  dup(0); // stderr

  for (;;)
  {
    printf("init: starting sh\n");

    // custom printing
    printf("\e[1;1H\e[2J");
    printf("\033[34;1m\n\
               AAA   VVVVVVVV           VVVVVVVV   66666666   \n\
              A:::A  V::::::V           V::::::V  6::::::6    \n\
             A:::::A V::::::V           V::::::V 6::::::6     \n\
            A:::::::AV::::::V           V::::::V6::::::6      \n\
           A:::::::::AV:::::V           V:::::V6::::::6       \n\
          A:::::A:::::AV:::::V         V:::::V6::::::6        \n\
         A:::::A A:::::AV:::::V       V:::::V6::::::6         \n\
        A:::::A   A:::::AV:::::V     V:::::V6::::::::66666    \n\
       A:::::A     A:::::AV:::::V   V:::::V6::::::::::::::66  \n\
      A:::::AAAAAAAAA:::::AV:::::V V:::::V 6::::::66666:::::6 \n\
     A:::::::::::::::::::::AV:::::V:::::V  6:::::6     6:::::6\n\
    A:::::AAAAAAAAAAAAA:::::AV:::::::::V   6:::::6     6:::::6\n\
   A:::::A             A:::::AV:::::::V    6::::::66666::::::6\n\
  A:::::A               A:::::AV:::::V      66:::::::::::::66 \n\
 A:::::A                 A:::::AV:::V         66:::::::::66   \n\
AAAAAAA                   AAAAAAAVVV            666666666     \n\
\033[0;0m");
    printf("\033[31;1mExtension of XV6\033[0;0m\n\n");

    pid = fork();
    if (pid < 0)
    {
      printf("init: fork failed\n");
      exit(1);
    }
    if (pid == 0)
    {
      exec("sh", argv);
      printf("init: exec sh failed\n");
      exit(1);
    }

    for (;;)
    {
      // this call to wait() returns if the shell exits,
      // or if a parentless process exits.
      wpid = wait((int *)0);
      if (wpid == pid)
      {
        // the shell exited; restart it.
        break;
      }
      else if (wpid < 0)
      {
        printf("init: wait returned an error\n");
        exit(1);
      }
      else
      {
        // it was a parentless process; do nothing.
      }
    }
  }
}
