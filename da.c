#include <stdio.h> // for reading and writing from/to the command line
#include <stdlib.h> // for the malloc/system functions
#include <string.h>
#include <fcntl.h> // for open flags
#include <stdbool.h>
#include <signal.h>
#include <errno.h>
#include <unistd.h> // for write/close/sleep functions
#include <sys/stat.h> // for stat structure (we use it to find out the number of bytes in a file)
#include "utils.h"

int daemon_pid; // daemon_pid is the pid read from the corresponding file

void write_instruction_to_daemon(char* instruction){   
    create_dir_if_not_exists(instruction_file_path);
    // TODO: use write_to_file function instead when implemented
    int fd = open(instruction_file_path, O_CREAT|O_TRUNC|O_WRONLY, S_IRWXU|S_IRWXG|S_IRWXO);    
    int len = strlen(instruction);
    write(fd, instruction, len);
    close(fd);

    if (daemon_pid > 0){ // if the daemon_pid was read correctly from file
        // we send a signal to the daemon to inform it that a command has been written to file 
        kill(daemon_pid, SIGUSR1); 
        // we wait a second for the daemon to process the instruction and give the
        // signal back
        sleep(2);
    } else {
        fprintf(stderr, "Couldn't send instruction to daemon because it is not running\n");
        exit(-1);
    }
}

pid_t read_daemon_pid_from_file() {
    int fd = open(daemon_pid_file_path, O_RDONLY);
    if(fd < 0){
        perror("Daemon hasn't started");
        return errno;
    }
    char *buf = (char*) malloc(10); // pid from file with maximum length of 10
    read(fd, buf, 10);
    close(fd);

    return atoi(buf);
}

void get_daemon_pid() {
    daemon_pid = read_daemon_pid_from_file();
}

void process_output_from_daemon(int signo) {
    char *res = read_from_file(output_file_path, "Couldn't open daemon output file\n");
    // print output to console
    printf("%s\n", res);
}

void write_da_pid_to_file(){
    create_dir_if_not_exists(da_pid_file_path);
    // get pid and convert to char*
    char* pidstring = malloc(10);
    int pid = getpid();
    itoa(pid, pidstring);
    // TODO: use write_to_file function instead when implemented
    int fd = open(da_pid_file_path, O_CREAT|O_TRUNC|O_WRONLY, S_IRWXU|S_IRWXG|S_IRWXO);
    if (fd < 0){
        perror("Could not create da pid file\n");
    }
    write(fd, pidstring, 10); // write pid to file
    close(fd);
}

int main(int argc, char** argv)
{   
    // da listens to the signal SIGUSR2 from the daemon and calls 
    // the function given as parameter when it receives it
    signal(SIGUSR2, process_output_from_daemon);
    // da writes its pid in a file so that the daemon knows 
    // to whom to give the signal back
    write_da_pid_to_file();

    get_daemon_pid();

    char* instruction = malloc(INSTR_LENGTH);
    if (argc == 1) {
        printf("No arguments found. Please introduce an analyze-option and a desired path.\n");
        printf("Use --help command for more information.\n");
        return 0;
    } else if (
        strcmp(argv[1], "-a") && strcmp(argv[1], "--add") && strcmp(argv[1], "-p") && strcmp(argv[1], "--priority")
        && strcmp(argv[1], "-S") && strcmp(argv[1], "--suspend") && strcmp(argv[1], "-R") && strcmp(argv[1], "--resume")
        && strcmp(argv[1], "-r") && strcmp(argv[1], "--remove") && strcmp(argv[1], "-i") && strcmp(argv[1], "--info")
        && strcmp(argv[1], "-l") && strcmp(argv[1], "--list") && strcmp(argv[1], "-h") && strcmp(argv[1], "--help")
    ) {
        printf("No such option, please choose a valid option.\nUse --help command for more information.\n");
        return 0;
    } else if (argc == 2 && strcmp(argv[1], "-l") && strcmp(argv[1], "--list") && strcmp(argv[1], "-h") && strcmp(argv[1], "--help")){
        printf("No task-id found. Please select an existing task-id.\n");
        return 0;
    } else if (argc >= 4 && (strcmp(argv[1], "-a") && strcmp(argv[1], "--add") && (!strcmp(argv[3], "-p") || !strcmp(argv[3], "--priority")))){
        printf("Cannot set priority for a nonexistent analysis task. Please use the -a or --add option.\n");
        return 0;
    } else {
        if (!strcmp(argv[1], "-a") || !strcmp(argv[1], "--add")){
            if (argc != 5 || (argc == 5 && strcmp(argv[3], "-p") && strcmp(argv[3], "--priority"))){
                printf("Invalid arguments for --add command.\nUse --help command for more information.\n");
                return 0;
            } else { // ADD command
                char* priority = malloc(7);
                // TODO check is valid path 
                if (!strcmp(argv[4], "1"))
                    strcpy(priority, "1");
                else if (!strcmp(argv[4], "2"))
                    strcpy(priority, "2");
                else if (!strcmp(argv[4], "3"))
                    strcpy(priority, "3");
                else printf("Invalid arguments for --add command.\nUse --help command for more information.\n");
                sprintf(instruction, "%s\n%s\n%s\n", ADD, argv[2], priority);
            }
        } else if (!strcmp(argv[1], "-l") || !strcmp(argv[1], "--list")){ 
            if (argc > 2){
                printf("Invalid arguments for --list command.\nUse --help command for more information.\n");
                return 0;
            } else { // LIST command
                sprintf(instruction, "%s\n", LIST);
            }
        } else if (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help")){ 
            if (argc > 2){
                printf("Invalid arguments for --help command.\n");
                return 0;
            } else { // HELP command
                sprintf(instruction, "%s\n", HELP);
            }
        } else if (argc != 3){
            printf("Invalid number of arguments.\nUse --help command for more information.\n");
            return 0;
        } else if (!strcmp(argv[1], "-S") || !strcmp(argv[1], "--suspend")){
            if(!atoi(argv[2]))
                printf ("Invalid task ID.");
            // SUSPEND command
            sprintf(instruction, "%s\n%s\n", SUSPEND, argv[2]);
        } else if (!strcmp(argv[1], "-R") || !strcmp(argv[1], "--resume")){
            if(!atoi(argv[2]))
                printf ("Invalid task ID.");
            // RESUME command
            sprintf(instruction, "%s\n%s\n", RESUME, argv[2]);
        } else if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--remove")){
            if(!atoi(argv[2]))
                printf ("Invalid task ID.");
            // REMOVE command
            sprintf(instruction, "%s\n%s\n", REMOVE, argv[2]);
        } else if (!strcmp(argv[1], "-i") || !strcmp(argv[1], "--info")){
            if(!atoi(argv[2]))
                printf ("Invalid task ID.");
            // INFO command
            sprintf(instruction, "%s\n%s\n", INFO, argv[2]);
        } else if (!strcmp(argv[1], "-p") || !strcmp(argv[1], "--print")){
            if(!atoi(argv[2]))
                printf ("Invalid task ID.");
            // PRINT command
            sprintf(instruction, "%s\n%s\n", PRINT, argv[2]);
        }
    }
    write_instruction_to_daemon(instruction);

    return 0;
}
