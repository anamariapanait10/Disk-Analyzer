#ifndef PROCESS_INSTRUCTIONS_H_INCLUDED
#define PROCESS_INSTRUCTIONS_H_INCLUDED

#include <stdio.h>  // for perror
#include <fcntl.h>  // for open flags
#include <unistd.h> // for write/close/sleep/pathconf functions
#include <string.h>
#include <fts.h>
#include <errno.h>
#include <pthread.h>
#include "utils.h"     // header for map related functions and itoa
#include "constants.h" // header for command-constants and paths

void help(char *res)
{
    sprintf(res, "%s",
            "Usage: da [OPTION]... [DIR]...\n"
            "Analyze the space occupied by the directory ai [DIR]\n"
            "\t-a, --add\t\tanalyze the new directory path for disk usage\n"
            "\t-p, --priority\t\tset priority for the new analysis (works only with -a argument)\n"
            "\t-S, --suspend <id>\t\tsuspend task with id <id>\n"
            "\t-R, --resume <id>\t\tresume task with <id>\n"
            "\t-r, --remove <id>\t\tremove the analysis with the given <id>\n"
            "\t-i, --info <id>\t\tprint status about the analysis with <id> (pending, progress, complete)\n"
            "\t-l, --list\t\tlist all analysis tasks, with their ID and the corresponding root priority\n"
            "\t-p, --print <id>\t\tprint analysis report for those tasks that are 'done'\n");
}

void add(char *path, char *priority, char *res){
    int id = get_next_task_id();
    int already_exists = map_find_task(tasks, path);
    if(already_exists == -1){
        map_insert(tasks, id, path);

        // create new thread for analyzing the disk
        pthread_t *new_thr;
        struct thread_args *args = malloc(sizeof(struct thread_args));
        args->path = path;
        args->priority = atoi(priority);
        if (pthread_create(new_thr, NULL, disk_analyzer, args)){
            perror(NULL);
            exit(-1);
        }
        
        list_insert(list_head, id, atoi(priority), new_thr);

        sprintf(res, "Created analysis task with ID %d for '%s' and priority '%s'.\n", id, path, priority);
    } else {
        sprintf(res, "Directory '%s' is already included in analysis with ID '%d'", path, already_exists);
    }
}


void print(int id, char *res){   
    struct thr_node *n = list_find_by_key(list_head, id);
    if(strcmp(n->done_status, "done") == 0){
        char *output_path, *msg;
        sprintf(output_path, "%s%d%s", output_file_path_prefix, id, ".txt");
        sprintf(msg, "Couldn't open %s file\n", output_path);
        char *res = read_from_file(output_path, msg);
    } else {
        sprintf(res, "Print analysis report available only for those tasks that are 'done'");
    }
}

void list(){
    printf("ID\tPRI\tPath\tDone\tStatus\tDetails");
    for(int i = 0; i < tasks->length; i++){
        if(tasks->lista[i] != NULL){
            struct fd_node *nod;
            for (nod = tasks->lista[i]; nod != NULL; nod = nod->next) {
                struct thr_node *n = list_find_by_key(list_head, nod->id);
                char *priority = (char*)malloc(3);
                priority = "***";
                priority += 3 - n->priority;
                struct fd_node *node = map_find(tasks, nod->id);
                char *path = (char*)node->val;
                printf("%d\t%s\t%s\t%s\t%d files, %d dirs) ", nod->id, priority, path, n->done_status, n->files, n->dirs);
            }
        }
        printf("\n");
    }
}

void get_output_of_instruction(char *instruction, char *res){
    char *command = strtok(instruction, "\n");

    switch (atoi(command)){
        case 0: // HELP command
            help(res);
            return;
        case 1: // ADD command
            ;
            char *path = strtok(NULL, "\n");
            char *priority = strtok(NULL, "\n");
            add(path, priority, res);
            break;
        case 2: // SUSPEND commnad
            break;
        case 3: // RESUME commnad
            break;
        case 4: // REMOVE commnad
            break;
        case 5: // INFO commnad
            break;
        case 6: // LIST commnad
            list();
            break;
        case 7: // PRINT commnad
            ;
            int id = atoi(strtok(NULL, "\n"));
            print(id, res);
            break;
        default:
            break;
    }
}

#endif