## Project #1: My Smart Shell

### *** Due on 11:59:59pm, April 26 (Sunday)***

### Goal
With the system calls learned in the class and a few additional ones, you are ready to manipulate processes in the system.
Let's build a pretty, colorful, and exciting shell by leveraging the system calls.


### Background
- *Shell* is a program that interpretes user inputs, and processes them accordingly. The example includes the Command Prompt in Windows, Bourne Shell in Linux, so forth.

- An user can input a command by writing a sentence on the shell. Upon receiving the input, the shell parses the requests into tokens, just as you did in PA0, and diverges the execution according to the first token.

- If the first token is one of *built-in* commands, it processes the command internally. The built-in command runs without any other executable, but handled directly by the shell program.

- If the first token is not the built-in command, the shell assumes the first token as the name of the executable file to run. 
The shell has a special environment variable, *`PATH`*, which lists the directories to look up for the executable file.
Each directory is separated by `:`. For example, if `PATH=/usr/bin:./bin:./sce213`, the shell should look up the file from `/usr/bin`, `./bin`, and `./sce213` directories. If the specified file exists, the shell should launch the executable with the remaining tokens as the arguments for the executable.


### Problem Specification
- The shell program `mysh` waits for your command line input after printing out a prompt `$`. When you enter a line of command, it will be parsed into tokens as of project assignment 0, and the framework will call `run_command()` function with the tokens. You need to implement following features in that function.
- Currently, the shell keep getting input commands from users and processes them until the user enters `exit`. In that case, the shell program exits.


#### Change prompt (20 pts)
- `mysh` prints out the prompt using the contents in `char __prompt[]`. Implement `prompt` built-in command that changes the prompt as follow;

- ```
  $ prompt #
  # prompt sce213
  sce213 prompt $
  $
  ```

- You may change `__prompt` value to implement this feature.


#### Execute any external commands (50 pts)
- When the shell gets non-built-in commands, it should run the executable as explained above in Background. Each command can be comprised of one exeutable followed by zero or more arguments. For example;
- ```bash
  $ /bin/ls
  Makefile  pa1.c  parser.c  parser.o   toy    toy.o
  mysh   	  pa1.o  parser.h  README.md  toy.c  types.h
  $ ls
  Makefile  pa1.c  parser.c  parser.o   toy    toy.o
  mysh  	  pa1.o  parser.h  README.md  toy.c  types.h
  $ pwd
  /home/sanghoon/pa/os/pa1
  $ cp mysh mysh.copied
  $ ls
  Makefile  mysh.copied  pa1.o	   parser.h  README.md  toy.c  types.h
  mysh 	    pa1.c        parser.c  parser.o  toy	      toy.o
  $ exit
  ```
- `ls`, `pwd`, and `cp` are not built-in commands. Thus, the shell should execute the executable on the `PATH` environment variable. The p-variants of `exec()` command family will help you doing this. When the specified command cannot be executed, print out the following message to `stderr`.
- ```C
	if (unable to execute the specified command) {
		fprintf(stderr, "No such file or directory\n");
	}
  ```

- Use `toy` application which is included in the repository for your development and testing. It prints out the arguments so that you can check whether your implementation handles input commands properly.
- ```bash
  $ ./toy arg1 arg2 arg3 arg4
  pid  = xxxxxx
  argc = 5
  argv[0] = ./toy
  argv[1] = arg1
  argv[2] = arg2
  argv[3] = arg3
  argv[4] = arg4
  done!
	```

- You should handle `stdin` and `stdout` of the child process properly in case `exec()` fails. Otherwise, supposed-to-be the input of the child is flushed into the parent process' `stdin`, making *ghost* input to the parent process. On mac, this will not be happened since BSD libc does not flush the buffered input on child's exit. Nevertheless, you will need to handle this issue to pass the test on PASubmit server.

- Hint: `fork(), exec(), wait(), waitpid()`, and `getenv()`

#### Change working directory (30 pts)
- Each process has so-called *working directory* on which the process is working. This value can be checked by executing `/bin/pwd` or calling `getcwd()` system call. Implement `cd` command which changes the current working directory.

- Each user has one's *home directory* which is denoted by `~`. The actual path is defined in `$HOME` environment variable. Make `cd` command to understand it

- ```bash
  $ pwd
  /home/directory/of/your/account  # Assume this is the home directory of the user
  $ cd /somewhere/i/dont/know
  $ pwd
  /somewhere/i/dont/know
  $ cd ~
  $ pwd
  /home/directory/of/your/account
  ```

- Hints: `chdir(), getcwd(), getenv("HOME")`



#### Terminate timed-out program (100 pts)

- Let the shell detect non-finishing programs and terminiate them. Let `timeout` built-in command set the desired time out specified in seconds (2 by default) or print out the current time-out value. When the executing program does not finish within the specified period, `mysh` should terminate the program by sending a `SIGKILL` signal to it. When the timeout is set to 0, disable this timed-out feature.
- ```
  $ timeout
  Current timeout is 0 second
  $ timeout 10
  Timeout is set to 10 seconds
  $ sleep 5
  ( ... after 5 seconds ... )
	$ sleep 20
	( ... after 10 seconds ... )
	sleep is timed out   # The shell terminated the program
  $ timeout 0
  Timeout is disabled
  $ sleep 100
  ( ... after 100 seconds ...)
  $
  ```

