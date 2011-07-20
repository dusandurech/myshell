
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <unistd.h>

#include "main.h"

#include "util.h"
#include "array.h"
#include "history.h"

#define HISTORY_MAX_COUNT	500

static char str_current_line[STR_LINE_SIZE];
static array_t *array_history;
static offset;

static char* get_path_history_file()
{
	static char path[STR_PATH_SIZE];

	if( path[0] == '\0' )
	{
		sprintf(path, "%s/." SHELL_NAME "_history", get_home_dir() );
	}

	return path;
}

int history_init()
{
	array_history = array_new();

	str_current_line[0] = '\0';
	offset = -1;

	return 0;
}

int history_load()
{
	char line[STR_LINE_SIZE];
	FILE *file;

	file = fopen(get_path_history_file(), "rt");

	if( file == NULL )
	{
		return -1;
	}

	while( fgets(line, STR_LINE_SIZE-1, file) != NULL )
	{
		int len;

		len = strlen(line);

		if( line > 0 )
		{
			line[len-1] = '\0';
		}

		history_add(line);
	}

	fclose(file);

	return 0;
}

int history_save()
{
	FILE *file;
	int i;

	file = fopen(get_path_history_file(), "wt");

	if( file == NULL )
	{
		return -1;
	}

	for(i = 0; i < array_history->count; i++)
	{
		char *s;

		s = (char *) array_get(array_history, i);
		fputs(s, file);
		fputc('\n', file);
	}

	fclose(file);

	return 0;
}

void history_backup_current_line(const char *str_command)
{
	strcpy(str_current_line, str_command);
}

void history_add(const char *str_command)
{
	array_add(array_history, strdup(str_command));

	if( array_history->count > HISTORY_MAX_COUNT )
	{
		array_del_item(array_history, 0, free);
	}

	offset = array_history->count;
}

int history_up()
{
	if( offset <= 0 )
	{
		return 0;
	}

	offset--;
	return 1;
}

int history_down()
{
	if( offset >= array_history->count )
	{
		return 0;
	}

	offset++;
	return 1;
}

char* history_get_select()
{
	if( offset == array_history->count )
	{
		return str_current_line;
	}

	if( offset < 0 || offset > array_history->count )
	{
		return NULL;
	}

	return array_get(array_history, offset);
}

int history_quit()
{
	array_destroy_item(array_history, free);

	return 0;
}

//#define TEST_HISTORY

#ifdef TEST_HISTORY
int main()
{
	return 0;
}
#endif
