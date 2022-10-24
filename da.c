#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#define LOW "1"
#define MEDIUM "2"
#define HIGH "3"

#define HELP "0"
#define ADD "10"
#define PRIORITY "20"
#define SUSPEND "30"
#define RESUME "40"
#define REMOVE "50"
#define INFO "60"
#define LIST "70"
#define PRINT "80"

int id = 1;

const char* path = "/home/user/my_repo";

void send_instruction_to_daemon(char* cmd)
{
    printf("%s", cmd);
}

int main(int argc, char** argv)
{
    char* cmd = malloc(50);
    if (argc == 1) 
        printf("No arguments found! Please introduce an analyze-option and a desired path!\n");
    else if (argc == 2 && strcmp(argv[1], "-l"))
            printf("No task-id found! Please select an existing task-id.");
    else if (argc == 5 && strcmp(argv[1], "-a") && strcmp(argv[1], "--add") && (!strcmp(argv[3], "-p") || !strcmp(argv[3], "--priority")))
            printf("Cannot set priority for a nonexistent analysis task! Please use the -a or --add option.\n");
    else if (strcmp(argv[1], "-a") && strcmp(argv[1], "-p") && strcmp(argv[1], "-S") && strcmp(argv[1], "-R") && strcmp(argv[1], "-r")
             && strcmp(argv[1], "-i") && strcmp(argv[1], "-l"))
            printf("Nonexistent option found! Please choose a valid option!\n");
    else
    {
        if (!strcmp(argv[1], "-a") || !strcmp(argv[1], "--add"))
        {
            printf("Created analysis task with ID %d for ’%s’ and priority %s\n", id, argv[2], argv[4]);
            strcpy(cmd, ADD);
            strcat(cmd, "\n");
            strcat(cmd, argv[2]);
            strcat(cmd, "\n");
            strcat(cmd, argv[3]);
            strcat(cmd, "\n");
            strcat(cmd, argv[4]);
            strcat(cmd, "\n");
        }
        else if (!strcmp(argv[1], "-S") || !strcmp(argv[1], "--suspend"))
        {
            strcpy(cmd, SUSPEND);
            strcat(cmd, "\n");
            strcat(cmd, argv[2]);
            strcat(cmd, "\n");
        }
        else if (!strcmp(argv[1], "-R") || !strcmp(argv[1], "--resume"))
        {
            strcpy(cmd, RESUME);
            strcat(cmd, "\n");
            strcat(cmd, argv[2]);
            strcat(cmd, "\n");
        }
        else if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--remove"))
        {
            strcpy(cmd, REMOVE);
            strcat(cmd, "\n");
            strcat(cmd, argv[2]);
            strcat(cmd, "\n");
        }
        else if (!strcmp(argv[1], "-i") || !strcmp(argv[1], "--info"))
        {
            strcpy(cmd, INFO);
            strcat(cmd, "\n");
            strcat(cmd, argv[2]);
            strcat(cmd, "\n");
        }
        else if (!strcmp(argv[1], "-l") || !strcmp(argv[1], "--list"))
        {
            strcpy(cmd, LIST);
            strcat(cmd, "\n");
        }
        else if (!strcmp(argv[1], "-p") || !strcmp(argv[1], "--print"))
        {
            strcpy(cmd, PRINT);
            strcat(cmd, "\n");
            strcat(cmd, argv[2]);
            strcat(cmd, "\n");
        }
    }
    send_instruction_to_daemon(cmd);
    return 0;
}