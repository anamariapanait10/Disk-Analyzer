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
#include "utils.h"

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
/*
tasks
{
    1: (1, "/home/ana"), (11, "/dsa")
    2: (2, "/home/so")
}

coada[
    (id, prioritate, th1)
]

threads_active[
    id: th1
]
threads_inactive[
    id: th1
]
*/

thr_Node *pq;
map_init(active_threads, 100);
map_init(inactive_threads, 100);

void add(char *path, char *priority, pthread_t *thr, char *res)
{
    int id = get_next_task_id();
    map_insert(tasks, id, path);
    push(&pq, id, atoi(*(priority)), thr);
    map_insert(active_threads, id, thr);
    sprintf(res, "Created analysis task with ID %d for '%s' and priority '%s'.\n", id, path, priority);
}

void print(int id, char *res)
{
}

int compare(const FTSENT **one, const FTSENT **two)
{
    return (strcmp((*one)->fts_name, (*two)->fts_name));
}

int disk_analyzer(char *path)
{
    size_t max_path = pathconf(".", _PC_PATH_MAX);
    char *buf = (char *)malloc(max_path);

    int fd = open(output_file_path, O_CREAT | O_TRUNC | O_WRONLY | O_APPEND, S_IRWXU | S_IRWXG | S_IRWXO);

    struct my_map m;
    map_init(&m, 10);
    // postorder
    FTS *file_system = NULL;
    FTSENT *node = NULL;

    char *paths[] = {path, NULL};

    if (!(file_system = fts_open(paths, FTS_COMFOLLOW | FTS_NOCHDIR, &compare)))
    {
        perror(NULL);
        return errno;
    }

    if (file_system != NULL)
    {
        while ((node = fts_read(file_system)) != NULL)
        {
            switch (node->fts_info)
            {
            case FTS_D:;
                struct fd_node *nod = map_find(&m, node->fts_statp->st_ino);
                if (nod == NULL)
                {
                    float *val = (float *)malloc(sizeof(float));
                    *val = node->fts_statp->st_size;
                    map_insert(&m, node->fts_statp->st_ino, val);
                }
                break;
            case FTS_F:
            case FTS_SL:;
                struct fd_node *nod1 = map_find(&m, node->fts_parent->fts_statp->st_ino);
                if (nod1 != NULL)
                {
                    *((float *)nod1->val) += node->fts_statp->st_size;
                }
                break;
            case FTS_DP:;
                struct fd_node *nod2 = map_find(&m, node->fts_parent->fts_statp->st_ino);
                if (nod2 != NULL)
                {
                    *((float *)nod2->val) += *((float *)map_find(&m, node->fts_statp->st_ino)->val);
                }
                break;
            default:
                break;
            }
        }
        fts_close(file_system);
    }

    // TODO: change 10000 to a better number and move it to constants file
    char *line = (char *)malloc(10000);
    // write the first line
    sprintf(line, "\tPath\tUsage\tSize\tAmount\n");
    write(fd, line, strlen(line));

    int total_size = 0;                                                                 // needed to calculate the percentage
    char *size = (char *)malloc(20);                                                    // needed to keep the size of the current directory
    char *last_dir = (char *)malloc(max_path), *current_dir = (char *)malloc(max_path); // needed for the prints of '|' between the groups of directories
    char *p = (char *)malloc(max_path);                                                 // needed to print only a part of the path string
    char *indx = (char *)malloc(max_path);                                              // needed for extracting the first directory from a path

    // preorder
    file_system = NULL;
    node = NULL;
    char *paths2[] = {path, NULL};
    if (!(file_system = fts_open(paths2, FTS_COMFOLLOW | FTS_NOCHDIR, &compare)))
    {
        perror(NULL);
        return errno;
    }

    if (file_system != NULL)
    {
        while ((node = fts_read(file_system)) != NULL)
        {
            switch (node->fts_info)
            {
            case FTS_D:
                size = convert_size_to_standard_unit(*((float *)map_find(&m, node->fts_statp->st_ino)->val));

                if (strcmp(node->fts_path, path))
                { // if current path is not root

                    // we don't print the root folder for its directories
                    strcpy(p, node->fts_path);
                    p += strlen(path); // skiping the root folder from the paths
                    strcat(p, "/");    // adding a final '/' to distinguish them as folders

                    // extract the first directory from a path in order to group them by this later
                    indx = strchr(p + 1, '/');
                    int n = (int)(indx - p) - 1;
                    strncpy(current_dir, p + 1, n);
                    current_dir[n] = '\0';

                    float percentage = (float)*((float *)map_find(&m, node->fts_statp->st_ino)->val) * 100 / (float)total_size;

                    if (strcmp(last_dir, current_dir))
                    {
                        // prints an extra line with '|' before printing the current line
                        // in order to group the directories
                        sprintf(line, "|\n|-%s\t%.1f%%\t%s\t%s\n", p, percentage, size, get_progress(percentage));
                    }
                    else
                    {
                        sprintf(line, "|-%s\t%.1f%%\t%s\t%s\n", p, percentage, size, get_progress(percentage));
                    }
                    strcpy(last_dir, current_dir);
                }
                else
                { // root directory
                    total_size = *((float *)map_find(&m, node->fts_statp->st_ino)->val);
                    sprintf(line, "%s/\t100%%\t%s\t%s\n", node->fts_path, size, get_progress(100));
                }
                write(fd, line, strlen(line));
                break;
            default:
                break;
            }
        }
        fts_close(file_system);
    }

    map_clear(&m);
    close(fd);
}

void get_output_of_instruction(char *instruction, char *res)
{
    char *command = strtok(instruction, "\n");

    switch (atoi(command))
    {
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