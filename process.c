
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <assert.h>

#define PROCESS_STDOUT_FILE		4
#define PROCESS_STDIN_FILE		8
#define PROCESS_STDOUT_APPEND_FILE	16
#define PROCESS_STDIN_DOC_HERE		32
#define PROCESS_NO_WAIT			64

typedef struct doument_here_struct
{
	char *text;
	char *eof;
	unsigned int length;
} doument_here_t;

typedef struct process_struct
{
	char *filename_exec;
	char **param;
	char **env;

	char *stdin_filename;
	char *stdout_filename;

	unsigned int flag;
	
	doument_here_t *doument_here;

	struct process_struct *pipe_process;
	struct process_struct *next_process;
} process_t;

process_t* process_new(char *filename_exec, char **param, char **env)
{
	process_t *process;

	process = (process_t *) malloc( sizeof(process_t) );
	memset(process, 0, sizeof(process_t));

	process->filename_exec = filename_exec;
	process->param = param;
	process->env = env;

	return process;
}

int process_exec(process_t *process)
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

int process_pipe(process_t *process)
{
	int fd[2];
	int status;

	pipe(fd);

	switch( fork() )
	{
		case 0 :
			close(1);
			dup(fd[1]);
			close(fd[0]);

			if( process->pipe_process != NULL && process->pipe_process->pipe_process != NULL )
			{
				process_pipe(process->pipe_process);
			}
			else
			{
				process_exec(process->pipe_process);
			}
		break;
		
		default :
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
				process_pipe(process);
			}
			else
			{
				process_exec(process);
			}
		break;

		default :	// parrent
			wait(&status);

			if( process->next_process != NULL )
			{
				process_run(process->next_process);
			}
		break;
	}

	return status;
}

void process_destroy(process_t *process)
{
	free(process);
}

void commandline_analyze(const char *str_commandline)
{
	int len;
	int i;

	len = strlen(str_commandline);

	for(i = 0; i < len; i++)
	{
		
	}
}


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

/*
	process_t *process1;
	char *args1[] = {"cat", NULL};

	process1 = process_new("/bin/cat", args1, NULL);
	process1->stdin_filename = "in";
	process1->flag |= PROCESS_STDIN_FILE;
	process1->stdout_filename = "out";
	process1->flag |= PROCESS_STDOUT_APPEND_FILE;
*/	
	process_run(process1);


	printf("koniec\n");

	return 0;
}
