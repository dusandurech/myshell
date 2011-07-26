
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

#define TYPE_SEPARATOR		1
#define TYPE_RECIRECTOR		2
#define TYPE_PIPE		3

#define DTL_NONE		0

#define SEP_THEN		1
#define SEP_AND			2
#define SEP_OR			3
#define SEP_NOWAIT		4

#define RDR_APPEND		6
#define RDR_OUT			7
#define RDR_DOC_HETE		8
#define RDR_IN			9

static int handler_not_implemented(process_t *process, array_t *array, int *offset)
{
	char *str_op = (char *) array_get(array, (*offset));

	fprintf(stderr, "Operator \'%s\' not implemented\n", str_op);
	return -1;
}

static int handler_sep_or(process_t *process, array_t *array, int *offset)
{
	process->flag |= PROCESS_OR;
	return 0;
}

static int handler_sep_and(process_t *process, array_t *array, int *offset)
{
	process->flag |= PROCESS_AND;
	return 0;
}

static int handler_sep_nowait(process_t *process, array_t *array, int *offset)
{
	process->flag |= PROCESS_NO_WAIT;
	return 0;
}

static int handler_rdr_out(process_t *process, array_t *array, int *offset)
{
	char *stdout_filename = (char *) array_get(array, ++(*offset));

	process->stdout_filename = strdup(stdout_filename);
	process->flag |= PROCESS_STDOUT_FILE;

	return 0;
}

static int handler_rdr_append(process_t *process, array_t *array, int *offset)
{
	char *stdout_filename = (char *) array_get(array, ++(*offset));

	process->stdout_filename = strdup(stdout_filename);
	process->flag |= PROCESS_STDOUT_APPEND_FILE;

	return 0;
}

static int handler_rdr_in(process_t *process, array_t *array, int *offset)
{
	char *stdin_filename = (char *) array_get(array, ++(*offset));

	process->stdin_filename = strdup(stdin_filename);
	process->flag |= PROCESS_STDIN_FILE;

	return 0;
}

typedef struct control_string_struct
{
	char *str;
	int len;
	int type;
	int detail;

	int (*fce)(process_t *process, array_t *array, int *offset);
} control_string_t;

static control_string_t control_string_list[] =
{
	{ .str = ";",		.len = 1,	.type = TYPE_SEPARATOR, 	.detail = SEP_THEN,	.fce =  NULL },
	{ .str = "||",		.len = 2,	.type = TYPE_SEPARATOR, 	.detail = SEP_OR,	.fce =  handler_sep_or },
	{ .str = "|",		.len = 1,	.type = TYPE_PIPE, 		.detail = DTL_NONE,	.fce =  NULL },
	{ .str = "&&",		.len = 2,	.type = TYPE_SEPARATOR, 	.detail = SEP_OR,	.fce =  handler_sep_and },
	{ .str = "&",		.len = 1,	.type = TYPE_SEPARATOR, 	.detail = SEP_NOWAIT,	.fce =  handler_sep_nowait },
	{ .str = ">>",		.len = 2,	.type = TYPE_RECIRECTOR, 	.detail = RDR_APPEND,	.fce =  handler_rdr_append },
	{ .str = ">",		.len = 1,	.type = TYPE_RECIRECTOR, 	.detail = RDR_OUT,	.fce =  handler_rdr_out },
	{ .str = "<<",		.len = 2,	.type = TYPE_RECIRECTOR, 	.detail = RDR_DOC_HETE,	.fce =  handler_not_implemented },
	{ .str = "<",		.len = 1,	.type = TYPE_RECIRECTOR, 	.detail = RDR_IN,	.fce =  handler_rdr_in }
};

#define CONTROL_STRING_COUNT		( sizeof(control_string_list) / sizeof(control_string_t) )

static control_string_t* work_control(process_t *process, array_t *array, int *offset)
{
	char *str;
	int i;

	if( process == NULL )
	{
		return NULL;
	}

	str = array_get(array, *offset);

	if( str == NULL )
	{
		return &control_string_list[0];
	}

	for(i = 0; i < CONTROL_STRING_COUNT; i++)
	{
		control_string_t *cs;

		cs = &control_string_list[i];

		if( strncmp(cs->str, str, cs->len) == 0 )
		{
			if( cs->fce != NULL )
			{
				int res;

				res = cs->fce(process, array, offset);
			}

			return cs;
		}
	}

	return NULL;
}

static process_t* append_to_process(process_t *process_root, process_t *process)
{
	if( process_root == NULL )
	{
		return process;
	}
	else
	{
		process_t *process_tmp = process_root;

		while( process_tmp->next_process != NULL )
		{
			process_tmp = process_tmp->next_process;
		}

		process_tmp->next_process = process;
	}

	return process_root;
}

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

	if( array->count == 0 )
	{
		array_destroy_item(array, free);
		return NULL;
	}

	array_arg = array_new();

	process_root = NULL;
	process_actual = NULL;
	process_main = NULL;

	filename_exec = NULL;
	prev_op = PREV_OP_NONE;

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
				//printf("filename_exec = %s\n", s);
				fprintf(stderr, "Command not found !\n");

				
				return NULL;
			}

			process = process_new();
			array_add(array_arg, strdup(s));
		
			continue;
		}

		if( ( control_string = work_control(process, array, &i) ) != NULL )
		{
			char **argv = (char **) array_get_clone_array(array_arg, strdup);
			char **env = (char **) env_import();

			process_set(process, filename_exec, argv, env);

			filename_exec = NULL;
			//array_print_string(array_arg);
			arrray_do_empty_item(array_arg, free);

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

			if( control_string->type == TYPE_PIPE )
			{
				prev_op = PREV_OP_PIPE;
			}
			else
			{
				process_root = append_to_process(process_root, process);
				process_main = NULL;
				process = NULL;
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
