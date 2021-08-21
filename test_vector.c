#include "gc.h"
#include "stack.h"
#include "util.h"

int
main ()
{
  stack_t stack = make_stack ();
  stack_push (&stack);

  cr_int size = 10;

  cr_object grid = alloc_vector (size);
  assert (IS_VECTOR (grid), "GRID is not a vector");

  bind (&stack, grid, make_interned_symbol_s ("grid"));

  /* Generate a simple multiplication table. */
  for (int i = 1; i <= size; i++)
    {
      cr_object column = alloc_vector (size);
      assert (IS_VECTOR (column), "COLUMN is not a vector");
      vector_set (grid, i - 1, column);

      for (int j = 1; j <= size; j++)
        {
          cr_object product = alloc_integer (i * j);
          assert (IS_INTEGER (product), "PRODUCT is not an integer");
          vector_set (column, j - 1, product);
        }
    }

  /* Go deeper to prevent entry from dangling after GC runs. */
  {
    int row = 5, column = 5;
    int product = row * column;

    cr_object entry = vector_get (vector_get (grid, row - 1), column - 1);
    assert (IS_INTEGER (entry), "ENTRY is not an integer");
    assert (AS_INTEGER (entry) == product, "Product calculated incorrectly");
  }

  stack_pop (&stack);
  run_gc (&memory);

  assert (stack_top (&stack) == NULL, "Stack not empty after pop");
  assert (!memory.objects_count, "Orphaned objects remain in memory");

  free_stack (&stack);
  free_memory ();

  return 0;
}
