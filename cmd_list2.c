#include "header.h"

int ls_flags(int argc, char **argv, char home_dir[])
{
    int flags = 0;

    int num_directories = 0;
    char *directories[10];

    for (int i = 1; i < argc; i++)
    {
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-a") == 0)
                flags |= 1;

            else if (strcmp(argv[i], "-l") == 0)
                flags |= 2;

            else if (strcmp(argv[i], "-al") == 0 || strcmp(argv[i], "-la") == 0)
                flags |= 3;

            else
            {
                printf("-bash: ls: invalid option '%s'\n", argv[i]);
                return 0;
            }
        }

        else
        {
            // handle multiple directories
            if (num_directories < 10)
            {
                if (strcmp(argv[i], "~") == 0)
                    directories[num_directories++] = home_dir;

                else if (check_substr(argv[i], "~/") == 0)
                {
                    argv[i] = remove_str(argv[i], "~/", 1);

                    char *file_path = malloc(strlen(home_dir) + strlen(argv[i]) + 2);
                    strcpy(file_path, home_dir);
                    strcat(file_path, "/");
                    strcat(file_path, argv[i]);

                    directories[num_directories++] = file_path;
                }

                else
                    directories[num_directories++] = argv[i];
            }
        }
    }

    if (num_directories == 0)
    {
        directories[0] = ".";
        num_directories = 1;
    }

    struct stat st;

    if (num_directories == 1)
    {
        if (stat(directories[0], &st) == 0 && (S_IFDIR & st.st_mode))
            ls_cmd_dir(flags, directories[0]);

        else if (stat(directories[0], &st) == 0 && (S_IFREG & st.st_mode))
            ls_cmd_file(flags, directories[0]);

        else
            printf("-bash: ls: cannot access '%s': No such file or directory\n", directories[0]);
    }

    else
    {
        for (int i = 0; i < num_directories; i++)
        {
            printf("%s:\n", directories[i]);

            if (stat(directories[i], &st) == 0)
            {
                if (S_IFDIR & st.st_mode)
                    ls_cmd_dir(flags, directories[i]);

                else if (S_IFREG & st.st_mode)
                    ls_cmd_file(flags, directories[i]);
            }

            else
                printf("-bash: ls: cannot access '%s': No such file or directory\n", directories[i]);

            if (i != (num_directories - 1))
                printf("\n");
        }
    }

    return 0;
}

