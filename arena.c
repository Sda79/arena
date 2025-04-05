#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>

#define DEFAULT_CAPACITY 8192

typedef struct Region Region;
typedef struct Arena Arena;

struct Region {
  Region *next;
  unsigned int capacity;
  unsigned int count;
  uintptr_t data[];
};

struct Arena {
  Region *head;
};

/*
 Creates a new region.
  - capacity : number of char to allocate for the region.
 Example :
  I want 2 char. (2 - 1/ 8) + 1 = 1 uintptr_t
  I want 64 char (64 - 1 / 8) + 1 = 8 uintptr_t
*/
Region *new_region(unsigned long long capacity) {
  Region *r;

  unsigned long long region_size = sizeof(Region) + ((capacity - 1) / sizeof(uintptr_t)) + 1;
  printf("Allocating %lu + ((%llu - 1) / %lu) + 1 = %llu char to region.\n", sizeof(Region), capacity, sizeof(uintptr_t), region_size);
  r = (Region*) malloc(region_size);
  
  r->next = NULL;
  r->capacity = capacity;
  r->count = 0;

  return r;
}

void free_region(Region *region) {
  free(region);
}

void *arena_alloc(Arena *arena, unsigned long long size) {
  
  if (arena->head == NULL) {
    printf("Empty arena allocating first region.\n");
    unsigned capacity;
    if (size < DEFAULT_CAPACITY) {
      capacity = DEFAULT_CAPACITY;
    } else {
      capacity = size;
    }
    arena->head = new_region(capacity);
  }
  
  Region *cursor = arena->head;
  while (size > cursor->capacity - cursor->count && cursor->next != NULL) {
    printf("Current region is too small trying next one\n");
    cursor = cursor->next;
  }

  if (size > cursor->capacity - cursor->count) {
    printf("No region can fit the data. Allocating new region\n");
    unsigned capacity;
    if (size < DEFAULT_CAPACITY) {
      capacity = DEFAULT_CAPACITY;
    } else {
      capacity = size;
    }
    cursor->next = new_region(capacity);
    cursor = cursor->next;
  }

  void *result = &cursor->data[(cursor->count) / sizeof(uintptr_t)];
  cursor->count += size;
  return result;
}

void free_arena(Arena *arena) {
  Region *region_to_free = arena->head;
  while(region_to_free != NULL) {
    arena->head = region_to_free->next;
    free_region(region_to_free);
    region_to_free = arena->head;
  }
}

int main() {
  Arena a = {0};
  char *str1 = (char*) arena_alloc(&a, 16);
  char *str2 = (char*) arena_alloc(&a, 16);
  
  strcpy(str2, "1234567890");
  strcpy(str1, "abcdefghijklmnopqrstuvwxyz");
  printf("str1 = %s\n", str1);
  printf("str2 = %s\n", str2);
  printf("&str1 = %p\n", str1);
  printf("&str2 = %p\n", str2);
  
  /* for (int i = 0; i < 1073741824; ++i) { */
  /*   str[i] = 'a'; */
  /* } */

  /* printf("%s\n", str); */
  
  free_arena(&a);
  
  return 0;
}
