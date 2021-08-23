#include <stdlib.h>

#include "gc.h"
#include "stack.h"
#include "util.h"

void
free_stack_frame (stack_frame_t* stack_frame)
{
  assert_non_null (stack_frame);

  if (stack_frame->symbols != NULL)
    free_symbol_alist (stack_frame->symbols);
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
bind (stack_t* stack, cr_object object, cr_object target)
{
  assert_non_null (stack);
  assert_non_null (object);
  assert_non_null (target);

  assert (IS_SYMBOL (target), "Can only bind a value to a symbol");

  stack_frame_t* frame = stack_top (stack);
  assert (frame != NULL, "Stack is empty");

  symbol_alist_t* new_binding = alloc_symbol_alist (target, object);

  symbol_alist_t* tail = NULL;
  symbol_alist_t* current = frame->symbols;

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
