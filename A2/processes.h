#ifndef PROCESSES
#define PROCESSES

void process(char a[][sz], int t, char *home, char *prev);
void exit_bg_process();
void execute_bg(char *args[]);
void execute_fg(char *args[]);
void jobs(char a[][sz], int t);
void sig(char a[][sz], int t);
void run_bg(char a[][sz], int t);
void run_fg(char a[][sz], int t);
void terminate_fg(int num);
void send_fg_bg(int num);

#endif