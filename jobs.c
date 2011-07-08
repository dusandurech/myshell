
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#include "main.h"
#include "array.h"
#include "signal.h"
#include "jobs.h"

typedef struct jobs_struct
{
	char *str_cmd;
	int id;
	pid_t session_pid;
} jobs_t;

static array_t *array_jobs;

static int get_free_id()
{
	static int free_id = 1;

	return free_id++;
}

static jobs_t* jobs_new_item(const char *str_cmd, const pid_t session_pid)
{
	jobs_t *jobs;

	jobs = (jobs_t *) malloc( sizeof(jobs_t) );
	jobs->str_cmd = strdup(str_cmd);
	jobs->session_pid = session_pid;
	jobs->id = get_free_id();

	return jobs;
}

static void jobs_print_item(const jobs_t *jobs)
{
	printf("[%d] %s %d\n", jobs->id, jobs->str_cmd, jobs->session_pid);
}

static void jobs_destroy_item(jobs_t *jobs)
{
	free(jobs->str_cmd);
	free(jobs);
}

static jobs_t* find_jobs(const int id)
{
	int i;

	for(i = 0; i < array_jobs->count; i++)
	{
		jobs_t *jobs;

		jobs = (jobs_t *) array_get(array_jobs, i);

		if( jobs->id == id )
		{
			return jobs;
		}
	}

	return NULL;
}

static int del_from_jobs(const int id)
{
	int i;

	for(i = 0; i < array_jobs->count; i++)
	{
		jobs_t *jobs;

		jobs = (jobs_t *) array_get(array_jobs, i);

		if( jobs->id == id )
		{
			array_del_item(array_jobs, i, jobs_destroy_item);
			return 1;
		}
	}

	return 0;
}

static void jobs_run_item(const jobs_t *jobs, const int jobs_type)
{
	int status;

	if( jobs_type == JOBS_RUN_IN_FG )
	{
		term_set_control(jobs->session_pid);
	}

	signal_send_to_session(jobs->session_pid, SIGCONT);

	if( jobs_type == JOBS_RUN_IN_FG )
	{
		int res;

		printf("WAIT\n");
		res = waitpid(-1, &status, WUNTRACED);
		printf("xxx %d\n", res);

		if( WIFEXITED(status) )
		{
			del_from_jobs(jobs->id);
			printf("Process exited\n");
		}

		if( WIFSIGNALED(status) )
		{
			del_from_jobs(jobs->id);
			printf("Process canceled\n");
		}

		if( WIFSTOPPED(status) )
		{
			printf("Process stopeed\n");
		}
	}
}

int jobs_init()
{
	array_jobs = array_new();

	return 0;
}

int jobs_add_process(const char *str_cmd, const pid_t session_pid)
{
	jobs_t *jobs;

	jobs = jobs_new_item(str_cmd, session_pid);
	array_add(array_jobs, jobs);

	return jobs->id;
}

int jobs_run(const int id, const int jobs_type)
{
	jobs_t *jobs;

	jobs = find_jobs(id);

	if( jobs != NULL )
	{
		jobs_run_item(jobs, jobs_type);
		return 1;
	}

	return 0;
}

void jobs_print_all()
{
	int i;

	for(i = 0; i < array_jobs->count; i++)
	{
		jobs_t *jobs;

		jobs = (jobs_t *) array_get(array_jobs, i);
		jobs_print_item(jobs);
	}
}

int jobs_print_process(const int id)
{
	jobs_t *jobs;

	jobs = find_jobs(id);

	if( jobs != NULL )
	{
		jobs_print_item(jobs);
		return 1;
	}

	return 0;
}

int jobs_quit()
{
	array_destroy_item(array_jobs, jobs_destroy_item);

	return 0;
}
