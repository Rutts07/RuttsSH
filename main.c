#include "header.h"

int time_taken;
bg_procs *head = NULL;
int fg_pid = -1;
char *fg_proc = NULL;
char *fg_proc_name = NULL;

int main()
{
    // Initializing bg_procs list
    head = init_bg_procs();

    char home_dir[1024], user_name[1024], system_name[1024], curr_dir[1024];
    int terminal_width = 0;
    int len = 0;

    getcwd(home_dir, sizeof(home_dir));
    strcpy(prev_dir, "");

    struct passwd *pw = getpwuid(getuid());
    if (!pw)
    {
        printf("Error getting user name\n");
        exit(1);
    }

    load_history(home_dir);

    strcpy(user_name, pw->pw_name);
    gethostname(system_name, sizeof(system_name));

    char *buffer = (char *)malloc(sizeof(char) * 1024);
    char *time_buffer = (char *)malloc(sizeof(char) * 1024);
    command = (char *)malloc(1024 * sizeof(char));

    ssize_t size;

    split_command_fore(" clear", home_dir);

    signal(SIGINT, handle_sigint);
    signal(SIGTSTP, handle_sigtstp);

    while (true)
    {
        getcwd(curr_dir, sizeof(curr_dir));
        print_finished_bg_procs();

        if (time_taken >= 1)
        {
            // update time for foreground processes
            sprintf(buffer, " (took %ds)", time_taken);
            time_buffer = strdup(buffer);
        }

        else
            time_buffer = strdup("");

        if (check_substr(curr_dir, home_dir) != -1)
            sprintf(buffer, "\033[0;35m%s@%s\033[0;37m:\033[0;36m~%s\033[33m%s\033[39m$ ", user_name, system_name, remove_str(curr_dir, home_dir, 1), time_buffer);

        else
            sprintf(buffer, "\033[0;35m%s@%s\033[0;37m:\033[0;36m%s\033[33m%s\033[39m$ ", user_name, system_name, curr_dir, time_buffer);

        printf("%s", buffer);

        terminal_width = strlen(buffer);
        time_taken = 0;

        int res = get_input(&size, terminal_width, home_dir);
        if (res == 0)
            split_command_fore(command, home_dir);
    }

    return 0;
}
