#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int task_id;
struct my_map *tasks;

int get_next_task_id(){
    return ++task_id;
}

void create_dir_if_not_exists(const char *path){
    struct stat st = {0};
    if (stat(path, &st) == -1) {
        mkdir("/tmp/disk-analyzer", 0777);
    }
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

void map_clear(struct my_map *m){
    for(int i = 0; i < m->length; i++)
        free(m->lista[i]);
    free(m->lista);
}

typedef struct thr_node {
    int id;
    
    int priority;

    pthread_t thr;
 
    struct thr_node* next;
 
} thr_Node;
 
Node* newNode(int id, int p, pthread_t thr)
{
    thr_Node* aux = (thr_Node*)malloc(sizeof(thr_Node));
    aux->id = id;
    aux->priority = p;
    aux->thr = thr;
    aux->next = NULL;
 
    return aux;
}
 
// Removes the element with the
// highest priority from the list
void pop(thr_Node** head)
{
    thr_Node* aux = *head;
    (*head) = (*head)->next;
    free(aux);
}

void push(thr_Node** head, int d, int p, pthread_t thr)
{
    thr_Node* start = (*head);
 
    thr_Node* aux = newNode(d, p, thr);
 
    // If the head of list has lesser
    // priority than new node, insert new
    // node before head node and change head node.
    if ((*head)->priority > p) {
        aux->next = *head;
        (*head) = aux;
    }
    else {
 
        // Traverse the list and find the
        // position to insert the new node
        while (start->next != NULL &&
            start->next->priority < p) {
            start = start->next;
        }
 
        // Either at the end of the list
        // or at required position
        aux->next = start->next;
        start->next = aux;
    }
}
 
// Function to check if list is empty
int isEmpty(thr_Node** head)
{
    return (*head) == NULL;
}

void* reverse (char *v)
{
      char *str = (char *) v, *s = str, *f = str + strlen(str) - 1;
      while (s < f)
      {
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

#endif