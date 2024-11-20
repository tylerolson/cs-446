#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MBLOCK_HEADER_SZ offsetof(mblock_t, payload)
#define MIN_ALLOC_SIZE 1024
#define MIN_SPLIT_SIZE (sizeof(mblock_t) + 8)

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

mlist_t mlist = {NULL};

void printMemList(const mblock_t* headptr);

void* mymalloc(size_t size);
void myfree(void* ptr);

mblock_t* findLastMemlistBlock(void);
mblock_t* findFreeBlockOfSize(size_t size);
void splitBlockAtSize(mblock_t* block, size_t newSize);
void coallesceBlockPrev(mblock_t* freedBlock);
void coallesceBlockNext(mblock_t* freedBlock);
mblock_t* growHeapBySize(size_t size);

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

void* mymalloc(size_t size) {
    if (size == 0) return NULL;

    mblock_t* block = findFreeBlockOfSize(size);

    if (block == NULL) {
        block = growHeapBySize(size);
        if (block == NULL) {
            return NULL;
        }
    }

    splitBlockAtSize(block, size);

    return &(block->payload);
}

void myfree(void* ptr) {
    if (ptr == NULL) return;

    mblock_t* block = (mblock_t*)((char*)ptr - MBLOCK_HEADER_SZ);

    void* heap_end = sbrk(0);
    if ((void*)block < (void*)mlist.head || (void*)block >= heap_end) {
        fprintf(stderr, "Invalid pointer passed to myfree\n");
        return;
    }

    block->status = 0;

    coallesceBlockNext(block);
    coallesceBlockPrev(block);
}

mblock_t* findLastMemlistBlock(void) {
    if (mlist.head == NULL) return NULL;

    mblock_t* current = mlist.head;
    while (current->next != NULL) {
        current = current->next;
    }
    return current;
}

mblock_t* findFreeBlockOfSize(size_t size) {
    mblock_t* current = mlist.head;
    while (current != NULL) {
        if (current->status == 0 && current->size >= size) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void splitBlockAtSize(mblock_t* block, size_t newSize) {
    size_t remaining_size = block->size - newSize;

    if (remaining_size >= MIN_SPLIT_SIZE) {
        mblock_t* new_block =
            (mblock_t*)((char*)block + MBLOCK_HEADER_SZ + newSize);
        new_block->size = remaining_size - MBLOCK_HEADER_SZ;
        new_block->status = 0;  // Free
        new_block->prev = block;
        new_block->next = block->next;

        if (block->next != NULL) {
            block->next->prev = new_block;
        }

        block->next = new_block;
        block->size = newSize;
    }

    block->status = 1;
}

void coallesceBlockPrev(mblock_t* freedBlock) {
    if (freedBlock->prev != NULL && freedBlock->prev->status == 0) {
        mblock_t* prev_block = freedBlock->prev;
        prev_block->size += MBLOCK_HEADER_SZ + freedBlock->size;
        prev_block->next = freedBlock->next;

        if (freedBlock->next != NULL) {
            freedBlock->next->prev = prev_block;
        }
    }
}

void coallesceBlockNext(mblock_t* freedBlock) {
    if (freedBlock->next != NULL && freedBlock->next->status == 0) {
        mblock_t* next_block = freedBlock->next;
        freedBlock->size += MBLOCK_HEADER_SZ + next_block->size;
        freedBlock->next = next_block->next;

        if (next_block->next != NULL) {
            next_block->next->prev = freedBlock;
        }
    }
}

mblock_t* growHeapBySize(size_t size) {
    size_t alloc_size = size + MBLOCK_HEADER_SZ;
    if (alloc_size < MIN_ALLOC_SIZE) {
        alloc_size = MIN_ALLOC_SIZE;
    }

    mblock_t* new_block = sbrk(alloc_size);
    if (new_block == (void*)-1) {
        return NULL;
    }

    new_block->prev = findLastMemlistBlock();
    new_block->next = NULL;
    new_block->size = alloc_size - MBLOCK_HEADER_SZ;
    new_block->status = 0;  // Free

    if (new_block->prev != NULL) {
        new_block->prev->next = new_block;
    } else {
        mlist.head = new_block;
    }

    return new_block;
}

int main(int argc, char* argv[]) {
    void* p1 = mymalloc(10);
    printf("Allocated p1 (10 bytes):\n");
    printMemList(mlist.head);

    void* p2 = mymalloc(100);
    printf("Allocated p2 (100 bytes):\n");
    printMemList(mlist.head);

    void* p3 = mymalloc(200);
    printf("Allocated p3 (200 bytes):\n");
    printMemList(mlist.head);

    void* p4 = mymalloc(500);
    printf("Allocated p4 (500 bytes):\n");
    printMemList(mlist.head);

    myfree(p3);
    p3 = NULL;
    printf("Freed p3:\n");
    printMemList(mlist.head);

    myfree(p2);
    p2 = NULL;
    printf("Freed p2:\n");
    printMemList(mlist.head);

    void* p5 = mymalloc(150);
    printf("Allocated p5 (150 bytes):\n");
    printMemList(mlist.head);

    void* p6 = mymalloc(500);
    printf("Allocated p6 (500 bytes):\n");
    printMemList(mlist.head);

    myfree(p4);
    p4 = NULL;
    printf("Freed p4:\n");
    printMemList(mlist.head);

    myfree(p5);
    p5 = NULL;
    printf("Freed p5:\n");
    printMemList(mlist.head);

    myfree(p6);
    p6 = NULL;
    printf("Freed p6:\n");
    printMemList(mlist.head);

    myfree(p1);
    p1 = NULL;
    printf("Freed p1:\n");
    printMemList(mlist.head);

    return 0;
}