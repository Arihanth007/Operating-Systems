#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-pointer-memaccess"

void reverse_string(char *c, int n)
{
    char temp;
    for (int i = 0; i < n / 2; i++)
    {
        temp = c[i];
        c[i] = c[n - i - 1];
        c[n - i - 1] = temp;
    }
}

void print_progress(long long int total_char, long long int cur_char, char *p)
{
    if (cur_char > total_char)
        cur_char = 0;
    long double progress = 100.0 * (total_char - cur_char) / total_char;
    sprintf(p, "\r%.2Lf%%", progress);
    fflush(stdout);
    write(1, p, sizeof p);
    return;
}

int main(int argc, char *argv[])
{
    int fd, fd2, sz = 1024 * 128; // 128kb chunks
    const char *f1 = argv[1];
    char c[sz * sizeof(char)], p[100], dir_path[500] = "Assignment/1_";
    strcat(dir_path, f1);

    mkdir("Assignment", 0777);

    fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("Open");
        exit(EXIT_FAILURE);
    }
    fd2 = open(dir_path, O_WRONLY | O_CREAT, S_IRWXU);
    if (fd2 < 0)
    {
        perror("Open");
        exit(EXIT_FAILURE);
    }

    struct stat st;
    if (stat(f1, &st) == -1)
    {
        perror("Stat");
        exit(EXIT_FAILURE);
    }

    const int chunk = sz;
    off_t tmp = lseek(fd, -chunk, SEEK_END), lst;
    int readnow, isLeft = 0;
    if (tmp <= chunk && tmp >= 0)
    {
        lst = tmp;
        isLeft = 1;
    }
    else if (tmp < 0)
    {
        lst = chunk;
        isLeft = 1;
    }

    print_progress(st.st_size, st.st_size, p);
    while (tmp > 0)
    {
        readnow = read(fd, c, chunk);
        c[readnow] = '\0';
        reverse_string(c, readnow);
        if (write(fd2, c, readnow) == -1 || readnow == -1)
        {
            perror("Read & Write");
            exit(EXIT_FAILURE);
        }
        print_progress(st.st_size, tmp, p);
        tmp = lseek(fd, -2 * readnow, SEEK_CUR);
        if (tmp <= chunk && tmp >= 0)
        {
            lst = tmp;
            isLeft = 1;
        }
    }

    if (isLeft)
    {
        if (lst == 0)
            lst = chunk;
        print_progress(st.st_size, lst, p);
        lseek(fd, 0, SEEK_SET);
        readnow = read(fd, c, lst);
        reverse_string(c, readnow);
        if (write(fd2, c, readnow) == -1 || readnow == -1)
        {
            perror("Read & Write");
            exit(EXIT_FAILURE);
        }
    }

    print_progress(st.st_size, 0, p);

    if (close(fd) < 0)
    {
        perror("Close");
        exit(EXIT_FAILURE);
    }
    if (close(fd2) < 0)
    {
        perror("Close");
        exit(EXIT_FAILURE);
    }

    return 0;
}