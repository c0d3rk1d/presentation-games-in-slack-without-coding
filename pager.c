// gcc -o pager  pager.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // for usleep
#include <termios.h> // for terminal input
#include <fcntl.h>   // for file control

#define MAX_LINE_LEN 1024
#define DEFAULT_PAGE_SIZE 37
#define DEFAULT_SLEEP_MS 25
#define MAX_LINES 10000 // maximum number of lines stored in memory

char *lines[MAX_LINES];
int total_lines = 0;

// Function to get a single character input without pressing enter
char getch()
{
    struct termios oldTermios, newTermios;
    char character;
    tcgetattr(0, &oldTermios); // Get current terminal attributes
    newTermios = oldTermios;
    newTermios.c_lflag &= ~(ICANON | ECHO); // Disable buffered I/O and echo
    tcsetattr(0, TCSANOW, &newTermios);     // Set new attributes
    character = getchar();                  // Read character
    tcsetattr(0, TCSANOW, &oldTermios);     // Restore old settings
    return (character);
}

// Load file lines into memory
int load_file(const char *filename)
{
    FILE *filePointer = fopen(filename, "r");
    if (!filePointer)
    {
        perror("File open error");
        return (0);
    }
    char buffer[MAX_LINE_LEN];
    while (fgets(buffer, sizeof(buffer), filePointer) && total_lines < MAX_LINES)
    {
        lines[total_lines] = strdup(buffer);
        total_lines++;
    }
    fclose(filePointer);
    return (1);
}

// Print a page of lines
void print_page(int start_line, int page_size, int sleep_ms)
{
    system("clear");
    int end_line = start_line + page_size;
    if (end_line > total_lines)
    {
        end_line = total_lines;
    }
    for (int index = start_line; index < end_line; index++)
    {
        if (index == end_line - 1)
        {
            // Last line of the file, last line of the page
            char *line = strdup(lines[index]); // Copy to modify
            size_t len = strlen(line);
            if (len > 0 && (line[len - 1] == '\n' || line[len - 1] == '\r'))
            {
                line[len - 1] = '\0'; // Strip newline
            }
            printf("%s", line);
            fflush(stdout);
            free(line);
        }
        else
        {
            printf("%s", lines[index]);
            fflush(stdout);
            usleep(sleep_ms * 1000);
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <filename> [-p pagesize] [-s sleepms]\n", argv[0]);
        return 1;
    }
    const char *filename = argv[1];
    int page_size = DEFAULT_PAGE_SIZE;
    int sleep_ms = DEFAULT_SLEEP_MS;
    // Parse optional arguments
    for (int index = 2; index < argc; index++)
    {
        if (strcmp(argv[index], "-p") == 0 && index + 1 < argc)
        {
            page_size = atoi(argv[++index]);
        }
        else if (strcmp(argv[index], "-s") == 0 && index + 1 < argc)
        {
            sleep_ms = atoi(argv[++index]);
        }
    }
    if (!load_file(filename))
    {
        return 1;
    }
    int current_line = 0;
    char command;
    print_page(current_line, page_size, sleep_ms);
    while (1)
    {
        command = getch();
        if (command == 'q' || command == 'Q')
        {
            system("clear");
            break;
        }
        else if (command == 'n' || command == 'N' || command == '>' || command == '.')
        {
            if (current_line + page_size < total_lines)
            {
                current_line += page_size;
                print_page(current_line, page_size, sleep_ms);
            }
        }
        else if (command == 'p' || command == 'P' || command == '<' || command == ',')
        {
            if (current_line - page_size >= 0)
            {
                current_line -= page_size;
                print_page(current_line, page_size, sleep_ms);
            }
            else
            {
                current_line = 0;
            }
        }
        else if (command == 'r' || command == 'R') {
            print_page(current_line, page_size, sleep_ms);
        }
    }
    for (int index = 0; index < total_lines; index++)
    {
        free(lines[index]);
    }
    return 0;
}
