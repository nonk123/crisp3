#include <stdlib.h>

#include "gc.h"
#include "util.h"

cr_object
alloc_base (memory_t* memory, int type)
{
  assert_non_null (memory);

  cr_object base = safe_malloc (sizeof *base);

  base->type = type;
  base->ref_count = 0;

  memory_own (memory, base);

  return base;
}

cr_object
alloc_integer (memory_t* memory, cr_int value)
{
  cr_object base = alloc_base (memory, INTEGER);
  base->integer = value;
  return base;
}

cr_object
alloc_real (memory_t* memory, cr_real value)
{
  cr_object base = alloc_base (memory, REAL);
  base->real = value;
  return base;
}

cr_object
alloc_char (memory_t* memory, cr_char value)
{
  cr_object base = alloc_base (memory, CHARACTER);
  base->character = value;
  return base;
}

cr_object
alloc_interned_symbol (memory_t* memory, cr_char* name, cr_int length)
{
  cr_object base = alloc_base (memory, SYMBOL);
  base->symbol = make_interned_symbol (name, length);
  return base;
}

cr_object
alloc_uninterned_symbol (memory_t* memory)
{
  cr_object base = alloc_base (memory, SYMBOL);
  base->symbol = make_uninterned_symbol ();
  return base;
}

cr_object
alloc_cons (memory_t* memory, cr_object car, cr_object cdr)
{
  cr_object base = alloc_base (memory, CONS);
  base->cons = (cr_cons){ .car = car, .cdr = cdr };
  return base;
}

cr_object
alloc_vector (memory_t* memory, cr_int capacity)
{
  cr_object base = alloc_base (memory, VECTOR);

  base->vector.capacity = capacity;
  base->vector.data = safe_calloc (capacity, sizeof (cr_object));

  for (int i = 0; i < capacity; i++)
    VECTOR_DATA (base)[i] = NIL;

  return base;
}

void
list_append (memory_t* memory, cr_object* list, cr_object value)
{
  assert_non_null (list);
  assert_non_null (*list);
  assert_non_null (value);

  if (IS_NIL (*list))
    {
      *list = alloc_cons (memory, value, NIL);
      return;
    }

  cr_object tail = NIL;
  cr_object current = *list;

  while (!IS_NIL (current))
    {
      assert (IS_CONS (current), "Expected nil (empty list) or a cons cell");
      tail = current;
      current = CDR (current);
    }

  CDR (tail) = alloc_cons (memory, value, NIL);
  own_object (tail, CDR (tail));
}

memory_t
make_memory ()
{
  memory_t memory;
  memory.objects = NULL;
  memory.objects_count = memory.capacity = 0;
  return memory;
}

void
free_memory (memory_t* memory)
{
  assert_non_null (memory);

  if (memory->objects != NULL)
    {
      free (memory->objects);
      memory->objects = NULL;
    }

  memory->capacity = 0;
  memory->objects_count = 0;
}

#define MEMORY_EXPANSION 1024

void
memory_own (memory_t* memory, cr_object object)
{
  assert_non_null (memory);
  assert_non_null (object);

  if (memory->objects_count == memory->capacity)
    {
      int capacity_before = memory->capacity;
      cr_object* objects_before = memory->objects;

      memory->capacity += MEMORY_EXPANSION;
      memory->objects = safe_calloc (memory->capacity, sizeof *objects_before);

      for (int i = 0; i < memory->capacity; i++)
        {
          if (i >= capacity_before)
            memory->objects[i] = NULL;
          else
            memory->objects[i] = objects_before[i];
        }

      free (objects_before);
    }

  for (int i = 0; i < memory->capacity; i++)
    {
      if (memory->objects[i] == NULL)
        {
          memory->objects[i] = object;
          memory->objects_count++;
          break;
        }
    }
}

#undef MEMORY_EXPANSION

void
run_gc (memory_t* memory)
{
  assert_non_null (memory);

  for (int i = 0; i < memory->capacity; i++)
    {
      cr_object object = memory->objects[i];

      if (object != NULL && object->ref_count == 0)
        {
          free_object (object);
          memory->objects[i] = NULL;
          memory->objects_count--;
        }
    }
}
