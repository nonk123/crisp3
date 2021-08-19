#ifndef PARSER_H
#define PARSER_H

#include "buffer.h"
#include "types.h"
#include "gc.h"

typedef struct parser_context_t parser_context_t;
struct parser_context_t
{
  int position, row, column;
  int f_line_end, f_comment, f_in_string, f_content;
  memory_t* memory;
  buffer_t* buffer;
};

parser_context_t make_parser_context(memory_t*, buffer_t*);

cr_object parse (parser_context_t*);

#endif /* PARSER_H */
