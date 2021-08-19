#include <stdlib.h>
#include <string.h>

#include "types.h"
#include "util.h"

struct cr_object_t nil = { .type = NIL_TYPE };

void
free_object (cr_object object)
{
  assert_non_null (object);
  assert (object->ref_count == 0, "Attempted to free an object in use");

  if (IS_VECTOR (object))
    {
      assert_non_null (VECTOR_DATA (object));
      free (VECTOR_DATA (object));
    }
  else if (IS_SYMBOL (object))
    {
      cr_symbol symbol = AS_SYMBOL (object);

      if (IS_INTERNED (symbol))
        {
          assert_non_null (symbol.name);
          free (symbol.name);
        }
    }

  free (object);
}

cr_symbol
make_interned_symbol (const char* name, int length)
{
  assert_non_null (name);

  cr_symbol symbol;

  assert (length <= MAX_SYMBOL_LENGTH, "Symbol length exceeds the limit");
  symbol.name = safe_calloc (length, sizeof *symbol.name);
  strncpy (symbol.name, name, length);

  symbol.type = INTERNED;
  symbol.length = length;

  return symbol;
}

cr_symbol
make_interned_symbol_s (const char* name)
{
  return make_interned_symbol (name, strlen (name) - 1);
}

cr_symbol
make_uninterned_symbol ()
{
  cr_symbol symbol;

  symbol.type = UNINTERNED;
  symbol.name = NULL;
  symbol.length = 0;

  return symbol;
}

void
free_symbol (cr_symbol* symbol)
{
  assert_non_null (symbol);

  if (symbol->name != NULL)
    {
      free (symbol->name);
      symbol->name = NULL;
    }
}

void
borrow_object (cr_object object)
{
  assert_non_null (object);

  if (IS_NIL (object))
    return;

  if (IS_CONS (object))
    {
      while (!IS_NIL (object))
        {
          object->ref_count++;
          borrow_object (CAR (object));
          object = CDR (object);
        }
    }
  else if (IS_VECTOR (object))
    {
      for (int i = 0; i < VECTOR_CAPACITY (object); i++)
        borrow_object (VECTOR_DATA (object)[i]);

      object->ref_count++;
    }
  else
    object->ref_count++;
}

void
return_object (cr_object object)
{
  assert_non_null (object);

  if (IS_NIL (object))
    return;

  if (IS_CONS (object))
    {
      while (!IS_NIL (object))
        {
          object->ref_count--;
          return_object (CAR (object));
          object = CDR (object);
        }
    }
  else if (IS_VECTOR (object))
    {
      for (int i = 0; i < VECTOR_CAPACITY (object); i++)
        return_object (VECTOR_DATA (object)[i]);

      object->ref_count--;
    }
  else
    object->ref_count--;
}

cr_object
vector_get (cr_object vector, cr_int index)
{
  assert_non_null (vector);
  assert (IS_VECTOR (vector), "Passed a non-vector object");
  assert (index >= 0 && index < VECTOR_CAPACITY (vector),
          "Index out of bounds");

  return VECTOR_DATA (vector)[index];
}

void
vector_set (cr_object vector, cr_int index, cr_object new_value)
{
  assert_non_null (vector);
  assert_non_null (new_value);

  assert (IS_VECTOR (vector), "Passed a non-vector object");
  assert (index >= 0 && index < VECTOR_CAPACITY (vector),
          "Index out of bounds");

  cr_object old_value = VECTOR_DATA (vector)[index];
  return_object (old_value);

  borrow_object (new_value);
  VECTOR_DATA (vector)[index] = new_value;
}
