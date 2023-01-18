#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fts.h>
#include <pthread.h>
#include "constants.h"

int task_id;
struct my_map *tasks;
struct thr_node **list_head;


pthread_mutex_t mtx_lock;

/*
tasks
{   
    0: (10, "/ceva")
    1: (1, "/home/ana"), (11, "/dsa")
    2: (2, "/home/so")
}

lista[
    (id, prioritate, th1, done_status, files, dirs, total_dirs, details)
]
*/


void log_daemon(char *msg){
    //pthread_mutex_lock(pthread_mutex_t *mutex)
    int fd = open(log_file_path, O_CREAT | O_APPEND | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    if (fd < 0){
        perror("Couldn't open log file\n");
    }
    write(fd, msg, strlen(msg));
    close(fd);
}

int get_next_task_id(){
    return ++task_id;
}

void create_dir_if_not_exists(const char *path){
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mode_t oldmask = umask(0);
        mkdir("/tmp/disk-analyzer", 0777);
        umask(oldmask);
    }
}

int compare(const FTSENT **one, const FTSENT **two){
    return (strcmp((*one)->fts_name, (*two)->fts_name));
}

int count_dirs(char *path){
    int count = 0;
    FTS *file_system = NULL;
    FTSENT *node = NULL;

    char *paths[] = {path, NULL};

    if (!(file_system = fts_open(paths, FTS_COMFOLLOW | FTS_NOCHDIR, &compare))){
        perror(NULL);
        return errno;
    }
    log_daemon("Inainte de parcurgere\n");
    if (file_system != NULL){
        while ((node = fts_read(file_system)) != NULL){
            switch (node->fts_info){
                case FTS_D:;
                    count++;
                    break;
                default:
                    break;
            }
        }
        fts_close(file_system);
    }
    log_daemon("Dupa parcurgere\n");
    return count;
}

// hash
struct fd_node {
    int id;
    void *val;
    struct fd_node *next;
};

struct my_map {
    int length;
    struct fd_node **lista;
};

void map_init(struct my_map *m, int n) {
    m->length = n;
    m->lista = (struct fd_node **) malloc(n * sizeof(struct fd_node *));
    for (int i = 0; i < n; i++)
        m->lista[i] = NULL;
}

struct fd_node *map_find(struct my_map *m, int key) {
    int mod = key % m->length;
    if (m->lista[mod] == NULL)
        return NULL;
    for (struct fd_node *nod = m->lista[mod]; nod != NULL; nod = nod->next) {
        if (nod->id == key)
            return nod;
    }
    return NULL;
}

int map_find_task(struct my_map *m, char *path){
    for(int i = 0; i < m->length; i++){
        if(m->lista[i] != NULL){
            for (struct fd_node *nod = m->lista[i]; nod != NULL; nod = nod->next) {
                if (strcmp((char*)nod->val, path) == 0)
                    return nod->id;
            }
        }
    }
    return -1;
}

void map_insert(struct my_map *m, int key, void *val) {
    int mod = key % m->length;
    if (m->lista[mod] == NULL) {
        m->lista[mod] = (struct fd_node *) malloc(sizeof(struct fd_node *));
        m->lista[mod]->val = val;
        m->lista[mod]->id = key;
        m->lista[mod]->next = NULL;
    } else {
        struct fd_node *nod;
        for (nod = m->lista[mod]; nod->next != NULL; nod = nod->next) {
            if (nod->id == key) {
                nod->val = val;
                return;
            }
        }
        if (nod->id == key) {
            nod->val = val;
            return;
        }
        nod->next = (struct fd_node *) malloc(sizeof(struct fd_node));
        nod->next->next = NULL;
        nod->next->id = key;
        nod->next->val = val;
    }
}
// for debugging
void map_print_int(struct my_map *m){
    for(int i = 0; i < m->length; i++){
        printf("%d: ", i);
        if(m->lista[i] != NULL){
            struct fd_node *nod;
            for (nod = m->lista[i]; nod != NULL; nod = nod->next) {
                printf("(%d, %d) ", nod->id, *((int*)nod->val));
            }
        }
        printf("\n");
    }
}