int ls_cmd_dir(int flags, char *directory)
{
    struct dirent **list;
    struct stat st;

    DIR *dir = opendir(directory);
    if (!dir)
    {
        if (errno == ENOTDIR)
            printf("-bash: ls: %s: Not a directory\n", directory);

        else
            printf("-bash: ls: %s: No such file or directory\n", directory);
        return 0;
    }

    int n = scandir(directory, &list, 0, alphasort_case_insensitive);
    if (n < 0)
    {
        // printf("-bash: ls: %s: Unable to read directory\n", directory);
        return 0;
    }

    int total_blocks = 0;
    for (int i = 0; i < n; i++)
    {

        if (!(flags & 1))
        {
            // ignore hidden files
            if (list[i]->d_name[0] == '.')
                continue;
        }

        char *file_path = malloc(strlen(directory) + strlen(list[i]->d_name) + 2);
        sprintf(file_path, "%s/%s", directory, list[i]->d_name);

        if (stat(file_path, &st) == 0)
            total_blocks += st.st_blocks;

        free(file_path);
    }

    if (flags == 0)
    {
        for (int i = 0; i < n; i++)
        {
            printf("\033[39m");

            // ignore the hidden files
            if (list[i]->d_name[0] != '.')
            {
                char *file_name = list[i]->d_name;
                char *file_path = malloc(strlen(directory) + strlen(file_name) + 2);
                strcpy(file_path, directory);
                strcat(file_path, "/");
                strcat(file_path, file_name);

                stat(file_path, &st);

                int ct = 0;

                if (S_ISDIR(st.st_mode))
                    ct = 1;

                else if ((S_IXUSR & st.st_mode) || (S_IXGRP & st.st_mode) || (S_IXOTH & st.st_mode))
                    ct = 2;

                if (ct == 1)
                    printf("\033[36m");

                else if (ct == 2)
                    printf("\033[32m");

                printf(" %s", file_name);
                printf("\033[39m\n");
            }

            free(list[i]);
        }

        free(list);

        return 0;
    }

    // -a flag
    else if (flags == 1)
    {
        for (int i = 0; i < n; i++)
        {
            printf("\033[39m");
            char *file_name = list[i]->d_name;

            char *file_path = malloc(strlen(directory) + strlen(file_name) + 2);
            strcpy(file_path, directory);
            strcat(file_path, "/");
            strcat(file_path, file_name);

            stat(file_path, &st);

            int ct = 0;

            if (S_ISDIR(st.st_mode))
                ct = 1;

            else if ((S_IXUSR & st.st_mode) || (S_IXGRP & st.st_mode) || (S_IXOTH & st.st_mode))
                ct = 2;

            if (ct == 1)
                printf("\033[36m");

            else if (ct == 2)
                printf("\033[32m");

            printf(" %s", file_name);
            printf("\033[39m\n");

            free(list[i]);
        }

        free(list);

        return 0;
    }

    // -l
    else if (flags == 2)
    {
        printf("\033[39mtotal %d\n", total_blocks / 2);
        for (int i = 0; i < n; i++)
        {
            printf("\033[39m");

            // ignore the hidden files
            if (list[i]->d_name[0] != '.')
            {
                char *file_name = list[i]->d_name;
                char *file_path = malloc(strlen(directory) + strlen(file_name) + 2);
                strcpy(file_path, directory);
                strcat(file_path, "/");
                strcat(file_path, file_name);

                if (stat(file_path, &st) == -1)
                {
                    printf("-bash: ls: %s: Unable to read file\n", file_name);
                    return 0;
                }

                int ct = 0;

                if (S_ISDIR(st.st_mode))
                    ct = 1;

                else if ((S_IXUSR & st.st_mode) || (S_IXGRP & st.st_mode) || (S_IXOTH & st.st_mode))
                    ct = 2;

                printf((S_ISDIR(st.st_mode)) ? "d" : "-");
                printf((st.st_mode & S_IRUSR) ? "r" : "-");
                printf((st.st_mode & S_IWUSR) ? "w" : "-");
                printf((st.st_mode & S_IXUSR) ? "x" : "-");
                printf((st.st_mode & S_IRGRP) ? "r" : "-");
                printf((st.st_mode & S_IWGRP) ? "w" : "-");
                printf((st.st_mode & S_IXGRP) ? "x" : "-");
                printf((st.st_mode & S_IROTH) ? "r" : "-");
                printf((st.st_mode & S_IWOTH) ? "w" : "-");
                printf((st.st_mode & S_IXOTH) ? "x" : "-");
                printf(" %3ld", st.st_nlink);
                printf(" %7s", getpwuid(st.st_uid)->pw_name);
                printf(" %7s", getpwuid(st.st_gid)->pw_name);
                printf(" %8ld ", st.st_size);

                char ls_time[100];
                time_t curr_time = time(NULL);
                struct tm current_time, fcreate_time;
                localtime_r(&curr_time, &current_time);
                localtime_r(&st.st_mtime, &fcreate_time);

                // handle the time
                if (current_time.tm_year == fcreate_time.tm_year)
                    strftime(ls_time, 100, "%b %d %H:%M", &fcreate_time);

                else
                    strftime(ls_time, 100, "%b %d  %Y", &fcreate_time);

                printf("%s ", ls_time);

                if (ct == 1)
                    printf("\033[36m");

                else if (ct == 2)
                    printf("\033[32m");

                printf("%s\033[39m\n", file_name);
            }

            free(list[i]);
        }

        free(list);
        return 0;
    }

    // -al or -la
    else if (flags == 3)
    {
        printf("\033[39mtotal %d\n", total_blocks / 2);
        for (int i = 0; i < n; i++)
        {
            char *file_name = list[i]->d_name;
            char *file_path = malloc(strlen(directory) + strlen(file_name) + 2);
            strcpy(file_path, directory);
            strcat(file_path, "/");
            strcat(file_path, file_name);

            if (stat(file_path, &st) == -1)
            {
                printf("-bash: ls: %s: Unable to read file\n", file_name);
                return 0;
            }

            int ct = 0;

            if (S_ISDIR(st.st_mode))
                ct = 1;

            else if ((S_IXUSR & st.st_mode) || (S_IXGRP & st.st_mode) || (S_IXOTH & st.st_mode))
                ct = 2;

            printf((S_ISDIR(st.st_mode)) ? "d" : "-");
            printf((st.st_mode & S_IRUSR) ? "r" : "-");
            printf((st.st_mode & S_IWUSR) ? "w" : "-");
            printf((st.st_mode & S_IXUSR) ? "x" : "-");
            printf((st.st_mode & S_IRGRP) ? "r" : "-");
            printf((st.st_mode & S_IWGRP) ? "w" : "-");
            printf((st.st_mode & S_IXGRP) ? "x" : "-");
            printf((st.st_mode & S_IROTH) ? "r" : "-");
            printf((st.st_mode & S_IWOTH) ? "w" : "-");
            printf((st.st_mode & S_IXOTH) ? "x" : "-");
            printf(" %3ld", st.st_nlink);
            printf(" %7s", getpwuid(st.st_uid)->pw_name);
            printf(" %7s", getpwuid(st.st_gid)->pw_name);
            printf(" %8ld ", st.st_size);

            char ls_time[100];
            time_t curr_time = time(NULL);
            struct tm current_time, fcreate_time;
            localtime_r(&curr_time, &current_time);
            localtime_r(&st.st_mtime, &fcreate_time);

            // handle the time
            if (current_time.tm_year == fcreate_time.tm_year)
                strftime(ls_time, 100, "%b %d %H:%M", &fcreate_time);

            else
                strftime(ls_time, 100, "%b %d  %Y", &fcreate_time);

            printf("%s ", ls_time);

            if (ct == 1)
                printf("\033[36m");

            else if (ct == 2)
                printf("\033[32m");

            printf("%s\033[39m\n", file_name);

            free(list[i]);
        }

        free(list);
        return 0;
    }

    else
        return 0;
}

