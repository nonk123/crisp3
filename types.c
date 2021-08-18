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

  free (object);
}

cr_symbol
make_interned_symbol (const char* name)
{
  assert_non_null (name);

  cr_symbol symbol;

  symbol.name = safe_calloc (MAX_SYMBOL_LENGTH, sizeof *symbol.name);
  strcpy (symbol.name, name);

  symbol.type = INTERNED;
  symbol.length = 0;

  return symbol;
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
