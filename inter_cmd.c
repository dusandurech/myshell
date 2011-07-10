
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>

#include "main.h"
#include "jobs.h"
#include "inter_cmd.h"

static int exit_flag;

static int cmd_jobs(const char *str_cmdline)
{
	jobs_print_all();

	return 1;
}

static int cmd_jobs_run_abstract(const char *str_cmdline, const int jobs_type)
{
	int jobs_id;
	int res;

	res = sscanf(str_cmdline+3, "%d", &jobs_id);

	if( res != 1 )
	{
		fprintf(stderr, "This is no numer !\n");
		return 1;
	}

	res = jobs_run(jobs_id, jobs_type);

	if( res == 0 )
	{
		fprintf(stderr, "This job id dont exists !\n");
		return 1;
	}
	
	return 1;
}

static int cmd_fg(const char *str_cmdline)
{
	return cmd_jobs_run_abstract(str_cmdline, JOBS_RUN_IN_FG);
}

static int cmd_bg(const char *str_cmdline)
{
	return cmd_jobs_run_abstract(str_cmdline, JOBS_RUN_IN_BG);
}

static int cmd_exit(const char *str_cmdline)
{
	exit_flag = 1;

	return 1;
}

typedef struct cmd_struct
{
	char *name;
	int len;
	int (*fce_handler)(const char *str_cmd);
} cmd_t;

static cmd_t cmd_list[] =
{
	{ .name = "jobs",	.len = 4,	.fce_handler = cmd_jobs },
	{ .name = "fg",		.len = 2,	.fce_handler = cmd_fg },
	{ .name = "bg",		.len = 2,	.fce_handler = cmd_bg },
	{ .name = "exit",	.len = 4,	.fce_handler = cmd_exit }
};

#define CMD_LIST_COUNT		( sizeof(cmd_list) / sizeof(cmd_t) )

int inter_cmd_init()
{
	exit_flag = 0;

	return 0;
}

int inter_cmd_is_exit()
{
	return exit_flag;
}

int inter_cmd_exec(const char *str_cmdline)
{
	int i;

	for(i = 0; i < CMD_LIST_COUNT; i++)
	{
		cmd_t *cmd = &cmd_list[i];

		if( strncmp(str_cmdline, cmd->name, cmd->len) == 0 )
		{
			if( cmd->fce_handler != NULL )
			{
				return cmd->fce_handler(str_cmdline);
			}

			return 1;
		}
	}

	return 0;
}
