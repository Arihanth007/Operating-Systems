#include "headers.h"

int main()
{
    char *inp_cmd = malloc(sz);
    size_t inp_len = 0, line_read;
    line_read = getline(&inp_cmd, &inp_len, stdin);

    printf("LINE: %s\n", inp_cmd);
    free(inp_cmd);

    return 0;
}