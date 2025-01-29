#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "string.h"

typedef uint32_t size_t;

#define HEAP_CAP (2048 * 2048)

typedef struct Block {
        uint32_t size;
        bool     free;
} Block_t;

typedef struct {
        uint8_t  raw[HEAP_CAP];
        uint32_t next_free;
} Heap_t;

Heap_t* prog = (Heap_t*)(0x2000000);
Heap_t* heap = (Heap_t*)(0xF00000);

void init_heap()
{
        Block_t* initial_block = (Block_t*)heap->raw;
        initial_block->size = HEAP_CAP - sizeof(Block_t); // Remaining heap after Block metadata
        initial_block->free = true;
        heap->next_free = 0;
        initial_block = (Block_t*)prog->raw;
        initial_block->size = HEAP_CAP - sizeof(Block_t); // Remaining heap after Block metadata
        initial_block->free = true;
        heap->next_free = 0;
        printf("Heap Initialized\n");
}

void* malloc(size_t size)
{
        if (size == 0 || size > HEAP_CAP) {
                return NULL;
        }

        uint32_t i = heap->next_free;
        while (i < HEAP_CAP) {
                Block_t* block = (Block_t*)&heap->raw[i];

                if (block->free && block->size >= size) {
                        // Allocate this block
                        size_t remaining_space = block->size - size - sizeof(Block_t);
                        if (remaining_space > sizeof(Block_t)) {
                                // Split the block if there's enough space for a new block
                                Block_t* new_block = (Block_t*)&heap->raw[i + sizeof(Block_t) + size];
                                new_block->size = remaining_space;
                                new_block->free = true;

                                block->size = size;
                        }
                        block->free = false;

                        heap->next_free = i + sizeof(Block_t) + block->size;
                        return &heap->raw[i + sizeof(Block_t)];
                }

                i += sizeof(Block_t) + block->size;
        }
        return NULL;
}

void * ualloc(size_t size)
{
        Heap_t * tmp = heap;
        heap = prog;
        void * x = malloc(size);
        heap = tmp;
        return x;
}

void free(void* ptr) {
        if (ptr == NULL || ptr < (void*)heap->raw || ptr >= (void*)(heap->raw + HEAP_CAP)) {
                return;
        }

        uint32_t offset = (uint8_t*)ptr - heap->raw - sizeof(Block_t);
        Block_t* block = (Block_t*)&heap->raw[offset];

        block->free = true;

        // Merge adjacent free blocks
        uint32_t i = 0; // Start from the beginning to find all possible merges
        while (i < HEAP_CAP) {
                Block_t* current = (Block_t*)&heap->raw[i];
                uint32_t next_offset = i + sizeof(Block_t) + current->size;
                if (next_offset >= HEAP_CAP) break; // Prevent out-of-bounds access

                Block_t* next = (Block_t*)&heap->raw[next_offset];

                if (current->free && next->free) {
                        current->size += sizeof(Block_t) + next->size; // Merge the next block
                } else {
                        i = next_offset;
                }
        }

        // Ensure `next_free` is properly updated
        if (offset < heap->next_free) {
                heap->next_free = offset;
        }
}

void ufree(void * ptr)
{
        Heap_t * tmp = heap;
        heap = prog;
        free(ptr);
        heap = tmp;
}

size_t remaining_heap_space()
{
        size_t total_free = 0;
        uint32_t i = 0;
        while (i < HEAP_CAP) {
                Block_t* block = (Block_t*)&heap->raw[i];
                if (block->free) {
                        total_free += block->size;
                }
                i += sizeof(Block_t) + block->size;
        }
        return total_free;
}

size_t uremain()
{
        Heap_t * tmp = heap;
        heap = prog;
        size_t x = remaining_heap_space();
        heap = tmp;
        return x;
}

void * realloc(void* ptr, size_t size)
{
        if (ptr == NULL) {
                return malloc(size);
        }

        if (size == 0) {
                free(ptr);
                return NULL;
        }

        uint32_t offset = (uint8_t*)ptr - heap->raw - sizeof(Block_t);
        Block_t* block = (Block_t*)&heap->raw[offset];

        if (size <= block->size) {
                return ptr; // No need to reallocate
        }

        Block_t* next_block = (Block_t*)&heap->raw[offset + sizeof(Block_t) + block->size];
        if (next_block->free && (block->size + sizeof(Block_t) + next_block->size >= size)) {
                // Expand the block if adjacent block is free and large enough
                block->size += sizeof(Block_t) + next_block->size;
                block->free = false;
                return ptr;
        }

        // Allocate a new block if no adjacent space is available
        void* new_ptr = malloc(size);
        if (new_ptr) {
                memcpy(new_ptr, ptr, block->size);
                free(ptr);
        }
        return new_ptr;
}

void * urealloc(void* ptr, size_t size)
{
        Heap_t * tmp = heap;
        heap = prog;
        void * x = realloc(ptr,size);
        heap = tmp;
        return x;
}
