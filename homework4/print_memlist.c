#include <ctype.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

typedef struct _mblock_t {
    struct _mblock_t* prev;
    struct _mblock_t* next;
    size_t size;
    int status;
    void* payload;
} mblock_t;

typedef struct _mlist_t {
    mblock_t* head;
} mlist_t;

void printMemList(const mblock_t* headptr);

int main(int argc, char* argv[]) { return 0; }

void printMemList(const mblock_t* head) {
    const mblock_t* p = head;

    size_t i = 0;
    while (p != NULL) {
        printf("[%ld] p: %p\n", i, (void*)p);
        printf("[%ld] p->size: %ld\n", i, p->size);
        printf("[%ld] p->status: %s\n", i,
               p->status > 0 ? "allocated" : "free");
        printf("[%ld] p->prev: %p\n", i, (void*)p->prev);
        printf("[%ld] p->next: %p\n", i, (void*)p->next);
        printf("___________________________\n");
        ++i;
        p = p->next;
    }
    printf("===========================\n");
}