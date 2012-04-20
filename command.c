
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

#define TOKEN_TYPE_CMD		1
#define TOKEN_TYPE_ARG		2
#define TOKEN_TYPE_SEP		3
#define TOKEN_TYPE_MOD		4

typedef struct token_struct
{
	char *str;
	int len;
	int type;
	int ref;
	struct token_struct *next;
} token_t;

token_t *token_new(char *str, int type, int ref)
{
	token_t *token;

	token = (token_t *) malloc( sizeof(token_t) );
	token->str = strdup(str);
	token->len = strlen(str);
	token->type = type;
	token->ref = ref;
	token->next = NULL;

	return token;
}

void token_print(token_t *token)
{
	char *type;

	type = "UNKNOW";

	switch( token->type )
	{
		case TOKEN_TYPE_CMD : type = "CMD"; break;
		case TOKEN_TYPE_ARG : type = "ARG"; break;
		case TOKEN_TYPE_SEP : type = "SEP"; break;
		case TOKEN_TYPE_MOD : type = "MOD"; break;
	}

	printf("--------------\n");
	printf("str = %s\n", token->str);
	printf("len = %d\n", token->len);
	printf("type = %s\n", type);
}

void token_print_all(token_t *token)
{
	while( token != NULL )
	{
		token_print(token);
		token = token->next;
	}

	printf("--------------\n");
}

static token_t* token_get_last(token_t *token_root)
{
	token_t *act = token_root;

	if( token_root == NULL )
	{
		return NULL;
	}
	else
	{
		while( act->next != NULL )
		{
			act = act->next;
		}
	}

	return act;
}

static token_t* token_append(token_t *token_root, token_t *token)
{
	token_t *last;

	if( token_root == NULL )
	{
		return token;
	}

	last = token_get_last(token_root);
	last->next = token;

	return token_root;
}

static token_t* add_word(token_t *token_root, char *str, int len, int ref)
{
	int type;

	type = TOKEN_TYPE_ARG;

	if( token_root == NULL || token_get_last(token_root)->type == TOKEN_TYPE_SEP )
	{
		type = TOKEN_TYPE_CMD;
	}

	token_root = token_append(token_root, token_new(str, type, ref) );
	memset(str, 0, len);

	return token_root;
}

void token_destroy(token_t *token)
{
	free(token->str);
	free(token);
}

void token_destroy_all(token_t *token)
{
	while( token != NULL )
	{
		token_t *del;

		del = token;
		token = token->next;

		token_destroy(del);
	}
}

static token_t* get_token(const char *str_commandline)
{
	const char *separator[] = { ";", "||", "|", "&&", "&", ">>", ">", "<" };

	token_t *token;
	
	char word[STR_SIZE];
	char pair;
	int len;
	int i;
	int j;

	memset(word, 0, STR_SIZE);
	len = strlen(str_commandline);
	pair = '\0';
	token = NULL;

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


		for(j = 0; j < 8; j++)
		{
			int len;
	
			len = strlen(separator[j]);
	
			if( strncmp(str_commandline+i, separator[j], len) == 0 )
			{
				if( word[0] != '\0' )
				{
					token = add_word(token, word, STR_SIZE, i-strlen(word));
				}	// no else !!!

				if( word[0] == '\0' )
				{
					int type;
					
					type = (j <= 4 ? TOKEN_TYPE_SEP : TOKEN_TYPE_MOD);
					token = token_append(token, token_new((char *)separator[j], type, i) );
				}
		
				i += len-1;
	
				break;
			}
		}
	
		if( j < 8 )
		{
			continue;
		}

		if( ( c == ' ' || c == '\0' ) )
		{
			if( word[0] != '\0' && pair == '\0'  )
			{
				token = add_word(token, word, STR_SIZE, i-strlen(word));
			}
		}
		else
		{
			strncat(word, &c, 1);
		}
	}

	if( pair != '\0' )
	{
		printf("Vstup nie je ukonceny znakom: >%c<\n", pair);
		return NULL;
	}

	return token;
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
	{ .str = "<",		.len = 1,	.flag = PROCESS_STDIN_FILE		}
};

#define CONTROL_STRING_COUNT		( sizeof(control_string_list) / sizeof(control_string_t) )