int ls_cmd_file(int flags, char *file_name)
{
    printf("\033[39m");
    struct stat st;

    if (stat(file_name, &st) == -1)
    {
        printf("-bash: ls: %s: Unable to read file\n", file_name);
        return 0;
    }

    // _ | -a
    if (flags == 0 || flags == 1)
    {
        if ((S_IXUSR & st.st_mode) || (S_IXGRP & st.st_mode) || (S_IXOTH & st.st_mode))
            printf("\033[32m");

        printf("%s\033[39m\n", file_name);
    }

    // -l | -la | -al flags
    else
    {
        printf("-");
        printf((st.st_mode & S_IRUSR) ? "r" : "-");
        printf((st.st_mode & S_IWUSR) ? "w" : "-");
        printf((st.st_mode & S_IXUSR) ? "x" : "-");
        printf((st.st_mode & S_IRGRP) ? "r" : "-");
        printf((st.st_mode & S_IWGRP) ? "w" : "-");
        printf((st.st_mode & S_IXGRP) ? "x" : "-");
        printf((st.st_mode & S_IROTH) ? "r" : "-");
        printf((st.st_mode & S_IWOTH) ? "w" : "-");
        printf((st.st_mode & S_IXOTH) ? "x" : "-");
        printf(" %3ld", st.st_nlink);
        printf(" %7s", getpwuid(st.st_uid)->pw_name);
        printf(" %7s", getpwuid(st.st_gid)->pw_name);
        printf(" %8ld ", st.st_size);

        char ls_time[100];
        time_t curr_time = time(NULL);
        struct tm current_time, fcreate_time;
        localtime_r(&curr_time, &current_time);
        localtime_r(&st.st_mtime, &fcreate_time);

        // handle the time
        if (current_time.tm_year == fcreate_time.tm_year)
            strftime(ls_time, 100, "%b %d %H:%M", &fcreate_time);

        else
            strftime(ls_time, 100, "%b %d  %Y", &fcreate_time);

        printf("%s ", ls_time);

        if ((S_IXUSR & st.st_mode) || (S_IXGRP & st.st_mode) || (S_IXOTH & st.st_mode))
            printf("\033[32m");

        printf("%s\033[39m\n", file_name);
    }

    return 0;
}

