
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>

#include "main.h"

#include "env.h"
#include "array.h"
#include "expand_var.h"

static int isAlphabet(char c)
{
	return ( c >= 'a' && c <= 'z' ) ||
	       ( c >= 'A' && c <= 'Z' ) ||
	       ( c >= '0' && c <= '9' );
}

char* expand_var(char *str_commandline)
{
	static char str[STR_LINE_SIZE];
	char varname[STR_SIZE];

	int count;
	int len;
	int i;
	char pair;
	int capVar;

	printf("str_commandline >%s<\n", str_commandline);

	memset(varname, 0, STR_SIZE);
	memset(str, 0, STR_LINE_SIZE);
	len = strlen(str_commandline);
	pair = '\0';
	capVar = 0;
	count = 0;

	for(i = 0; i < len; i++)
	{
		char c;

		c = str_commandline[i];

		if( capVar == 1 && ! isAlphabet(c) )
		{
			int l;

			capVar = 0;
			l = strlen(env_get(varname));
			strncpy(str+count, env_get(varname), l);
			count += l;

			//printf("%s", env_get(varname));
			memset(varname, 0, STR_SIZE);
		}

		if( c == '\\' )
		{
			str[count++] = c;
			//putchar(c);
			c = str_commandline[++i];
			str[count++] = c;
			//putchar(c);
			continue;
		}

		if( pair == '\0' && ( c == '\'' || c == '\"' ) )
		{
			str[count++] = c;
			//putchar(c);

			pair = c;
			continue;
		}

		if( pair == c )
		{
			str[count++] = c;
			//putchar(c);

			pair = '\0';
			continue;
		}

		if( pair != '\'' && c == '$' )
		{
			capVar = 1;
			continue;
		}

		if( capVar == 1 )
		{
			strncat(varname, &c, 1);
			continue;
		}

		str[count++] = c;
		//putchar(c);
	}

	//putchar('\n');

	return str;
}

//#define TEST_EXPAND_VAR

#ifdef TEST_EXPAND_VAR
int main(int argc, char **argv, char **env)
{
	printf("%s\n", expand_var("echo $USER \"$USER\" \\\"$USER\\\" \'$USER\'"));
}
#endif
