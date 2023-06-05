#include "header.h"

bg_procs *init_bg_procs()
{
    bg_procs *init = malloc(sizeof(bg_procs));

    init->pid = 0;    // stores total number of bg_procs
    init->pindex = 0; // stores max index of current bg_proc
    init->name = "";
    init->cmd_name = "";
    init->next = NULL;
    init->prev = NULL;

    return init;
}

// adds process to bg_procs list in alphabetical order
int add_bg_proc(int pid, char *process_name, char *command_name)
{
    bg_procs *curr = head;
    curr->pid = curr->pid + 1;
    curr->pindex = curr->pindex + 1;

    bg_procs *new_proc = malloc(sizeof(bg_procs));

    new_proc->pid = pid;
    new_proc->pindex = curr->pindex;
    new_proc->name = strdup(process_name);
    new_proc->cmd_name = strdup(command_name);
    new_proc->next = NULL;
    new_proc->prev = NULL;

    if (curr->next == NULL)
    {
        // if no bg_procs exist, add the new_proc to the head
        curr->next = new_proc;
        new_proc->prev = curr;
    }

    else
    {
        curr = curr->next;
        while (curr != NULL)
        {
            // insert the new_proc lexicographically
            int res = strcmp(command_name, curr->name);

            if (res <= 0)
            {
                new_proc->next = curr;
                new_proc->prev = curr->prev;

                curr->prev->next = new_proc;
                curr->prev = new_proc;

                return head->pindex;
            }

            if (curr->next == NULL)
                break;

            curr = curr->next;
        }

        curr->next = new_proc;
        new_proc->prev = curr;
    }

    return head->pindex;
}

int rem_bg_proc(int pid)
{
    bg_procs *c_head = head;

    // if no bg_procs exist, return
    // decrement number of bg_procs
    if (c_head->pid > 0)
        c_head->pid = c_head->pid - 1;

    else
        return 0;

    int found = 0;
    int max_index = 0;

    bg_procs *curr = c_head->next;
    while (curr != NULL)
    {
        if (curr->pid == pid && found == 0)
        {
            kill(curr->pid, SIGKILL);
            curr->prev->next = curr->next;

            if (curr->next != NULL)
                curr->next->prev = curr->prev;

            free(curr);
            found = 1;
        }

        else
        {
            if (curr->pindex > max_index)
                max_index = curr->pindex;
        }

        curr = curr->next;
    }

    if (found == 0)
        printf("No process with pid %d exists\n", pid);

    c_head->pindex = max_index;

    return 0;
}

int print_finished_bg_procs()
{
    bg_procs *curr = head;

    if (curr->pid > 0)
    {
        int status;

        curr = curr->next;
        while (curr != NULL)
        {
            if (waitpid(curr->pid, &status, WUNTRACED | WNOHANG) == curr->pid)
            {
                if (WIFEXITED(status))
                {
                    printf("%s with pid = %d exited normally\n", curr->name, curr->pid);
                    rem_bg_proc(curr->pid);
                }

                else if (WIFSIGNALED(status))
                {
                    printf("%s with pid = %d was terminated by a signal\n", curr->name, curr->pid);
                    rem_bg_proc(curr->pid);
                }

                // zombie process
                else if (WIFSTOPPED(status))
                    printf("%s with pid = %d stopped\n", curr->name, curr->pid);
                // kill(curr->pid, SIGKILL);
            }

            curr = curr->next;
        }
    }

    return 0;
}

int kill_processes()
{
    if (head->pid > 0)
    {
        bg_procs *curr = head->next;
        while (curr != NULL)
        {
            kill(curr->pid, SIGKILL);
            rem_bg_proc(curr->pid);

            curr = curr->next;
        }
    }

    free(head);
    return 0;
}

// Get status of a process
int proc_stat(int pid)
{
    FILE *fp;
    char *file = (char *)malloc(1024 * sizeof(char));
    sprintf(file, "/proc/%d/stat", pid);
    fp = fopen(file, "r");

    if (fp == NULL)
    {
        // printf("-bash: pinfo: %d: No such process\n", pid);
        return 0;
    }

    else
    {
        char *token;
        char *instruct;
        char *saveptr;

        fseek(fp, 0, SEEK_END);
        int size = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char buffer[1024];

        char c;
        int i = 0;
        while ((c = fgetc(fp)) != EOF)
        {
            buffer[i] = c;
            i++;
        }

        fclose(fp);

        token = strtok_r(buffer, " ", &saveptr);

        if (token == NULL)
        {
            // printf("-bash: pinfo: %d: no such process\n", pid);
            return 0;
        }

        char **argv = (char **)malloc(1024 * sizeof(char *));

        i = 0;
        while (token != NULL)
        {
            argv[i] = token;
            token = strtok_r(NULL, " ", &saveptr);

            i++;
        }

        // running processes
        if (strcmp(argv[2], "R") == 0 || strcmp(argv[2], "S") == 0 || strcmp(argv[2], "D") == 0)
            return 1;

        // stopped processes
        else if (strcmp(argv[2], "T") == 0)
            return 2;

        else
            return 0;
    }
}

