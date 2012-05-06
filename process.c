
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#include "main.h"

#include "array.h"
#include "dir.h"
#include "jobs.h"
#include "terminal.h"

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

static int do_process_to_string(process_t *process, char *str)
{
	int len;
	int i;

	len = 0;

	if( process->prev_process != NULL && process->prev_process->flag & PROCESS_PIPE )
	{
		len += do_process_to_string(process->prev_process, str);
		len += sprintf(str+len, "| ");
	}

	for(i = 0; process->param[i] != NULL; i++)
	{
		len += sprintf(str+len, "%s ", process->param[i]);
	}

	return len;
}

char* process_to_string(process_t *process)
{
	static char str[STR_SIZE];

	do_process_to_string(process, str);

	return str;
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

	if( process->flag & PROCESS_PIPE )
	{
		printf("|");
	}

	if( process->flag & PROCESS_STDIN_DOC_HERE )
	{
		printf("<< \"DOCUMENT HETE\" ");
	}

	putchar('\n');

	printf("prev process %p\n", process->prev_process);
	printf("next process %p\n", process->next_process);

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

		if( process->flag & PROCESS_STDOUT_FILE )
		{
			unlink(process->stdout_filename);
		}

		fd = open(process->stdout_filename, mask, 0666);

		if( fd < 0 )
		{
			fprintf(stderr, "Nemozem otvorit subor %s pre zapis !", process->stdout_filename);
		}
	}

	if( process->flag & PROCESS_STDIN_FILE )
	{
		int fd;

		close(0);
		fd = open(process->stdin_filename, O_RDONLY);

		if( fd < 0 )
		{
			fprintf(stderr, "Nemozem otvorit subor %s pre citanie !", process->stdin_filename);
		}
	}

	//fprintf(stderr, "EXEC %s\n", process->filename_exec);

	res = execve(process->filename_exec, process->param, process->env);

	if( res < 0 )
	{
		fprintf(stderr, "Nemozem spustit subor %s !", process->filename_exec);
	}

	return res;
}

static pid_t create_session()
{
	pid_t session_pid;
	pid_t process_pid;
	
	session_pid = getpid();
	process_pid = getpid();

	setpgid(process_pid, session_pid);

	return session_pid;
}

static int add_to_session(pid_t process_pid, pid_t session_pid)
{
	setpgid(process_pid, session_pid);

	return 0;
}

static int process_pipe(process_t *process, pid_t session_pid)
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

			if( process->prev_process != NULL &&
			    process->prev_process->prev_process != NULL  )
			{
				process_pipe(process->prev_process, session_pid);
			}
			else
			{
				add_to_session(getpid(), session_pid);
				process_exec(process->prev_process);
			}
		break;
		
		default :
			add_to_session(getpid(), session_pid);

			close(0);
			dup(fd[0]);
			close(fd[1]);

			process_exec(process);
		break;
	}

	return 0;
}

int process_wait(const pid_t wait_pid, const char *cmd)
{
	pid_t ret_pid;
	int status;

	do{
		//printf("wait in process\n");
		ret_pid = waitpid(-1, &status, WUNTRACED|WCONTINUED);
		//printf("wait pid = %d\n", ret_pid);

		if( WIFSTOPPED(status) )
		{
			int jobs_id;
	
			printf("Process stopeed\n");
	
			jobs_id = jobs_add_process(cmd, ret_pid, JOBS_STAT_STOP);
			jobs_print_process(jobs_id);
		}

		if( WIFCONTINUED(status) )
		{
			int jobs_id;
	
			if( ret_pid != wait_pid )
			{
				printf("Process continued\n");
			}

			jobs_id = jobs_add_process(cmd, ret_pid, JOBS_STAT_RUN);
			jobs_print_process(jobs_id);

			ret_pid = -1;

			continue;
		}

		if( WIFEXITED(status) )
		{
			if( ret_pid != wait_pid )
			{
				int exit_status;
	
				exit_status = WEXITSTATUS(status);
				printf("Process exited %d\n", exit_status);
			}

			jobs_clean_process(ret_pid);
		}

		if( WIFSIGNALED(status) )
		{

			if( ret_pid != wait_pid )
			{
				int signum;
	
				signum = WTERMSIG(status);
				printf("Process killed by signal %d\n", signum);
			}

			jobs_clean_process(ret_pid);
		}

	}while( ret_pid != wait_pid );

	return status;
}

