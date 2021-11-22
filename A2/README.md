# Shell Implementation

## Commands Implemented

- `echo.c` and `echo.h` implement `echo` command.

- `cd.c` and `cd.h` implement `cd` command with `.`, `..`, `-` and `~` flags.

- `ls.c` and `ls.h` implement `ls` command with `-l` and `-a` flags.

- `pinfo.c` and `pinfo.h` implement `pinfo` command that can take `pid` as argument. Pinfo makes use of files present in `/proc/<pid>` that give information about the process.

- `processes.c` and `processes.h` implements system commands that aren't mentioned as requirement which can run either in the foreground or background. `Jobs`, `sig`, `fg`, `bg` and `signal handling` are also handled here. Jobs lists all the running and suspeded processes that were spawned by the shell. Sig is used to send signals to the processes via the `kill` command. Fg and Bg manipulate spawned processes from foreground and background.

- `repeat`, `redirection`, `piping`, `replay` and handling logic is implemented in `main.c`. Piping and redirection is implemented using hidden files. The output of one command is saved in these files and used as the input to the next command. While it is slower and has some security concerns (any process can access these files).

- `history` that can also take the number of commands to be displayed as an argument is implemented in `functions.c`, `functions.h`.

- `functions.c` and `functions.h` also implements tokenizers, getting input and other essential functions. The tokenizer replaces multiple tabs and spaces by a single space. `;`, `>`, `<`, `>>`, `<<` and some other special characters have their special meanings in tokenisation.

- `makefile` is used to compile the program by running `make` followed by `./a.out`.

## Assumptions/Behaviour

- We have permission to create and read files.

- All testing apart from `pinfo` and other commands that require special files was done on `MacOS`.

- `ls -l` displays the block sizes correctly on `Mac` but will be different on `Linux` (since I was primarly working and testing on MacOS, decided to not change the code for Linux).

- Taken maximum number of commands separated by `;` to be 100, maximum number of arguments of a command to be 100, size of each argument and command doesn't cross 1024 characters and maximum pid value of a process is $10^6$.

- Assumed a maximum of 100 processes that can be run in foreground or background.

## Functioning of some important commands

- When a process is brought from background to foreground or vice verse it is considered as a new process and given a new job ID (PID remains the same).

## Running the shell

`make && ./a.out`.

Works on make version of 4.3 and gcc is 10.3.0 on debian.
