all: shell clear

shell: compilation
	gcc -g main.o functions.o echo.o cd.o ls.o processes.o

compilation:
	gcc -c main.c functions.c echo.c cd.c ls.c processes.c

clear:
	rm main.o functions.o echo.o cd.o ls.o processes.o