/**********************************************************************
 * Copyright (c) 2020
 *  Sang-Hoon Kim <sanghoonkim@ajou.ac.kr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTIABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 **********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "types.h"
#include "parser.h"

/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING FROM THIS LINE ******       */
/**
 * String used as the prompt (see @main()). You may change this to
 * change the prompt */
static char __prompt[MAX_TOKEN_LEN] = "$";

/**
 * Time out value. It's OK to read this value, but ** SHOULD NOT CHANGE
 * IT DIRECTLY **. Instead, use @set_timeout() function below.
 */
static unsigned int __timeout = 2;
	
bool timers = true;
static void set_timeout(unsigned int timeout)
{
	__timeout = timeout;

	if (__timeout == 0) {
		fprintf(stderr, "Timeout is disabled\n");
		timers = false;
	} else {
		fprintf(stderr, "Timeout is set to %d second%s\n",
				__timeout,
				__timeout >= 2 ? "s" : "");
	}
}
/*          ****** DO NOT MODIFY ANYTHING UP TO THIS LINE ******      */
/*====================================================================*/


/***********************************************************************
 * run_command()
 *
 * DESCRIPTION
 *   Implement the specified shell features here using the parsed
 *   command tokens.
 *
 * RETURN VALUE
 *   Return 1 on successful command execution
 *   Return 0 when user inputs "exit"
 *   Return <0 on error
 */


char name[1000];			//To save names
char* child;				//child name
pid_t pid;				//pid
int status;
unsigned int sleeptime = 0;	
int cnt = 0;
int loopcnt = 1;
char* text[1000];			//save the command text
int idx = 0;
int textidx = 0;
int forcnt = 0;

void signal_handler(){			//using sigkill to kill child process
	kill(pid,SIGKILL);
	fprintf(stderr,"%s is timed out\n",child);
	
}

struct sigaction sigact = {
	.sa_handler = signal_handler,		//sigaction
	.sa_flags = 0,
},old_sa;

static int run_command(int nr_tokens, char *tokens[])
{
	/* This function is all yours. Good luck! */
	
	memset(text,0,1000);	
	if (strncmp(tokens[0], "exit", strlen("exit")) == 0) {
		return 0;
	}
	else if(strncmp(tokens[0],"prompt",strlen("prompt")) == 0) {		//prompt change
		strcpy(__prompt,tokens[1]);
	
	}
	else if(strncmp(tokens[0],"cd",strlen("cd"))==0){
		char* path;
		path = getenv("HOME");

		if(strncmp(tokens[1],"~",strlen("~"))==0){
			if(chdir(path) == -1){
				fprintf(stderr,"No such file or directory\n");
			}
		}	
		else if(chdir(tokens[1]) == -1){
			fprintf(stderr,"No such file or directory\n");
		}

	
	}
	else if(strncmp(tokens[0],"timeout",strlen("timeout"))==0){
		sigaction(SIGALRM,&sigact,&old_sa);
		if(nr_tokens == 1){		//only timeout
			fprintf(stderr, "Current timeout is %d second\n",__timeout);
		}
		else{
			set_timeout(atoi(tokens[1]));
		}
		
	}
	/*else if(strncmp(tokens[0],"sleep",strlen("sleep"))==0){
		sleeptime = atoi(tokens[1]);
		if(sleeptime > __timeout){
			sleep(sleeptime);
		}
		else{
			sleep(__timeout);
		}
	
	}*/
	else if(strncmp(tokens[0],"for",strlen("for"))==0){
		cnt = nr_tokens;
		//printf("check\n%d\n",loopcnt);
		while(cnt > 0){		//return atoi = 0 nan			how handle String "for"....?
			if(atoi(tokens[idx]) != 0){
				loopcnt = loopcnt * atoi(tokens[idx]);
				forcnt--;
				//printf("forcnt check\n");
				//printf("check1\n%d\n",loopcnt);
			}
			else if(strncmp(tokens[idx],"for",strlen("for") ) !=0){
				//printf("check2\n");
				//printf("%s\n",tokens[idx]);					
				//strcpy(text[textidx],tokens[idx]);
				text[textidx] = tokens[idx];
				//printf("%s\n",text[textidx]);	
				textidx++;	
			}
			else{
				//printf("forcnt check2222\n");				
				forcnt++;
			}
			idx++;
			cnt--;
			//printf("%d\n",textidx);
		}
		if(forcnt == 1){			//String "for"
			text[textidx] = "for";
			textidx++;			
		}
			
		while(loopcnt > 0){	
			//printf("check3\n%s\n",text[0]);
			if(strncmp(text[0],"cd",strlen("cd"))==0){
				char* path;
				path = getenv("HOME");

				if(strncmp(text[1],"~",strlen("~"))==0){
					if(chdir(path) == -1){
					fprintf(stderr,"No such file or directory\n");
					}
				}		
				else if(chdir(text[1]) == -1){
					fprintf(stderr,"No such file or directory\n");
				}
			}
			else{		//external commands (no cd)
				pid = fork();
				if(pid == 0){ 	//child process
					if(execvp(text[0],text) < 0){	//execvp function
						/*
						this part will be closing part
						*/	
						fprintf(stderr,"No such file or directory\n");
						//exit(-1);		//infinite loop....
						kill(getpid(),SIGKILL);	//not this line program will be infinite loop....
						exit(0);
					}
				}
				else{
					//child = tokens[0];		//child is killed process's name
					/*if(timers){
						alarm(__timeout);
					}*/
				
				waitpid(pid,&status,0);
				}	
			}
			loopcnt--;
		}//while(loopcnt > 0)	
		
		memset(text,0,1000);
		textidx = 0;
		idx = 0;
		loopcnt = 1;
		forcnt = 0;
		
	}
	else{		//external commands (no cd)
		pid = fork();
		if(pid == 0){ 	//child process
			if(execvp(tokens[0],tokens) < 0){	//execvp function
				/*
				this part will be closing part
				*/	
				fprintf(stderr,"No such file or directory\n");
				kill(getpid(),SIGKILL);			//child kill (kill ghost)
				exit(-1);
			}
		}
		else{
			child = tokens[0];		//child is killed process's name
			if(timers){
				alarm(__timeout);
			}
		}
		waitpid(pid,&status,0);
		alarm(0);
	}
	/*
	fork();
	exec();
	...
	*/

	return 1;
}


