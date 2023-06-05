#ifndef HEADER_H
#define HEADER_H

// Standard C headers
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

// Header for system calls
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <errno.h>
#include <signal.h>

// Variable to store command
extern char *command;

// Header for line-buffered input
#include <termios.h>

// Variable to store foreground process pid
extern int fg_pid;
extern char *fg_proc;
extern char *fg_proc_name;

// Variables to store history
#define MAX_HISTORY_SIZE 20
extern char *history[MAX_HISTORY_SIZE];
extern int history_size;
extern int history_index;

// Doubly linked list to store the background processes
typedef struct BG_Procs
{
    int pid;
    int pindex;
    char *name;
    char *cmd_name;
    struct BG_Procs *next;
    struct BG_Procs *prev;
} bg_procs;
extern bg_procs *head;

bg_procs *init_bg_procs();
int add_bg_proc(int pid, char *process_name, char *command_name);
int rem_bg_proc(int pid);
int print_finished_bg_procs();
int kill_processes();

// Variable for cd - command
extern char prev_dir[1024];

// Variable to store time taken for foreground processes
extern int time_taken;

// functions to remove home_dir from absolute path
char *remove_str(char str[], char substr[], int flag);
int check_substr(char str[], char substr[]);
int check_lastoc(char str[], char substr[]);
char *get_last_token(char *inp);

// functions to sort the files
int alphasort_case_insensitive(const struct dirent **a, const struct dirent **b);

// Functions for I/O redirection and Piping
int io_redirect(char *command, char home_dir[]);
int piping(char *command, char home_dir[]);
int fork_pipes(int read, int write, int std_in, int std_out, char *command, char home_dir[]);

// Command parse functions
int parse_command(char *command, char home_dir[]);

// Common C functions
int echo_cmd(int argc, char **argv);
int cd_cmd(int argc, char **argv, char home_dir[]);
int help_cmd();

// ls functions
int ls_flags(int argc, char **argv, char home_dir[]);
int ls_cmd_dir(int flags, char *directory);
int ls_cmd_file(int flags, char *file_name);

// Background process functions
int split_command_back(char *command, char home_dir[]);
int run_backproc(char *command);

// Foreground process functions
int split_command_fore(char *command, char home_dir[]);
int run_foreproc(char *command, char *instruct, int argc, char **argv);

// pinfo function
int pinfo_cmd(int argc, char **argv);

// Discover command
int discover_flags(int argc, char **argv, char home_dir[]);
int discover_cmd(int flags, char *dir, char *file, char home_dir[], char *rel_path);

// History functions
int history_cmd(int argc, char **argv);
int update_history(char *command, char home_dir[]);
int save_history(char home_dir[]);
int load_history(char home_dir[]);

// jobs functions
int jobs_flags(int argc, char **argv);
int jobs_bg_procs(int flags);

// sig functions
int sig_cmd(int argc, char **argv);

// fg functions
int fg_cmd(int argc, char **argv);

// bg functions
int bg_cmd(int argc, char **argv);

// Line-buffered input functions using termios - rawmode
void enableRawMode();
void disableRawMode();
int get_input(ssize_t *size, int terminal_width, char home_dir[]);

// Handle signals
void handle_sigint(int sig);    // Ctrl+C
void handle_sigtstp(int sig);   // Ctrl+Z
void handle_exit();             // Ctrl+D

// Auto completion function
char **auto_completion(char *last_word, int *num);

// Auto suggestion functions
int auto_suggestion(char *input);
int suggest(char *input);

#endif