- On a timed-out, print out the name of the application followed by " is timed out\n" to `stderr` using `fprintf`. Use the exact format to pass testcase checking.
- ```C
  /* Assume that char *name points to the name of the application */
  fprintf(stderr, "%s is timed out\n", name);
  ```

- You can use the `toy` for your testing as well. It will sleep for `@N` seconds if you run it with `sleep @N` as its arguments.
- ```bash
  $ ./toy sleep 10
  pid  = xxxxxx
  argc = 3
  argv[0] = ./toy
  argv[1] = sleep
  argv[2] = 10
  ( ... toy will sleep for 10 seconds ... )
  done!     # This output means the toy finished execution properly.
	$ timeout 5
  Timeout is set to 5 seconds
  $ ./toy sleep 10
  pid  = xxxxxx
  argc = 3
  argv[0] = ./toy
  argv[1] = sleep
  argv[2] = 10
	...
  ( ... after 10 seconds ...)
  ./toy is timed out
  $
	```

- Use `__timeout` variable to read the current timeout value. Use `set_timeout()` function to set the value. DO NOT MODIFY `__timeout` variable DIRECTLY.

- Hint: `sigaction(), alarm()`, and `kill()`.

- Make sure `alarm` is disabled once the program exits otherwise the shell itself might be be terminated. Use `sigaction` over `signal` for code portability (Use `sigaction()` instead of `signal()`). Also, have a look at `man 7 signal` to get an extensible explanation about signal handling.



#### Repeat the command n times (50 pts)
- Implement `for <N> <command ...>` built-in command to run the commands N-times. `for` can be nested.

- ```
  $ for 5 pwd
  /home/pasubmit/some/directory/in/depth
  /home/pasubmit/some/directory/in/depth
  /home/pasubmit/some/directory/in/depth
  /home/pasubmit/some/directory/in/depth
  /home/pasubmit/some/directory/in/depth
  $ for 4 cd ..
  $ pwd
  /home/pasubmit
  $ for 2 for 3 echo hello world
  hello world
  hello world
  hello world
  hello world
  hello world
  hello world
  $ for 20 for 10 for 2 echo nested for
  nested for
  (... 398 more "nested for" ...)
  nested for
  $
  ```



### Restriction

- DO NOT USE `system()` system call. You will get 0 pts if it is used.
- Implement only `prompt`, `timeout`, `for`, `cd`, and `exit` as *built-in command* (i.e., handle the features in the shell process), whereas other commands should be processed by *external programs* created with `fork()` and `exec()`. ***Implementing external program's features (e.g., printing out a message for handling `echo` command, sleeping/handling alarm by parsing `./toy` command and `sleep` argument, etc) will be penalized with -250 pts***
- For sure, you should implement the features by calling relevent system calls; just printing out expected results will make your score -250 pts.
- DO NOT TRY to figure out the testing situation by parsing the number of tokens and/or token contents. Such a forged execution will be screened offline and punished with -250 pts as well.
- It is advised to test your code on your computer first. You might be be penalized if you blow up the pasubmit system multiple times with obvious mistakes. In this sense, it would be best to test your implementation on your machine using `make test-run`, `make test-timeout`, `make test-cd`, `make test-for`, and `make test-prompt`. `make test-all` runs all the tests one after the other. Have a look at `Makefile` to learn making test automatic.
- For your coding practice, the compiler is set to halt on some (important) warnings. Write your code to comply the C99 standard.


### Submission / Grading

- Use [PAsubmit](https://sslab.ajou.ac.kr/pasubmit) for submission
	- 280 pts in total
- Source: pa1.c (250 pts in total)
  - Points will be prorated by testcase results.
- Document: *One PDF* document (20 pts). It should include followings;
  - Outline how programs are launched and how arguments are passed
  - How the timed-out feature is implemented
  - How the `for` built-in command is implemented
  - Lessons learned if you have any
  - NO MORE THAN ***THREE*** PAGES
  - DO NOT INCLUDE COVER PAGE. No need to specify your name nor student ID on the document.
  - YOU WILL GET 0 pts for the report if you do not comply the statements.
- Git repository URL at git.ajou.ac.kr (10 pts)
  - To get the points, you should actually use the repository to manage your code (i.e., have more than two commits which are hours aparts). You will not get any point if you just committed your final code.
  - Refer to https://www.youtube.com/channel/UC-Un-7jmeP-1OaHkS7awO0Q for using gitlab at Ajou University.
	- How to create your repository to submit:
		- Clone this repository into your computer.
		- Create a *private* project from http://git.ajou.ac.kr.
		- Push the local repository onto the gitlab repository.
		- Generate a deploy token from Settings/Repository/Deploy Token.
		- Register at PASubmit using the repository URL and deploy token.
		- PASubmit only accepts the repository address over HTTP.
	- The repository will be checked (at least) twice, during the registration and after the submission deadline. The status will be updated accordingly.
- Free to make a question through AjouBb. However, YOU MIGHT NOT GET AN ANSWER IF THE QUESTIONS ARE ABOUT THOSE ALREADY SPECIFIED ON THIS HANDOUT.
- QUESTIONS OVER EMAIL WILL BE IGNORED unless it concerns your privacy.
