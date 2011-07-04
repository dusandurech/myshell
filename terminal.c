
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <termios.h>

#include "terminal.h"

#define TERM_LINE_SIZE	256

static FILE *input;
static FILE *output;

static struct termios init_term;
static struct termios new_term;

static char line[TERM_LINE_SIZE];
static int offset;
static int len;

static void append(char c)
{
	int i;

	memmove(line+offset+1, line+offset, TERM_LINE_SIZE-(offset+1));

	line[offset++] = c;
	len++;

	fprintf(stdout,"\r%s", line);

	for(i = 0; i < len-offset; i++)
	{
		fputc('\10', stdout);
	}
}

static void move_left()
{
	if( offset == 0 )
	{
		return;
	}

	offset--;
	fputc('\10', stdout);
	//printf("c = LEFT\n");

}

static void move_right()
{
	if( offset == len )
	{
		return;
	}

	offset++;
	fputc('\33', stdout);
	fputc('[', stdout);
	fputc('C', stdout);
}

static void backspace()
{
	int i;

	if( len == 0 )
	{
		return;
	}

	memmove(line+offset-1, line+offset, TERM_LINE_SIZE-(offset+1));

	len--;
	offset--;

	fprintf(stdout,"\r%s", line);

	fputc(' ', stdout);
	fputc('\10', stdout);

	for(i = 0; i < len-offset; i++)
	{
		fputc('\10', stdout);
	}
}

int term_init()
{
	if( ! isatty( fileno(stdout) ) )
	{
		fprintf(stderr, "You are not a terminal !\n");
		return -1;
	}

	input = fopen("/dev/tty", "r");
	output = fopen("/dev/tty", "w");

	if( input == NULL || output == NULL )
	{
		fprintf(stderr, "Could not open /dev/tty !\n");
		return -1;
	}

	tcgetattr(fileno(input), &init_term);

	new_term = init_term;
	new_term.c_lflag &= ~ICANON;
	new_term.c_lflag &= ~ECHO;
	
	if( tcsetattr(fileno(input), TCSANOW, &new_term) != 0 )
	{
		fprintf(stderr, "Could not set attributes !\n");
		return -1;
	}

	return 0;
}

int term_readline(char *str_line)
{
	int c;

	memset(line, 0, TERM_LINE_SIZE);
	offset = 0;
	len = 0;

	do{
		switch( (c = fgetc(input)) )
		{
			case '\n' :
				fputc('\n', stdout);
			break;

			case 127 :
				backspace();
			break;

			case 27 :
				switch( (c = fgetc(input)) )
				{
					case 91 :
						switch( (c = fgetc(input)) )
						{
							case 68 :
								move_left();
							break;
							case 67 :
								move_right();
							break;
						}
					break;
				}
			break;

			default :
				append(c);
			break;
		}
	}while( c != '\n' );

	memcpy(str_line, line, len+1);

	return len;
}

int term_quit()
{
	tcsetattr(fileno(input), TCSANOW, &init_term);

	fclose(input);
	fclose(output);

	return 0;
}

#ifdef TEST_TERMINAL
int main()
{
	char str[80];

	term_init();

	do{
		term_readline(str);
		printf("%s\n", str);
	}while( strcmp(str, "exit") != 0 );

	term_quit();

	return 0;
}
#endif
