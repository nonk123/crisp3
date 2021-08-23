#include "parser.h"
#include "util.h"

#include <math.h>
#include <string.h>

parser_context_t
make_parser_context (buffer_t* buffer)
{
  assert_non_null (buffer);

  parser_context_t ctx;

  ctx.row = ctx.column = ctx.position = 0;
  ctx.f_line_end = ctx.f_comment = ctx.f_content = ctx.f_in_string = 0;
  ctx.buffer = buffer;

  return ctx;
}

cr_object
parse_file (const char* name)
{
  buffer_t buffer = make_buffer ();
  buffer_read_by_file_name (&buffer, name);

  parser_context_t ctx = make_parser_context (&buffer);
  cr_object result = parse (&ctx);

  free_buffer (&buffer);
  return result;
}

#define HAS_NEXT (ctx->position < ctx->buffer->length)
#define CURRENT (ctx->buffer->data[ctx->position])
#define PEEK(n)                                                               \
  (ctx->position + (n) >= 0 ? (ctx->position + (n) < ctx->buffer->length      \
                                   ? ctx->buffer->data[ctx->position + (n)]   \
                                   : ctx->buffer->data[ctx->position - 1])    \
                            : ctx->buffer->data[0])
#define IS_STRING_LITERAL(n) (PEEK (n) == '\"' && PEEK (n - 1) != '\\')
#define IS_SEPARATOR(n)                                                       \
  (PEEK (n) == ' ' || PEEK (n) == '\t' || PEEK (n) == '\r' || PEEK (n) == '\n')

#define p_assert(expr, message, ...)                                          \
  assert ((expr), message " (line %d:%d)" __VA_OPT__ (, ) __VA_ARGS__,        \
          ctx->row + 1, ctx->column)

void
advance (parser_context_t* ctx)
{
  ctx->position++;

  if (ctx->f_line_end)
    {
      ctx->row++;
      ctx->column = 0;
    }
  else
    ctx->column++;
}

cr_object
parse_list (parser_context_t* ctx)
{
  cr_object result = NIL;
  int terminated = 0;

  while (HAS_NEXT)
    {
      if (CURRENT == ')')
        {
          advance (ctx);
          terminated = 1;
          break;
        }

      list_append (&result, parse (ctx));

      /* Cons detection. */

      while (HAS_NEXT && IS_SEPARATOR (0))
        advance (ctx);

      if (CURRENT == '.' && IS_SEPARATOR (1))
        {
          advance (ctx);

          CDR (result) = parse (ctx);

          while (HAS_NEXT && IS_SEPARATOR (0))
            advance (ctx);

          assert (CURRENT == ')', "Extra elements in cons cell");
          advance (ctx);

          return result;
        }
    }

  p_assert (terminated, "No matching ) found for the list");
  return result;
}

cr_object
parse_vector (parser_context_t* ctx)
{
  cr_object result = NIL;
  int terminated = 0;
  cr_int length = 0;

  while (HAS_NEXT)
    {
      if (IS_SEPARATOR (0))
        {
          advance (ctx);
          continue;
        }

      if (CURRENT == ']')
        {
          p_assert (length > 0, "Vector cannot be empty");
          advance (ctx);
          terminated = 1;
          break;
        }

      list_append (&result, parse (ctx));
      length++;
    }

  p_assert (terminated, "No matching ] found for the vector");

  cr_object vector = alloc_vector (length);

  for (int i = 0; i < length; i++)
    {
      vector_set (vector, i, CAR (result));
      result = CDR (result);
    }

  return vector;
}