void map_print_char(struct my_map *m, char *res){
    for(int i = 0; i < m->length; i++){
        sprintf(res, "%d: ", i);
        if(m->lista[i] != NULL){
            struct fd_node *nod;
            for (nod = m->lista[i]; nod != NULL; nod = nod->next) {
                sprintf(res, "(%d, %s) ", nod->id, (char*)nod->val);
            }
        }
        sprintf(res, "\n");
    }
}

void map_clear(struct my_map *m){
    for(int i = 0; i < m->length; i++)
        free(m->lista[i]);
    free(m->lista);
}

struct thr_node {
    int id;
    int priority;
    pthread_t *thr;
    char *done_status;
    int files, dirs, total_dirs;
    struct thr_node *next;
};

void list_init(){
    list_head = malloc(sizeof(struct thr_node*));
    *list_head = NULL;
}

void list_insert(struct thr_node **head_ref, int id, int priority, pthread_t *thr){
     pthread_mutex_lock(&mtx_lock);
    struct thr_node *new_node = (struct thr_node*) malloc(sizeof(struct thr_node));
   
    new_node->id = id;
    new_node->priority = priority;
    new_node->thr = thr;
    new_node->done_status = (char*)malloc(15);
    new_node->done_status = "preparing";
    new_node->files = 0;
    new_node->dirs = 0;
    // find out total number of subdirectories
    struct fd_node *node = map_find(tasks, id);
    char *path = (char*)(node->val);
    new_node->total_dirs = count_dirs(path);

    new_node->next = *head_ref;
    *head_ref = new_node;
     pthread_mutex_unlock(&mtx_lock);

}

void list_delete(struct thr_node **head_ref, int key){
    struct thr_node *aux = (struct thr_node*)malloc(sizeof(struct thr_node));
    aux = *list_head;
    struct thr_node *prev = (struct thr_node*)malloc(sizeof(struct thr_node));

    if(aux != NULL && aux->id == key){
        *head_ref = aux->next;
        free(aux);
    } else {
        while(aux != NULL && aux->id != key){
            prev = aux;
            aux = aux->next;
        }

        if(aux != NULL){
            prev->next = aux->next;
            free(aux);
        }
    }
}

struct thr_node* list_find_by_key(struct thr_node** head_ref, int key){
    struct thr_node *current = (struct thr_node*)malloc(sizeof(struct thr_node));
    current = *list_head;
    while(current != NULL){
        if(current->id == key){
            return current;
        }
        current = current->next;
    }
    return NULL;
}