static int set_flag_to_process(process_t *process, char *str)
{
	int i;

	if( process == NULL )
	{
		return 0;
	}

	if( str == NULL )
	{
		return 0;
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

	return 1;
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

static void print_ref(char *str, int ref)
{
	int i;

	printf("%s\n", str);

	for(i = 0; i < ref; i++)
	{
		putchar(' ');
	}

	putchar('^');
	putchar('\n');
}

static int token_check(token_t *token, char *str)
{
	token_t *token_prev;
	int fd_count[3];
	char *res;

	res = NULL;

	token_prev = NULL;

	while( token != NULL )
	{
		if( token->type == TOKEN_TYPE_CMD )
		{
			memset(fd_count, 0, 3*sizeof(int));
		}

		if( token->type == TOKEN_TYPE_SEP )
		{
			if( ( !strcmp(token->str, "||") || !strcmp(token->str, "&&") || !strcmp(token->str, "|") ) &&
			    (token->next == NULL || token->next->type != TOKEN_TYPE_CMD) )
			{
				printf("CHYBA: za separatorom %s nenasleduje prikaz\n", token->str);
				print_ref(str, token->ref);

				return -1;
			}
		}

		if( token->type == TOKEN_TYPE_SEP )
		{
			if( token->next != NULL && token->next->type != TOKEN_TYPE_CMD )
			{
				printf("CHYBA: za separatorom %s nenasleduje prikaz\n", token->str);
				print_ref(str, token->ref);

				return -1;
			}
		}

		if( token->type == TOKEN_TYPE_MOD )
		{
			int i;

			if( !strcmp(token->str, "<") ) fd_count[0]++;
			if( !strcmp(token->str, ">") ) fd_count[1]++;
			if( !strcmp(token->str, ">>") ) fd_count[1]++;

			for(i = 0; i < 3; i++)
			{
				if( fd_count[i] > 1 )
				{
					printf("CHYBA: viac krat pouzity presmerovavac vstupu alebo vystupu\n");
					print_ref(str, token->ref);
	
					return -1;
				}
			}

			if( token->next == NULL || token->next->type != TOKEN_TYPE_ARG )
			{
				printf("CHYBA: za presmerovacom %s nenasleduje argument presmerovania\n", token->str);
				print_ref(str, token->ref);

				return -1;
			}
		}

		if( token->type == TOKEN_TYPE_CMD && (res=get_command_path(token->str)) == NULL )
		{
			free(res);
			printf("CHYBA: prikaz %s neexistuje\n", token->str);
			print_ref(str, token->ref);

			return -1;
		}
	
		if( res != NULL )
		{
			free(res);
			res = NULL;
		}

		token_prev = token;
		token = token->next;
	}

	return 0;
}

process_t* command(char *str_command)
{
	process_t *process_root;
	process_t *process;

	char *filename_exec;
	char *str_command_expand;

	token_t *token_root;
	token_t *token;
	array_t *array_arg;

	int end;
	int i;

	str_command_expand = expand_var(str_command);
	token_root = get_token(str_command_expand);

	token = token_root;
	end = 0;
	process_root = NULL;
	process = NULL;
	array_arg = array_new();
	filename_exec = NULL;

	//token_print_all(token);

	if( token_check(token, str_command_expand) == -1 )
	{
		return NULL;
	}

	while( end == 0 )
	{
		int type;

		if( token == NULL )
		{
			type = TOKEN_TYPE_SEP;
		}
		else
		{
			type = token->type;
		}

		switch( type )
		{
			case TOKEN_TYPE_CMD:
				filename_exec = get_command_path(token->str);
	
				if( filename_exec == NULL )
				{
					fprintf(stderr, "Command %s not found !\n", token->str);
					return NULL;
				}
	
				process = process_new();
				array_add(array_arg, strdup(token->str));
			break;

			case TOKEN_TYPE_ARG:
				array_add(array_arg, strdup(token->str));
			break;

			case TOKEN_TYPE_SEP:
				if( process != NULL )
				{
					char **argv = (char **) array_get_clone_array(array_arg, strdup);
					char **env = (char **) env_import();
			
					process_set(process, filename_exec, argv, env);

					if( token != NULL)
					{
						set_flag_to_process(process, token->str);
					}

					process_root = append_to_process(process_root, process);
		
					process = NULL;
					filename_exec = NULL;
					arrray_do_empty_item(array_arg, free);
				}

				if( token == NULL )
				{
					end = 1;
				}
			break;

			case TOKEN_TYPE_MOD:
				set_flag_to_process(process, token->str);

				if( token->str[0] == '>' )
				{
					process->stdout_filename = strdup(token->next->str);
				}
				else if( token->str[0] == '<' )
				{
					process->stdin_filename = strdup(token->next->str);
				}

				token = token->next;
			break;
		}

		if( end == 1 )
		{
			break;
		}

		token = token->next;
	}

	token_destroy_all(token_root);
	array_destroy_item(array_arg, free);

	//process_print(process_root);

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
