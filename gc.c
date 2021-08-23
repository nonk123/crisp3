#include <stdlib.h>
#include <string.h>

#include "gc.h"
#include "util.h"

memory_t memory = { NULL, 0, 0 };

void
free_memory ()
{
  if (memory.objects != NULL)
    {
      free (memory.objects);
      memory.objects = NULL;
    }

  memory.objects_count = 0;
  memory.capacity = 0;
}

symbol_alist_t*
alloc_symbol_alist (cr_object symbol, cr_object value)
{
  assert_non_null (value);

  symbol_alist_t* symbol_alist = safe_malloc (sizeof *symbol_alist);
  assert_mem (symbol_alist);

  borrow_object (symbol);
  symbol_alist->symbol = symbol;

  borrow_object (value);
  symbol_alist->value = value;

  symbol_alist->next = NULL;

  return symbol_alist;
}

void
free_symbol_alist (symbol_alist_t* alist)
{
  assert_non_null (alist);

  return_object (alist->symbol);
  return_object (alist->value);

  if (alist->next != NULL)
    free_symbol_alist (alist->next);

  free (alist);
}

#define MEMORY_EXPANSION 1024

void
memory_own (cr_object object)
{
  assert_non_null (object);

  if (memory.objects_count == memory.capacity)
    {
      int capacity_before = memory.capacity;
      cr_object* objects_before = memory.objects;

      memory.capacity += MEMORY_EXPANSION;
      memory.objects = safe_calloc (memory.capacity, sizeof *objects_before);

      for (int i = 0; i < memory.capacity; i++)
        {
          if (i >= capacity_before)
            memory.objects[i] = NULL;
          else
            memory.objects[i] = objects_before[i];
        }

      free (objects_before);
    }

  for (int i = 0; i < memory.capacity; i++)
    {
      if (memory.objects[i] == NULL)
        {
          memory.objects[i] = object;
          memory.objects_count++;
          break;
        }
    }
}

#undef MEMORY_EXPANSION

cr_object
alloc_base (int type, size_t size)
{
  cr_object base = safe_malloc (size);

  base->type = type;
  base->ref_count = 0;

  memory_own (base);

  return base;
}

cr_object
alloc_integer (cr_int value)
{
  cr_object_int object = AS_INTEGER (alloc_base (INTEGER, sizeof *object));
  object->value = value;
  return AS_OBJECT (object);
}

cr_object
alloc_real (cr_real value)
{
  cr_object_real object = AS_REAL (alloc_base (REAL, sizeof *object));
  object->value = value;
  return AS_OBJECT (object);
}

cr_object
alloc_char (cr_char value)
{
  cr_object_char object
      = AS_CHARACTER (alloc_base (CHARACTER, sizeof *object));
  object->value = value;
  return AS_OBJECT (object);
}

cr_object
alloc_interned_symbol (cr_char* name, int length)
{
  assert_non_null (name);

  cr_object_symbol object = AS_SYMBOL (alloc_base (SYMBOL, sizeof *object));

  assert (length <= MAX_SYMBOL_LENGTH, "Symbol length exceeds the limit");
  object->name = safe_calloc (length, sizeof *object->name);

  for (int i = 0; i < length; i++)
    object->name[i] = name[i];

  object->type = INTERNED;
  object->length = length;

  return AS_OBJECT (object);
}

cr_object
alloc_symbol_s (const char* name)
{
  int length = strlen (name) - 1;
  cr_char* converted = safe_calloc (length, sizeof (*converted));

  for (int i = 0; i < length; i++)
    converted[i] = name[i];

  cr_object object = alloc_interned_symbol (converted, length);
  free (converted);

  return object;
}

cr_object
alloc_uninterned_symbol ()
{
  cr_object_symbol object = AS_SYMBOL (alloc_base (SYMBOL, sizeof *object));

  object->type = UNINTERNED;
  object->name = NULL;
  object->length = 0;

  return AS_OBJECT (object);
}

cr_object
alloc_cons (cr_object car, cr_object cdr)
{
  cr_object_cons object = AS_CONS (alloc_base (CONS, sizeof *object));

  object->car = car;
  object->cdr = cdr;

  return AS_OBJECT (object);
}

cr_object
alloc_vector (cr_int length)
{
  cr_object_vector object = AS_VECTOR (alloc_base (VECTOR, sizeof *object));

  object->data = safe_calloc (length, sizeof (cr_object));
  object->length = length;

  for (int i = 0; i < length; i++)
    VECTOR_DATA (object)[i] = NIL;

  return AS_OBJECT (object);
}

void
list_append (cr_object* list, cr_object value)
{
  assert_non_null (list);
  assert_non_null (*list);
  assert_non_null (value);

  if (IS_NIL (*list))
    {
      *list = alloc_cons (value, NIL);
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

  CDR (tail) = alloc_cons (value, NIL);
  own_object (tail, CDR (tail));
}

void
run_gc ()
{
  for (int i = 0; i < memory.capacity; i++)
    {
      cr_object object = memory.objects[i];

      if (object != NULL && object->ref_count == 0)
        {
          free_object (object);
          memory.objects[i] = NULL;
          memory.objects_count--;
        }
    }
}
