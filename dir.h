
#ifndef DIR_H

#define DIR_H

#include "array.h"

typedef struct dir_struct
{
	char *path;
	array_t *item;
} dir_t;

extern dir_t* dir_new(const char *dirname);
extern void dir_print(dir_t *dir);
extern void dir_destroy(dir_t *dir);

#endif

