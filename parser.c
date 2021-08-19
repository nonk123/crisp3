#include "parser.h"
#include "util.h"

#include <math.h>

parser_context_t
make_parser_context (memory_t* memory, buffer_t* buffer)
{
  parser_context_t ctx;

  ctx.row = ctx.column = ctx.position = 0;
  ctx.f_line_end = ctx.f_comment = ctx.f_content = 0;

  ctx.memory = memory;
  ctx.buffer = buffer;

  return ctx;
}

#define HAS_NEXT (ctx->position < ctx->buffer->length)
#define CURRENT (ctx->buffer->data[ctx->position])
#define PEEK(N)                                                               \
  (ctx->position + (N) >= 0 ? (ctx->position + (N) < ctx->buffer->length      \
                                   ? ctx->buffer->data[ctx->position + (N)]   \
                                   : ctx->buffer->data[ctx->position - 1])    \
                            : ctx->buffer->data[0])
#define IS_STRING_LITERAL (CURRENT == '\"' && PEEK (-1) != '\\')
#define IS_SEPARATOR                                                          \
  (CURRENT == ' ' || CURRENT == '\t' || CURRENT == '\r' || CURRENT == '\n')

void
advance (parser_context_t* ctx)
{
  ctx->position += 1;

  if (ctx->f_line_end)
    {
      ctx->row++;
      ctx->column = 0;
    }
  else
    ctx->column += 1;
}

cr_object
parse_list (parser_context_t* ctx)
{
  cr_object result = NIL;
  int terminated = 0;

  while (HAS_NEXT)
    {
      list_append (ctx->memory, &result, parse (ctx));

      if (CURRENT == ')')
        {
          advance (ctx);
          terminated = 1;
          break;
        }
    }

  assert (terminated, "No matching ) found for the list");
  return result;
}

cr_object
parse_string (parser_context_t* ctx)
{
  buffer_t buffer = make_buffer ();

  while (HAS_NEXT)
    {
      if (IS_STRING_LITERAL)
        {
          ctx->f_in_string = 0;
          advance (ctx);
          break;
        }

      if (CURRENT == '\\')
        {
          advance (ctx);

          switch (CURRENT)
            {
            case 'n':
              buffer_push (&buffer, '\n');
              break;
            case 'r':
              buffer_push (&buffer, '\r');
              break;
            case 't':
              buffer_push (&buffer, '\t');
              break;
            case '\\':
              buffer_push (&buffer, '\\');
              break;
            case '"':
              buffer_push (&buffer, '"');
              break;
            default:
              error ("Unknown escape sequence");
            }
        }

      /* TODO: use cr_char for buffer characters. */
      buffer_push (&buffer, CURRENT);
      advance (ctx);
    }

  assert (!ctx->f_in_string, "Unterminated string literal");

  cr_object result = alloc_vector (ctx->memory, buffer.length);

  for (int i = 0; i < buffer.length; i++)
    {
      cr_object char_object = alloc_char (ctx->memory, buffer.data[i]);
      VECTOR_DATA (result)[i] = char_object;
    }

  free_buffer (&buffer);

  return result;
}

cr_object
parse_number (parser_context_t* ctx)
{
  buffer_t buffer = make_buffer ();
  int is_real = 0; /* integer if 0 */

  cr_int integer = 0;

  cr_real real = 0.0;
  cr_real fractional_part = 0.0;

  cr_int sign = 0;

  if (CURRENT == '+')
    {
      sign = 1;
      advance (ctx);
    }
  else if (CURRENT == '-')
    {
      sign = -1;
      advance (ctx);
    }
  else
    {
      assert (CURRENT >= '0' && CURRENT <= '9', "Expected digit, '+', or '-'");
      sign = 1;
    }

  int starting_position = ctx->position;
  int fp_position = -1;

  while (HAS_NEXT)
    {
      if (CURRENT == '.')
        {
          assert (fp_position == -1, "Unexpected '.'");
          fp_position = ctx->position;
          is_real = 1;
        }
      else
        {
          if (IS_SEPARATOR || CURRENT < '0' || CURRENT > '9')
            break;

          buffer_push (&buffer, CURRENT);
        }

      advance (ctx);
    }

  if (fp_position == -1)
    fp_position = ctx->position;

  int fp_offset = fp_position - starting_position;

  if (is_real)
    {
      for (int i = 0; i < fp_offset; i++)
        {
          real *= 10.0;
          real += buffer.data[i] - '0';
        }

      for (int i = fp_offset; i < buffer.length; i++)
        {
          fractional_part *= 10.0;
          fractional_part += buffer.data[i] - '0';
        }

      real += fractional_part / pow (10.0, buffer.length - fp_offset);
    }
  else
    {
      for (int i = 0; i < buffer.length; i++)
        {
          integer *= 10;
          integer += buffer.data[i] - '0';
        }
    }

  free_buffer (&buffer);

  cr_object number = is_real ? alloc_real (ctx->memory, real * sign)
                             : alloc_integer (ctx->memory, integer * sign);
  return number;
}

cr_object
parse_symbol (parser_context_t* ctx)
{
  buffer_t buffer = make_buffer ();

  while (HAS_NEXT)
    {
      if (!((CURRENT >= 'a' && CURRENT <= 'z')
            || (CURRENT >= 'A' && CURRENT <= 'z') || CURRENT == '!'
            || (CURRENT >= '#' && CURRENT <= '%')
            || (CURRENT >= '*' && CURRENT <= ':')
            || (CURRENT >= '<' && CURRENT <= '@') || CURRENT == '^'
            || CURRENT == '_' || CURRENT == '~'))
        break;

      /* TODO: use cr_char for buffer characters. */
      buffer_push (&buffer, CURRENT);
      advance (ctx);
    }

  assert (buffer.length > 0, "Parsed an empty identifier");

  cr_object symbol
      = alloc_interned_symbol (ctx->memory, buffer.data, buffer.length);
  free_buffer (&buffer);
  return symbol;
}

cr_object
parse (parser_context_t* ctx)
{
  while (HAS_NEXT)
    {
      /* Handle CRLF vs CR vs LF equally. */
      ctx->f_line_end = (CURRENT == '\r' && PEEK (1) == '\n')
                        || (CURRENT == '\n' && PEEK (-1) != '\r')
                        || CURRENT == '\r';

      if (ctx->f_line_end)
        ctx->f_comment = 0;

      if (CURRENT == ';' && !ctx->f_in_string)
        ctx->f_comment = 1;

      ctx->f_content = !ctx->f_line_end && !ctx->f_comment
                       && (CURRENT != ' ' && CURRENT != '\t');

      if (!ctx->f_content)
        {
          advance (ctx);
          continue;
        }

      if (IS_STRING_LITERAL)
        {
          advance (ctx);
          return parse_string (ctx);
        }

      if (CURRENT == '(')
        {
          advance (ctx);
          return parse_list (ctx);
        }

      if ((CURRENT >= '0' && CURRENT <= '9')
          || ((CURRENT == '-' || CURRENT == '+')
              && (PEEK (1) >= '0' && PEEK (1) <= '9')))
        return parse_number (ctx);

      return parse_symbol (ctx);
    }

  assert (HAS_NEXT, "Reached end of file");
  return NULL;
}
