
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "main.h"

#include "process.h"
#include "command.h"
#include "terminal.h"

int main(int argc, char **argv, char **env)
{
	char str_command[STR_LINE_SIZE];

	process_t *process;

	term_init();

	do{
		term_print_status();
		term_readline(str_command);

		if( strcmp(str_command, "exit") != 0 )
		{
			process = command(str_command);

			term_set_old();

			process_run(process);

			term_set_new();

			process_destroy(process);
		}

	}while( strcmp(str_command, "exit") != 0 );

	term_quit();

	return 0;
}
