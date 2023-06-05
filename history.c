#include "header.h"

char *history[MAX_HISTORY_SIZE];
int history_index = 0;
int history_size = 0;

int history_cmd(int argc, char **argv)
{
    if (argc == 2)
    {
        if (strcmp(argv[1], "-c") == 0)
        {
            for(int i = 0; i < history_size; i++)
                history[history_size] = NULL;

            history_size = 0;
        }

        else
            printf("-bash: history: %s: invalid option\n", argv[1]);

        return 0;
    }

    if (argc > 2)
    {
        printf("-bash: history: too many arguments\n");
        return 0;
    }

    if (history_size >= 10)
    {
        for (int i = history_size - 10; i < history_size; i++)
            printf(" %d %s\n", i + 1, history[i]);
    }

    else
    {
        for (int i = 0; i < history_size; i++)
            printf(" %d %s\n", i + 1, history[i]);
    }

    return 0;
}

int update_history(char * command, char home_dir[])
{
    if (command[0] == '\0' || command[0] == ' ' || command[0] == '\t' || command[0] == '\n' || strlen(command) == 1)
        return 0;
    
    // check if command already exits in history
    if (history_size > 0)
    {
        if (strcmp(history[history_size - 1], command) == 0)
            return 0;
    }

    if (history_size < MAX_HISTORY_SIZE)
        history[history_size++] = strdup(command);

    else 
    {
        free(history[0]);

        for (int i = 1; i < MAX_HISTORY_SIZE; i++)
            history[i-1] = history[i];

        history[MAX_HISTORY_SIZE - 1] = strdup(command);
    }

    history_index = history_size;
    save_history(home_dir);
    
    return 0;
}

int save_history(char home_dir[])
{
    char history_file[1024];
    strcpy(history_file, home_dir);
    strcat(history_file, "/.history.txt");

    FILE *fp = fopen(history_file, "w");

    if (fp == NULL)
    {
        printf("-bash: history: save failed\n");
        // exit(1);
        return 0;
    }

    char * buffer = (char *) malloc(1024 * sizeof(char));
    for (int i = 0; i < history_size; i++)
        fprintf(fp, "%s\n", history[i]);
        
    fclose(fp);
    return 0;
}

int load_history(char home_dir[])
{
    char history_file[1024];
    strcpy(history_file, home_dir);
    strcat(history_file, "/.history.txt");

    FILE *fp = fopen(history_file, "r");

    // previous history file not found
    if (fp == NULL)
        return 0;

    ssize_t line_size;
    char *line = NULL;
    ssize_t bytes_read = getline(&line, &line_size, fp);
    line[strlen(line)-1] = '\0';
    // line = remove_str(line, "'", 0);

    while(bytes_read >= 0 && history_size <= MAX_HISTORY_SIZE)
    {
        if (strlen(line) > 1)
            history[history_size++] = strdup(line);

        bytes_read = getline(&line, &line_size, fp);
        line[strlen(line)-1] = '\0';
    }

    history_index = history_size;
        
    fclose(fp);
    return 0;
}