// Handle flags for jobs command
int jobs_flags(int argc, char **argv)
{
    int flags = 0;

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-r") == 0)
                flags |= 1;

            else if (strcmp(argv[i], "-s") == 0)
                flags |= 2;

            else if (strcmp(argv[i], "-rs") == 0 || strcmp(argv[i], "-sr") == 0)
                flags |= 3;

            else
            {
                printf("-bash: jobs: invalid option '%s'\n", argv[i]);
                return 0;
            }
        }

        else
        {
            printf("-bash: jobs: invalid option '%s'\n", argv[i]);
            return 0;
        }
    }

    if (flags == 0)
        flags = 3;

    jobs_bg_procs(flags);
}

// jobs command
int jobs_bg_procs(int flags)
{
    bg_procs *curr = head;
    // printf("%d\n", curr->pid);

    curr = curr->next;
    while (curr != NULL)
    {
        int status = proc_stat(curr->pid);

        if (status == 1 && (flags & 1))
            printf("[%d] Running %-10.20s [%d]\n", curr->pindex, curr->cmd_name, curr->pid);

        if (status == 2 && (flags & 2))
            printf("[%d] Stopped %-10.20s [%d]\n", curr->pindex, curr->cmd_name, curr->pid);

        curr = curr->next;
    }

    return 0;
}

int sig_cmd(int argc, char **argv)
{
    if (argc == 3)
    {
        int pindex = atoi(argv[1]);
        int signal = atoi(argv[2]);

        if (pindex > 0 && signal > 0 && signal < 32)
        {
            bg_procs *curr = head->next;
            while (curr != NULL)
            {
                if (curr->pindex == pindex)
                {
                    kill(curr->pid, signal);
                    return 0;
                }

                curr = curr->next;
            }

            printf("-bash: sig: No process with index [%d] exists\n", pindex);
        }

        else
            printf("-bash: sig: invalid arguments\n");
    }

    else if (argc < 3)
        printf("-bash: sig: too few arguments\n");

    else
        printf("-bash: sig: too many arguments\n");

    return 0;
}

int fg_cmd(int argc, char **argv)
{
    if (argc == 2)
    {
        int pindex = atoi(argv[1]);

        if (pindex > 0)
        {
            bg_procs *curr = head->next;
            while (curr != NULL)
            {
                if (curr->pindex == pindex)
                {
                    int status;
                    int pid = curr->pid;
                    time_t start = time(NULL);

                    printf("%s\n", curr->cmd_name);
                    fg_pid = pid;
                    fg_proc = strdup(curr->name);
                    fg_proc_name = strdup(curr->cmd_name);

                    if (kill(pid, SIGCONT) < 0)
                        printf("-bash: fg: Process with index [%d] could not be restarted\n", pindex);

                    waitpid(pid, &status, WUNTRACED);

                    fg_pid = -1;
                    strcpy(fg_proc, "");
                    strcpy(fg_proc_name, "");

                    if (!WIFSTOPPED(status))
                        rem_bg_proc(pid);

                    else
                        printf("%s with pid = %d stopped\n", curr->name, curr->pid);

                    time_t end = time(NULL);
                    time_taken += ((double)(end - start));

                    return 0;
                }

                curr = curr->next;
            }

            printf("-bash: fg: No process with index [%d] exists\n", pindex);
            return 0;
        }

        else
            printf("-bash: fg: invalid arguments\n");
    }

    else if (argc < 2)
        printf("-bash: fg: too few arguments\n");

    else
        printf("-bash: fg: too many arguments\n");

    return 0;
}

int bg_cmd(int argc, char **argv)
{
    if (argc == 2)
    {
        int pindex = atoi(argv[1]);

        if (pindex > 0)
        {
            bg_procs *curr = head->next;
            while (curr != NULL)
            {
                if (curr->pindex == pindex)
                {
                    int pid = curr->pid;
                    kill(pid, SIGCONT);
                    return 0;
                }

                curr = curr->next;
            }

            printf("-bash: bg: No process with index [%d] exists\n", pindex);
            return 0;
        }

        else
            printf("-bash: bg: invalid arguments\n");
    }

    else if (argc < 2)
        printf("-bash: bg: too few arguments\n");

    else
        printf("-bash: bg: too many arguments\n");

    return 0;
}
