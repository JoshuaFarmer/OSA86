#ifndef HEAP_H
#define HEAP_H

#include <stdint.h>
#include <stdbool.h>
#include "string.h"

typedef uint32_t size_t;

uint32_t HEAP_SIZE;
uint32_t HEAP_BASE;
#define ALIGNMENT 128
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

static uint8_t *BITMAP = NULL;
static size_t   BITMAP_SIZE = 0;

typedef struct
{
        void    *d;
        uint32_t sz;
} MAPPED;

void init_heap(void)
{
        HEAP_SIZE   = MAX_ADDR / 8;
        HEAP_BASE   = MAX_ADDR - HEAP_SIZE;
        BITMAP_SIZE = (HEAP_SIZE + ALIGNMENT - 1) / ALIGNMENT;
        BITMAP_SIZE = (BITMAP_SIZE + 7) & ~7;
        BITMAP      = (uint8_t *)(HEAP_BASE - (BITMAP_SIZE / 8));
        memset((void *)HEAP_BASE, 0, HEAP_SIZE);
        memset((void *)BITMAP, 0, BITMAP_SIZE / 8);
        printf("HEAP Initialized\n");
}

MAPPED mmap(size_t size)
{
        MAPPED empty={0};
        if (size == 0) return empty;

        size_t aligned_size = ALIGN(size);
        size_t num_bits = aligned_size / ALIGNMENT;

        for (size_t i = 0; i < BITMAP_SIZE; i++)
        {
                if ((BITMAP[i / 8] & (1 << (i % 8)))) continue;

                bool found = true;
                for (size_t j = 0; j < num_bits; j++)
                {
                        if (i + j >= BITMAP_SIZE || (BITMAP[(i + j) / 8] & (1 << ((i + j) % 8))))
                        {
                                found = false;
                                break;
                        }
                }

                if (found)
                {
                        for (size_t j = 0; j < num_bits; j++)
                        {
                                BITMAP[(i + j) / 8] |= (1 << ((i + j) % 8));
                        }

                        MAPPED map = {.d=(void *)(HEAP_BASE + i * ALIGNMENT),.sz=aligned_size};
                        return map;
                }
        }

        return empty;
}

void umap(MAPPED map)
{
        if (!map.d) return;

        size_t index    = (int)((uint8_t *)map.d - HEAP_BASE) / ALIGNMENT;
        size_t num_bits = map.sz/ALIGNMENT;

        for (size_t i = 0; i < num_bits; i++)
        {
                BITMAP[(index + i) / 8] &= ~(1 << ((index + i) % 8));
        }
}

size_t remaining_heap_space(void)
{
        size_t free_space = 0;
        for (size_t i = 0; i < BITMAP_SIZE; i++)
        {
                if (!(BITMAP[i >> 3] & (1 << (i % 8))))
                {
                        free_space += ALIGNMENT;
                }
        }
        return free_space;
}

void *malloc(size_t sz)
{
        if (sz == 0) return NULL;
        MAPPED map = mmap(sz+4);
        if (!map.d)  return NULL;
        *(uint32_t *)map.d = map.sz;
        map.d += 4;
        return map.d;
}

void free(void *d)
{
        MAPPED map;
        map.d  = (void *)((int)d-4);
        map.sz = *(uint32_t *)map.d;
        umap(map);
}

#endif
