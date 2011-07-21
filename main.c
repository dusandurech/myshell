
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

int main(int argc, char **argv, char **env)
{
	char str_command[STR_LINE_SIZE];

	process_t *process;

	history_init();
	signal_init();
	term_init();
	inter_cmd_init();
	jobs_init();

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

	history_quit();
	term_quit();
	jobs_quit();

	return 0;
}
