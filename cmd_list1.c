#include "header.h"

char prev_dir[1024];

int split_command_fore(char *command, char home_dir[])
{
    // implement the fork processes
    char *token;
    char *bg_proc;
    char *instruct;
    char *saveptr;

    update_history(command, home_dir);

    token = strtok_r(command, ";", &saveptr);
    while (token != NULL)
    {
        split_command_back(token, home_dir);
        token = strtok_r(NULL, ";", &saveptr);
    }

    return 0;
}

int split_command_back(char *command, char home_dir[])
{
    // implement the fork processes
    char *token;
    char *bg;
    char *instruct;
    char *saveptr;

    // check if command is to be run in background
    if (check_substr(command, "&") != -1)
    {
        token = strtok_r(command, "&", &saveptr);

        // Due to linked list implementation, no upper limit on bg_procs
        run_backproc(token);

        if (check_substr(saveptr, "&") != -1)
            split_command_back(saveptr, home_dir);

        else
            piping(saveptr, home_dir);
    }

    // no background process
    else
        piping(command, home_dir);

    return 0;
}

int parse_command(char *command, char home_dir[])
{
    char *token;
    char *instruct;
    char *saveptr;

    token = strtok_r(command, " \t\r\n", &saveptr);
    instruct = token;

    if (token == NULL)
        return 0;

    char **argv = (char **)malloc(1024 * sizeof(char *));

    int i = 0;
    while (token != NULL)
    {
        argv[i] = token;
        token = strtok_r(NULL, " \t\r\n", &saveptr);

        i++;
    }

    if (strcmp(instruct, "exit") == 0)
    {
        kill_processes();
        exit(0);
    }

    else if (strcmp(instruct, "pwd") == 0)
    {
        char curr_dir[1024];
        getcwd(curr_dir, sizeof(curr_dir));
        printf("%s\n", curr_dir);
    }

    else if (strcmp(instruct, "cd") == 0)
        cd_cmd(i, argv, home_dir);

    else if (strcmp(instruct, "echo") == 0)
        echo_cmd(i, argv);

    else if (strcmp(instruct, "ls") == 0)
        ls_flags(i, argv, home_dir);

    else if (strcmp(instruct, "pinfo") == 0)
        pinfo_cmd(i, argv);

    else if (strcmp(instruct, "discover") == 0)
        discover_flags(i, argv, home_dir);

    else if (strcmp(instruct, "history") == 0)
        history_cmd(i, argv);

    else if (strcmp(instruct, "help") == 0)
        help_cmd(i, argv);

    else if (strcmp(instruct, "jobs") == 0)
        jobs_flags(i, argv);

    else if (strcmp(instruct, "sig") == 0)
        sig_cmd(i, argv);

    else if (strcmp(instruct, "fg") == 0)
        fg_cmd(i, argv);

    else if (strcmp(instruct, "bg") == 0)
        bg_cmd(i, argv);

    else
        run_foreproc(command, instruct, i, argv);

    free(argv);
    return 0;
}

int echo_cmd(int argc, char **argv)
{
    for (int i = 1; i < argc; i++)
        printf("%s ", remove_str(argv[i], "\"", 0));

    printf("\n");
    return 0;
}