/***********************************************************************
 * initialize()
 *
 * DESCRIPTION
 *   Call-back function for your own initialization code. It is OK to
 *   leave blank if you don't need any initialization.
 *
 * RETURN VALUE
 *   Return 0 on successful initialization.
 *   Return other value on error, which leads the program to exit.
 */
static int initialize(int argc, char * const argv[])
{
	return 0;
}


/***********************************************************************
 * finalize()
 *
 * DESCRIPTION
 *   Callback function for finalizing your code. Like @initialize(),
 *   you may leave this function blank.
 */
static void finalize(int argc, char * const argv[])
{

}


/*====================================================================*/
/*          ****** DO NOT MODIFY ANYTHING BELOW THIS LINE ******      */

static bool __verbose = true;
static char *__color_start = "[0;31;40m";
static char *__color_end = "[0m";

/***********************************************************************
 * main() of this program.
 */
int main(int argc, char * const argv[])
{
	char command[MAX_COMMAND_LEN] = { '\0' };
	int ret = 0;
	int opt;

	while ((opt = getopt(argc, argv, "qm")) != -1) {
		switch (opt) {
		case 'q':
			__verbose = false;
			break;
		case 'm':
			__color_start = __color_end = "\0";
			break;
		}
	}

	if ((ret = initialize(argc, argv))) return EXIT_FAILURE;

	if (__verbose)
		fprintf(stderr, "%s%s%s ", __color_start, __prompt, __color_end);

	while (fgets(command, sizeof(command), stdin)) {	
		char *tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens = 0;

		if (parse_command(command, &nr_tokens, tokens) == 0)
			goto more; /* You may use nested if-than-else, however .. */

		ret = run_command(nr_tokens, tokens);
		if (ret == 0) {
			break;
		} else if (ret < 0) {
			fprintf(stderr, "Error in run_command: %d\n", ret);
		}

more:
		if (__verbose)
			fprintf(stderr, "%s%s%s ", __color_start, __prompt, __color_end);
	}

	finalize(argc, argv);

	return EXIT_SUCCESS;
}
