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
#include <errno.h>
#include <ctype.h>

#include "types.h"

#define MAX_NR_TOKENS 32	/* Maximum number of tokens in a command */
#define MAX_TOKEN_LEN 64	/* Maximum length of single token */
#define MAX_COMMAND	256		/* Maximum length of command string */

/***********************************************************************
 * parse_command
 *
 * DESCRIPTION
 *	Parse @command, put each command token into @tokens[], and 
 *	set @nr_tokens with the number of tokens.
 *
 * A command token is defined as a string without any whitespace (i.e., *space*
 * and *tab* in this programming assignment). Suppose @command as follow;
 *
 *   @command = "  Hello world   Ajou   University!!  "
 *
 * Then, @nr_tokens = 4 and @tokens should be
 *
 *   tokens[0] = "Hello"
 *   tokens[1] = "world"
 *   tokens[2] = "Ajou"
 *   tokens[3] = "University!!"
 *
 * Another exmaple is;
 *   command = "ls  -al   /home/operating_system /hw0  "
 *
 * then, nr_tokens = 4, and tokens is
 *   tokens[0] = "ls"
 *   tokens[1] = "-al"
 *   tokens[2] = "/home/operating_system"
 *   tokens[3] = "/hw0"
 *
 *
 * @command can be expressed with double quotation mark(") to quote a string
 * that contains whitespaces. Suppose following @command;
 *
 *   @command = "  We will get over with the "COVID 19" virus   "
 *
 * Then, @nr_tokens = 8, and @tokens are
 *
 *   tokens[0] = "We"
 *   tokens[1] = "will"
 *   tokens[2] = "get"
 *   tokens[3] = "over"
 *   tokens[4] = "with"
 *   tokens[5] = "the"
 *   tokens[6] = "COVID 19"
 *   tokens[7] = "virus"
 *
 * Note that tokens[6] does not contain the quotation marks.
 * Also, one @command can have multiple quoted strings, but they will not be
 * nested
 * (i.e., quote another string in a quote)
 *
 *   This "is a possible" case for a command --> This, is a possible, case, for,
 *	                                             a, command
 *   "This " is "what I told you" --> This, is, what I told you
 *
 * RETURN VALUE
 *	Return 0 after filling in @nr_tokens and @tokens[] properly
 *
 */
int myStrlen(char *ch){			//strlen 함수 구현
	int res=0;
	while(ch[res] != '\0'){
		res++;
	}
	return res;
}
void delchar(char* str,char tok) {			//토큰 parameter로 받아서 str에서 지우는
	for (int i = 0;i < myStrlen(str);i++) {			
		if (str[i] == tok) {
			for (int j = i;j < myStrlen(str);j++) {
				if (j == myStrlen(str) - 1) {
					str[j] = '\0';
				}
				str[j] = str[j + 1];
			}
		}
	}
}
int NumQuote(char *ch){		//문자열에서 따옴표 갯수 반환하는 함수
	int len = myStrlen(ch);
	int cnt = 0;
	for(int i=0;i<len;i++){
		if(ch[i] == '\"')
			cnt++;
	}
	return cnt;
}

bool NullOrSpace(char ch){		//parameter로 받은 문자가 널문자거나 스페이스 인것을 체크해주는 함수
	if(ch == '\0' || isspace(ch)) return true;
	return false;
}

char * MyStrtok(char *input){		//strtok함수 구현
	char *cursor, *end;		//현재 위치를 알려줄 cursor 변수와 문자열의 끝을 알려주기 위한 변수 end
	char *res = NULL;		//토큰 짤라내고 짤린 문자열 반환하기 위한 변수
	bool flag = false;		//따옴표에 있는 스페이스인지 아닌지를 확인하기 위한 부울형 flag 변수
	int num = 0;			//따옴표의 수를 저장하기 위한 변수
	
	if(input != NULL){		//input이 있을 때 
		cursor = input;		//문자열의 시작 주소를 가져옴
		end = &cursor[myStrlen(input)];	//문자열의 끝 주소를 저장
	}
	
	if(cursor == NULL || cursor == end) return NULL;	//cursor가 NULL이거나 끝이면 NULL반환

	while(cursor < end && NullOrSpace(*cursor) == true && !flag)	//cursor가 end위치보다 앞이거나 커서값이 NULL이거나 스페이스이거나 따옴표 안의 스페이스가 아닌 경우
		*(cursor++) = '\0';		//cursor값을 NULL로 넣어주고 cursor를 다음으로 보내주고 
	res = cursor;				//떼어낸 문자열을 반환

	while(cursor < end && ( NullOrSpace(*cursor) == false || (flag && num == NumQuote(cursor) ) ) ){	//따옴표안의 스페이스 이거나 기존에 설정한 Num값과 새로운 Num값이 같을 경우
		if(cursor[0] == '\"'){		//cursor 부분이 따옴표일 때
			flag = true;		//flag를 true로 바꿔줌 -> 따옴표가 나왔다는 뜻
		}
		num = NumQuote(cursor);		//num값 다시 바꿔줌(이는 기존 cursor의 따옴표 개수와 달라질수 있으니 바꿔주는 것)
		cursor++;			
	}
	*cursor = '\0';				
	if(res == cursor) res = NULL;		
	return res;
}
static int parse_command(char *command, int *nr_tokens, char *tokens[])
{
	/**
	 * TODO
	 *
	 * Followings are some example code. Delete them and implement your own
	 * code here
	 */
	char *word = NULL;
	int cnt = 0;
	word = MyStrtok(command);		//구현한 MyStrtok함수로 command 스페이스 기준으로 잘라내기
	
	while(true){
		tokens[cnt] = word;		//잘라낸 문자열 저장
		word = MyStrtok(NULL);		//NULL을 parameter로 한번 더 잘라줌

		cnt++;			
		if(word == NULL) break;	
	}
	*nr_tokens = cnt;			//토큰 갯수 설정

	for(int i=0;i<cnt;i++){			//잘라낸 단어들에서 따옴표 제거
		delchar(tokens[i],'\"');
	}	
		
	return 0;
}


/***********************************************************************
 * The main function of this program.
 * SHOULD NOT CHANGE THE CODE BELOW THIS LINE
 */
int main(int argc, char *argv[])
{
	char line[MAX_COMMAND] = { '\0' };
	FILE *input = stdin;
	if (argc == 2) {
		input = fopen(argv[1], "r");
		if (!input) {
			fprintf(stderr, "No input file %s\n", argv[1]);
			return -EINVAL;
		}
	}
	while (fgets(line, sizeof(line), input)) {
		char *tokens[MAX_NR_TOKENS] = { NULL };
		int nr_tokens= 0;
		parse_command(line, &nr_tokens, tokens);

		fprintf(stderr, "nr_tokens = %d\n", nr_tokens);
		for (int i = 0; i < nr_tokens; i++) {
			fprintf(stderr, "tokens[%d] = %s\n", i, tokens[i]);
		}
		printf("\n");
	}

	if (input != stdin) fclose(input);

	return 0;
}
