#include <stdio.h> // for reading and writing from/to the command line
#include <stdlib.h> // for the malloc/system functions
#include <string.h>
#include <fcntl.h> // for open flags
#include <stdbool.h>
#include <signal.h>
#include <unistd.h> // for write/close/sleep functions
#include <sys/types.h> //pid_t type
#include <inttypes.h>
#include <iso646.h>
#include <errno.h> 
#include "constants.h" // header for command-constants and paths
#include "utils.h" // header for map related functions and itoa
#include "process_instructions.h"

int daemon_pid; // daemon_pid is the pid read from the corresponding file

char* read_from_file(char* file){ 
	FILE* fptr;
	char buffer;
	char* result = "";
	fptr = fopen(file, "r"); //open file in read mode

	if(fptr == NULL){
		perror("Error opening file!\n");
		return NULL;	
	}

	int r;
	while((r = fread(fptr, &buffer, 1) > 0) //read content of file
		strcat(result, buffer);

	if(ferror(fptr)) { //check if error
		perror("Error reading from file!\n");
		return NULL;
    }
	fclose(fptr);
	return result;
}

int write_to_file(char* file, char* text){
	char* fptr;
	fptr = fopen(file, "w"); //open file in write mode; if it does not exist, it will be created
    
	if(fptr == NULL){ 
		perror("Error opening file!\n");
		return errno;	
	}

	int w = fwrite(text, 1, sizeof(text), fptr); //write text to file
	if(w < 0){
		perror("Error writing to file!\n");
		return errno;
	}
	fclose(fptr);
	return 0;
}

pid_t read_pid(){
	pid_t result;
	char* r = read_from_file(daemon_pid_path);

	if(r == NULL)
		return (pid_t)0;

	intmax_t xmax;
	char* tmp;

	//convert string to pid_t;
	errno = 0;
	xmax = strtoimax(r, &tmp, 10);
	if(errno != 0 || tmp == r || *tmp != '\0' || xmax != (pid_t)xmax){
		perror("Bad PID!\n");
		return (pid_t)0;
	}
	result = (pid_t)xmax;
    
	return result;
}

void write_instruction_to_daemon(char* instruction){   
    create_dir_if_not_exists(instruction_file_path);
    // TODO: use write_to_file function instead when implemented
    /*int fd = open(instruction_file_path, O_CREAT|O_TRUNC|O_WRONLY, S_IRWXU|S_IRWXG|S_IRWXO);    
    int len = strlen(instruction);
    write(fd, instruction, len);
    close(fd);*/
    write_to_file("instruction_file_path", instruction);

    if (daemon_pid > 0){ // if the daemon_pid was read correctly from file
        // we send a signal to the daemon to inform it that a command has been written to file 
        kill(daemon_pid, SIGUSR1); 
        // we wait a second for the daemon to process the instruction and give the
        // signal back
        sleep(1);
    } else {
        fprintf(stderr, "Couldn't send instruction to daemon because it is not running\n");
        exit(-1);
    }
}

pid_t read_daemon_pid_from_file() {
    int fd = open(daemon_pid_file_path, O_RDONLY);
     if(fd < 0){ // daemon never started yet
        map_init(tasks, 10);
        return -1;
    }
    char *buf = (char*) malloc(10); // pid from file with maximum length of 10
    read(fd, buf, 10);
    close(fd);

    return atoi(buf);
}

void start_daemon() {
    system("./disk-analyzer-daemon");
}

bool is_daemon_running() {
    daemon_pid = read_daemon_pid_from_file();
    return daemon_pid > 0;
}

void start_daemon_if_not_running() {
    if(!is_daemon_running()){
        start_daemon();
        daemon_pid = read_daemon_pid_from_file();
    }
}

void process_output_from_daemon(int signo) {
    // TODO: use read_from_file function instead when implemented
    /*int fd = open(daemon_pid_file_path, O_RDONLY);
     if(fd < 0){
        perror("Couldn't open daemon output file\n");
        return;
    }
    // TODO: change 4000 to the exact number of bytes of the file
    char *buf = (char*) malloc(4000);
    read(fd, buf, 4000);
    close(fd); */
    char* buf = read_from_file("daemon_pid_file_path");

    // print output to console
    printf("%s\n", buf);
}

void write_pid_to_file(){
    create_dir_if_not_exists(da_pid_file_path);
    // get pid and convert to char*
    char* pidstring = malloc(10);
    int pid = getpid();
    itoa(pid, pidstring);
    // TODO: use write_to_file function instead when implemented
    int fd = open(da_pid_file_path, O_CREAT|O_TRUNC|O_WRONLY, S_IRWXU|S_IRWXG|S_IRWXO);    
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
    write_pid_to_file();

    start_daemon_if_not_running();

    // TODO: maybe change 500 to a constant and put it in constants.h
    char* instruction = malloc(500);
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
                
                // TODO: add validation for priority and path
                if (!strcmp(argv[4], "1"))
                    strcpy(priority, "low");
                else if (!strcmp(argv[4], "2"))
                    strcpy(priority, "medium");
                else if (!strcmp(argv[4], "3"))
                    strcpy(priority, "high");

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
            // TODO: add validation for id
            // SUSPEND command
            sprintf(instruction, "%s\n%s\n", SUSPEND, argv[2]);
        } else if (!strcmp(argv[1], "-R") || !strcmp(argv[1], "--resume")){
            // TODO: add validation for id
            // RESUME command
            sprintf(instruction, "%s\n%s\n", RESUME, argv[2]);
        } else if (!strcmp(argv[1], "-r") || !strcmp(argv[1], "--remove")){
            // TODO: add validation for id
            // REMOVE command
            sprintf(instruction, "%s\n%s\n", REMOVE, argv[2]);
        } else if (!strcmp(argv[1], "-i") || !strcmp(argv[1], "--info")){
            // TODO: add validation for id
            // INFO command
            sprintf(instruction, "%s\n%s\n", INFO, argv[2]);
        } else if (!strcmp(argv[1], "-p") || !strcmp(argv[1], "--print")){   
            // TODO: add validation for id
            // PRINT command
            sprintf(instruction, "%s\n%s\n", PRINT, argv[2]);
        }
    }
    write_instruction_to_daemon(instruction);

    return 0;
}
