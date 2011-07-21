
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

int term_init()
{
	if( ! isatty( fileno(stdout) ) )
	{
		fprintf(stderr, "You are not a terminal !\n");
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

void term_cursor_left()
{
	fputc('\10', stdout);
}

void term_cursor_right()
{
	fputc('\33', stdout);
	fputc('[', stdout);
	fputc('C', stdout);
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

int term_set_control(pid_t session_pid)
{
	int res;

	res = tcsetpgrp(0, session_pid);

	if( res != 0 )
	{
		fprintf(stderr, "set_control_term ERROR %d !!!!!!!!!!!!!!!!!!!!\n", res);
	}

	return res;
}

int term_get_file_fd()
{
	return fileno(input);
}

int term_quit()
{
	term_set_old();

//	fclose(input);
//	fclose(output);

	return 0;
}
