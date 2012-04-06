
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

typedef struct control_string_struct
{
	char *str;
	int len;
	unsigned int flag;
	int detail;
} control_string_t;

static control_string_t control_string_list[] =
{
	{ .str = ";",		.len = 1,	.flag = 0				},
	{ .str = "||",		.len = 2,	.flag = PROCESS_OR			},
	{ .str = "|",		.len = 1,	.flag = PROCESS_PIPE			},
	{ .str = "&&",		.len = 2,	.flag = PROCESS_AND			},
	{ .str = "&",		.len = 1,	.flag = PROCESS_NO_WAIT			},
	{ .str = ">>",		.len = 2,	.flag = PROCESS_STDOUT_APPEND_FILE	},
	{ .str = ">",		.len = 1,	.flag = PROCESS_STDOUT_FILE		},
	{ .str = "<<",		.len = 2,	.flag = 0				},
	{ .str = "<",		.len = 1,	.flag = PROCESS_STDIN_FILE		}
};

#define CONTROL_STRING_COUNT		( sizeof(control_string_list) / sizeof(control_string_t) )

static int is_separator(process_t *process, char *str)
{
	int i;

	if( process == NULL )
	{
		return 0;
	}

	if( str == NULL )
	{
		return 1;
	}

	for(i = 0; i < CONTROL_STRING_COUNT; i++)
	{
		control_string_t *cs;

		cs = &control_string_list[i];

		if( strncmp(cs->str, str, cs->len) == 0 )
		{
			process->flag |= cs->flag;
			return 1;
		}
	}

	return 0;
}

static process_t* append_to_process(process_t *process_root, process_t *process)
{
	if( process_root == NULL )
	{
		return process;
	}
	else
	{
		process_t *process_act = process_root;

		while( process_act->next_process != NULL )
		{
			process_act = process_act->next_process;
		}

		process_act->next_process = process;
		process->prev_process = process_act;
	}

	return process_root;
}

process_t* command(char *str_command)
{
	process_t *process_root;
	process_t *process;
	char *filename_exec;
	char *str_command_expand;
	array_t *array;
	array_t *array_arg;
	int i;

	str_command_expand = expand_var(str_command);
	array = get_words(str_command_expand);

	if( array->count == 0 )
	{
		array_destroy_item(array, free);
		return NULL;
	}

	process_root = NULL;
	process = NULL;
	array_arg = array_new();
	filename_exec = NULL;

	for(i = 0; i <= array->count; i++)
	{
		char *s = (char *) array_get(array, i);
		control_string_t *control_string;

		//printf(">%s<\n", s);

		if( s != NULL && filename_exec == NULL )
		{
			filename_exec = get_command_path(s);

			if( filename_exec == NULL )
			{
				fprintf(stderr, "Command not found !\n");
				return NULL;
			}

			process = process_new();
			array_add(array_arg, strdup(s));
		
			continue;
		}

		if( is_separator(process, s) )
		{
			if( process != NULL )
			{
				char **argv = (char **) array_get_clone_array(array_arg, strdup);
				char **env = (char **) env_import();
	
				process_set(process, filename_exec, argv, env);
	
				process_root = append_to_process(process_root, process);

				process = NULL;
				filename_exec = NULL;
				arrray_do_empty_item(array_arg, free);
			}
		}
		else
		{
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

//#define TEST_COMMAND

#ifdef TEST_COMMAND
int main(int argc, char **argv, char **env)
{
	process_t *process;

	process = command("echo begin; echo \"\\\"hello world\\\"\"; ls -al | wc -l | wc -c | tr '3' 'X'; echo \"\'END\'\"; echo end");
	//process = command("echo Hello;");

	process_print(process);

	signal_init();
	term_init();
	jobs_init();

	term_set_old();

	printf("\nrun process:\n");

	//process_run(process);

	term_set_control(getpid());
	term_set_new();

	term_quit();

	process_destroy(process);

	return 0;
}
#endif
