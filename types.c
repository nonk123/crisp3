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
      cr_object_symbol symbol = AS_SYMBOL (object);

      if (IS_INTERNED (symbol))
        {
          assert_non_null (symbol->name);
          free (symbol->name);
          symbol->name = NULL;
        }
    }

  free (object);
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
          own_object (CAR (object), object);
          object = CDR (object);
        }
    }
  else if (IS_VECTOR (object))
    {
      object->ref_count++;

      for (int i = 0; i < VECTOR_LENGTH (object); i++)
        own_object (VECTOR_DATA (object)[i], object);
    }
  else
    object->ref_count++;
}

void
own_object (cr_object object, cr_object owner)
{
  for (int i = 0; i < owner->ref_count; i++)
    borrow_object (object);
}

void
disown_object (cr_object object, cr_object owner)
{
  for (int i = 0; i < owner->ref_count; i++)
    return_object (object);
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
      for (int i = 0; i < VECTOR_LENGTH (object); i++)
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
  assert (index >= 0 && index < VECTOR_LENGTH (vector), "Index out of bounds");

  return VECTOR_DATA (vector)[index];
}

void
vector_set (cr_object vector, cr_int index, cr_object new_value)
{
  assert_non_null (vector);
  assert_non_null (new_value);

  assert (IS_VECTOR (vector), "Passed a non-vector object");
  assert (index >= 0 && index < VECTOR_LENGTH (vector), "Index out of bounds");

  cr_object old_value = VECTOR_DATA (vector)[index];
  disown_object (old_value, vector);

  own_object (new_value, vector);
  VECTOR_DATA (vector)[index] = new_value;
}
