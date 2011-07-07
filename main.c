
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <signal.h>

#include "main.h"

#include "process.h"
#include "command.h"
#include "terminal.h"
#include "signal.h"

int main(int argc, char **argv, char **env)
{
	char str_command[STR_LINE_SIZE];

	process_t *process;

	signal_init();
	term_init();

	do{
		term_print_status();
		term_readline(str_command);

		//fgets(str_command, STR_LINE_SIZE-1, stdin);

		if( strcmp(str_command, "exit") != 0 )
		{
			process = command(str_command);

			term_set_old();

			process_run(process);

			if( tcsetpgrp(0, getpid()) != 0 )
			{
				fprintf(stderr, "ERROR 2 !!!!!!!!!!!!!!!!!!!!\n");
			}

			term_set_new();

			process_destroy(process);
		}

	}while( strcmp(str_command, "exit") != 0 );

	term_quit();

	return 0;
}
