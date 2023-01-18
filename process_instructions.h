#ifndef PROCESS_INSTRUCTIONS_H_INCLUDED
#define PROCESS_INSTRUCTIONS_H_INCLUDED

#include <stdio.h>  // for perror
#include <fcntl.h>  // for open flags
#include <unistd.h> // for write/close/sleep/pathconf functions
#include <string.h>
#include <fts.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h> // for malloc
#include <limits.h>
#include "utils.h"

void da_help(char *res){
    log_daemon("Reached da_help function\n");
    sprintf(res, "%s",
            "Usage: da [OPTION]... [DIR]...\n"
            "Analyze the space occupied by the directory ai [DIR]\n"
            "\t-a, --add\t\tanalyze the new directory path for disk usage\n"
            "\t-p, --priority\t\tset priority for the new analysis (works only with -a argument)\n"
            "\t-S, --suspend <id>\tsuspend task with id <id>\n"
            "\t-R, --resume <id>\tresume task with <id>\n"
            "\t-r, --remove <id>\tremove the analysis with the given <id>\n"
            "\t-i, --info <id>\t\tprint status about the analysis with <id> (pending, progress, done)\n"
            "\t-l, --list\t\tlist all analysis tasks, with their ID and the corresponding root path\n"
            "\t-p, --print <id>\tprint analysis report for those tasks that are 'done'\n");
}

void da_add(char *path, char *priority, char *res){
    log_daemon("Reached da_add function\n");
    int id = get_next_task_id();
    int already_exists = map_find_task(tasks, path);
    char *sir = malloc(100);
    map_print_char(tasks, sir);
    log_daemon(sir);
    pthread_t *new_thr = malloc(sizeof(pthread_t));
    if(already_exists == -1){
        map_insert(tasks, id, path);
        log_daemon("Will create thread\n");
        // create new thread for analyzing the disk
        struct thread_args *args = malloc(sizeof(struct thread_args));
        args->path = path;
         char nr[10];
        sprintf(nr, "%d\n",atoi(priority));
        log_daemon(priority);
        args->priority = atoi(priority);
        args->id = id;
        log_daemon("Before pthread_create\n");
        char *r = malloc(10);
        list_print(list_head, r);
        log_daemon(r);
        if (pthread_create(new_thr, NULL, disk_analyzer, args)){
            char *s = malloc(30);
            sprintf(s, "Could not create thread: %d\n", errno);           
            log_daemon(s);
            perror(NULL);
        }
        //pthread_join(*new_thr, NULL);
        list_print(list_head, r);
        log_daemon(r);
        log_daemon("After pthread_create\n");
        sprintf(res, "Created analysis task with ID %d for '%s' and priority '%s'.\n", id, path, priority);
    } else {
        log_daemon("Directory is already included in analysis\n");
        sprintf(res, "Directory '%s' is already included in analysis with ID '%d'", path, already_exists);
    }
}

void da_suspend(int id, char *res){
    log_daemon("Reached da_suspend function\n");
}

void da_resume(int id, char *res){
    log_daemon("Reached da_resume function\n");
}

void da_remove(int id, char *res){
    log_daemon("Reached da_resume function\n");
    struct thr_node *n = list_find_by_key(list_head, id);
    if(n == NULL){
        sprintf(res, "No existing analysis for task ID %d, there is nothing to remove", id);
    } else {
        struct fd_node *node = map_find(tasks, n->id);
        char *path = (char*)node->val;
        int ret = pthread_cancel(*n->thr);
        if(ret == -1){
            log_daemon(res);
        } else {
            void *thr_ret;
            pthread_join(*n->thr, &thr_ret);
            if (thr_ret != PTHREAD_CANCELED){
                log_daemon(res);
            }
        }
        list_delete(list_head, id);
        map_delete(tasks, id);
        sprintf(res, "Removed analysis task with ID %d, status %s for %s", id, n->done_status, path);
    }
}

void da_info(int id, char *res){
    log_daemon("Reached da_info function\n");
    struct thr_node *n = list_find_by_key(list_head, id);
    if(n == NULL){
        sprintf(res, "No existing analysis for task ID %d, no info can be displayed", id);
    } else {
         sprintf(res, "ID\tPRI\tPath\tDone Status\tDetails\n");
         char priority[3]="***";
         struct fd_node *node = map_find(tasks, n->id);
         char *path = (char*)node->val;
         sprintf(res + strlen(res), "%d\t%s\t%s\t%s\t%d files, %d dirs\n", n->id, priority+ (3-n->priority), path, n->done_status, n->files, n->dirs);
       
        }
    }


void da_list(char *res){
    log_daemon("Reached da_list function\n");
    sprintf(res, "ID\tPRI\tPath\tDone Status\tDetails\n");
    for(int i = 0; i < tasks->length; i++){
        if(tasks->lista[i] != NULL){
            struct fd_node *nod;
            for (nod = tasks->lista[i]; nod != NULL; nod = nod->next) {
                struct thr_node *n = list_find_by_key(list_head, nod->id);
                char priority[3]="***";
                struct fd_node *node = map_find(tasks, nod->id);
                char *path = (char*)node->val;
                sprintf(res + strlen(res), "%d\t%s\t%s\t%s\t%d files, %d dirs\n", nod->id, priority+ (3-n->priority), path, n->done_status, n->files, n->dirs);
            }
        }
        
    }
}

void da_print(int id, char *res){
    log_daemon("Reached da_print function\n");
    struct thr_node *n = list_find_by_key(list_head, id);
    if(n == NULL){
        sprintf(res, "No existing analysis for task ID %d", id);
    } else if(strcmp(n->done_status, "done") == 0){
        char *output_path, *msg;
        sprintf(output_path, "%s%d.txt", output_file_path_prefix, id);
        sprintf(msg, "Couldn't open %s file\n", output_path);
        res = read_from_file(output_path, msg);
    } else {
        sprintf(res, "Print analysis report available only for those tasks that are 'done'");
    }
}

void get_output_of_instruction(char *instruction, char *res){
    char *command = strtok(instruction, "\n");
    int id;
    switch (atoi(command)){
        case 0: // HELP command
            da_help(res);
            break;
        case 1: // ADD command
            ;
            char *path = malloc(PATH_MAX);
            path = strtok(NULL, "\n");
            char *priority = malloc(PATH_MAX);
            priority = strtok(NULL, "\n");
            log_daemon(priority);
            da_add(path, priority, res);
            break;
        case 2: // SUSPEND command
            ;
            id = atoi(strtok(NULL, "\n"));
            da_suspend(id, res);
            break;
        case 3: // RESUME commnad
            ;
            id = atoi(strtok(NULL, "\n"));
            da_resume(id, res);
            break;
        case 4: // REMOVE commnad
            ;
            id = atoi(strtok(NULL, "\n"));
            da_remove(id, res);
            break;
        case 5: // INFO commnad
            ;
            id = atoi(strtok(NULL, "\n"));
            da_info(id, res);
            break;
        case 6: // LIST commnad
            da_list(res);
            break;
        case 7: // PRINT commnad
            ;
            id = atoi(strtok(NULL, "\n"));
            da_print(id, res);
            break;
        default:
            break;
    }
}

#endif