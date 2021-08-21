OBJS = util.o buffer.o types.o gc.o stack.o parser.o
TESTS = test_list test_vector test_parser

CFLAGS=-ggdb -lm -Wall

all: crisp3

clean:
	rm -f crisp3 $(TESTS) main.o $(OBJS)

tests: $(TESTS)

.PHONY: all clean tests

crisp3: main.o $(OBJS)
	$(CC) $(CFLAGS) -o $@ main.o $(OBJS)

test_%: test_%.c $(OBJS)
	$(CC) $(CFLAGS) -o $@ $@.c $(OBJS)
	@./$@ && echo "$@: success" || echo "$@: fail"

test_parser: test.lisp

main.o: main.c
util.o: util.c util.h
buffer.o: buffer.c buffer.h util.o
types.o: types.c types.h util.o
gc.o: gc.c gc.h util.o types.o
stack.o: stack.c stack.h gc.o util.o
parser.o: parser.c parser.h buffer.o types.o gc.o util.h
