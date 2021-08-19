#include "parser.h"
#include "util.h"

void
print_object (cr_object object)
{
  switch (object->type)
    {
    case NIL_TYPE:
      printf ("nil");
      break;
    case INTEGER:
      printf ("%d", AS_INTEGER (object));
      break;
    case REAL:
      printf ("%lf", AS_REAL (object));
      break;
    case CHARACTER:
      printf ("?%d", AS_CHARACTER (object));
      break;
    case SYMBOL:
      for (int i = 0; i < SYMBOL_LENGTH (object); i++)
        putchar (SYMBOL_NAME (object)[i]);

      break;
    case CONS:
      putchar ('(');

      print_object (CAR (object));

      object = CDR (object);

      while (!IS_NIL (object))
        {
          putchar (' ');
          print_object (CAR (object));

          if (IS_CONS (object))
            object = CDR (object);
          else
            object = NIL;
        }

      putchar (')');

      break;
    case VECTOR:
      putchar ('[');

      print_object (VECTOR_DATA (object)[0]);

      for (int i = 1; i < VECTOR_CAPACITY (object); i++)
        {
          putchar (' ');
          print_object (VECTOR_DATA (object)[i]);
        }

      putchar (']');

      break;
    }
}

int
main ()
{
  memory_t memory = make_memory ();

  buffer_t buffer = make_buffer ();
  buffer_read_by_file_name (&buffer, "test.lisp");

  parser_context_t parser_ctx = make_parser_context (&memory, &buffer);

  cr_object result = parse (&parser_ctx);

  printf ("Parser result: ");
  print_object (result);
  putchar ('\n');

  run_gc (&memory);

  assert (!memory.objects_count, "Orphaned objects remain in memory");

  free_buffer (&buffer);
  free_memory (&memory);

  return 0;
}
