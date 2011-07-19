
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "array.h"
#include "dir.h"
#include "automat.h"
#include "util.h"
#include "regexp.h"

static void do_expand_regexp(automat_t *automat, char *line, array_t *array)
{
	dir_t *dir;
	char *current_regexp;
	int status_begin;
	int i;

	//printf("call %s\n", line);

	current_regexp = (char *) automat_get_curent_regexp(automat);
	status_begin = automat_get_current_status(automat);

	//printf("first line = %s >%s<\n", line, automat_get_curent_regexp(automat));

	if( automat_get_current_status(automat) == -1 )
	{
		return;
	}

	if( line[0] == '\0' )
	{
		dir = dir_new("./");
	}
	else
	{
		dir = dir_new(line);
	}

	if( dir == NULL )
	{
		return;
	}

	if( current_regexp[0] == '\0' )
	{
		//printf("FIN\n");
		array_add(array, strdup(line));
		return;
	}

	for(i = 0; i < dir->item->count; i++)
	{
		char *s = (char *) array_get(dir->item, i);

		current_regexp = (char *) automat_get_curent_regexp(automat);

		if(  current_regexp[0] == '.' )
		{
			if( s[0] != '.' )
			{
				continue;
			}
		}
		else
		{
			if( s[0] == '.' )
			{
				continue;
			}
		}

		//printf("automat_get_curent_regexp = %s %s\n", current_regexp, s);

		//printf("%s\n", s);

		automat_steps_for_string(automat, s);

		//printf("status = %d\n", automat_get_current_status(automat));

		if( automat_get_current_status(automat) == -1 )
		{
			automat_set_current_status(automat, status_begin);
			continue;
		}

		if( automat_is_final_status(automat) )
		{
			int len;
	
			len = strlen(line);
			append_file_to_path(line, s);
			//printf("OK %s\n", line);
			array_add(array, strdup(line));
			line[len] = '\0';

			automat_set_current_status(automat, status_begin);

			continue;
		}

		if( automat_get_current_status(automat) != -1 )
		{
			int status_prev;
			int status_next;

			//printf("line = %s >%s<\n", line, automat_get_curent_regexp(automat));

			status_prev = automat_get_current_status(automat);
			automat_step(automat, '/');
			status_next = automat_get_current_status(automat);

			if( status_next == status_prev+1 )
			{
				int len;
	
				//printf("status_next = %d\n", status_next);
				len = strlen(line);
				append_file_to_path(line, s);
				strcat(line, "/");
				//printf("line = %s >%s<\n", line, automat_get_curent_regexp(automat));
				do_expand_regexp(automat, line, array);
				line[len] = '\0';
	
			}

			automat_set_current_status(automat, status_begin);
			continue;
		}
	}

	dir_destroy(dir);
}

static void expand_nothing_regexp(char *str_regexp, array_t *array)
{
	char *str;

	str = get_del_metacharakter(str_regexp);
	array_add(array, strdup(str));
}

int expand_regexp(char *str_regexp, array_t *array)
{
	char str[STR_PATH_SIZE];
	automat_t *automat;
	int count_old;

	memset(str, 0, STR_PATH_SIZE);

	automat = automat_new(str_regexp);
	count_old = array->count;
	//automat_print(automat);

	do_expand_regexp(automat, str, array);

	automat_destroy(automat);
	
	if( array->count == count_old )
	{
		expand_nothing_regexp(str_regexp, array);
	}

	return array->count - count_old;
}

//#define TEST_EXPAND_REGEXP

#ifdef TEST_EXPAND_REGEXP
int main()
{
	array_t *array;

	array = array_new();
	expand_regexp("*.c", array);

	array_print_string(array);
	array_destroy(array);

	return 0;
}
#endif
 
