# Shell Implementation

## Commands Implemented

`echo.c` and `echo.h` implement `echo` command.

`cd.c` and `cd.h` implement `cd` command with `.`, `..`, `-` and `~` flags.

`ls.c` and `ls.h` implement `ls` command with `-l` and `-a` flags.

`pinfo.c` and `pinfo.h` implement `pinfo` command that can take `pid` as argument.

`processes.c` and `processes.h` implements all other system commands can run either in the foreground or background.

`repeat` is implemented in `main.c`.

`history` that can also take the number of commands to be displayed as an argument is implemented in `functions.c`, `functions.h`.

`functions.c` and `functions.h` also implements tokenizers, getting input and other essential functions.

`makefile` is used to compile the program by running `make` followed by `./a.out`.

## Assumptions/Behaviour

All testing apart from `pinfo` was done on `MacOS`.

`ls -l` displays the block sizes correctly on `Mac` but will be different on `Linux` (since I was primarly working and testing on MacOS, decided to not change the code for Linux).

Running a background process will lead to the shell printing `Process with pid exited normally/abnormally` even for foreground processes on MacOS but not try for Linux.
