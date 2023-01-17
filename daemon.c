#include <stdio.h>
#include <string.h>
#include <fcntl.h> // for open flags
#include <signal.h>
#include <unistd.h>               // for write/close/sleep functions
#include "process_instructions.h" // header with functions for processing the instructions


void write_output_to_da(char *output){
    // TODO: use write_to_file function instead when implemented
    int fd = open(output_file_path, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    write(fd, output, 1000000);
    close(fd);
}

void process_input_from_da(int signo){
    log_daemon("Reached function process_input_from_da\n");
    // read da pid from file
    // TODO: use read_from_file function instead when implemented
    int fd = open(da_pid_file_path, O_RDONLY);
    char *buf = (char *)malloc(10); // pid from file with maximum length of 10
    read(fd, buf, 10);
    close(fd);

    // TODO: change 1000000/500 to the exact number of bytes needed
    char *res = (char *)malloc(1000000);
    char *instruction = (char *)malloc(500);
    fd = open(instruction_file_path, O_RDONLY);
    read(fd, instruction, 500);
    get_output_of_instruction(instruction, res);
    log_daemon(res);
    write_output_to_da(res);

    // send finish signal to da
    int pid = atoi(buf);
    kill(pid, SIGUSR2);
}

void init(){   
    tasks = malloc(sizeof(struct my_map));
    map_init(tasks, 10);
    list_init();
    signal(SIGUSR1, process_input_from_da);
    // delete old content of log file
    // int fd = open(log_file_path, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    // if (fd < 0){
    //     perror("Couldn't open log file\n");
    // }
    // write(fd, "\0", 1);
    // close(fd);
}

int main()
{    
    init();

    // write daemon pid to file
    create_dir_if_not_exists(daemon_pid_file_path);
    // TODO: use write_to_file function instead when implemented
    int fd = open(daemon_pid_file_path, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0)
    {
        perror("Couldn't open daemon input file\n");
        return errno;
    }
    char *pidstring = malloc(10);
    int pid = getpid();
    itoa(pid, pidstring);
    write(fd, pidstring, 10);
    close(fd);

    struct thr_node *current = (struct thr_node*)malloc(sizeof(struct thr_node));
    void *res;
    while (1){
        // wait for all threads to finish
        /*char *r = malloc(10);
        list_print(list_head, r);
        log_daemon(r);*/
        current = *list_head;   
        while(current != NULL){
            pthread_join(*current->thr, &res);
            current = current->next;
        }
    }

    return 0;
}