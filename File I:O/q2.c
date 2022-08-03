#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsizeof-pointer-memaccess"

// Reversal of string
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

// Since the arguements are single digit
// Hardcoded the cases
int char_to_int(char c)
{
    int n;
    switch (c)
    {
    case '1':
        n = 1;
        break;
    case '2':
        n = 2;
        break;
    case '3':
        n = 3;
        break;
    case '4':
        n = 4;
        break;
    case '5':
        n = 5;
        break;
    case '6':
        n = 6;
        break;
    case '7':
        n = 7;
        break;
    case '8':
        n = 8;
        break;
    case '9':
        n = 9;
        break;
    default:
        n = 0;
        break;
    }
    return n;
}

// Prints the progress percentage
// upto 2 decimal points
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

// Takes three argument:
// Input file name, no.of divisions, which division
int main(int argc, char *argv[])
{
    // Chunks are read in sizes of 128kb
    int fd, fd2, sz = 1024 * 128;
    char c[sz * sizeof(char)], p[100], dir_path[500] = "Assignment/2_";

    // Error handling of arguments
    if (argc < 4)
    {
        sprintf(p, "Error: Some arguments are missing.\n");
        write(1, p, strlen(p));
        return 0;
    }

    // Handling creation of directory
    const char *f1 = argv[1];
    strcat(dir_path, f1);
    mkdir("Assignment", 0777);

    // Opens read file
    fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("Open");
        return 0;
    }
    // Opens write file or creates it
    // Given user permissions
    fd2 = open(dir_path, O_WRONLY | O_CREAT, S_IRWXU);
    if (fd2 < 0)
    {
        perror("Open");
        return 0;
    }

    // For obtaining details about the read file
    struct stat st;
    if (stat(f1, &st) == -1)
    {
        perror("Stat");
        return 0;
    }

    // Handling edge cases of passed arguements
    int a = char_to_int(*argv[2]), b = char_to_int(*argv[3]);
    if (a == 0 || b == 0)
    {
        return 0;
    }

    // Declaring begining and end pointer positions
    const int chunk = sz;
    off_t tmp, lst, start, end;
    long long int div = st.st_size / a, to_read;
    start = (b - 1) * div;
    end = start + div;
    to_read = end - start;
    tmp = lseek(fd, end - chunk, SEEK_SET);

    // Dividing reads according to chunk size
    long long int readnow, isLeft = 0, read_chunks, left_read;
    read_chunks = to_read / chunk;
    left_read = to_read % chunk;

    // Reads all the chunks from the readfile
    // Writes the reversal of the string
    // Prints the percentage
    for (long long int i = 0; i < read_chunks; i++)
    {
        readnow = read(fd, c, chunk);
        c[readnow] = '\0';
        reverse_string(c, readnow);
        if (write(fd2, c, readnow) == -1 || readnow == -1)
        {
            perror("Read & Write");
            return 0;
        }
        print_progress(st.st_size, tmp, p);
        tmp = lseek(fd, -2 * readnow, SEEK_CUR);
    }

    // Covers the left over characters
    // that are smaller than the chunk size
    tmp = lseek(fd, start, SEEK_SET);
    readnow = read(fd, c, left_read);
    c[readnow] = '\0';
    reverse_string(c, readnow);
    if (write(fd2, c, readnow) == -1 || readnow == -1)
    {
        perror("Read & Write");
        return 0;
    }
    print_progress(st.st_size, 0, p);

    // Closes both files
    if (close(fd) < 0)
    {
        perror("Close");
        return 0;
    }
    if (close(fd2) < 0)
    {
        perror("Close");
        return 0;
    }

    return 0;
}