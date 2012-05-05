
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <signal.h>

#include "main.h"

#include "process.h"
#include "command.h"
#include "inter_cmd.h"
#include "terminal.h"
#include "readline.h"
#include "history.h"
#include "signal.h"
#include "jobs.h"

static int run_script(char *filename)
{
	char str_command[STR_LINE_SIZE];
	FILE *file;

	file = fopen(filename, "rt");

	if( file == NULL )
	{
		fprintf(stderr, "Nemozem otvorit script \'%s\'\n", filename);
		return -1;
	}

	while( fgets(str_command, STR_LINE_SIZE-1, file) != NULL )
	{
		process_t *process;
		int len;

		len = strlen(str_command);

		if( len > 1 && str_command[len-1] < ' ' )
		{
			str_command[len-1] = '\0';
		}

		if( inter_cmd_exec(str_command) == 0 )
		{
			process = command(str_command);

			if( process != NULL )
			{
				term_set_old();
				process_run(process);
				process_destroy(process);
				term_set_control(getpid());
				term_set_new();
			}
		}
	}

	fclose(file);

	return 0;
}

static void interactive_mode()
{
	char str_command[STR_LINE_SIZE];
	process_t *process;

	history_load();

	do{
		readline_print_status();
		readline(str_command);

		term_set_old();

		if( inter_cmd_exec(str_command) == 0 )
		{
			process = command(str_command);

			if( process != NULL )
			{
				process_run(process);
				process_destroy(process);
			}
		}

		term_set_control(getpid());
		term_set_new();

	}while( inter_cmd_is_exit() == 0 );

	history_save();
}

int main(int argc, char **argv, char **env)
{
	history_init();
	signal_init();
	term_init();
	inter_cmd_init();
	jobs_init();

	if( argc > 1 )
	{
		int i;

		for(i = 1; i < argc; i++)
		{
			run_script(argv[i]);
		}
	}
	else
	{
		interactive_mode();
	}

	history_quit();
	term_quit();
	jobs_quit();

	return EXIT_SUCCESS;
}
