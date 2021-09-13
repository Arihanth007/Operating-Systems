#include <stdio.h>
#include <unistd.h>

int main()
{
	int forkReturn, i, loopsize;

	loopsize = 100;
	int a = 0;
	forkReturn = fork();

	if (forkReturn == 0)
	{
		// Child process
		for (i = 0; i < loopsize; i++)
		{
			printf("child: %d\n", ++a);
		}
	}
	else
	{
		// Parent process
		for (i = 0; i < loopsize; i++)
		{
			printf("parent: %d\n", --a);
		}
	}
	printf("%d\n", a);
	return 0;
}
