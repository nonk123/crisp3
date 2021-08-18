#include <stdlib.h>

#include "buffer.h"
#include "util.h"

buffer_t
make_buffer ()
{
  buffer_t buffer;

  buffer.data = NULL;
  buffer.capacity = buffer.length = 0;

  return buffer;
}

void
free_buffer (buffer_t* buffer)
{
  assert_non_null (buffer);

  buffer->capacity = buffer->length = 0;

  if (buffer->data != NULL)
    {
      free (buffer->data);
      buffer->data = NULL;
    }
}

#define EXPAND_AMOUNT 128

void
buffer_push (buffer_t* buffer, char character)
{
  assert_non_null (buffer);

  if (buffer->length >= buffer->capacity)
    {
      buffer->capacity += EXPAND_AMOUNT;
      buffer->data = safe_realloc (buffer->data,
                                   buffer->capacity * sizeof *buffer->data);
    }

  buffer->data[buffer->length++] = character;
}

#undef EXPAND_AMOUNT

void
buffer_read (buffer_t* buffer, FILE* fh)
{
  assert_non_null (buffer);
  assert_non_null (fh);

  char current;

  while ((current = fgetc (fh)) != EOF)
    buffer_push (buffer, current);
}

void
buffer_read_by_file_name (buffer_t* buffer, const char* file_name)
{
  assert_non_null (buffer);
  assert_non_null (file_name);

  FILE* fh = fopen (file_name, "r");
  assert_non_null (fh);

  buffer_read (buffer, fh);
  fclose (fh);
}
