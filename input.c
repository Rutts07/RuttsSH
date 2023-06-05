#include "header.h"

char *command;

void die(const char *s)
{
    perror(s);
    exit(1);
}

struct termios orig_termios;

void disableRawMode()
{
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1)
        die("tcsetattr");
}

/**
 * Enable row mode for the terminal
 * The ECHO feature causes each key you type to be printed to the terminal, so you can see what you’re typing.
 * Terminal attributes can be read into a termios struct by tcgetattr().
 * After modifying them, you can then apply them to the terminal using tcsetattr().
 * The TCSAFLUSH argument specifies when to apply the change: in this case, it waits for all pending output to be written to the terminal, and also discards any inputut that hasn’t been read.
 * The c_lflag field is for “local flags”
 */

void enableRawMode()
{
    if (tcgetattr(STDIN_FILENO, &orig_termios) == -1)
        die("tcgetattr");

    atexit(disableRawMode);

    struct termios raw = orig_termios;

    raw.c_lflag &= ~(ICANON | ECHO);

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1)
        die("tcsetattr");
}

/**
 * stdout and stdin are buffered we disable buffering on that
 * After entering in raw mode we read characters one by one
 * Up arrow keys and down arrow keys are represented by 3 byte escape codes
 * starting with ascii number 27 i.e. ESC key
 * This way we interpret arrow keys
 * Tabs are usually handled by the term, but here we are simulating tabs for the sake of simplicity
 * Backspace move the cursor one control character to the left
 */

int get_input(ssize_t *size, int terminal_width, char home_dir[])
{
    history_index = history_size;

    int string_size = terminal_width;

    char *input;
    char c;
    int pt;
    
    int erase = 0;
    int arrow = 0;

    while (1)
    {
        setbuf(stdout, NULL);
        enableRawMode();

        memset(input, '\0', 1024);
        pt = 0;

        while (read(STDIN_FILENO, &c, 1) == 1)
        {
            // delete the previous suggestion
            for (int i = 0; i < erase; i++)
                printf(" ");

            for (int i = 0; i < erase; i++)
                printf("\b");

            if (iscntrl(c))
            {
                if (c == 10)
                    break;

                // ctrl + N
                else if (c == 14)
                {
                    int index = suggest(strdup(input));
                    
                    if (index != -1)
                    {
                        while (string_size > terminal_width)
                        {
                            printf("\b \b");
                            string_size--;
                        }

                        printf("\033[0;39m%s\033[0;39m", history[index]);

                        string_size = terminal_width + strlen(history[index]);
                        strcpy(input, history[index]);
                        pt = strlen(input);
                    }
                }

                // ctrl + D
                else if (c == 4)
                    handle_exit();

                else if (c == 27)
                {
                    char buf[3];
                    buf[2] = 0;

                    if (read(STDIN_FILENO, buf, 2) == 2)
                    {
                        arrow = 1;
                        
                        // show previous history - UP arrow
                        if (strcmp("[A", buf) == 0 && history_index > 0)
                        {
                            while (string_size > terminal_width)
                            {
                                printf("\b \b");
                                string_size--;
                            }

                            printf("\033[36m%s\033[39m", history[--history_index]);

                            string_size = terminal_width + strlen(history[history_index]);

                            strcpy(input, history[history_index]);
                            // strcpy(input2, history[history_index]);
                            pt = strlen(input);
                        }

                        // show previous history - DOWN arrow
                        if (strcmp("[B", buf) == 0 && history_index < history_size)
                        {
                            while (string_size > terminal_width)
                            {
                                printf("\b \b");
                                string_size--;
                            }

                            // empty the buffer
                            if (history_index == (history_size - 1))
                            {
                                printf("\033[39m");

                                string_size = terminal_width;

                                memset(input, '\0', 1024);
                                pt = 0;

                                ++history_index;
                            }

                            else
                            {
                                printf("\033[36m%s\033[39m", history[++history_index]);

                                string_size = terminal_width + strlen(history[history_index]);

                                strcpy(input, history[history_index]);
                                pt = strlen(input);
                            }
                        }
                    }
                }

                else if (c == 127)
                {
                    // backspace
                    if (c == 0x7f)
                    {
                        if (string_size > terminal_width)
                        {
                            // go one char left
                            printf("\b");
                            // overwrite the char with whitespace
                            printf(" ");
                            // go back to "now removed char position"
                            printf("\b");

                            if (pt > 0)
                                input[--pt] = '\0';

                            string_size--;
                        }
                    }
                }

                else if (c == 9)
                {
                    // TAB character
                    // input[pt++] = '\t' creates problems for backspace

                    char *token;
                    char *last_word = "\0";
                    char *saveptr;

                    char *dup_input = strdup(input);

                    // check for executables and files with relative path
                    if (check_lastoc(dup_input, "./") != -1)
                        dup_input = remove_str(dup_input, "./", 0);

                    token = strtok_r(dup_input, " \t\r\n", &saveptr);

                    int i = 0;
                    while (token != NULL)
                    {
                        last_word = (char *)malloc(strlen(token) + 1);
                        strcpy(last_word, token);
                        token = strtok_r(NULL, " \t\r\n", &saveptr);

                        i++;
                    }

                    char **files = NULL;
                    int index = 0;
                    int num = 0;

                    // ignore flags
                    if (!(last_word[0] == '-'))
                        files = auto_completion(last_word, &num);

                    if (num > 1)
                    {
                        printf("\n");
                        for (int i = 0; i < num; i++)
                            printf("\033[37m%s\n", files[i]);
                        printf("\033[39m");

                        update_history(input, home_dir);
                        disableRawMode();
                        return 1;
                    }

                    else if (num == 1)
                    {
                        index = check_lastoc(input, last_word);
                        int len = strlen(files[0]);

                        while (string_size > (terminal_width + index))
                        {
                            printf("\b \b");
                            string_size--;
                            input[--pt] = '\0';
                        }

                        printf("\033[37m");

                        for (int i = 0; i < len; i++)
                        {
                            c = files[0][i];
                            printf("%c", c);
                            input[pt++] = c;
                            string_size++;
                        }

                        if (files[0][len - 1] != '/')
                        {
                            // space after file name
                            printf(" ");
                            input[pt++] = ' ';
                            string_size++;
                        }

                        printf("\033[39m");

                        free(files);
                        continue;
                    }

                    else
                    {
                        // TABS should be 4 spaces
                        for (int i = 0; i < 4; i++)
                        {
                            printf(" ");
                            input[pt++] = ' ';
                        }

                        string_size = string_size + 4;
                    }
                }

                else if (c == 4)
                {
                    disableRawMode();
                    exit(0);
                }
                    

                else
                {
                    printf("%d\n", c);
                    string_size = string_size + 1;
                }
            }

            else
            {
                input[pt++] = c;
                printf("%c", c);
                string_size++;
            }

            if (arrow == 0)
                erase = auto_suggestion(strdup(input));

            else
            {
                arrow = 0;
                erase = 0;
            }
        }

        disableRawMode();
        printf("\n");

        *size = strlen(input);
        command = input;
        return 0;
    }
}
