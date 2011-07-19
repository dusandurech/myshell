
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "automat.h"

typedef struct automat_count_status_struct
{
	int status;
	int in_char;
	int out_status;
	int inc;
} automat_count_status_t;

static automat_count_status_t automat_count_status[] = 
{
	{ .status = 1,		.in_char = '?',		.out_status = 1,	.inc = 1 },
	{ .status = 1,		.in_char = '*',		.out_status = 1,	.inc = 0 },
	{ .status = 1,		.in_char = '\"',	.out_status = 2,	.inc = 0 },
	{ .status = 1,		.in_char = '\'',	.out_status = 3,	.inc = 0 },
	{ .status = 1,		.in_char = '[',		.out_status = 4,	.inc = 1 },
	{ .status = 1,		.in_char = '\\',	.out_status = 8,	.inc = 0 },
	{ .status = 1,		.in_char = -1,		.out_status = 1,	.inc = 1 },

	{ .status = 2,		.in_char = '\\',	.out_status = 5,	.inc = 0 },
	{ .status = 2,		.in_char = '\"',	.out_status = 1,	.inc = 0 },
	{ .status = 2,		.in_char = -1,		.out_status = 2,	.inc = 1 },

	{ .status = 3,		.in_char = '\\',	.out_status = 6,	.inc = 0 },
	{ .status = 3,		.in_char = '\'',	.out_status = 1,	.inc = 0 },
	{ .status = 3,		.in_char = -1,		.out_status = 3,	.inc = 1 },

	{ .status = 5,		.in_char = -1,		.out_status = 2,	.inc = 0 },
	{ .status = 6,		.in_char = -1,		.out_status = 3,	.inc = 0 },

	{ .status = 4,		.in_char = '[',		.out_status = 1,	.inc = 0 },
	{ .status = 4,		.in_char = '\\',	.out_status = 7,	.inc = 0 },
	{ .status = 4,		.in_char = ']',		.out_status = 1,	.inc = 0 },

	{ .status = 7,		.in_char = -1,		.out_status = 4,	.inc = 0 },
	{ .status = 8,		.in_char = -1,		.out_status = 1,	.inc = 1 }

};

#define AUTOMAT_COUNT_STATUS_COUNT		( sizeof(automat_count_status) / sizeof(automat_count_status_t) )

int get_count_status(const char *str_regexp)
{
	int count;
	int status;
	int len;
	char c;
	int i;
	int j;

	status = 1;
	count = 0;

	len = strlen(str_regexp);

	//printf("str_regexp = %s\n", str_regexp);

	for(i = 0; i < len; i++)
	{
		c = str_regexp[i];

		for(j = 0; j < AUTOMAT_COUNT_STATUS_COUNT; j++)
		{
			if( automat_count_status[j].status == status &&
			    ( automat_count_status[j].in_char == c || automat_count_status[j].in_char == -1) )
			{
				//printf("%d -> %d\n", status, automat_count_status[j].out_status);

				status = automat_count_status[j].out_status;
				count += automat_count_status[j].inc;

				break;
			}
		}
	}

	//printf("count = %d\n", count);

	return count;
}

//-------------------------------------

typedef struct automat_del_metacharakter_struct
{
	int status;
	int in_char;
	int out_status;
	int cfg;
} automat_del_metacharakter_t;

#define CFG_NONE		0
#define CFG_COPY		1

static automat_del_metacharakter_t automat_del_metacharakter[] = 
{
	{ .status = 1,		.in_char = '?',		.out_status = 1,	.cfg = CFG_COPY },
	{ .status = 1,		.in_char = '*',		.out_status = 1,	.cfg = CFG_COPY },
	{ .status = 1,		.in_char = '\"',	.out_status = 2,	.cfg = CFG_NONE },
	{ .status = 1,		.in_char = '\'',	.out_status = 3,	.cfg = CFG_NONE },
	{ .status = 1,		.in_char = '\\',	.out_status = 8,	.cfg = CFG_NONE },
	{ .status = 1,		.in_char = -1,		.out_status = 1,	.cfg = CFG_COPY },

	{ .status = 2,		.in_char = '\"',	.out_status = 1,	.cfg = CFG_NONE },
	{ .status = 2,		.in_char = -1,		.out_status = 2,	.cfg = CFG_COPY },

	{ .status = 3,		.in_char = '\'',	.out_status = 1,	.cfg = CFG_NONE },
	{ .status = 3,		.in_char = -1,		.out_status = 3,	.cfg = CFG_COPY },

	{ .status = 8,		.in_char = -1,		.out_status = 1,	.cfg = CFG_COPY }
};

#define AUTOMAT_DEL_METACHARAKTER_COUNT		( sizeof(automat_del_metacharakter) / sizeof(automat_del_metacharakter_t) )

char* get_del_metacharakter(const char *str_regexp)
{
	static char str[STR_SIZE];
	int off;
	int status;
	int len;
	char c;
	int i;
	int j;

	status = 1;
	off = 0;

	memset(str, 0, STR_SIZE);
	len = strlen(str_regexp);

	//printf("str_regexp = %s\n", str_regexp);

	for(i = 0; i < len; i++)
	{
		c = str_regexp[i];

		for(j = 0; j < AUTOMAT_DEL_METACHARAKTER_COUNT; j++)
		{
			if( automat_del_metacharakter[j].status == status &&
			    ( automat_del_metacharakter[j].in_char == c || automat_del_metacharakter[j].in_char == -1) )
			{
				//printf("%d -> %d\n", status, automat_del_metacharakter[j].out_status);

				status = automat_del_metacharakter[j].out_status;
				
				switch( automat_del_metacharakter[j].cfg )
				{
					case CFG_NONE :
					break;

					case CFG_COPY :
						str[off++] = c;
					break;
				}

				break;
			}
		}
	}

	//printf("count = %d\n", count);

	return str;
}

//#define TEST_AUTOMAT

#ifdef TEST_AUTOMAT
int main()
{
	printf("get_count_status = %d\n", get_count_status("a?b[\\a-\\zABC0-9\\X\\\\]d\\*e[^*?\"\'!^]g"));
	printf("del_apostrof_metacharekter = >%s<\n", get_del_metacharakter("\"[Hh]ello world?\"[Cc]o ty na to"));
	return 0;
}
#endif
