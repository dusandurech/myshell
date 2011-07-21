
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>

#include "main.h"

#include "env.h"
#include "array.h"
#include "dir.h"

#include "expand_var.h"
#include "expand_regexp.h"

#include "process.h"
#include "command.h"

static array_t* get_words(const char *str_commandline)
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
				expand_regexp(word, array);
				//array_add(array, strdup(word) );
				memset(word, 0, STR_SIZE);
			}
		}
		else
		{
			strncat(word, &c, 1);
		}
	}

#if 0
	for(i = 0; i < array->count; i++)
	{
		char *s = (char *) array_get(array, i);

		printf(">%s<\n", s);
	}
#endif

	return array;
}

static void destroy_words(char **words)
{
	int i;

	for(i = 0; words[i] != NULL; i++)
	{
		free(words[i]);
	}

	free(words);
}

static char* get_command_path(char *str_command)
{
	char env_path[STR_PATH_SIZE];
	char *dirname;

	strcpy(env_path, getenv("PATH"));

	dirname = strtok(env_path, ":");

	while( dirname != NULL )
	{
		char fullpath[STR_PATH_SIZE];
		dir_t *dir;

		sprintf(fullpath, "%s/%s", dirname, str_command);

		if( access(fullpath, F_OK|R_OK|X_OK) == 0 )
		{
			return strdup(fullpath);
		}

		dirname = strtok(NULL, ":");
	}

	return NULL;
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
	char *str_command_expand;
	array_t *array;
	array_t *array_arg;
	int i;
	int prev_op;

	str_command_expand = expand_var(str_command);
	array = get_words(str_command_expand);

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

		if( s != NULL && filename_exec == NULL )
		{
			filename_exec = get_command_path(s);

			if( filename_exec == NULL )
			{
				//printf("filename_exec = %s\n", s);
				fprintf(stderr, "Command not found !\n");
				return NULL;
			}

			process = process_new();
			array_add(array_arg, strdup(s));
		
			continue;
		}

		if( process != NULL && ( s == NULL || strcmp(s, ";") == 0 || strcmp(s, "|") == 0 ||
					 strcmp(s, "&&") == 0 || strcmp(s, "||") == 0 || strcmp(s, "&") == 0) )
		{
			process_set(process, filename_exec, (char **)array_get_clone_array(array_arg, strdup), env_import());

			filename_exec = NULL;
			//array_print_string(array_arg);
			arrray_do_empty_item(array_arg, free);

			if( s != NULL && strcmp(s, "&&") == 0 )
			{
				process->flag |= PROCESS_AND;
			}

			if( s != NULL && strcmp(s, "||") == 0 )
			{
				process->flag |= PROCESS_OR;
			}

			if( s != NULL && strcmp(s, "&") == 0 )
			{
				process->flag |= PROCESS_NO_WAIT;
			}

			if( process_main == NULL )
			{
				process_main = process;
				process_actual = process_main;
			}

			if( prev_op == PREV_OP_PIPE )
			{
				process->pipe_process = process_main;
				process_main = process;
				process = NULL;

				prev_op = PREV_OP_NONE;
			}

			if( s != NULL && strcmp(s, "|") == 0 )
			{
				prev_op = PREV_OP_PIPE;
			}
			else
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
				process = NULL;
			}
		}
		else
		{
			if( s != NULL && strcmp(s, ">") == 0 )
			{
				char *stdout_filename = (char *) array_get(array, ++i);

				process->stdout_filename = strdup(stdout_filename);
				process->flag |= PROCESS_STDOUT_FILE;
	
				continue;
			}

			if( s != NULL && strcmp(s, ">>") == 0 )
			{
				char *stdout_filename = (char *) array_get(array, ++i);

				process->stdout_filename = strdup(stdout_filename);
				process->flag |= PROCESS_STDOUT_APPEND_FILE;
	
				continue;
			}

			if( s != NULL && strcmp(s, "<") == 0 )
			{
				char *stdin_filename = (char *) array_get(array, ++i);

				process->stdin_filename = strdup(stdin_filename);
				process->flag |= PROCESS_STDIN_FILE;

				continue;
			}

			if( process != NULL && s != NULL )
			{
				array_add(array_arg, strdup(s));
			}
		}
	}

	array_destroy_item(array, free);
	array_destroy_item(array_arg, free);

	return process_root;
}

#ifdef TEST_COMMAND
int main(int argc, char **argv, char **env)
{
	process_t *process;

	process = command("echo begin; echo \"\\\"hello world\\\"\"; ls -al | wc -l | wc -c | tr '3' 'X'; echo \"\'END\'\"; echo end");
	//process = command("echo Hello world;");

	process_print(process);

	printf("\nrun process:\n");
	process_run(process);

	process_destroy(process);

	return 0;
}
#endif
