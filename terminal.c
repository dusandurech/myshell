
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <termios.h>

#include "main.h"

#include "util.h"
#include "terminal.h"

static FILE *input;
static FILE *output;

static struct termios old_term;
static struct termios new_term;

static char line[STR_LINE_SIZE];
static int offset;
static int len;

static void append(char c)
{
	int i;

	memmove(line+offset+1, line+offset, STR_LINE_SIZE-(offset+1));

	line[offset++] = c;
	len++;

	for(i = offset-1; i < len; i++)
	{
		fputc(line[i], stdout);
	}

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

	if( len == 0 || offset == 0 )
	{
		return;
	}

	memmove(line+offset-1, line+offset, STR_LINE_SIZE-(offset+1));

	len--;
	offset--;

	//fprintf(stdout,"\r%s", line);

	fputc('\10', stdout);

	for(i = offset; i < len; i++)
	{
		fputc(line[i], stdout);
	}

	fputc(' ', stdout);

	for(i = 0; i < len-offset; i++)
	{
		fputc('\10', stdout);
	}

	fputc('\10', stdout);
}

static char* get_current_short_dir()
{
	static char path[STR_PATH_SIZE];

	char *home_dir;
	char *current_dir;
	int home_dir_len;
	int current_dir_len;

	home_dir = get_home_dir();
	current_dir = get_current_dir();

	home_dir_len = strlen(home_dir);
	current_dir_len = strlen(current_dir);

	if( current_dir_len >= home_dir_len && strncmp(home_dir, current_dir, home_dir_len) == 0 )
	{
		path[0] = '~';
		strcpy(path+1, current_dir+home_dir_len);
	}
	else
	{
		strcpy(path, current_dir);
	}

	return path;
}

static char get_prompt()
{
	if( getuid() == 0 )
	{
		return '#';
	}
	else
	{
		return '$';
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

	tcgetattr(fileno(input), &old_term);

	new_term = old_term;
	new_term.c_lflag &= ~ICANON;
	new_term.c_lflag &= ~ECHO;
	
	term_set_new();

	return 0;
}

void term_print_status()
{
	fprintf(stdout, "%s@%s:%s%c ", get_username(), get_nodename(), get_current_short_dir(), get_prompt());
}

int term_readline(char *str_line)
{
	int c;

	memset(line, 0, STR_LINE_SIZE);
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

int term_set_old()
{
	if( tcsetattr(fileno(input), TCSANOW, &old_term) != 0 )
	{
		fprintf(stderr, "Could not set old attributes !\n");
		return -1;
	}

	return 0;
}

int term_set_new()
{
	if( tcsetattr(fileno(input), TCSANOW, &new_term) != 0 )
	{
		fprintf(stderr, "Could not set new attributes !\n");
		return -1;
	}

	return 0;
}

int term_quit()
{
	term_set_old();

	fclose(input);
	fclose(output);

	return 0;
}

#ifdef TEST_TERMINAL
int main()
{
	char str[STR_LINE_SIZE];

	term_init();

	do{
		term_readline(str);
		printf("%s\n", str);
	}while( strcmp(str, "exit") != 0 );

	term_quit();

	return 0;
}
#endif
