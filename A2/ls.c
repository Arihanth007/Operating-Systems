#include "headers.h"
#include "functions.h"

void get_permissions(char *file1)
{
    char str[11], *s = str;
    struct stat st;
    mode_t owner, group, other;

    if (stat(file1, &st) == -1)
    {
        perror("Stat: ");
        return;
    }

    if (S_ISREG(st.st_mode))
        *s++ = '-';
    else if (S_ISDIR(st.st_mode))
        *s++ = 'd';
    else if (S_ISCHR(st.st_mode))
        *s++ = 'c';
    else if (S_ISBLK(st.st_mode))
        *s++ = 'b';
    else if (S_ISFIFO(st.st_mode))
        *s++ = 'f';
    else if (S_ISLNK(st.st_mode))
        *s++ = 'l';
    else
        *s++ = 's';

    owner = st.st_mode & S_IRWXU;
    group = st.st_mode & S_IRWXG;
    other = st.st_mode & S_IRWXO;

    *s++ = owner & S_IRUSR ? 'r' : '-';
    *s++ = owner & S_IWUSR ? 'w' : '-';
    *s++ = owner & S_IXUSR ? 'x' : '-';

    *s++ = group & S_IRGRP ? 'r' : '-';
    *s++ = group & S_IWGRP ? 'w' : '-';
    *s++ = group & S_IXGRP ? 'x' : '-';

    *s++ = other & S_IROTH ? 'r' : '-';
    *s++ = other & S_IWOTH ? 'w' : '-';
    *s++ = other & S_IXOTH ? 'x' : '-';

    *s = '\0';

    printf("%s\t", str);

    return;
}

void get_info(char *file1)
{
    char a[50];
    struct stat st;
    if (stat(file1, &st) == -1)
    {
        perror("Stat");
        return;
    }
    printf("%d\t", st.st_nlink);
    printf("%lld\t", st.st_size);

    strncpy(a, ctime(&st.st_mtime), 16);
    for (int i = 4; i < 16; i++)
        printf("%c", a[i]);
    printf("\t");
}

void get_user_info(char *file1)
{
    char a[50];
    struct stat st;
    if (stat(file1, &st) == -1)
    {
        perror("Stat");
        return;
    }
    printf("%d\t", st.st_nlink);
    printf("%lld\t", st.st_size);

    strncpy(a, ctime(&st.st_mtime), 16);
    for (int i = 4; i < 16; i++)
        printf("%c", a[i]);
    printf("\t");
}

void print_ls(char *dir, int op_a, int op_l)
{
    //Here we will list the directory
    struct dirent *d;
    DIR *dh = opendir(dir);
    if (!dh)
    {
        if (errno == ENOENT)
            perror("Directory doesn't exist");
        else
            perror("Unable to read directory");
        return;
    }
    //While the next entry is not readable we will print directory files
    while ((d = readdir(dh)) != NULL)
    {
        //If hidden files are found we continue
        if (!op_a && d->d_name[0] == '.')
            continue;

        get_permissions(d->d_name);
        get_info(d->d_name);
        printf("%s\t", d->d_name);

        if (op_l)
            printf("\n");
    }
    if (!op_l)
        printf("\n");
}

void ls(char *token, char *home, char *prev)
{
    char args[100][sz], flags[5], directories[sz];
    int t = 0, f = 0, d = 0, isL = 0, isA = 0;
    token = strtok(NULL, " ");
    while (token != NULL)
    {
        strcpy(args[t++], token);
        token = strtok(NULL, " ");
    }

    for (int i = 0; i < t; i++)
    {
        if (args[i][0] == '-')
        {
            flags[f++] = args[i][1];
            if (strlen(args[i]) > 2)
                flags[f++] = args[i][2];
        }
        else
            strcpy(&directories[d++], args[i]);
    }
    if (d == 0)
        strcpy(&directories[d++], ".");

    for (int i = 0; i < f; i++)
    {
        if (flags[i] == 'a')
            isA = 1;
        else if (flags[i] == 'l')
            isL = 1;
    }

    for (int i = 0; i < d; i++)
        print_ls(&directories[i], isA, isL);
}
