# Shell Implementation

## Commands Implemented

`echo`

`cd` with `.`, `..`, `-` and `~` flags

`ls` with `-l` and `-a` flags

`pinfo` that can take `pid` as argument

All other system commands can run either in the foreground or background

`repeat`

`history` that can also take the number of commands to be displayed as an argument

## Assumptions

All testing apart from `pinfo` was done on `MacOS`.

`ls -l` displays the block sizes correctly on `Mac` but will be different on `Linux`.
