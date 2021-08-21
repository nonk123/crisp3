#ifndef PARSER_H
#define PARSER_H

#include "buffer.h"
#include "gc.h"
#include "types.h"

typedef struct parser_context_t parser_context_t;
struct parser_context_t
{
  int position, row, column;
  int f_line_end, f_comment, f_in_string, f_content;
  buffer_t* buffer;
};

parser_context_t make_parser_context (buffer_t*);

cr_object parse (parser_context_t*);

cr_object parse_file (const char*);

#endif /* PARSER_H */