int discover_flags(int argc, char **argv, char home_dir[])
{
    // -d -> 1,
    // -f -> 2,
    // -d -f -> 3,
    // -f -d -> 3

    int flags = 0;

    char *dir = "";
    char *file = "";

    for (int i = 1; i < argc; i++)
    {
        // flags
        if (argv[i][0] == '-')
        {
            if (strcmp(argv[i], "-d") == 0)
            {
                if (flags == 2 || flags == 3)
                    flags = 3;

                else
                    flags = 1;
            }

            else if (strcmp(argv[i], "-f") == 0)
            {
                if (flags == 1 || flags == 3)
                    flags = 3;

                else
                    flags = 2;
            }

            else if (strcmp(argv[i], "-df") == 0 || strcmp(argv[i], "-fd") == 0)
                flags = 3;

            else
            {
                printf("-bash: discover: invalid option '%s'\n", argv[i]);
                return 0;
            }
        }

        // file names
        else if (argv[i][0] == '\"')
        {
            if (strcmp(file, "") == 0)
                file = remove_str(argv[i], "\"", 0);

            else
            {
                printf("-bash: discover: too many file names\n");
                return 0;
            }
        }

        // handle directory
        else
        {
            if (strcmp(dir, "") == 0)
            {
                if (strcmp(argv[i], "~") == 0)
                {
                    dir = (char *)malloc(strlen(home_dir) + 1);
                    strcpy(dir, home_dir);
                }

                else if (check_substr(argv[i], "~/") != -1)
                {
                    char *temp = remove_str(argv[i], "~/", 0);
                    dir = (char *)malloc(strlen(home_dir) + strlen(temp) + 2);
                    sprintf(dir, "%s/%s", home_dir, temp);
                }

                else
                {
                    dir = malloc(strlen(argv[i]) + 1);
                    strcpy(dir, argv[i]);
                }
            }

            else
            {
                printf("-bash: discover: too many directories\n");
                return 0;
            }
        }
    }

    char rel_path[1024];

    if (strcmp(dir, "") == 0)
    {
        char curr_dir[1024];
        getcwd(curr_dir, sizeof(curr_dir));

        dir = (char *)malloc(strlen(curr_dir) + 1);
        strcpy(dir, curr_dir);

        strcpy(rel_path, ".");
    }

    else
    {
        if (check_substr(dir, home_dir) == 0)
        {
            char *temp = strdup(dir);
            remove_str(temp, home_dir, 1);
            sprintf(rel_path, "~%s", temp);
        }

        else
            strcpy(rel_path, dir);
    }

    if (rel_path[strlen(rel_path) - 1] == '/')
        rel_path[strlen(rel_path) - 1] = '\0';

    discover_cmd(flags, dir, file, home_dir, rel_path);

    return 0;
}

