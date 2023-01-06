#ifndef LIST
#define LIST

struct element{
    int id;
    uint64_t data_ptr;
    struct element * next;
    struct element * prev;
};

struct list{
    struct element * head;
    struct element * tail;
    int length;
    pthread_mutex_t lock;
};

struct list * list_init();
void list_insert(struct list * list, uint64_t data_ptr, int id);
int list_delete(struct list * list, int id);
void print_list(struct list * list);
void insert_before_last(struct list * list, uint64_t data_ptr, int id);

#endif