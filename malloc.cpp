#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <assert.h>>

namespace mymem {
    typedef struct memblock {
        size_t size;
        memblock* next;
        int free;
        int data;
    };

    #define META_SIZE sizeof(memblock)
    #define MMAP_THRESHOLD (size_t) 3*1024*1024
    memblock* base = nullptr;
    
    memblock* request_space(size_t size, memblock* tail) {
        memblock* blk;
        blk = (memblock*) sbrk(0);
        void* request;
        if (size <= MMAP_THRESHOLD) {
            request = sbrk(META_SIZE + size);
            assert((void*)blk == request);
        }
        else {
            
            request = mmap(NULL,META_SIZE+size,MAP_PRIVATE, MAP_ANONYMOUS, 0, 0);

        }
        
        if (request == (void *) -1) return nullptr;
        if (tail) tail->next = (memblock*) request;
        blk = (memblock*) request;
        blk->next = nullptr;
        blk->free = 0;
        blk->data = 0x12345678;
        blk->size = size;
        return tail;
    } 
    memblock* find_free_block(size_t size, memblock **tail) {
        memblock* cur = base;
        while (cur && !(cur->free && cur->size >= size)) {
            *tail = cur;
            cur = cur->next;
        }
        return cur;
    }
    void* malloc(size_t size) {
        if (size <= 0) nullptr;
        if (!base) {
            base = request_space(size, NULL);
            if (!base) {
                return nullptr;
            }
        }
        memblock* tail;
        memblock* blk = find_free_block(size, &tail);
        if (!blk) {
            blk = request_space(size);
            if (!blk) return nullptr;
            blk->free = 0;

            return (blk+1);
        }
        return nullptr;
    }
}