cr_object
parse_string (parser_context_t* ctx)
{
  buffer_t buffer = make_buffer ();

  while (HAS_NEXT)
    {
      if (IS_STRING_LITERAL (0))
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
              p_assert (0, "Unknown escape sequence");
            }
        }

      /* TODO: use cr_char for buffer characters. */
      buffer_push (&buffer, CURRENT);
      advance (ctx);
    }

  p_assert (!ctx->f_in_string, "Unterminated string literal");

  cr_object result = alloc_vector (buffer.length);

  for (int i = 0; i < buffer.length; i++)
    {
      cr_object char_object = alloc_char (buffer.data[i]);
      vector_set (result, i, char_object);
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
      p_assert (CURRENT >= '0' && CURRENT <= '9',
                "Expected a digit, '+', or '-'");
      sign = 1;
    }

  int starting_position = ctx->position;
  int fp_position = -1;

  while (HAS_NEXT)
    {
      if (CURRENT == '.')
        {
          p_assert (fp_position == -1, "Unexpected '.'");
          fp_position = ctx->position;
          is_real = 1;
        }
      else
        {
          if (IS_SEPARATOR (0) || CURRENT < '0' || CURRENT > '9')
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

  cr_object number
      = is_real ? alloc_real (real * sign) : alloc_integer (integer * sign);
  return number;
}

cr_object
parse_character (parser_context_t* ctx)
{
  assert (CURRENT == '?', "Invalid character literal");
  advance (ctx);

  cr_object value = parse_number (ctx);
  assert (IS_INTEGER (value), "Only integer-valued characters are allowed");
  assert (AS_INTEGER_V (value) >= -128 && AS_INTEGER_V (value) <= 127,
          "Only signed, single-byte values are supported");
  return alloc_char (AS_INTEGER_V (value));
}

cr_object
parse_symbol (parser_context_t* ctx)
{
  buffer_t buffer = make_buffer ();

  while (HAS_NEXT)
    {
      if ((CURRENT >= 'A' && CURRENT <= 'Z')
          || (CURRENT >= 'a' && CURRENT <= 'z')
          || (CURRENT >= '#' && CURRENT <= '%')
          || (CURRENT >= '*' && CURRENT <= ':')
          || (CURRENT >= '<' && CURRENT <= '@')
          || (CURRENT == '!' || CURRENT == '^')
          || (CURRENT == '_' || CURRENT == '~'))
        /* TODO: use cr_char for buffer characters. */
        buffer_push (&buffer, CURRENT);
      else
        {
          p_assert (IS_SEPARATOR (0) || (CURRENT == '(' || CURRENT == ')')
                        || (CURRENT == '[' || CURRENT == ']'),
                    "Unrecognised character");
          break;
        }

      advance (ctx);
    }

  p_assert (buffer.length > 0, "Empty identifier");

  if (buffer.length == 3 && !strncmp (buffer.data, "nil", 3))
    {
      free_buffer (&buffer);
      return NIL;
    }

  cr_object symbol = alloc_interned_symbol (buffer.data, buffer.length);
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
                        || (CURRENT == '\r' && PEEK (1) != '\n');

      if (CURRENT == ';' && !ctx->f_in_string)
        ctx->f_comment = 1;

      if (ctx->f_line_end)
        ctx->f_comment = 0;

      ctx->f_content = !ctx->f_comment && !IS_SEPARATOR (0);

      if (!ctx->f_content)
        {
          advance (ctx);
          continue;
        }

      if (IS_STRING_LITERAL (0))
        {
          advance (ctx);
          return parse_string (ctx);
        }

      if (CURRENT == '(')
        {
          advance (ctx);
          return parse_list (ctx);
        }

      if (CURRENT == '[')
        {
          advance (ctx);
          return parse_vector (ctx);
        }

      if (CURRENT == '?')
        return parse_character (ctx);

      if ((CURRENT >= '0' && CURRENT <= '9')
          || ((CURRENT == '-' || CURRENT == '+')
              && (PEEK (1) >= '0' && PEEK (1) <= '9')))
        return parse_number (ctx);

      return parse_symbol (ctx);
    }

  p_assert (HAS_NEXT, "Reached end of file");
  return NULL;
}
