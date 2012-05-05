
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <unistd.h>
#include <termios.h>
#include <termcap.h>

#include "main.h"
#include "util.h"
#include "terminal.h"
#include "config.h"

static FILE *input;
static FILE *output;

static struct termios old_term;
static struct termios new_term;

typedef struct terminal_struct
{
	void (*term_cursor_right)();
	void (*term_cursor_left)();
} terminal_t;

static void vt100_cursor_right()
{
	fputc('\33', stdout);
	fputc('[', stdout);
	fputc('C', stdout);
}

static void vt100_right_left()
{
	fputc('\33', stdout);
	fputc('[', stdout);
	fputc('D', stdout);

	//fputc('\10', stdout);
}

#ifdef SUPPORT_LIB_TERMCAP
static void termcap_cursor_move(char *control_string)
{
	static char buf[30];
	char *ap = buf;
	char *str;

	if( buf[0] == '\0' )
	{
		char *env_term;

		env_term = getenv("TERM");
		tgetent(buf, env_term);
	}

	str = tgetstr(control_string, &ap);
	fputs(str, stdout);
}

static void termcap_cursor_right()
{
	termcap_cursor_move("nd");
}

static void termcap_right_left()
{
	termcap_cursor_move("le");
}

static const terminal_t terminal_termcap =
{
	.term_cursor_right = termcap_cursor_right,
	.term_cursor_left = termcap_right_left,
};
#endif

static const terminal_t terminal_vt100 =
{
	.term_cursor_right = vt100_cursor_right,
	.term_cursor_left = vt100_right_left,
};

#ifdef SUPPORT_LIB_TERMCAP
static const terminal_t *terminal_main = &terminal_termcap;
#else
static const terminal_t *terminal_main = &terminal_vt100;
#endif

int term_init()
{
	if( ! isatty( fileno(stdout) ) )
	{
		//fprintf(stderr, "You are not a terminal !\n");

		input = NULL;
		output = NULL;

		return -1;
	}

	input = stdin;
	output = stdout;

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
}

void term_cursor_right()
{
	terminal_main->term_cursor_right();
}

void term_cursor_left()
{
	terminal_main->term_cursor_left();
}

void term_putc(char c)
{
	fputc(c, stdout);
}

void term_puts(char *s)
{
	fputs(s, stdout);
}

int term_getc()
{
	return fgetc(stdin);
}

int term_set_old()
{
	if( input != NULL && tcsetattr(fileno(input), TCSANOW, &old_term) != 0 )
	{
		fprintf(stderr, "Could not set old attributes !\n");
		return -1;
	}

	return 0;
}

int term_set_new()
{
	if( input != NULL && tcsetattr(fileno(input), TCSANOW, &new_term) != 0 )
	{
		fprintf(stderr, "Could not set new attributes !\n");
		return -1;
	}

	return 0;
}

int term_set_control(pid_t session_pid)
{
	int res;

	if( input == NULL )
	{
		return 0;
	}

	res = tcsetpgrp(0, session_pid);

	if( res != 0 )
	{
		fprintf(stderr, "set_control_term ERROR %d !!!!!!!!!!!!!!!!!!!!\n", res);
	}

	return res;
}

int term_get_file_fd()
{
	if( input == NULL )
	{
		return -1;
	}

	return fileno(input);
}

int term_quit()
{
	term_set_old();

//	fclose(input);
//	fclose(output);

	return 0;
}
