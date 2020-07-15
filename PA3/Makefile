TARGET	= lock
CFLAGS	= -g -c -D_POSIX_C_SOURCE -D_GNU_SOURCE
CFLAGS += -std=c99 -Wimplicit-function-declaration -Werror
CFLAGS +=

LDFLAGS += -lpthread

HEADERS=$(wildcard ./*.h)

.PHONY: all
all: lock

lock: pa3.o main.o generator.o counter.o tester.o
	gcc $^ -o $@ $(LDFLAGS)

%.o: %.c $(HEADERS)
	gcc $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET) *.o *.dSYM
