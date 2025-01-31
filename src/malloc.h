#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "string.h"

typedef uint32_t size_t;

#include <stddef.h>
#include <stdint.h>

#define HEAP_BASE 0x2000000
#define HEAP_SIZE 1024*1024
#define ALIGNMENT 4
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))
typedef struct Block {
    size_t size;
    struct Block *next;
} Block;

static Block *free_list = (Block *)HEAP_BASE;

void init_heap(void) {
    free_list = (Block *)HEAP_BASE;
    free_list->size = HEAP_SIZE;
    free_list->next = NULL;
}

void * malloc(size_t size) {
        if (free_list == NULL || free_list->size == 0) {
                init_heap();
        }

        size_t total_size = ALIGN(size) + ALIGN(sizeof(Block));
        Block *prev = NULL;
        Block *curr = free_list;
        while (curr) {
                if (curr->size >= total_size) {
                        if (curr->size >= total_size + sizeof(Block) + ALIGNMENT) {
                                Block *new_block = (Block *)((uint8_t *)curr + total_size);
                                new_block->size = curr->size - total_size;
                                new_block->next = curr->next;
                                if (prev) {
                                        prev->next = new_block;
                                } else {
                                        free_list = new_block;
                                }
                                curr->size = total_size;
                        } else {
                                if (prev) {
                                        prev->next = curr->next;
                                } else {
                                        free_list = curr->next;
                                }
                        }
                        return (void *)((uint8_t *)curr + ALIGN(sizeof(Block)));
                }
                prev = curr;
                curr = curr->next;
        }
        return NULL;
}

void free(void *ptr) {
        if (!ptr)
                return;
        Block *block = (Block *)((uint8_t *)ptr - ALIGN(sizeof(Block)));
        Block *prev = NULL;
        Block *curr = free_list;
        while (curr && curr < block) {
                prev = curr;
                curr = curr->next;
        }
        if (prev && (uint8_t *)prev + prev->size == (uint8_t *)block) {
                prev->size += block->size;
                block = prev;
        } else {
                block->next = curr;
                if (prev) {
                prev->next = block;
                } else {
                free_list = block;
                }
        }
        if (curr && (uint8_t *)block + block->size == (uint8_t *)curr) {
                block->size += curr->size;
                block->next = curr->next;
        }
}

size_t remaining_heap_space(void) {
        size_t free_space = 0;
        Block *curr = free_list;
        while (curr) {
                free_space += curr->size;
                curr = curr->next;
        }
        return free_space;
}
