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

// priority queue
typedef struct node {
    int data;
 
    // Lower values indicate higher priority
    int priority;
 
    struct node* next;
 
} Node;
 
// Function to Create A New Node
Node* newNode(int d, int p)
{
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->data = d;
    temp->priority = p;
    temp->next = NULL;
 
    return temp;
}
 
// Return the value at head
int peek(Node** head)
{
    return (*head)->data;
}
 
// Removes the element with the
// highest priority from the list
void pop(Node** head)
{
    Node* temp = *head;
    (*head) = (*head)->next;
    free(temp);
}
 
// Function to push according to priority
void push(Node** head, int d, int p)
{
    Node* start = (*head);
 
    // Create new Node
    Node* temp = newNode(d, p);
 
    // Special Case: The head of list has lesser
    // priority than new node. So insert new
    // node before head node and change head node.
    if ((*head)->priority > p) {
 
        // Insert New Node before head
        temp->next = *head;
        (*head) = temp;
    }
    else {
 
        // Traverse the list and find a
        // position to insert new node
        while (start->next != NULL &&
            start->next->priority < p) {
            start = start->next;
        }
 
        // Either at the ends of the list
        // or at required position
        temp->next = start->next;
        start->next = temp;
    }
}
 
// Function to check is list is empty
int isEmpty(Node** head)
{
    return (*head) == NULL;
}

void reverse(char *s){
    int i, j;
    char aux;
    for (i = 0, j = strlen(s)-1; i < j; i++, j--) {
        aux = s[i];
        s[i] = s[j];
        s[j] = aux;
    }
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