int discover_cmd(int flags, char *dir, char *file, char home_dir[], char rel_path[])
{
    if (strcmp(file, "") == 0)
    {
        struct dirent **list;
        struct stat st;

        DIR *direct = opendir(dir);
        if (!direct)
        {
            printf("-bash: discover: %s: No such file or directory\n", dir);
            return 0;
        }

        int n = scandir(dir, &list, 0, alphasort_case_insensitive);
        if (n < 0)
        {
            printf("-bash: discover: %s: Unable to read directory\n", dir);
            return 0;
        }

        if (flags != 2)
            printf("\033[36m%s\n\033[39m", rel_path);

        for (int i = 0; i < n; i++)
        {
            char *file_name = list[i]->d_name;
            // ignore the higher directories
            if ((strcmp(file_name, ".") != 0) && (strcmp(file_name, "..") != 0))
            {
                char *file_path = malloc(strlen(dir) + strlen(file_name) + 2);
                sprintf(file_path, "%s/%s", dir, file_name);

                stat(file_path, &st);

                int ct = 0;

                if (S_ISDIR(st.st_mode))
                {
                    char *temp = (char *)malloc(strlen(rel_path) + strlen(file_name) + 2);
                    sprintf(temp, "%s/%s", rel_path, file_name);
                    discover_cmd(flags, file_path, file, home_dir, temp);
                }

                if (flags != 1)
                {
                    if ((S_IXUSR & st.st_mode) || (S_IXGRP & st.st_mode) || (S_IXOTH & st.st_mode))
                        printf("\033[32m%s/%s\n\033[39m", rel_path, file_name);

                    else
                        printf("\033[39m%s/%s\n\033[39m", rel_path, file_name);
                }
            }

            free(list[i]);
        }

        free(list);
        return 0;
    }

    else
    {
        struct dirent **list;
        struct stat st;

        DIR *direct = opendir(dir);
        if (!direct)
        {
            printf("-bash: discover: %s: No such file or directory\n", dir);
            return 0;
        }

        int n = scandir(dir, &list, 0, alphasort_case_insensitive);
        if (n < 0)
        {
            printf("-bash: discover: %s: Unable to read directory\n", dir);
            return 0;
        }

        int ret = 0;

        for (int i = 0; i < n; i++)
        {
            char *file_name = list[i]->d_name;

            // ignore the higher directories
            if ((strcmp(file_name, ".") != 0) && (strcmp(file_name, "..") != 0))
            {
                char *file_path = malloc(strlen(dir) + strlen(file_name) + 2);
                sprintf(file_path, "%s/%s", dir, file_name);

                stat(file_path, &st);

                int ct = 0;

                if (S_ISDIR(st.st_mode))
                {
                    char *temp = (char *)malloc(strlen(rel_path) + strlen(file_name) + 2);
                    sprintf(temp, "%s/%s", rel_path, file_name);
                    discover_cmd(flags, file_path, file, home_dir, temp);
                }

                if (strcmp(file_name, file) == 0)
                {
                    if (flags != 1)
                    {
                        if ((S_IXUSR & st.st_mode) || (S_IXGRP & st.st_mode) || (S_IXOTH & st.st_mode))
                            printf("\033[32m%s/%s\n\033[39m", rel_path, file_name);

                        else
                            printf("\033[39m%s/%s\n\033[39m", rel_path, file_name);
                    }
                }
            }

            free(list[i]);
        }

        free(list);
        return 0;
    }
}

int pinfo_cmd(int argc, char **argv)
{
    pid_t pid;

    if (argc > 2)
    {
        printf("-bash: pinfo: too many arguments\n");
        return 0;
    }

    else if (argc == 2)
        pid = atoi(argv[1]);

    else
        pid = getpid();

    FILE *fp;
    char *file = (char *)malloc(1024 * sizeof(char));
    sprintf(file, "/proc/%d/stat", pid);
    fp = fopen(file, "r");

    if (fp == NULL)
    {
        printf("-bash: pinfo: %d: No such process\n", pid);
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
            printf("-bash: pinfo: %d: no such process\n", pid);
            return 0;
        }

        char **argv = (char **)malloc(1024 * sizeof(char *));

        i = 0;
        while (token != NULL)
        {
            // printf("%s ", token);
            argv[i] = token;
            token = strtok_r(NULL, " ", &saveptr);

            i++;
        }

        printf("pid : %d\n", pid);

        if (strcmp(argv[7], argv[0]) == 0)
            printf("Process Status : %s+\n", argv[2]);

        else
            printf("Process Status : %s\n", argv[2]);

        printf("Memory : %s {Virtual Memory}\n", argv[22]);

        buffer[0] = '\0';
        int buffer_size = 1024;

        sprintf(file, "/proc/%d/exe", pid);
        if ((size = readlink(file, buffer, buffer_size)) != -1)
        {
            buffer[size] = '\0';
            printf("Executable Path : %s\n", buffer);
        }

        else
            printf("Executable Path : %s\n", argv[0]);
    }

    return 0;
}
