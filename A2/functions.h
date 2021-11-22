#ifndef FUNCTIONS
#define FUNCTIONS
#define sz 1024

void get_input(char *string);
void query_history(char a[][sz], int t);
void my_tokenizer(char *string, char *output);
void print(char *str);
int min(int a, int b);
int max(int a, int b);
int my_compare(const void *a, const void *b);

#endif