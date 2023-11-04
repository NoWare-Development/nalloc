/*
 * nAlloc is an arena allocator written in C
 * Made by Nyvyme in 2023
 */

// write "#define NALLOC_IMPLEMENTATION to use it in your project"

#ifndef NALLOC_H
#define NALLOC_H

#include <stddef.h>

typedef struct _block block;
typedef struct _arena arena;

void nalloc_arena_init(arena *a);
void nalloc_arena_free(arena *a);
void *nalloc_arena_alloc(arena *a, size_t size);
void *nalloc_arena_calloc(arena *a, size_t nmemb, size_t size);

#endif // NALLOC_H

#ifdef NALLOC_IMPLEMENTATION

#define NALLOC_ARENA_DEFAULT_BLOCK_SIZE 10 * 1024

#include <stdlib.h>
#include <string.h>
#include <stddef.h>

typedef struct _block {
  struct _block *prev;
  size_t size, used;
  char data[];
} block;

typedef struct _arena {
  block *blocks;
  size_t block_size;
} arena;

union align {
  int i;
  long l;
  long *lp;
  void *p;
  void (*fp)(void);
  float f;
  double d;
  long double ld;
};

union header {
  arena arena;
  union align a;
};

static inline block *new_block(size_t size, block *prev) {
  block *b = malloc(sizeof(union header) + size);

  if (b == NULL) {
    fprintf(stderr, "new_block(): failed to allocate %zu bytes\n", size);
    exit(1);
  }

  b->size = size;
  b->used = 0;
  b->prev = prev;
  return b;
}

static inline void free_blocks(block *b) {
  while (b) {
    block *prev = b->prev;
    free(b);
    b = prev;
  }
}

void nalloc_arena_init(arena *a, size_t szblock) {
  a->blocks = new_block(szblock, NULL);
  a->block_size = szblock;
}

void nalloc_arena_free(arena *a) {
  free_blocks(a->blocks);
  a->blocks = NULL;
}

void *nalloc_arena_alloc(arena *a, size_t size) {
  size = (size + sizeof(union align) - 1) /
    sizeof(union align) * sizeof(union align);
  if (a->blocks->used + size > a->blocks->size) {
    a->blocks = new_block(size + a->block_size, a->blocks);
  }

  a->blocks->used += size;
  return (void*)(a->blocks->data + a->blocks->used - size);
}

void *nalloc_arena_calloc(arena *a, size_t nmemb, size_t size) {
  void *p = nalloc_arena_alloc(a, nmemb * size);
  memset(p, 0, nmemb * size);
  return p;
}

#endif // NALLOC_IMPLEMENTATION
