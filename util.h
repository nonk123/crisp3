#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <stdlib.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define eprintf(message, ...)                                                 \
  fprintf (stderr, message __VA_OPT__ (, ) __VA_ARGS__)

#define assert(expr, message, ...)                                            \
  do                                                                          \
    {                                                                         \
      if (!(expr))                                                            \
        {                                                                     \
          eprintf ("File: %s:%d\n", __FILE__, __LINE__);                      \
          eprintf ("Assertion failed: %s\nDetails: ", #expr);                 \
          eprintf (message __VA_OPT__ (, ) __VA_ARGS__);                      \
          eprintf ("\n");                                                     \
                                                                              \
          exit (1);                                                           \
        }                                                                     \
    }                                                                         \
  while (0);

#define assert_non_null(expr)                                                 \
  assert ((expr) != NULL, "\"%s\" cannot be null", #expr)

#define assert_mem(expr) assert ((expr) != NULL, "Memory allocation failed")

void* safe_malloc (int);
void* safe_calloc (int, int);
void* safe_realloc (void*, int);

#endif /* UTIL_H */
