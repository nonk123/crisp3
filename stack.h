#ifndef STACK_H
#define STACK_H

#include "types.h"

#define STACK_LIMIT 1024

typedef struct symbol_list_t symbol_list_t;
struct symbol_list_t
{
  cr_symbol symbol;
  cr_object value;
  symbol_list_t* next;
};

typedef struct stack_frame_t stack_frame_t;
struct stack_frame_t
{
  symbol_list_t* symbols;
};

typedef struct stack_t stack_t;
struct stack_t
{
  stack_frame_t* frames;
  int length;
};

symbol_list_t* alloc_symbol_list (cr_symbol, cr_object);

void free_stack_frame (stack_frame_t*);

stack_t make_stack ();
void free_stack (stack_t*);

void stack_push (stack_t*);
void stack_pop (stack_t*);

stack_frame_t* stack_top (stack_t*);

void bind (stack_t*, cr_object, cr_symbol);

#endif /* STACK_H */