struct thr_node* list_find_by_thr(struct thr_node** head_ref, pthread_t thr){
    struct thr_node *current = (struct thr_node*)malloc(sizeof(struct thr_node));
    current = *list_head;
    while(current != NULL){
        if(pthread_equal(*(current->thr), thr)){
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void list_print(struct thr_node** head_ref, char *res){
    res[0] = '\0';
    struct thr_node *current = (struct thr_node*)malloc(sizeof(struct thr_node));
    current = *list_head;
    while(current != NULL){
        sprintf(res + strlen(res), "Id: %d, ", current->id);
        current = current->next;
    }
    sprintf(res + strlen(res), "\n");
}

void* reverse (char *v){
      char *str = (char *) v, *s = str, *f = str + strlen(str) - 1;
      while (s < f){
            *s ^= *f;
            *f ^= *s;
            *s ^= *f;
            s++;
            f--;
      }
      return str;
}

void itoa(int n, char *s){
    int i = 0;
    // generate digits in reverse order
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0); 

    s[i] = '\0';
    reverse(s);
}

char *get_progress(float percent){
    char *res = (char*)malloc(51);
    res = "##################################################";
    int number_of_hashtags = percent * 50 / 100;
    res += (50-number_of_hashtags);
    return res;
}

char *convert_size_to_standard_unit(float bytes){
    char *size = (char*)malloc(20);
    if(bytes >= 1024){ // KB
        bytes /= 1024;
        sprintf(size, "%.1f", bytes);
        strcat(size, "KB");
    }

    if(bytes >= 1024){ // MB
        bytes /= 1024;
        sprintf(size, "%.1f", bytes);
        strcat(size, "MB");
    }

    if(bytes >= 1024){ // GB
        bytes /= 1024;
        sprintf(size, "%.1f", bytes);
        strcat(size, "GB");
    }

    if(bytes >= 1024){ // TB
        bytes /= 1024;
        sprintf(size, "%.1f", bytes);
        strcat(size, "TB");
    }

    return size;
}


char *read_from_file(const char *path, char *error_msg){
    int fd = open(path, O_RDONLY);
    if(fd < 0){
        log_daemon("Eroare deschidere fisier");
        //exit(-1);
    }

    struct stat sb;
    if(stat(path, &sb)) {
        perror(path);
        //exit(-1);
    }

    char *buf = (char*) malloc(sb.st_size);
    read(fd, buf, sb.st_size);
    close(fd);

    return buf;
}

void update_done_status(struct thr_node *node){
    if(node->total_dirs == 0 ){}
    float percent = node->dirs * 100 * 1./ node->total_dirs;
    if(percent == 100){
        node->done_status = "done";
    } else {
        sprintf(node->done_status, "%.1f%% in progress", percent);
    }
}

struct thread_args {
    char *path;
    int priority, id;
};

void* disk_analyzer(void *args){   
    log_daemon("Reached disk_analyzer function\n");
    struct thread_args *th_args = (struct thread_args*)args;
    char *path = th_args->path;
    int id = (int)th_args->id, pri = (int)th_args->priority;

    log_daemon("Before malloc\n");
    // TODO add mutex for this
    pthread_t *pth = malloc(sizeof(pthread_t));
    *pth = pthread_self();
    log_daemon("Before list_insert\n");
     char nr[10];
    sprintf(nr, "%d\n",pri);
    log_daemon(nr);
    list_insert(list_head, id, pri, pth);

    log_daemon("Before set priority to thread\n");
    // set thread priority
    int policy;
    struct sched_param param;
    pthread_getschedparam(pthread_self(), &policy, &param);
    param.sched_priority = sched_get_priority_max(policy) - (th_args->priority-1);
    pthread_setschedparam(pthread_self(), policy, &param);
    log_daemon("After set priority to thread\n");

    size_t max_path = pathconf(".", _PC_PATH_MAX);
    char *buf = (char *)malloc(max_path);

    struct thr_node *n = list_find_by_thr(list_head, pthread_self());
    log_daemon("After list_find_by_thr\n");
    char *output_path = (char *)malloc(max_path);
    log_daemon("Before output_path malloc\n");
    char *s = malloc(1000);
    list_print(list_head, s);
    log_daemon(s);

    sprintf(output_path, "%s_%d.txt", output_file_path_prefix, n->id);   
    
    log_daemon("After output_path malloc\n");
    int fd = open(output_path, O_CREAT | O_TRUNC | O_WRONLY, S_IRWXU | S_IRWXG | S_IRWXO);
    if(fd < 0){
        log_daemon("Could not open output path\n");
        perror("Could not open output path");
       // exit(-1);
    }
    log_daemon("After output_path open\n");
    struct my_map m;
    map_init(&m, 10);
    // postorder
    FTS *file_system = NULL;
    FTSENT *node = NULL;

    char *paths[] = {path, NULL};

    if (!(file_system = fts_open(paths, FTS_COMFOLLOW | FTS_NOCHDIR, &compare))){
        perror(NULL);
       // exit(-1);
    }
    log_daemon("Before postorder traversal\n");
    sprintf(s, "%d", (file_system==NULL));
    log_daemon(s);
    if (file_system != NULL){
        while ((node = fts_read(file_system)) != NULL){
            //sprintf(s, "%d", node->fts_info);
            //log_daemon(s);
            
            switch (node->fts_info){
                case FTS_D:;
                    struct fd_node *nod = map_find(&m, node->fts_statp->st_ino);
                    if (nod == NULL){
                        float *val = (float *)malloc(sizeof(float));
                        *val = node->fts_statp->st_size;
                        map_insert(&m, node->fts_statp->st_ino, val);
                    }
                    break;
                case FTS_F:
                case FTS_SL:;
                    struct fd_node *nod1 = map_find(&m, node->fts_parent->fts_statp->st_ino);
                    if (nod1 != NULL){
                        *((float *)nod1->val) += node->fts_statp->st_size;
                    }
                    n->files++;
                    break;
                case FTS_DP:;
                    struct fd_node *nod2 = map_find(&m, node->fts_parent->fts_statp->st_ino);
                    if (nod2 != NULL){
                        *((float *)nod2->val) += *((float *)map_find(&m, node->fts_statp->st_ino)->val);
                    }
                    n->dirs++;
                     char nr[10];
                     itoa(n->dirs,nr);
                     log_daemon(nr);
                     
                    break;
                default:
                    break;
            }
            
        }
        
        fts_close(file_system);
    }
    log_daemon("After postorder traversal\n");
    log_daemon("Before preorder traversal\n");
    // TODO: change 10000 to a better number and move it to constants file
    char *line = (char *)malloc(10000);
    // write the first line
    sprintf(line, "    Path\t\tUsage\tSize\t\tAmount\n");
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
    if (!(file_system = fts_open(paths2, FTS_COMFOLLOW | FTS_NOCHDIR, &compare))){
        perror(NULL);
      //  exit(-1);
    }

    if (file_system != NULL){
        while ((node = fts_read(file_system)) != NULL){
            switch (node->fts_info){
                case FTS_D:
                    size = convert_size_to_standard_unit(*((float *)map_find(&m, node->fts_statp->st_ino)->val));

                    if (strcmp(node->fts_path, path)){ // if current path is not root

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

                        if (strcmp(last_dir, current_dir)){
                            // prints an extra line with '|' before printing the current line
                            // in order to group the directories
                            sprintf(line, "|\n|-%s\t%.1f%%\t%s\t%s\n", p, percentage, size, get_progress(percentage));
                        } else {
                            sprintf(line, "|-%s\t%.1f%%\t%s\t%s\n", p, percentage, size, get_progress(percentage));
                        }
                        strcpy(last_dir, current_dir);
                    } else { // root directory
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
    char nrDir[1000];
    itoa(list_find_by_thr(list_head,pthread_self())->dirs, nrDir);
    log_daemon(nrDir);
    log_daemon("After preorder traversal\n");
    map_clear(&m);
    close(fd);
    n->done_status = "done";
    log_daemon("Finish disk_analyzer function\n");
}

void map_delete(struct my_map *m, int id){
    log_daemon("In map_delete");
    //pthread_mutex_lock(&mtx_lock);
    struct fd_node *nod = (struct fd_node*)malloc(sizeof(struct fd_node));
    nod = m->lista[id%m->length];
    struct fd_node *prev = (struct fd_node*)malloc(sizeof(struct fd_node));
    if(nod != NULL && nod->id == id){
        m->lista[id%m->length] = nod->next;
        free(nod);
    } else {
        while(nod != NULL && nod->id != id){
            prev = nod;
            nod = nod->next;
        }

        if(nod != NULL){
            prev->next = nod->next;
            free(nod);
        }
    }
    //pthread_mutex_unlock(&mtx_lock);
    log_daemon("After map_delete");
}

#endif