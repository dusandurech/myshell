
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>
#include <fcntl.h>

#include "array.h"
#include "dir.h"

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

	if( process->flag & PROCESS_STDIN_FILE )
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

#define STR_SIZE	256

array_t* get_words(const char *str_commandline)
{
	const char *separator[] = { ";", "||", "|", "&&", "&", ">>", ">", "<<", "<" };

	array_t *array;
	char word[STR_SIZE];
	char pair;
	int len;
	int i;
	int j;

	memset(word, 0, STR_SIZE);
	array = array_new();

	len = strlen(str_commandline);
	pair = '\0';

	for(i = 0; i <= len; i++)
	{
		char c;

		c = str_commandline[i];
		//printf("c = >%c<\n", c);

		if( c == '\\' )
		{
			c = str_commandline[++i];
			strncat(word, &c, 1);
			continue;
		}

		if( ( c == '\"' || c == '\'' ) && pair == '\0' )
		{
			pair = c;
			continue;
		}

		if( pair == c && pair != '\0' )
		{
			pair = '\0';
			continue;
		}

		if( pair != '\0' )
		{
			strncat(word, &c, 1);
			continue;
		}

		for(j = 0; j < 9; j++)
		{
			int len;
	
			len = strlen(separator[j]);
	
			if( strncmp(str_commandline+i, separator[j], len) == 0 )
			{
				if( word[0] != '\0' )
				{
					array_add(array, strdup(word) );
				}
	
				array_add(array, strdup(separator[j]) );
	
				//printf("separator add >%s< >%s<\n", word, separator[j]);
	
				memset(word, 0, STR_SIZE);
				i += len-1;
	
				break;
			}
		}
	
		if( j < 9 )
		{
			continue;
		}

		if( ( c == ' ' || c == '\0' ) )
		{
			if( word[0] != '\0' && pair == '\0'  )
			{
				//printf("add world >%s<\n", word);
				array_add(array, strdup(word) );
				memset(word, 0, STR_SIZE);
			}
		}
		else
		{
			strncat(word, &c, 1);
		}
	}

	for(i = 0; i < array->count; i++)
	{
		char *s = (char *) array_get(array, i);

		printf(">%s<\n", s);
	}

	return array;
}

char* get_command_path(char *str_command)
{
	char env_path[STR_SIZE];
	char *dirname;

	strcpy(env_path, getenv("PATH"));

	dirname = strtok(env_path, ":");

	while( dirname != NULL )
	{
		dir_t *dir;
		int i;

		//printf("dirname = %s\n", dirname);

		dir = dir_new(dirname);

		for(i = 0; i < dir->item->count; i++)
		{
			char *s = (char *) array_get(dir->item, i);

			if( strcmp(str_command, s) == 0 )
			{
				char fullpath[STR_SIZE];

				sprintf(fullpath, "%s/%s", dirname, s);

				return strdup(fullpath);
			}
		}

		dir_destroy(dir);

		dirname = strtok(NULL, ":");
	}

	return NULL;
}

char** get_array_from_array(array_t *array)
{
	char **res;
	int i;

	res = (char **) malloc( (array->count+1) * sizeof(char *) );

	for(i = 0; i < array->count; i++ )
	{
		char *s;

		s = (char *)array_get(array, i);
		res[i] = strdup(s);
	}

	res[array->count] = NULL;

	return res;
}

#define PREV_OP_NONE	0
#define PREV_OP_PIPE	1

process_t* command(char *str_command)
{
	process_t *process;
	process_t *process_root;
	process_t *process_actual;
	process_t *process_main;

	char *filename_exec;
	array_t *array;
	array_t *array_arg;
	int i;
	int prev_op;

	array = get_words(str_command);

	array_arg = array_new();

	process_root = NULL;
	process_actual = NULL;
	process_main = NULL;

	filename_exec = NULL;
	prev_op = PREV_OP_NONE;

	for(i = 0; i <= array->count; i++)
	{
		char *s = (char *) array_get(array, i);

		//printf(">%s<\n", s);

		if( filename_exec == NULL )
		{
			filename_exec = get_command_path(s);
			array_add(array_arg, s);
		
			continue;
		}

		if( s == NULL || strcmp(s, ";") == 0 || strcmp(s, "|") == 0 )
		{
			process = process_new(filename_exec, get_array_from_array(array_arg), NULL);

			filename_exec = NULL;
			arrray_do_empty(array_arg);

			if( process_main == NULL )
			{
				process_main = process;
				process_actual = process_main;
			}

			if( prev_op == PREV_OP_PIPE )
			{
				process->pipe_process = process_main;
				process_main = process;

				prev_op = PREV_OP_NONE;
			}

			if( s == NULL || strcmp(s, ";") == 0 )
			{
				if( process_root == NULL )
				{
					process_root = process_main;
				}
				else
				{
					process_t *process_tmp = process_root;

					while( process_tmp->next_process != NULL )
					{
						process_tmp = process_tmp->next_process;
					}

					process_tmp->next_process = process_main;
				}

				process_main = NULL;
			}
	
			if( s != NULL && strcmp(s, "|") == 0 )
			{
				prev_op = PREV_OP_PIPE;
			}
		}
		else
		{
			array_add(array_arg, s);
		}
	}

	return process_root;
}

int main(int argc, char **argv, char **env)
{
/*
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
*/

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
	process = command("echo \"\\\"hello world\\\"\"; ls -al | wc -l | wc -c | tr '3' 'X'; echo \"\'END\'\"");

	process_print(process);

	printf("\nrun process:\n");
	process_run(process);

	//printf("%s\n", get_command_path("ls"));

	printf("koniec\n");

	return 0;
}
