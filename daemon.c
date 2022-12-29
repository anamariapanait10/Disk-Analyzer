#include <stdio.h>
#include <string.h>
#include <fcntl.h> // for open flags
#include <signal.h>
#include <unistd.h> // for write/close/sleep functions
#include "process_instructions.h" // header with functions for processing the instructions


void write_output_to_da(char* output){   
    // TODO: use write_to_file function instead when implemented
    int fd = open(output_file_path, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU|S_IRWXG|S_IRWXO);    
    write(fd, output, 4000);
    close(fd);
}

void process_input_from_da(int signo) {
    // read da pid from file
    // TODO: use read_from_file function instead when implemented
    int fd = open(da_pid_file_path, O_RDONLY);
    char *buf = (char*) malloc(10); // pid from file with maximum length of 10
    read(fd, buf, 10);
    close(fd);
    
    // TODO: change 1000000/500 to the exact number of bytes needed
    char *res = (char*) malloc(1000000);
    char *instruction = (char*) malloc(500);
    int fd = open(instruction_file_path, O_RDONLY);
    read(fd, instruction, 500);
    get_output_of_instruction(instruction, res);
    write_output_to_da(res);

    // send finish signal to da
    int pid = atoi(buf);
    kill(pid, SIGUSR2); 
}


static void daemonize(){
    // more info here: https://stackoverflow.com/questions/17954432/creating-a-daemon-in-linux
    pid_t pid;

    pid = fork(); // Fork off the parent process

    if (pid < 0) // An error occurred 
        exit(EXIT_FAILURE);

    if (pid > 0) // Success: Let the parent terminate
        exit(EXIT_SUCCESS);

    if (setsid() < 0) // On success: The child process becomes session leader
        exit(EXIT_FAILURE);

    // Catch, ignore and handle signals
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork(); // Fork off for the second time

    if (pid < 0) // An error occurred
        exit(EXIT_FAILURE);

    if (pid > 0) // Success: Let the parent terminate
        exit(EXIT_SUCCESS);

    // write daemon pid to file
    create_dir_if_not_exists(daemon_pid_file_path);
    // TODO: use write_to_file function instead when implemented
    int fd = open(daemon_pid_file_path, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU|S_IRWXG|S_IRWXO);    
    if(fd < 0){
        perror("Couldn't open daemon input file\n");
        return;
    }
    char* pidstring = malloc(10);
    pid = getpid();
    itoa(pid, pidstring);
    write(fd, pidstring, 10); 
    close(fd);

    signal(SIGUSR1, process_input_from_da);
}


int main() {

    daemonize();
/*
    umask(0);

    char * myfifo = "/var/run/myfifo";
    
    int fd = open(myfifo, O_CREAT | O_RDONLY | O_EXCL, S_IRWXU|S_IRWXG|S_IRWXO);
    if (fd < 0) {
        // failure
        if (errno == EEXIST) {
            // the file already existed
            fd = open(myfifo, O_RDONLY, S_IRWXU|S_IRWXG|S_IRWXO);
        }
    } else {
        if(mkfifo(myfifo, S_IRWXU|S_IRWXG|S_IRWXO)){
            perror(NULL);
        }
        fd = open(myfifo, O_RDONLY, S_IRWXU|S_IRWXG|S_IRWXO);
    }
    char* buf = (char*)malloc(sizeof(char)*4000);
    buf[0]='\0';
    read(fd, buf, 4000);
    close(fd);

    fisier = (char *)malloc(sizeof(buf));

    strcpy(fisier, buf);
    strcat(fisier, "/output.txt");

    disk_analyzer(buf);
*/
    // fd = open("/var/run/output.txt", O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU|S_IRWXG|S_IRWXO);
    // write(fd,buf, 4000);
    // close(fd);

    return 0;
}