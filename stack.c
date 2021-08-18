#include <stdlib.h>

#include "gc.h"
#include "stack.h"
#include "util.h"

symbol_list_t*
alloc_symbol_list (cr_symbol symbol, cr_object value)
{
  assert_non_null (value);

  symbol_list_t* symbol_list = safe_malloc (sizeof *symbol_list);
  assert_mem (symbol_list);

  symbol_list->symbol = symbol;
  symbol_list->value = value;
  symbol_list->next = NULL;

  return symbol_list;
}

void
free_stack_frame (stack_frame_t* stack_frame)
{
  assert_non_null (stack_frame);

  symbol_list_t* symbol_list = stack_frame->symbols;

  while (symbol_list != NULL)
    {
      return_object (symbol_list->value);
      free_symbol (&symbol_list->symbol);

      symbol_list_t* tmp = symbol_list;
      symbol_list = symbol_list->next;
      free (tmp);
    }
}

stack_t
make_stack ()
{
  stack_t stack;

  stack.frames = safe_calloc (STACK_LIMIT, sizeof *stack.frames);
  stack.length = 0;

  return stack;
}

void
free_stack (stack_t* stack)
{
  assert_non_null (stack);

  if (stack->frames != NULL)
    {
      free (stack->frames);
      stack->frames = NULL;
    }
}

void
stack_push (stack_t* stack)
{
  assert_non_null (stack);
  assert (stack->length < STACK_LIMIT, "Stack overflow");
  stack->frames[stack->length++] = (stack_frame_t){ .symbols = NULL };
}

void
stack_pop (stack_t* stack)
{
  assert_non_null (stack);
  free_stack_frame (stack_top (stack));
  stack->length--;
}

stack_frame_t*
stack_top (stack_t* stack)
{
  assert_non_null (stack);

  if (stack->length == 0)
    return NULL;

  return &stack->frames[stack->length - 1];
}

void
bind (stack_t* stack, cr_object object, cr_symbol target)
{
  assert_non_null (stack);
  assert_non_null (object);

  stack_frame_t* frame = stack_top (stack);
  assert (frame != NULL, "Stack is empty");

  symbol_list_t* new_binding = safe_malloc (sizeof *new_binding);

  borrow_object (object);

  new_binding->symbol = target;
  new_binding->value = object;
  new_binding->next = NULL;

  symbol_list_t* tail = NULL;
  symbol_list_t* current = frame->symbols;

  while (current != NULL)
    {
      tail = current;
      current = current->next;
    }

  if (tail == NULL)
    frame->symbols = new_binding;
  else
    tail->next = new_binding;
}
