#include <stdio.h> // for reading and writing from/to the command line
#include <stdlib.h> // for the malloc function
#include <string.h>
#include "constants.h" // header for command-constants and paths

const char* path = "/home/user/my_repo";
int id = 0;

void send_instruction_to_daemon(char* instruction)
{
    printf("%s", instruction);
}

int main(int argc, char** argv)
{
    char* instruction = malloc(50);
    if (argc == 1) 
        printf("No arguments found. Please introduce an analyze-option and a desired path.\nUse --help command for more information.\n");
    else if (strcmp(argv[1], "-a") && strcmp(argv[1], "--add") && strcmp(argv[1], "-p") && strcmp(argv[1], "--priority") && strcmp(argv[1], "-S")
             && strcmp(argv[1], "--suspend") && strcmp(argv[1], "-R") && strcmp(argv[1], "--resume") && strcmp(argv[1], "-r") && strcmp(argv[1], "--remove")
             && strcmp(argv[1], "-i") && strcmp(argv[1], "--info") && strcmp(argv[1], "-l") && strcmp(argv[1], "--list"))
            printf("No option found. Please choose a valid option.\nUse --help command for more information.\n");
    else if (argc == 2 && strcmp(argv[1], "-l") && strcmp(argv[1], "--list"))
            printf("No task-id found. Please select an existing task-id.\n");
    else if (argc >= 4 && (strcmp(argv[1], "-a") && strcmp(argv[1], "--add") && (!strcmp(argv[3], "-p") || !strcmp(argv[3], "--priority"))))
            printf("Cannot set priority for a nonexistent analysis task. Please use the -a or --add option.\n");
    else
    {
        if (!strcmp(argv[1], "-a") || !strcmp(argv[1], "--add"))
        {
            if (argc == 5 && strcmp(argv[3], "-p") && strcmp(argv[3], "--priority"))
                printf("Invalid arguments for --add command.\nUse --help command for more information.\n");
            else if (argc == 4)
                printf("Invalid arguments for --add command.\nUse --help command for more information.\n");
            else
            {
                char* priority = malloc(1);
                strcpy(priority, "'low'.");
                if (argc == 5)
                {
                    if (!strcmp(argv[4], "2"))
                        strcpy(priority, "'medium'.");
                    else if (!strcmp(argv[4], "3"))
                        strcpy(priority, "'high'.");
                }
                printf("Created analysis task with ID %d for ’%s’ and priority %s\n", id, argv[2], priority);
                strcpy(instruction, ADD);
                strcat(instruction, "\n");
                strcat(instruction, argv[2]);
                strcat(instruction, "\n");
                strcat(instruction, "-p\n");
                strcat(instruction, priority);
                strcat(instruction, "\n");
            }
        }
        else if (!strcmp(argv[1], "-l") || !strcmp(argv[1], "--list"))
        {
            if (argc > 2)
                printf("Invalid arguments for --list command.\nUse --help command for more information.\n");
            else
            {
                strcpy(instruction, LIST);
                strcat(instruction, "\n");
            }
        }
        else
        {
            if (argc != 3)
                printf("Invalid number of arguments.\nUse --help command for more information.\n");
            else if (!strcmp(argv[1], "-S") || !strcmp(argv[1], "--suspend"))
            {
                strcpy(instruction, SUSPEND);
                strcat(instruction, "\n");
                strcat(instruction, argv[2]);
                strcat(instruction, "\n");
            }
            else if (!strcmp(argv[1], "-R") || !strcmp(argv[1], "--resume"))
            {
                strcpy(instruction, RESUME);
                strcat(instruction, "\n");
                strcat(instruction, argv[2]);
                strcat(instruction, "\n");
            }
            else if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--remove"))
            {
                strcpy(instruction, REMOVE);
                strcat(instruction, "\n");
                strcat(instruction, argv[2]);
                strcat(instruction, "\n");
            }
            else if (!strcmp(argv[1], "-i") || !strcmp(argv[1], "--info"))
            {
                strcpy(instruction, INFO);
                strcat(instruction, "\n");
                strcat(instruction, argv[2]);
                strcat(instruction, "\n");
            }
            else if (!strcmp(argv[1], "-p") || !strcmp(argv[1], "--print"))
            {
                strcpy(instruction, PRINT);
                strcat(instruction, "\n");
                strcat(instruction, argv[2]);
                strcat(instruction, "\n");
            }
        }
    }
    send_instruction_to_daemon(instruction);
    return 0;
}
