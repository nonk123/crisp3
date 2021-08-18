#include "gc.h"
#include "stack.h"
#include "util.h"

int
main ()
{
  memory_t memory = make_memory ();

  stack_t stack = make_stack ();
  stack_push (&stack);

  cr_object a = alloc_integer (&memory, 69000);
  cr_object b = alloc_integer (&memory, 420);

  assert (IS_INTEGER (a), "A is not an integer");
  assert (IS_INTEGER (b), "B is not an integer");

  bind (&stack, a, make_interned_symbol ("a"));
  bind (&stack, b, make_interned_symbol ("b"));

  cr_object list = alloc_cons (&memory, a, alloc_cons (&memory, b, NIL));

  assert (IS_CONS (list), "LIST is not a cons cell");
  assert (IS_CONS (CDR (list)), "CDR of LIST is not a cons cell");
  assert (IS_NIL (CDDR (list)), "LIST is unterminated");

  bind (&stack, list, make_interned_symbol ("list"));

  cr_int sum = 0;
  cr_object current = list;

  while (!IS_NIL (current))
    {
      sum += AS_INTEGER (CAR (current));
      current = CDR (current);
    }

  stack_pop (&stack);
  run_gc (&memory);

  assert (stack_top (&stack) == NULL, "Stack not empty after pop");
  assert (!memory.objects_count, "Orphaned objects remain in memory");
  assert (sum == 69420, "The sum of 69000 and 420 must be equal to 69420");

  free_memory (&memory);
  free_stack (&stack);

  return 0;
}