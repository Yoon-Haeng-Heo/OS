# SCE213 Operating Systems (Spring 2020)

## Project #0: Implementing a Command Line Parser

### Due on the end of March 29 (Sunday) (23:59:59)


### Introduction

The purpose of this project is to bootstrap the term project to implement a shell program, and to get familiar with the programming assignment submission system.


### Problem Specification

We will implement a shell program soon. To that end, you should firstly make the shell program to understand user inputs and commands.

In this project, the framework passes input strings, and your task is to **split the input strings into command tokens**. By C standard, a string is defined as a sequence of characters which is terminated with `\0`. Command tokens are strings that are obtained by splitting the input string with delimitors which is whitespace characters in this PA. Use `isspace()` function to check whether a character is whitespace or not. Any heading and trailing whitespace should be removed from the command tokens.

For example, if the input string is " Hello   world ", the command tokes are "Hello" and "world". **Note that the input string is split into tokens delimited by whitespaces (' ' in this example), and the tokens do not contain whitespaces**.

Another example string is " cp   a.out  pa0.c /tmp ". It should be broken into "cp", "a.out", "pa0.c", and "/tmp".

The input string can be expressed with double quotation marks(") to quote a string that contains whitespaces. Suppose the input string is "  We will get over with the "COVID 19" virus   ", then the tokens are We, will, get, over, with, the, COVID 19, and virus. The token may contain whitespaces if it is quoted. Note that the tokens do not contain quotation marks though.

The required functionality must be implemented in `parse_command()` function. The user input is passed through `command`. After parsing `command` into tokens, the tokens should be assigned to `tokens[]` in order, which is passed as the argument of the `parse_command()` function. Also, the number of tokens should be assigined to `*nr_tokens` which will be used by the framework.

Below diagram illustrates a correct result for an input string "  ls  -al   /home/sce213 ". Note that each token is also a string which is terminated with `\0`.

```c
char *command   --> "  ls  -al   /home/sce213 "
 
*nr_tokens = 3
tokens[0]  -->  "ls"
tokens[1]  -->  "-al"
tokens[2]  -->  "/home/sce213"
tokens[3]... = NULL
```


### Restrictions

- You **should not use any string manipulation functions from any libraries**. The banned functions include `strtok, strtok_r, strlen, fscanf` and/or similars. This implies that you should implement your own string manipulation functions if it is needed. **You will get 0 point if you use any of them**. Note that `malloc()` and `free()` are not string manipulation functions and OK to use. If unsure, question to TA/professor through AjouBb.
- Use `isspace()` C library function to check whether the character can be assumed as a whitespace.
- Printing messages to stand output (e.g., `printf()`) is totally OK. However, do not `fprintf(stderr ...)` otherwise the grading system cannot grade your submission properly.



### Logistics

- This is an individual project; you work on the assignement alone.
- You can use **up to 4 slip tokens throughout this semester**.

- You can start this program assignment by cloning this repository at https://github.com/sslab-ajou/sce213-pa0-2020s. Everything is in `pa0.c`. Please do not modify the `main()` function, but implement your code in `parse_command()`. You can add any functions or macros if you need, for sure.

- This is an OS/architecture-independent project; you may use Visual Studio on Windows or vim/gcc on Linux. However, the grading will be done on a Linux machine. So, write code as neutral as possible to compilers.
- All submission and grading are done automatically through https://sslab.ajou.ac.kr/pasubmit. Please follow the instruction explained in the class.
	- Submit `pa0.c` file to the PA submission site at https://sslab.ajou.ac.kr/pasubmit. (80 pts)
	- Explain your parsing strategy on a **1-page PDF document**. Submit the document as the report in the PASubmit. (20 pts)
- `input` file in the provided code contains the input sequence for the grading.



### Tips and Notes

- Briefly speaking, the implementation might look like; start scanning from the beginning of `command` skipping all whitespaces until meet a non-whitespace character. That will be the beginning of the token. Then, resume scanning until meet any whitespace character (' ' or '`\t`'). That will be the end of the token. Repeat this until entire `command` is scanned. Slight modification is required to handle the quotation marks. Assign the starting position of each token into `tokens[]` and count `nr_tokens` accordingly.
- Read comments in the skeleton code carefully.
- Make sure every string is terminated with '\0'.
- Post freely on QnA board on AjouBb to question about the project.


Good luck and have fun!