int cd_cmd(int argc, char **argv, char home_dir[])
{
    char curr_dir[1024];
    getcwd(curr_dir, sizeof(curr_dir));

    if (argc == 1)
    {
        chdir(home_dir);
        strcpy(prev_dir, curr_dir);

        return 0;
    }

    else if (argc == 2)
    {
        // special args
        if (strcmp(argv[1], "-") == 0)
        {
            if (strcmp(prev_dir, "") == 0)
            {
                printf("-bash: cd: OLDPWD not set\n");
                return 0;
            }

            else
            {
                printf("%s\n", prev_dir);
                chdir(prev_dir);

                strcpy(prev_dir, curr_dir);
            }
        }

        else if (strcmp(argv[1], "~") == 0)
        {
            chdir(home_dir);
            strcpy(prev_dir, curr_dir);

            return 0;
        }

        else if (strcmp(argv[1], "..") == 0)
        {
            strcpy(prev_dir, curr_dir);
            chdir("..");
        }

        else if (check_substr(argv[1], "~/") == 0)
        {
            argv[1] = remove_str(argv[1], "~/", 1);

            char *file_path = malloc(strlen(home_dir) + strlen(argv[1]) + 2);
            strcpy(file_path, home_dir);
            strcat(file_path, "/");
            strcat(file_path, argv[1]);

            if (chdir(file_path) == -1)
            {
                printf("%s %s\n", home_dir, file_path);
                printf("-bash: cd: %s: No such file or directory\n", file_path);
                chdir(curr_dir);
                return 0;
            }

            strcpy(prev_dir, curr_dir);
        }

        else
        {
            if (chdir(argv[1]) == -1)
                printf("-bash: cd: %s: No such file or directory\n", argv[1]);

            else
                strcpy(prev_dir, curr_dir);
        }
    }

    else
    {
        printf("-bash: cd: too many arguments\n");
        return 0;
    }
}

int help_cmd()
{
    puts("\nGNU bash, version 3.2 (x86_64-pc-wsl2-linux-gnu)"
         "\nCopyright @Rutts07\n"
         "\nThese shell commands are defined internally. Type `help` to see this list.\n"
         "\n pwd"
         "\n echo [arg ...]"
         "\n cd [.] [~] [..] [-] [arg ...]"
         "\n ls [.] [~] [..] [- a | l | al | la] [arg ...]"
         "\n foreproc [.] [~] [..] [arg ...] using execvp"
         "\n backproc [.] [~] [..] [arg ...] using execvp"
         "\n pinfo [pid]"
         "\n jobs"
         "\n fg"
         "\n bg"
         "\n sig [pid] [sig_num]"
         "\n discover [.] [~] [..] [- d | f | df | fd] ['filename']"
         "\n help"
         "\n history [-c]"
         "\n clear using execvp"
         "\n exit"

         "\n"
         "\nThis shell supports I/O redirection and piping."
         "\nThis shell supports Ctrl+C and Ctrl+Z signals."
         "\nThis shell supports autosuggestion (Ctrl+N) and autocompletion (Tab)."
         "\n\nUse this shell at your own risk\n");

    return 0;
}

int run_foreproc(char *command, char *instruct, int argc, char **argv)
{
    int status;
    time_t start = time(NULL);

    fg_proc_name = strdup(command);

    // fork a child process
    int pid = fork();

    if (pid == 0)
    {
        argv[argc] = NULL;

        if (execvp(instruct, argv) < 0)
        {
            printf("-bash: %s: command not found\n", command);
            exit(EXIT_FAILURE);
        }
    }

    else if (pid < 0)
        printf("-bash : fork failed\n");

    else
    {
        fg_pid = pid;
        fg_proc = strdup(instruct);

        waitpid(pid, &status, WUNTRACED);

        fg_pid = -1;
        strcpy(fg_proc, "");
        strcpy(fg_proc_name, "");
    }

    time_t end = time(NULL);
    time_taken += ((double)(end - start));

    return 0;
}

