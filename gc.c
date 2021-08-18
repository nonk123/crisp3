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
  return base;
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
borrow_object (cr_object object)
{
  assert_non_null (object);

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
