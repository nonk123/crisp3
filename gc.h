#ifndef GC_H
#define GC_H

#include "types.h"

typedef struct memory_t memory_t;
struct memory_t
{
  cr_object* objects;
  int objects_count, capacity;
};

cr_object alloc_integer (memory_t*, cr_int);
cr_object alloc_real (memory_t*, cr_real);
cr_object alloc_char (memory_t*, cr_char);
cr_object alloc_interned_symbol (memory_t*, cr_char*, cr_int);
cr_object alloc_uninterned_symbol (memory_t*);
cr_object alloc_cons (memory_t*, cr_object, cr_object);
cr_object alloc_vector (memory_t*, cr_int);

void list_append (memory_t*, cr_object*, cr_object);

memory_t make_memory ();
void free_memory (memory_t*);

void memory_own (memory_t*, cr_object);

void run_gc (memory_t*);

#endif /* GC_H */
