#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

// Clears the output buffer
void clear_string(char *p)
{
    for (int i = 0; i < strlen(p); i++)
        p[i] = 0;
}

// Main logic of checking permissons
void print_permissions(char *file_name, char *p)
{
    char output[2][5] = {"No", "Yes"};
    struct stat st;

    if (stat(file_name, &st))
    {
        perror("Stat");
        return;
    }

    // User permissions
    sprintf(p, "User has read permission on %s: %s\n", file_name, output[(S_IRUSR & st.st_mode) != 0]);
    write(1, p, strlen(p));
    sprintf(p, "User has read permission on %s: %s\n", file_name, output[(S_IWUSR & st.st_mode) != 0]);
    write(1, p, strlen(p));
    sprintf(p, "User has read permission on %s: %s\n\n", file_name, output[(S_IXUSR & st.st_mode) != 0]);
    write(1, p, strlen(p));

    // Group permissions
    sprintf(p, "Group has read permission on %s: %s\n", file_name, output[(S_IRGRP & st.st_mode) != 0]);
    write(1, p, strlen(p));
    sprintf(p, "Group has read permission on %s: %s\n", file_name, output[(S_IWGRP & st.st_mode) != 0]);
    write(1, p, strlen(p));
    sprintf(p, "Group has read permission on %s: %s\n\n", file_name, output[(S_IXGRP & st.st_mode) != 0]);
    write(1, p, strlen(p));

    // Others permissions
    sprintf(p, "Others has read permission on %s: %s\n", file_name, output[(S_IROTH & st.st_mode) != 0]);
    write(1, p, strlen(p));
    sprintf(p, "Others has read permission on %s: %s\n", file_name, output[(S_IWOTH & st.st_mode) != 0]);
    write(1, p, strlen(p));
    sprintf(p, "Others has read permission on %s: %s\n", file_name, output[(S_IXOTH & st.st_mode) != 0]);
    write(1, p, strlen(p));
}

int main(int argc, char *argv[])
{
    char p[500], dir_path1[100] = "Assignment/1_", dir_path2[100] = "Assignment/2_";

    // Error handling of arguments
    if (argc < 2)
    {
        sprintf(p, "Error: Some arguments are missing.\n");
        write(1, p, strlen(p));
        return 0;
    }

    int fd;
    const char *f1 = argv[1];
    char directory[50] = "Assignment", output[2][5] = {"No", "Yes"};
    strcat(dir_path1, f1);
    strcat(dir_path2, f1);

    struct stat dir;
    if (stat(directory, &dir))
    {
        perror("Stat");
        return 0;
    }

    // Checking for parent directory
    sprintf(p, "Directory is created: %s\n\n", output[S_ISDIR(dir.st_mode)]);
    write(1, p, strlen(p));

    // First file permissions
    print_permissions(dir_path1, p);

    sprintf(p, "\n");
    write(1, p, strlen(p));

    // Second file permissions
    print_permissions(dir_path2, p);

    return 0;
}