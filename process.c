
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>

#include "main.h"

#include "array.h"
#include "dir.h"

#include "process.h"

#define PROCESS_GPID_NONE	-1

process_t* process_new()
{
	process_t *process;

	process = (process_t *) malloc( sizeof(process_t) );
	memset(process, 0, sizeof(process_t));

	return process;
}

void process_set(process_t *process, char *filename_exec, char **param, char **env)
{
	process->filename_exec = filename_exec;
	process->param = param;
	process->env = env;
}

void process_print(const process_t *process)
{
	int i;

	printf("---------------------\n");
	printf("process %p\n", process);

	printf("exec %s ", process->filename_exec);

	for(i = 0; process->param[i] != NULL; i++)
	{
		printf("%s ", process->param[i]);
	}

	if( process->flag & PROCESS_STDOUT_FILE )
	{
		printf(">%s ", process->stdout_filename);
	}

	if( process->flag & PROCESS_STDIN_FILE )
	{
		printf("<%s ", process->stdin_filename);
	}

	if( process->flag & PROCESS_STDOUT_APPEND_FILE )
	{
		printf(">>%s ", process->stdout_filename);
	}

	if( process->flag & PROCESS_STDIN_DOC_HERE )
	{
		printf("<< \"DOCUMENT HETE\" ");
	}

	putchar('\n');

	printf("pipe process %p\n", process->pipe_process);
	printf("next process %p\n", process->next_process);

	if( process->pipe_process != NULL )
	{
		process_print(process->pipe_process);
	}

	if( process->next_process != NULL )
	{
		process_print(process->next_process);
	}
}

static int process_exec(process_t *process)
{
	int res;

	if( process->flag & PROCESS_STDOUT_FILE || process->flag & PROCESS_STDOUT_APPEND_FILE )
	{
		mode_t mask;
		int fd;

		mask = O_WRONLY|O_CREAT;

		if( process->flag & PROCESS_STDOUT_APPEND_FILE )
		{
			mask |= O_APPEND;
		}

		close(1);

		fd = open(process->stdout_filename, mask, 0666);

		if( fd < 0 )
		{
			fprintf(stderr, "I dont open %s file for write !", process->stdout_filename);
		}
	}

	if( process->flag & PROCESS_STDIN_FILE )
	{
		int fd;

		close(0);
		fd = open(process->stdin_filename, O_RDONLY);

		if( fd < 0 )
		{
			fprintf(stderr, "I dont open %s file for read !", process->stdin_filename);
		}
	}

	res = execve(process->filename_exec, process->param, process->env);

	return res;
}

static int process_pipe(process_t *process, pid_t grp_pid)
{
	int fd[2];
	int status;

	pipe(fd);

	if( grp_pid == PROCESS_GPID_NONE )
	{
		grp_pid = getpid();
		
		setpgid(getpid(), grp_pid);

		if( tcsetpgrp(0, grp_pid) != 0 )
		{
			fprintf(stderr, "ERROR !!!!!!!!!!!!!!!!!!!!\n");
		}
	}

	switch( fork() )
	{
		case 0 :
			close(1);
			dup(fd[1]);
			close(fd[0]);

			if( process->pipe_process != NULL && process->pipe_process->pipe_process != NULL )
			{
				process_pipe(process->pipe_process, grp_pid);
			}
			else
			{
				setpgid(getpid(), grp_pid);
				process_exec(process->pipe_process);
			}
		break;
		
		default :
			setpgid(getpid(), grp_pid);

			close(0);
			dup(fd[0]);
			close(fd[1]);

			process_exec(process);
		break;
	}

	return 0;
}

int process_run(process_t *process)
{
	pid_t pid;
	int status;
	int fd[2];

	//memcpy(process->next_process->fd, fd, sizeof(fd));

	switch( ( pid = fork() ) )
	{
		case -1 :	// error
			fprintf(stderr, "fork error !\n");
			return -1;
		break;

		case 0 :	// child
			if( process->pipe_process != NULL )
			{
				process_pipe(process, PROCESS_GPID_NONE);
			}
			else
			{
				process_exec(process);
			}
		break;

		default :	// parrent
			if( ! ( process->flag & PROCESS_NO_WAIT ) )
			{
				wait(&status);
			}

			if( process->flag & PROCESS_AND )
			{
				if( status == 0 )
				{
					if( process->next_process != NULL )
					{
						process_run(process->next_process);
					}
				}
			}
			else if( process->flag & PROCESS_OR )
			{
				if( status != 0 )
				{
					if( process->next_process != NULL )
					{
						process_run(process->next_process);
					}
				}
			}
			else if( process->next_process != NULL )
			{
				process_run(process->next_process);
			}
		break;
	}

	return status;
}

void process_destroy(process_t *process)
{
	int i;

	if( process->filename_exec != NULL )
	{
		free(process->filename_exec);	
	}

	if( process->param != NULL )
	{
		for(i = 0; process->param[i] != NULL; i++)
		{
			free(process->param[i]);
		}

		free(process->param);
	}

	if( process->stdout_filename != NULL )
	{
		free(process->stdout_filename);
	}

	if( process->stdin_filename != NULL )
	{
		free(process->stdin_filename);
	}

	if( process->pipe_process != NULL )
	{
		process_destroy(process->pipe_process);
	}

	if( process->next_process != NULL )
	{
		process_destroy(process->next_process);
	}

	free(process);
}

#ifdef TEST_PROCESS
int main(int argc, char **argv, char **env)
{
	process_t *process1;
	process_t *process2;
	process_t *process3;
	process_t *process4;
	int status;

	char *args4[] = {"ls", "-al", NULL};
	char *args3[] = {"wc", "-l", NULL};
	char *args2[] = {"wc", "-c", NULL};
	char *args1[] = {"tr", "3", "X", NULL};

	process1 = process_new("/usr/bin/tr", args1, NULL);
	process2 = process_new("/usr/bin/wc", args2, NULL);
	process3 = process_new("/usr/bin/wc", args3, NULL);
	process4 = process_new("/bin/ls", args4, NULL);

	process1->pipe_process = process2;
	process2->pipe_process = process3;
	process3->pipe_process = process4;

	process_run(process1);
/*
	process_t *process1;
	char *args1[] = {"cat", NULL};

	process1 = process_new("/bin/cat", args1, NULL);
	process1->stdin_filename = "in";
	process1->flag |= PROCESS_STDIN_FILE;
	process1->stdout_filename = "out";
	process1->flag |= PROCESS_STDOUT_APPEND_FILE;
*/	


	//command("ls -al | wc -l | wc -c | tr 3 X");

	process_t *process;

	//process = command("echo Hello world; echo dalsi riadok");
	//process = command("echo \"\\\"hello world\\\"\"; ls -al | wc -l | wc -c | tr '3' 'X'; echo \"\'END\'\"");
	//process = command("echo ahoj svet >> msg; cat < msg");
	//process = command("echo ahoj && echo svet");
	//process = command("[ 1 -eq 2 ] && echo ok");

	process_print(process);

	printf("\nrun process:\n");
	process_run(process);

	process_destroy(process);

	//printf("%s\n", get_command_path("ls"));

	printf("koniec\n");

	return 0;
}
#endif
