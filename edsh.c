#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <pwd.h>
#include <ctype.h>
#include "commands.h"

#define COLOR_RESET "\033[0m"
#define COLOR_CYAN "\033[0;36m"
#define INPUT_SIZE 256
#define HOSTNAME_SIZE 64
#define USERNAME_SIZE 32
#define DIRECTORY_SIZE 256

// Global variables
char input[INPUT_SIZE];
char hostname[HOSTNAME_SIZE];
char username[USERNAME_SIZE];
char directory[DIRECTORY_SIZE];

// Process command code
void processCommand(char *command)
{
    // remove leading whitespaces
    char *ptr = command;
    while (*ptr && isspace((unsigned char)*ptr))
    {
        ptr++;
    }
    if (ptr != command)
    {
        memmove(command, ptr, strlen(ptr) + 1);
    }

    // remove trailing whitespaces
    ptr = command + strlen(command) - 1;
    while (ptr >= command && isspace((unsigned char)*ptr))
    {
        ptr--;
    }
    *(ptr + 1) = '\0';

    executeCommands(command);
}

// Main function code
int main(int argc, char *argv[])
{
    // getting hostname and logged-in username
    if (gethostname(hostname, sizeof(hostname)) == -1)
    {
        perror("gethostname");
        exit(EXIT_FAILURE);
    }

    if (getlogin_r(username, sizeof(username)) != 0)
    {
        struct passwd *pw = getpwuid(geteuid());
        if (pw == NULL)
        {
            perror("getpwuid");
            exit(EXIT_FAILURE);
        }
        snprintf(username, sizeof(username), "%s", pw->pw_name);
    }

    // main while loop
    while (1)
    {
        // get the current working directory
        if (getcwd(directory, sizeof(directory)) == NULL)
        {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }

        fprintf(stdout, "%s%s@%s:%s%s> ",
                COLOR_CYAN, username, hostname, directory,
                COLOR_RESET); // printing the user prompt

        // taking the user input
        if (fgets(input, INPUT_SIZE, stdin) == NULL)
        {
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        input[strlen(input) - 1] = '\0'; // removing the terminating enter key input

        if (strlen(input) > 0)
        {
            processCommand(input);
        }
    }
}
