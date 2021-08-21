#ifndef GC_H
#define GC_H

#include "types.h"

typedef struct memory_t memory_t;
struct memory_t
{
  cr_object* objects;
  int objects_count, capacity;
};

extern memory_t memory;

void free_memory ();

cr_object alloc_integer (cr_int);
cr_object alloc_real (cr_real);
cr_object alloc_char (cr_char);
cr_object alloc_interned_symbol (cr_char*, cr_int);
cr_object alloc_uninterned_symbol ();
cr_object alloc_cons (cr_object, cr_object);
cr_object alloc_vector (cr_int);

void list_append (cr_object*, cr_object);

void run_gc ();

#endif /* GC_H */
