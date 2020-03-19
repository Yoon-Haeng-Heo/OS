TARGET	= pa0
CFLAGS	= -g -c -D_POSIX_C_SOURCE
CFLAGS += -std=c99 -Wimplicit-function-declaration -Werror
LDFLAGS	=

all: pa0

pa0: pa0.o
	gcc $(LDFLAGS) $^ -o $@

%.o: %.c
	gcc $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) *.o
