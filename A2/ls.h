#ifndef LS
#define LS

void ls(char *token, char *home);
void print_ls(char *dir, int op_a, int op_l);
void get_permissions(char *file1);
void get_info(char *file1);
void total_blocks(char *dir, int isA);
int get_blocks(char *file1);
int isDirectory(const char *path);

#endif