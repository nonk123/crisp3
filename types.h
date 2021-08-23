#ifndef TYPES_H
#define TYPES_H

#define AS_OBJECT(object) ((cr_object) (object))

#define NIL (&nil)
#define IS_NIL(object) ((object) == NIL)

#define IS_INTEGER(object) ((object)->type == INTEGER)
#define AS_INTEGER(object) ((cr_object_int) (object))
#define AS_INTEGER_V(object) (AS_INTEGER (object)->value)

#define IS_REAL(object) ((object)->type == REAL)
#define AS_REAL(object) ((cr_object_real) (object))
#define AS_REAL_V(object) (AS_REAL (object)->value)

#define IS_CHARACTER(object) ((object)->type == CHARACTER)
#define AS_CHARACTER(object) ((cr_object_char) (object))
#define AS_CHARACTER_V(object) (AS_CHARACTER (object)->value)

#define IS_SYMBOL(object) ((object)->type == SYMBOL)
#define AS_SYMBOL(object) ((cr_object_symbol) (object))

#define SYMBOL_NAME(object) (AS_SYMBOL (object)->name)
#define SYMBOL_LENGTH(object) (AS_SYMBOL (object)->length)

#define IS_INTERNED(symbol) ((symbol)->type == INTERNED)
#define IS_UNINTERNED(symbol) ((symbol)->type == UNINTERNED)

#define IS_CONS(object) ((object)->type == CONS)
#define AS_CONS(object) ((cr_object_cons) (object))

#define CAR(object) (AS_CONS (object)->car)
#define CDR(object) (AS_CONS (object)->cdr)
#define CDDR(object) (CDR (CDR (object)))

#define IS_VECTOR(object) ((object)->type == VECTOR)
#define AS_VECTOR(object) ((cr_object_vector) (object))

#define VECTOR_DATA(object) (AS_VECTOR (object)->data)
#define VECTOR_LENGTH(object) (AS_VECTOR (object)->length)

typedef unsigned char small_enum;

typedef int cr_int;
typedef double cr_real;
typedef char cr_char;

#define MAX_SYMBOL_LENGTH 128

typedef struct cr_object_t cr_object_base;
typedef cr_object_base* cr_object;

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
  short int ref_count;
};

typedef struct cr_object_int_t* cr_object_int;
struct cr_object_int_t
{
  cr_object_base base;
  cr_int value;
};

typedef struct cr_object_real_t* cr_object_real;
struct cr_object_real_t
{
  cr_object_base base;
  cr_real value;
};

typedef struct cr_object_char_t* cr_object_char;
struct cr_object_char_t
{
  cr_object_base base;
  cr_char value;
};

typedef struct cr_object_symbol_t* cr_object_symbol;
struct cr_object_symbol_t
{
  cr_object_base base;
  cr_char* name;
  cr_int length;
  enum
  {
    INTERNED,
    UNINTERNED,
  } type;
};

typedef struct cr_object_cons_t* cr_object_cons;
struct cr_object_cons_t
{
  cr_object_base base;
  cr_object car, cdr;
};

typedef struct cr_object_vector_t* cr_object_vector;
struct cr_object_vector_t
{
  cr_object_base base;
  cr_object* data;
  cr_int length;
};

extern cr_object_base nil;

void free_object (cr_object);

void borrow_object (cr_object);
void return_object (cr_object);

void own_object (cr_object, cr_object);
void disown_object (cr_object, cr_object);

cr_object vector_get (cr_object, cr_int);
void vector_set (cr_object, cr_int, cr_object);

#endif /* TYPES_H */
