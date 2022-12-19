#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include "list.h"


struct element * create_new_element(uint64_t data_ptr, int id){
    struct element * element = (struct element *)malloc(sizeof(struct element));
    element->next = NULL;
    element->id = id;
    element->data_ptr = data_ptr;
    return element;
}

struct list * list_init(){
    struct list * list = (struct list *)malloc(sizeof(struct list));
    pthread_mutex_init(&list->lock, NULL);
    struct element * element = create_new_element(0, 0);
    element->prev = NULL;
    list->head = element;
    list->tail = element;
    list->length = 1;
    return list;
}

void list_insert(struct list * list, uint64_t data_ptr, int id){

    pthread_mutex_lock(&list->lock);
    struct element * element = create_new_element(data_ptr, id);
    element->prev = list->tail;
    list->tail->next = element;
    list->length += 1;
    list->tail = element;
    pthread_mutex_unlock(&list->lock);

}

void insert_before_last(struct list * list, uint64_t data_ptr, int id){
    
    pthread_mutex_lock(&list->lock);
    if(list->length == 1){
        return;
    }
    struct element * element = create_new_element(data_ptr, id);
    struct element * second_last = list->tail->prev;
    second_last->next = element;
    element->prev = second_last;
    element->next = list->tail;
    list->tail->prev = element;
    list->length += 1;
    pthread_mutex_unlock(&list->lock);

}

int list_delete(struct list * list, int id){
    pthread_mutex_lock(&list->lock);
    for(struct element * element = list->head; element != NULL; element = element->next){
        if(element->id == id){
            if(element->next == NULL){
                list->tail = element->prev;
            }
            struct element * back = element->prev;
            element->prev->next = element->next;
            element->next->prev = back;
            free(element);
            list->length -= 1;
            pthread_mutex_unlock(&list->lock);
            return 0;
        }
    }
    pthread_mutex_unlock(&list->lock);
    return -1;
}

void print_list(struct list * list){
    
    pthread_mutex_lock(&list->lock);
    for(struct element * element = list->head; element != NULL; element = element->next){
        printf("%d ", element->id);
    }
    printf("\n");
    pthread_mutex_unlock(&list->lock);

}
