
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <sys/types.h>

#include "dir.h"

dir_t* dir_new(const char *dirname)
{
	DIR *dir;
	struct dirent *item;
	dir_t *my_dir;

	dir = opendir(dirname);

	if( dir == NULL )
	{
		//fprintf(stderr, "Path %s can not load\n", dirname);
		return NULL;
	}

	my_dir = (dir_t *) malloc( sizeof(dir_t) );
	my_dir->path = strdup(dirname);
	my_dir->item = array_new();

	while( ( item = readdir(dir) ) != NULL )
	{
		array_add(my_dir->item, strdup(item->d_name));
	}

	closedir(dir);

	return my_dir;
}

void dir_print(dir_t *dir)
{
	int i;

	printf("path = %s\n", dir->path);

	for(i = 0; i < dir->item->count; i++)
	{
		char *s = (char *) array_get(dir->item, i);

		printf("%s\n", s);
	}
}

void dir_destroy(dir_t *dir)
{
	array_destroy_item(dir->item, free);
	free(dir->path);
	free(dir);
}
