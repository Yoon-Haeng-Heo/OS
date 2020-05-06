TARGET	= mysh
CFLAGS	= -g -c -D_POSIX_C_SOURCE
CFLAGS += -std=c99 -Wimplicit-function-declaration -Werror
CFLAGS += # Add your own cflags here if necessary
LDFLAGS	=

all: mysh toy

mysh: pa1.o parser.o
	gcc $(LDFLAGS) $^ -o $@

toy: toy.o
	gcc $(LDFLAGS) $^ -o $@

%.o: %.c
	gcc $(CFLAGS) $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET) toy *.o *.dSYM


.PHONY: test-run
test-run: $(TARGET) toy testcases/test-run
	./$< -q < testcases/test-run

.PHONY: test-timeout
test-timeout: $(TARGET) toy testcases/test-timeout
	./$< -q < testcases/test-timeout

.PHONY: test-cd
test-cd: $(TARGET) testcases/test-cd
	./$< -q < testcases/test-cd

.PHONY: test-for
test-for: $(TARGET) testcases/test-for
	./$< -q < testcases/test-for

.PHONY: test-prompt
test-prompt: $(TARGET) testcases/test-prompt
	./$< < testcases/test-prompt


test-all: test-run test-timeout test-cd test-for test-prompt
	echo
