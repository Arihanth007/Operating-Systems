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
    write(1, p, strlen(p));
    return;
}

int main(int argc, char *argv[])
{
    int fd, fd2, sz = 1024 * 128; // 128kb chunks
    char c[sz * sizeof(char)], p[100], dir_path[500] = "Assignment/1_";
    if (argc < 2)
    {
        sprintf(p, "Error: Some arguments are missing.\n");
        write(1, p, strlen(p));
        exit(1);
    }

    const char *f1 = argv[1];
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
    off_t tmp, lst, start, end;
    long long int div = st.st_size, to_read;
    start = 0;
    end = st.st_size;
    to_read = end - start;
    tmp = lseek(fd, end - chunk, SEEK_SET);

    // printf("Char in file = %lld\n", st.st_size);
    // printf("Start = %lld, End = %lld\n", start, end);
    // printf("tmp = %lld\n", tmp);

    int readnow, isLeft = 0, read_chunks, left_read;
    read_chunks = to_read / chunk;
    left_read = to_read % chunk;

    for (long long int i = 0; i < read_chunks; i++)
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
    }

    tmp = lseek(fd, start, SEEK_SET);
    readnow = read(fd, c, left_read);
    c[readnow] = '\0';
    reverse_string(c, readnow);
    if (write(fd2, c, readnow) == -1 || readnow == -1)
    {
        perror("Read & Write");
        exit(EXIT_FAILURE);
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