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

typedef struct symbol_alist_t symbol_alist_t;
struct symbol_alist_t
{
  cr_object symbol;
  cr_object value;
  symbol_alist_t* next;
};

symbol_alist_t* alloc_symbol_alist (cr_object, cr_object);
void free_symbol_alist (symbol_alist_t*);

cr_object alloc_integer (cr_int);
cr_object alloc_real (cr_real);
cr_object alloc_char (cr_char);
cr_object alloc_interned_symbol (cr_char*, cr_int);
cr_object alloc_symbol_s (const char*);
cr_object alloc_uninterned_symbol ();
cr_object alloc_cons (cr_object, cr_object);
cr_object alloc_vector (cr_int);

void list_append (cr_object*, cr_object);

void run_gc ();

#endif /* GC_H */
