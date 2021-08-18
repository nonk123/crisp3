OBJS = util.o buffer.o types.o gc.o stack.o
TESTS = test_list

CFLAGS=-ggdb -Wall

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

main.o: main.c tokenizer.h
util.o: util.c util.h
buffer.o: buffer.c buffer.h util.o
types.o: types.c types.h util.o
gc.o: gc.c gc.h util.o types.o
stack.o: stack.c stack.h gc.o util.o
