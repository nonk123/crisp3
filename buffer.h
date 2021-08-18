#ifndef BUFFER_H
#define BUFFER_H

#include <stdio.h>

typedef struct buffer_t buffer_t;
struct buffer_t
{
  char* data;
  int capacity;
  int length;
};

buffer_t make_buffer ();

void free_buffer (buffer_t*);

void buffer_push (buffer_t*, char);

void buffer_read (buffer_t*, FILE*);

void buffer_read_by_file_name (buffer_t*, const char*);

#endif /* BUFFER_H */
