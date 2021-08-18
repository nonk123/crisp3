#include "util.h"

void*
safe_malloc (int size)
{
  void* result = malloc (size);
  assert_mem (result);
  return result;
}

void*
safe_calloc (int count, int elt_size)
{
  void* result = calloc (count, elt_size);
  assert_mem (result);
  return result;
}

void*
safe_realloc (void* destination, int new_size)
{
  void* result = realloc (destination, new_size);
  assert_mem (result);
  return result;
}
