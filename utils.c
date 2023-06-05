#include "header.h"

int alphasort_case_insensitive(const struct dirent **a, const struct dirent **b)
{
    return strcasecmp((*a)->d_name, (*b)->d_name);
}

char *remove_str(char str[], char substr[], int flag)
{
    // if flag = 0, delete all occurences of substr
    // if flag = 1, delete first occurence of substr

    int sub_len = strlen(substr);
    int str_len = strlen(str);

    int i = 0;
    while (i < str_len)
    {
        if (strstr(&str[i], substr) == &str[i])
        {
            memmove(&str[i], &str[i + sub_len], str_len - i);
            str_len -= sub_len;

            if (flag)
                break;
        }

        else
            i++;
    }

    return str;
}

int check_substr(char str[], char substr[])
{
    // return index of first occurence of substr in str
    // return -1 if substr not found in str

    int sub_len = strlen(substr);
    int str_len = strlen(str);

    int i = 0;
    while (i < str_len)
    {
        if (strstr(&str[i], substr) == &str[i])
            return i;

        else
            i++;
    }

    return -1;
}

int check_lastoc(char str[], char substr[])
{
    // return index of last occurence of substr in str
    // return -1 if substr not found in str

    int sub_len = strlen(substr);
    int str_len = strlen(str);
    int index = -1;

    int i = 0;
    while (i < str_len)
    {
        if (strstr(&str[i], substr) == &str[i])
            index = i;

        i++;
    }

    return index;
}

char **auto_completion(char *last_word, int *num)
{
    // return 0 if no suggestion
    // return num files in directory if suggestion

    char **files = NULL;
    int num_files = 0;

    struct dirent **list;
    struct stat st;

    int n = scandir(".", &list, 0, alphasort_case_insensitive);

    if (last_word == NULL || last_word[0] == '\0')
    {
        files = (char **)malloc(n * sizeof(char *));
        for (int i = 0; i < n; i++)
        {
            if (list[i]->d_name[0] != '.')
            {
                // printf("\n%s", list[i]->d_name);
                stat(list[i]->d_name, &st);

                // check for directory and add "/" accordingly
                if (S_ISDIR(st.st_mode))
                {
                    if (num_files == 0)
                        files = (char **)malloc(sizeof(char *));

                    else
                        files = (char **)realloc(files, (num_files + 1) * sizeof(char *));

                    files[num_files] = (char *)malloc(strlen((list[i]->d_name) + 2) * sizeof(char));
                    strcpy(files[num_files], list[i]->d_name);
                    strcat(files[num_files], "/\0");

                    num_files++;
                }

                else
                {
                    if (num_files == 0)
                        files = (char **)malloc(sizeof(char *));

                    else
                        files = (char **)realloc(files, (num_files + 1) * sizeof(char *));

                    files[num_files] = (char *)malloc(strlen((list[i]->d_name) + 1) * sizeof(char));
                    strcpy(files[num_files], list[i]->d_name);
                    strcat(files[num_files], "\0");

                    num_files++;
                }
            }

            free(list[i]);
        }

        free(list);

        *num = num_files;
        return files;
    }

    for (int i = 0; i < n; i++)
    {
        // ignore the hidden files
        if (list[i]->d_name[0] != '.' && check_substr(list[i]->d_name, last_word) == 0)
        {
            // printf("\n%s", list[i]->d_name);
            stat(list[i]->d_name, &st);

            // check for directory and add "/" accordingly
            if (S_ISDIR(st.st_mode))
            {
                if (num_files == 0)
                    files = (char **)malloc(sizeof(char *));

                else
                    files = (char **)realloc(files, (num_files + 1) * sizeof(char *));

                files[num_files] = (char *)malloc(strlen((list[i]->d_name) + 2) * sizeof(char));
                strcpy(files[num_files], list[i]->d_name);
                strcat(files[num_files], "/\0");

                num_files++;
            }

            else
            {
                if (num_files == 0)
                    files = (char **)malloc(sizeof(char *));

                else
                    files = (char **)realloc(files, (num_files + 1) * sizeof(char *));

                files[num_files] = (char *)malloc(strlen((list[i]->d_name) + 1) * sizeof(char));
                strcpy(files[num_files], list[i]->d_name);
                strcat(files[num_files], "\0");

                num_files++;
            }
        }

        free(list[i]);
    }

    free(list);

    *num = num_files;
    return files;
}

char *get_last_token(char *inp)
{
    char input[1024];
    strcpy(input, inp);

    char *token = strtok(input, " \t\r\n");
    char *last_token = NULL;

    while (token != NULL)
    {
        last_token = token;
        token = strtok(NULL, " ");
    }

    return last_token;
}

int auto_suggestion(char *input)
{
    if (input == NULL || input[0] == '\0' || strlen(input) == 0)
        return 0;

    for (int i = 0; i < history_size; i++)
    {
        if (check_substr(history[i], input) == 0 && strcmp(history[i], input) != 0)
        {
            for (int j = strlen(input); j < strlen(history[i]); j++)
                printf("\033[37m%c", history[i][j]);

            // going back to the end of the input
            for (int j = strlen(input); j < strlen(history[i]); j++)
                printf("\b");

            printf("\033[39m");

            return strlen(history[i]) - strlen(input);
        }
    }

    return 0;
}

int suggest(char *input)
{    
    if (input == NULL || input[0] == '\0' || strlen(input) == 0)
        return -1;

    for (int i = 0; i < history_size; i++)
    {
        if (check_substr(history[i], input) == 0 && strcmp(history[i], input) != 0)
            return i;
    }

    return -1;
}
