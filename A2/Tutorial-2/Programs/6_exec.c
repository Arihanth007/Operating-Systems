#include <stdio.h>
#include <unistd.h>

int main()
{
  // A null terminated array of character pointers
  char *args[] = {"cat", "6_exec.c", NULL};
  execvp("cat", args);

  /*
    All statements are ignored after execvp() call as this whole  
    process is replaced by another process (ls) 
  */
  printf("Ending-----");

  return 0;
}