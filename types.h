#ifndef TYPES_H
#define TYPES_H

#define NIL (&nil)
#define IS_NIL(object) ((object) == NIL)

#define IS_INTEGER(object) ((object)->type == INTEGER)
#define AS_INTEGER(object) ((object)->integer)

#define IS_REAL(object) ((object)->type == REAL)
#define AS_REAL(object) ((object)->real)

#define IS_CHARACTER(object) ((object)->type == CHARACTER)
#define AS_CHARACTER(object) ((object)->character)

#define IS_VECTOR(object) ((object)->type == VECTOR)
#define AS_VECTOR(object) ((object)->vector)

#define VECTOR_DATA(object) (AS_VECTOR (object).data)
#define VECTOR_CAPACITY(object) (AS_VECTOR (object).capacity)

#define IS_CONS(object) ((object)->type == CONS)
#define AS_CONS(object) ((object)->cons)

#define CAR(object) (AS_CONS (object).car)
#define CDR(object) (AS_CONS (object).cdr)
#define CDDR(object) (CDR (CDR (object)))

typedef int cr_int;
typedef double cr_real;
typedef char cr_char;

typedef struct cr_object_t* cr_object;

typedef struct cr_cons cr_cons;
struct cr_cons
{
  cr_object car, cdr;
};

typedef struct cr_vector cr_vector;
struct cr_vector
{
  cr_object* data;
  cr_int capacity;
};

#define MAX_SYMBOL_LENGTH 128

typedef struct cr_symbol cr_symbol;
struct cr_symbol
{
  enum
  {
    INTERNED,
    UNINTERNED,
  } type;
  cr_char* name;
  cr_int length;
};

struct cr_object_t
{
  enum
  {
    NIL_TYPE,
    INTEGER,
    REAL,
    CHARACTER,
    SYMBOL,
    CONS,
    VECTOR,
  } type;
  union
  {
    cr_int integer;
    cr_real real;
    cr_char character;
    cr_symbol symbol;
    cr_cons cons;
    cr_vector vector;
  };
  short int ref_count;
};

extern struct cr_object_t nil;

void free_object (cr_object);

cr_symbol make_interned_symbol (const char*);
cr_symbol make_uninterned_symbol ();
void free_symbol (cr_symbol*);

#endif /* TYPES_H */
