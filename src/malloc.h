#ifndef HEAP_H
#define HEAP_H
#include <stdint.h>
#include <stdbool.h>
#include "string.h"

typedef uint32_t size_t;

uint32_t HEAP_SIZE;
uint32_t HEAP_BASE;
#define ALIGNMENT 16
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

typedef struct Block 
{
        size_t size;
        struct Block *next;
} Block;

static Block *free_list = NULL;

void init_heap(void)
{
        HEAP_SIZE = MAX_ADDR / 8;
        HEAP_BASE = (MAX_ADDR - HEAP_SIZE);
        memset((void *)HEAP_BASE, 0, HEAP_SIZE);
        free_list = (Block *)HEAP_BASE;
        free_list->size = HEAP_SIZE - sizeof(Block);
        free_list->next = NULL;
}

void *malloc(size_t size)
{
        if (size == 0) return NULL;
        if (free_list == NULL)
        {
                init_heap();
        }
        size_t total_size = ALIGN(size) + ALIGN(sizeof(Block));
        Block *prev = NULL;
        Block *curr = free_list;
        while (curr)
        {
                if (curr->size >= total_size)
                {
                        if (curr->size >= total_size + sizeof(Block))
                        {
                                Block *new_block = (Block *)((uint8_t *)curr + total_size);
                                new_block->size = curr->size - total_size;
                                new_block->next = curr->next;
                                if (prev)
                                {
                                        prev->next = new_block;
                                }
                                else
                                {
                                        free_list = new_block;
                                }
                                curr->size = total_size;
                        }
                        else
                        {
                                if (prev)
                                {
                                        prev->next = curr->next;
                                }
                                else
                                {
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

void free(void *ptr)
{
        if (!ptr) return;
        Block *block = (Block *)((uint8_t *)ptr - ALIGN(sizeof(Block)));
        Block *prev = NULL;
        Block *curr = free_list;
        while (curr && curr < block)
        {
                prev = curr;
                curr = curr->next;
        }

        if (prev && (uint8_t *)prev + prev->size == (uint8_t *)block)
        {
                prev->size += block->size;
                block = prev;
        }
        else
        {
                block->next = curr;
                if (prev)
                {
                        prev->next = block;
                }
                else
                {
                        free_list = block;
                }
        }

        if (curr && (uint8_t *)block + block->size == (uint8_t *)curr)
        {
                block->size += curr->size;
                block->next = curr->next;
        }
}

size_t remaining_heap_space(void)
{
        size_t free_space = 0;
        Block *curr = free_list;
        while (curr)
        {
                free_space += curr->size;
                curr = curr->next;
        }
        return free_space;
}

#endif
