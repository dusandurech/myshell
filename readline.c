
#include <stdio.h>
#include <string.h>

#include <unistd.h>
#include <termios.h>

#include "main.h"

#include "util.h"
#include "history.h"
#include "terminal.h"
#include "readline.h"

static char line[STR_LINE_SIZE];
static int offset;
static int len;

static void append(char c)
{
	int i;

	//fprintf(stderr, "c = %d\n", c);

	if( len >= STR_LINE_SIZE-1 )
	{
		return;
	}

	memmove(line+offset+1, line+offset, STR_LINE_SIZE-(offset+1));

	line[offset++] = c;
	len++;

	for(i = offset-1; i < len; i++)
	{
		term_putc(line[i]);
	}

	for(i = 0; i < len-offset; i++)
	{
		term_cursor_left();
	}
}

static void move_left()
{
	if( offset == 0 )
	{
		return;
	}

	offset--;
	term_cursor_left();
	//printf("c = LEFT\n");

}

static void move_right()
{
	if( offset == len )
	{
		return;
	}

	offset++;
	term_cursor_right();
}

static void move_begin()
{
	while( offset > 0 )
	{
		move_left();
	}
}

static void move_end()
{
	while( offset < len )
	{
		move_right();
	}
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

	term_cursor_left();

	for(i = offset; i < len; i++)
	{
		term_putc(line[i]);
	}

	term_putc(' ');

	for(i = 0; i < len-offset; i++)
	{
		term_cursor_left();
	}

	term_cursor_left();
}

static void set_string_from_history()
{
	char *str;
	int my_len;
	int i;

	str = history_get_select();

	//printf("str = >%s<\n", str);

	if( str == NULL )
	{
		return;
	}

	readline_clean();

	my_len = strlen(str);

	for(i = 0; i < my_len; i++)
	{
		char c;

		c = str[i];
		append(c);
	}
}

static void move_up()
{
	if( history_up() )
	{
		set_string_from_history();
	}
}

static void move_down()
{
	if( history_down() )
	{
		set_string_from_history();
	}
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

void readline_print_status()
{
	char str_prompt[STR_SIZE];

#if 0
	term_puts("myshell");
	term_putc(get_prompt());
	term_putc(' ');
#endif

	sprintf(str_prompt, "%s@%s:%s%c ", get_username(), get_nodename(), get_current_short_dir(), get_prompt());
	term_puts(str_prompt);
}

void readline_clean()
{
	while( offset < len )
	{
		move_right();
	}

	while( len > 0 )
	{
		backspace();
	}

	memset(line, 0, STR_LINE_SIZE);
	offset = 0;
	len = 0;
}

int readline(char *str_line)
{
	int c;

	memset(line, 0, STR_LINE_SIZE);
	offset = 0;
	len = 0;

	do{
		c = term_getc();
	
/*
		printf("c = %d\n", c);

		if( is_send_signal_int() )
		{
			fprintf(stderr, "sig");
			term_putc('\n');
			str_line[0] = '\0';

			return 0;
		}
*/
		switch( c )
		{
			case -1 :
				fprintf(stderr, "\n-1\n");
			break;

			case '\n' :
				term_putc('\n');
			break;

			case 1 :
				move_begin();
			break;

			case 5 :
				move_end();
			break;

			case 127 :
				backspace();
				history_backup_current_line(line);
			break;

			case 27 :
				switch( (c = term_getc()) )
				{
					case 91 :
						switch( (c = term_getc()) )
						{
							case 65 :
								move_up();
							break;
							case 66 :
								move_down();
							break;

							case 68 :
								move_left();
							break;
							case 67 :
								move_right();
							break;

							default:
								//printf("c = %d\n", c);
							break;
						}
					break;
				}
			break;

			default :
				//printf("c = %d\n", c);
				if( c >= ' ' )
				{
					append(c);
					history_backup_current_line(line);
				}
			break;
		}
	}while( c != '\n' );

	history_add(line);
	history_backup_current_line("");

	memcpy(str_line, line, len+1);

	return len;
}

//#define TEST_READLINE

#ifdef TEST_READLINE
int main()
{
	char str[STR_LINE_SIZE];

	term_init();
	history_init();

	do{
		readline_print_status();
		readline(str);
		printf("%s\n", str);
	}while( strcmp(str, "exit") != 0 );

	history_quit();
	term_quit();

	return 0;
}
#endif