int process_run(process_t *process)
{
	pid_t pid;
	pid_t session_pid;
	int is_pipe;
	int status;
	int fd[2];

	if( process == NULL )
	{
		return;
	}

	is_pipe = 0;

	if( process->flag & PROCESS_PIPE )
	{
		is_pipe = 1;

		while( process->flag & PROCESS_PIPE )
		{
			process = process->next_process;
		}
	}

	switch( ( pid = fork() ) )
	{
		case -1 :	// error
			fprintf(stderr, "fork error !\n");
			return -1;
		break;

		case 0 :	// child
			session_pid = create_session();

			if( ! ( process->flag & PROCESS_NO_WAIT ) )
			{
				term_set_control(session_pid);
			}

			signal_set_for_process();

#if 0
			if( process->pipe_process != NULL )
			{
				process_pipe(process, session_pid);
			}
			else
			{
				process_exec(process);
			}
#endif
			if( is_pipe )
			{
				process_pipe(process, session_pid);
			}
			else
			{
				process_exec(process);
			}
		break;

		default :	// parrent
			session_pid = pid;		// ugly, hack :)

			if( ! ( process->flag & PROCESS_NO_WAIT ) )
			{
				status = process_wait(session_pid, process_to_string(process));
			}
			else
			{
				int jobs_id;

				jobs_id = jobs_add_process(process->filename_exec, session_pid, JOBS_STAT_RUN);
				jobs_print_process(jobs_id);
			}

			if( process->flag & PROCESS_AND )
			{
				if( status == 0 )
				{
					process_run(process->next_process);
				}
			}
			else if( process->flag & PROCESS_OR )
			{
				if( status != 0 )
				{
					process_run(process->next_process);
				}
			}
			else
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

	if( process->next_process != NULL )
	{
		process_destroy(process->next_process);
	}

	free(process);
}

//#define TEST_PROCESS

#ifdef TEST_PROCESS
int main(int argc, char **argv, char **env)
{
	process_t *process0;
	process_t *process1;
	process_t *process2;
	process_t *process3;
	process_t *process4;
	process_t *process5;
	int status;

	char *args0[] = {"echo", "begin", NULL};
	char *args1[] = {"ls", "-al", NULL};
	char *args2[] = {"wc", "-l", NULL};
	char *args3[] = {"wc", "-c", NULL};
	char *args4[] = {"tr", "3", "X", NULL};
	char *args5[] = {"echo", "end", NULL};

	signal_init();
	term_init();
	jobs_init();

	process0 = process_new();
	process1 = process_new();
	process2 = process_new();
	process3 = process_new();
	process4 = process_new();
	process5 = process_new();

	process_set(process0, "/bin/echo", args0, NULL);
	process0->flag = 0;

	process_set(process1, "/bin/ls", args1, NULL);
	process1->flag = PROCESS_PIPE;

	process_set(process2, "/usr/bin/wc", args2, NULL);
	process2->flag = PROCESS_PIPE;

	process_set(process3, "/usr/bin/wc", args3, NULL);
	process3->flag = PROCESS_PIPE;

	process_set(process4, "/usr/bin/tr", args4, NULL);
	process4->flag = 0;

	process_set(process5, "/bin/echo", args5, NULL);
	process5->flag = 0;

	process0->prev_process = NULL;
	process0->next_process = process1;

	process1->prev_process = NULL;
	process1->next_process = process2;

	process2->prev_process = process1;
	process2->next_process = process3;

	process3->prev_process = process2;
	process3->next_process = process4;

	process4->prev_process = process3;
	process4->next_process = process5;

	process5->prev_process = process4;
	process5->next_process = NULL;

	term_set_old();

	process_print(process1);

	process_run(process0);

	term_set_control(getpid());
	term_set_new();

	term_quit();

/*
	process_t *process1;
	char *args1[] = {"cat", NULL};

	process1 = process_new("/bin/cat", args1, NULL);
	process1->stdin_filename = "in";
	process1->flag |= PROCESS_STDIN_FILE;
	process1->stdout_filename = "out";
	process1->flag |= PROCESS_STDOUT_APPEND_FILE;
*/	


#if 0
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
#endif

	printf("koniec\n");

	return 0;
}
#endif
