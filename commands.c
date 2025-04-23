#include "commands.h"
#include "ctype.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

#define MAX_ARGS 128
#define MAX_COMMANDS 10

char *argv[MAX_ARGS];

FILE *handleRedirection(int *numArgsRef)
{
    for (int i = 0; i < *numArgsRef; i++)
    {
        if (strcmp(argv[i], ">") == 0)
        {
            if (i == *numArgsRef - 1)
            {
                printf("Please specify a filename.\n");
                return NULL;
            }

            FILE *fp = fopen(argv[i + 1], "w");
            if (fp == NULL)
            {
                perror("Error opening file");
                return NULL;
            }

            argv[i] = NULL;  // Cut off args at redirection
            *numArgsRef = i; // Ignore redirection args
            return fp;
        }
    }

    return stdout;
}

void callEcho(FILE *outputFile, int numArgs)
{
    for (int i = 1; i < numArgs; i++) // start at 1 to skip "echo"
    {
        fprintf(outputFile, "%s", argv[i]);
        if (i < numArgs - 1)
            fprintf(outputFile, " ");
    }
    fprintf(outputFile, "\n");
}

void changeDirectory()
{
    char *newDir = argv[1];
    if (chdir(newDir) != 0)
    {
        printf("Invalid directory\n");
    }
}

int parseArgs(char *input)
{
    int numArgs = 0;
    char *ptr = input;

    while (*ptr != '\0')
    {
        while (isspace(*ptr))
            ptr++;
        if (*ptr == '\0')
            break;

        if (*ptr == '"' || *ptr == '\'')
        {
            char quote = *ptr++;
            argv[numArgs++] = ptr;
            while (*ptr && *ptr != quote)
                ptr++;
            if (*ptr)
                *ptr++ = '\0';
        }
        else
        {
            argv[numArgs++] = ptr;
            while (*ptr && !isspace(*ptr))
                ptr++;
            if (*ptr)
                *ptr++ = '\0';
        }

        if (numArgs >= MAX_ARGS)
            break;
    }

    argv[numArgs] = NULL;
    return numArgs;
}

void callPwd(FILE *outputFile, int numArgs)
{
    char *pwd = getcwd(NULL, 0); // mallocs internally so need to free
    if (pwd == NULL)
    {
        printf("Error getting working directory");
    }
    fprintf(outputFile, "%s\n", pwd);
    free(pwd);
}

void callLs(FILE *outputFile, const char *path)
{
    DIR *dir = opendir(path);
    if (dir == NULL)
    {
        printf("Error opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_name[0] != '.')
        {
            fprintf(outputFile, "%s  ", entry->d_name);
        }
    }
    fprintf(outputFile, "\n");
    closedir(dir);
}

void callMkdir()
{
    char *newDir = argv[1];
    int status = mkdir(newDir, 0777);

    if (status != 0)
    {
        printf("Error creating directory %s\n", newDir);
    }
}

void callTouch()
{
    FILE *fp = fopen(argv[1], "w");

    if (fp == NULL)
    {
        printf("Error opening file\n");
        return;
    }

    fclose(fp);
}

void removeFile()
{
    // naive approach to protecting from deleting itself. This does not handle if file was renamed, etc.
    if (strcmp(argv[1], ".") == 0 || strcmp(argv[1], "edsh") == 0)
    {
        fprintf(stderr, "Error: Attempted to delete the running shell. Operation blocked.\n");
        return;
    }
    FILE *fp = fopen(argv[1], "r");
    char confirm[5];

    if (fp == NULL)
    {
        printf("File not found. \n");
    }

    else
    {
        printf("Confirm you want to remove %s by entering y or yes: ", argv[1]);
        scanf("%s", confirm);

        for (int i = 0; confirm[i]; i++)
        {
            confirm[i] = tolower(confirm[i]);
        }

        if (strcmp("y", confirm) == 0 || strcmp("yes", confirm) == 0)
        {
            // remove the file
            if (remove(argv[1]) != 0)
            {
                printf("Error removing the file.\n");
            }
        }
    }
}

/*
Only handling piping, does not handle && or ||
*/
int split_by_pipe(char *line, char *commands[])
{
    int count = 0;
    char *token = strtok(line, "|");
    while (token && count < MAX_COMMANDS)
    {
        while (isspace(*token))
            token++;
        commands[count++] = token;
        token = strtok(NULL, "|");
    }
    return count;
}

void executeCommands(char *inputLine)
{
    char *commands[MAX_COMMANDS];
    int numCmds = split_by_pipe(inputLine, commands);

    for (int i = 0; i < numCmds; i++)
    {
        char *commandLine = commands[i];

        int numArgs = parseArgs(commandLine);
        if (numArgs == 0)
            continue;

        char *commandName = argv[0];
        FILE *outputFile = handleRedirection(&numArgs);
        if (!outputFile)
            outputFile = stdout;

        if (strcmp(commandName, "exit") == 0)
        {
            printf("Exiting the terminal\n");
            exit(EXIT_SUCCESS);
        }
        else if (strcmp(commandName, "clear") == 0)
        {
            printf("\033[2J\033[1;1H"); // ANSI clear screen
        }
        else if (strcmp(commandName, "echo") == 0)
        {
            callEcho(outputFile, numArgs);
        }
        else if (strcmp(commandName, "pwd") == 0)
        {
            callPwd(outputFile, numArgs);
        }

        else if (strcmp(commandName, "cd") == 0)
        {
            changeDirectory();
        }

        else if (strcmp(commandName, "ls") == 0)
        {
            callLs(outputFile, ".");
        }

        else if (strcmp(commandName, "mkdir") == 0)
        {
            callMkdir();
        }

        else if (strcmp(commandName, "touch") == 0)
        {
            callTouch();
        }

        else if (strcmp(commandName, "rm") == 0)
        {
            removeFile();
        }

        // TO DO:  mv, execute programs

        else
        {
            printf("Unknown command: %s\n", commandName);
        }

        if (outputFile != stdout)
            fclose(outputFile);
    }
}