int run_backproc(char *command)
{
    char *token;
    char *instruct;
    char *saveptr;

    char *cmd = strdup(command);

    token = strtok_r(command, " \t\r\n", &saveptr);
    instruct = token;

    if (token == NULL)
        return 0;

    char **argv = (char **)malloc(1024 * sizeof(char *));

    int i = 0;
    while (token != NULL)
    {
        argv[i] = token;
        token = strtok_r(NULL, " \t\r\n", &saveptr);

        i++;
    }

    if (strcmp(instruct, "exit") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    else if (strcmp(instruct, "pwd") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    else if (strcmp(instruct, "clear") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    else if (strcmp(instruct, "cd") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    else if (strcmp(instruct, "echo") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    else if (strcmp(instruct, "ls") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    else if (strcmp(instruct, "pinfo") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    else if (strcmp(instruct, "discover") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    else if (strcmp(instruct, "history") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    else if (strcmp(instruct, "help") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    else if (strcmp(instruct, "jobs") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    else if (strcmp(instruct, "sig") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    else if (strcmp(instruct, "fg") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    else if (strcmp(instruct, "bg") == 0)
    {
        printf("-bash: %s: cannot run in background\n", instruct);
        return 0;
    }

    int status;
    pid_t pid;

    pid = fork();
    if (pid == 0)
    {
        setpgid(0, 0);

        argv[i] = NULL;
        if (execvp(instruct, argv) < 0)
        {
            printf("-bash: %s: command not found\n", instruct);
            exit(0);
        }

        exit(0);
    }

    else if (pid < 0)
        printf("-bash: fork failed\n");

    else
    {
        int pindex = add_bg_proc(pid, instruct, cmd);
        printf("[%d] %d\n", pindex, pid);
    }

    return 0;
}

// handle Ctrl + C
void handle_sigint(int sig)
{
    disableRawMode();

    if (fg_pid != -1)
    {
        kill((pid_t)fg_pid, SIGINT);
        // kill((pid_t) fg_pid, SIGKILL);
        printf("\n-bash: ^C: %d: %s terminated\n", fg_pid, fg_proc);

        fg_pid = -1;
        strcpy(fg_proc, "");
    }

    return;
}

// handle Ctrl + Z
void handle_sigtstp(int sig)
{
    disableRawMode();

    // suspend the foreground process
    if (fg_pid != -1)
    {
        add_bg_proc(fg_pid, fg_proc, fg_proc_name);

        kill((pid_t)fg_pid, SIGTSTP);
        printf("-bash: ^Z: %d: %s suspended\n", fg_pid, fg_proc);

        fg_pid = -1;
        strcpy(fg_proc, "");
    }

    return;
}

// handle Ctrl + D
void handle_exit()
{
    // kill all runninng background processes
    kill_processes();

    printf("\n-bash: ^D: terminated\n");

    exit(0);
}

int io_redirect(char *command, char home_dir[])
{
    int std_in = dup(STDIN_FILENO);
    int std_out = dup(STDOUT_FILENO);

    if (check_substr(command, "<") == -1 && check_substr(command, ">") == -1 && check_substr(command, ">>") == -1)
        parse_command(command, home_dir);

    char *token;
    char *saveptr;

    // check for Output Redirection
    if (check_substr(command, ">>") != -1)
    {
        token = strtok_r(command, ">", &saveptr);
        saveptr = strtok_r(NULL, ">", &saveptr);

        if (saveptr == NULL)
        {
            printf("-bash: \">>\": No output file specified\n");
            return 0;
        }

        // change stdout to file
        char *file_name = remove_str(saveptr, " ", 0);

        int f_out = open(file_name, O_WRONLY | O_APPEND | O_CREAT, 0644);
        if (dup2(f_out, STDOUT_FILENO) < 0)
        {
            printf("-bash: %s: No such file or directory\n", file_name);
            dup2(std_out, STDOUT_FILENO);
            return 0;
        }
        close(f_out);

        io_redirect(token, home_dir);
    }

    else if (check_substr(command, ">") != -1)
    {
        token = strtok_r(command, ">", &saveptr);

        if (saveptr == NULL)
        {
            printf("-bash: \">\": No output file specified\n");
            return 0;
        }

        // change stdout to file
        char *file_name = remove_str(saveptr, " ", 0);

        int f_out = open(file_name, O_WRONLY | O_TRUNC | O_CREAT, 0644);
        if (dup2(f_out, STDOUT_FILENO) < 0)
        {
            printf("-bash: %s: No such file or directory\n", file_name);
            dup2(std_out, STDOUT_FILENO);
            return 0;
        }
        close(f_out);

        io_redirect(token, home_dir);
    }

    // check for Input Redirection
    else if (check_substr(command, "<") != -1)
    {
        token = strtok_r(command, "<", &saveptr);

        if (saveptr == NULL)
        {
            printf("-bash: \"<\": No input file specified\n");
            return 0;
        }

        // change stdin to file
        char *file_name = remove_str(saveptr, " ", 0);

        int f_in = open(file_name, O_RDONLY);
        if (dup2(f_in, STDIN_FILENO) < 0)
        {
            printf("-bash: %s: No such file or directory\n", file_name);
            dup2(std_in, STDIN_FILENO);
            return 0;
        }
        close(f_in);

        io_redirect(token, home_dir);
    }

    dup2(std_in, STDIN_FILENO);
    dup2(std_out, STDOUT_FILENO);

    return 0;
}

int piping(char *command, char home_dir[])
{
    // printf("%d %d\n", STDIN_FILENO, STDOUT_FILENO);

    int std_in = dup(STDIN_FILENO);
    int std_out = dup(STDOUT_FILENO);

    if (check_substr(command, "|") == -1)
    {
        io_redirect(command, home_dir);

        dup2(std_in, STDIN_FILENO);
        dup2(std_out, STDOUT_FILENO);

        return 0;
    }

    // calculate number of pipes
    int num_pipes = 0;
    for (int i = 0; i < strlen(command); i++)
    {
        if (command[i] == '|')
            num_pipes++;
    }

    char *token;
    char *saveptr;

    token = strtok_r(command, "|", &saveptr);
    if (saveptr == NULL)
    {
        printf("-bash: \"|\": No command specified\n");
        return 0;
    }

    else
    {
        int **fd = (int **)malloc(num_pipes * sizeof(int *));
        for (int i = 0; i < num_pipes; i++)
            fd[i] = (int *)malloc(2 * sizeof(int));

        int read, write;

        int i = 0;
        while (token != NULL)
        {
            if (i < num_pipes)
            {
                if (pipe(fd[i]) < 0)
                {
                    printf("-bash: pipe: failed to create pipe\n");
                    return 0;
                }
            }

            if (i == 0)
                // read from STDIN
                read = std_in;

            else
                // read from previous pipe
                read = fd[i - 1][0];

            if (i == num_pipes)
                break;

            else
                // write to current pipe
                write = fd[i][1];

            fork_pipes(read, write, std_in, std_out, token, home_dir);
            close(write);

            token = strtok_r(NULL, "|", &saveptr);
            i++;
        }

        dup2(std_out, STDOUT_FILENO);

        if (token != NULL)
        {
            dup2(read, STDIN_FILENO);
            io_redirect(token, home_dir);
        }

        else
            printf("-bash: \"|\": No command specified\n");

        // free the pipes
        for (int i = 0; i < num_pipes; i++)
            free(fd[i]);

        free(fd);
    }

    dup2(std_in, STDIN_FILENO);
    dup2(std_out, STDOUT_FILENO);

    return 0;
}

int fork_pipes(int read, int write, int std_in, int std_out, char *command, char home_dir[])
{
    int pid = fork();

    if (pid == 0)
    {
        if (dup2(read, STDIN_FILENO) < 0)
        {
            printf("-bash: %s: pipe read end failed\n", command);
            dup2(std_in, STDIN_FILENO);
            return 0;
        }
        close(read);

        if (dup2(write, STDOUT_FILENO) < 0)
        {
            printf("-bash: %s: pipe write end failed\n", command);
            dup2(std_out, STDOUT_FILENO);
            return 0;
        }
        close(write);

        io_redirect(command, home_dir);

        // To solve ioctl error
        dup2(std_in, STDIN_FILENO);
        dup2(std_out, STDOUT_FILENO);

        exit(0);
    }

    return pid;
}
