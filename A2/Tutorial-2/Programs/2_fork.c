#include <stdio.h>
#include <unistd.h>

int main()
{

	/* 
      Return value of fork() is stored in variable forkReturn. 
      When fork creates B as a copy of A, the value of the variable forkReturn is set to zero in B. 
      Whereas its value is equal to (process ID of B) in process A. 
      These processes now run seperately with differnt process IDs.
    */
	int forkReturn = fork();
	// printf("BOTH\n");
	if (forkReturn == 0)
	{
		// forkReturn is ZERO for the child process.
		printf("\nI'm the CHILD\nMy ID is %d and value returned from fork = %d\n\n", getpid(), forkReturn);
		return 0;
	}
	else
	{
		// forkReturn is non-ZERO for the parent.
		printf("\nI'm the PARENT\nMy ID is %d and value returned from fork = %d\n", getpid(), forkReturn);
		printf("Note that the value in my variable 'forkReturn' is same as the process ID of my child\n\n");
		return 0;
	}
}
