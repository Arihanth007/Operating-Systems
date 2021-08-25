#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
// #include <time.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-pointer-memaccess"

// clock_t start, end;
// double cpu_time_used;

void reverse_string(char *c, int n)
{
    for (int i = 0; i < n / 2; i++)
    {
        char temp = c[i];
        c[i] = c[n - i - 1];
        c[n - i - 1] = temp;
        // swap(c[i], c[n - i - 1]);
    }
}

void print_progress(long long int total_char, long long int cur_char, char *p)
{
    if (cur_char > total_char)
        cur_char = 0;
    long double progress = 100.0 * (total_char - cur_char) / total_char;
    memset(p, 0, sizeof(p));
    sprintf(p, "\r%.2Lf%%", progress);
    fflush(stdout);
    write(1, p, sizeof p);
    return;
}

int main(int argc, char *argv[])
{
    // start = clock();
    int fd, fd2, sz = 1024 * 128; // 128kb chunks
    const char *f1 = argv[1];
    // char *c = (char *)calloc(sz, sizeof(char)), *p = (char *)calloc(100, sizeof(char));
    char c[sz * sizeof(char)], p[100];
    char output_file[500] = "1_", dir_path[500] = "Assignment/";
    strcat(output_file, f1);
    strcat(dir_path, output_file);

    mkdir("Assignment", 0777);
    // mkdir("Assignment", 0700);

    fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("Program"); // print program detail "Success or failure"
        exit(1);
    }
    fd2 = open(dir_path, O_WRONLY | O_CREAT, S_IRWXU);
    if (fd2 < 0)
    {
        perror("Program"); // print program detail "Success or failure"
        exit(1);
    }

    struct stat st;
    if (stat(f1, &st) == -1)
    {
        perror("stat");
        exit(EXIT_FAILURE);
    }
    // printf("\nSize of file = %lld\n", st.st_size);

    // memset(c, 0, sizeof(c));
    fflush(stdin);
    // const int chunk = 128;
    const int chunk = sz;
    off_t tmp = lseek(fd, -chunk, SEEK_END), lst;
    // printf("Location from the end is %lld\n", tmp);
    int cnt = 1, readnow, isLeft = 0;
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
    // while (readnow > 0 && tmp > 0)
    while (tmp > 0)
    {
        readnow = read(fd, c, chunk);
        // printf("readnow = %d\n", readnow);
        c[readnow] = '\0';
        reverse_string(c, readnow);
        // printf("%s", c);
        write(fd2, c, readnow);
        // memset(c, 0, sizeof(c));
        fflush(stdin);
        print_progress(st.st_size, tmp, p);
        tmp = lseek(fd, -2 * readnow, SEEK_CUR);
        // printf("Location from the end is %lld\n", tmp);
        if (tmp <= chunk && tmp >= 0)
        {
            lst = tmp;
            isLeft = 1;
        }
        cnt++;
    }

    if (isLeft)
    {
        if (lst == 0)
            lst = chunk;
        print_progress(st.st_size, lst, p);
        lseek(fd, 0, SEEK_SET);
        // memset(c, 0, sz * sizeof(char));
        fflush(stdin);
        readnow = read(fd, c, lst);
        reverse_string(c, readnow);
        // printf("%s", c);
        write(fd2, c, readnow);
        // printf("\n%lld\n", lst);
        cnt++;
    }

    // printf("\n\nNumber of chunks read = %d\n", cnt - 1);
    print_progress(st.st_size, 0, p);
    // free(c);

    if (close(fd) < 0)
    {
        perror("c1");
        exit(1);
    }
    if (close(fd2) < 0)
    {
        perror("c2");
        exit(1);
    }

    // end = clock();
    // cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    // printf("\n%f\n", cpu_time_used);
    return 0;
}