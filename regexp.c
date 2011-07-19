
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "automat.h"
#include "regexp.h"

#define ALPHABET_COUNT	(126-32+1)

static int** get_matrix(const int count_status)
{
	int **matrix;
	int i;
	int j;
	
	matrix = (int **) malloc(count_status * sizeof(int *));

	for(i = 0; i < count_status; i++)
	{
		matrix[i] = (int *) malloc((ALPHABET_COUNT+1) * sizeof(int));

		matrix[i][0] = i;

		for(j = 1; j < (ALPHABET_COUNT+1); j++)
		{
			matrix[i][j] = -1;
		}
	}

	return matrix;
}

static int get_alphabet_id(const char c)
{
	if( c < ' ' || c > '~' )
	{
		return -1;
	}

	return c - ' ';
}

static int regexp_range(automat_t *automat, int current_status, char c)
{
	static char range_first = '\0';
	static char prev_c = '\0';
	static char line_prev_c = '\0';
	static int line_flag = 0;
	int id;
	int i;

	//printf("prev_c = %c c = %c\n", prev_c, c);

	if(  prev_c != '\\' && c == '\\' )
	{
		prev_c = c;
		return 1;
	}

	if( prev_c == '\\' )
	{
		prev_c = c;

		if( c == '\\' )
		{
			prev_c = '\0';
		}
		
		if( line_flag == 0 )
		{
			id = get_alphabet_id(c);
			automat->matrix[current_status][1+id] = current_status+1;

			return 1;
		}
	}

	if( line_flag == 0 && c == '[' )
	{
		prev_c = '\0';
		range_first = '\0';
		line_flag = 0;

		return 1;
	}

	if( range_first == '\0' )
	{
		range_first = c;

		if( range_first == '!' || range_first == '^' )
		{
			return 1;
		}
	}

	if( line_flag == 0 && c == ']' )
	{
		if( range_first == '!' || range_first == '^' )
		{
			for(i = 0; i < ALPHABET_COUNT; i++)
			{
				if( automat->matrix[current_status][1+i] == -1 )
				{
					automat->matrix[current_status][1+i] = current_status+1;
				}
				else
				{
					automat->matrix[current_status][1+i] = -1;
				}
			}
		}

		return 0;
	}

	if( line_flag == 0 && c == '-' )
	{
		line_prev_c = prev_c;
		line_flag = 1;

		return 1;
	}

	if( line_flag == 1 )
	{
		int prev_id;
		int next_id;

		prev_id = get_alphabet_id(line_prev_c);
		next_id = get_alphabet_id(c);
		line_flag = 0;

		for(i = prev_id; i <= next_id; i++)
		{
			automat->matrix[current_status][1+i] = current_status+1;
		}

		return 1;
	}

	id = get_alphabet_id(c);
	automat->matrix[current_status][1+id] = current_status+1;
	prev_c = c;

	return 1;
}

static void regexp_normal_char(automat_t *automat, int current_status, int last, char c)
{
	int id;
	int i;

	for(i = 0; i < ALPHABET_COUNT; i++)
	{
		automat->matrix[current_status][1+i] = last;
	}

	id = get_alphabet_id(c);
	automat->matrix[current_status][1+id] = current_status+1;
}

static void gen_matrix(automat_t *automat, const char *str_regexp)
{
	int last;
	int len;
	int i;
	int current_status;
	char pair;
	int id;

	len = strlen(str_regexp);
	last = -1;
	pair = '\0';
	current_status  = 0;

	for(i = 0; i < len; i++)
	{
		char c;
		int j;

		c = str_regexp[i];

		//printf("c = %c\n", c);

		if( pair == '\'' || pair == '\"' )
		{
			if( pair == c )
			{
				pair = '\0';
			}
			else
			{
				regexp_normal_char(automat, current_status, last, c);
				current_status++;
			}

			continue;
		}

		if( pair == '[' )
		{
			if( ! regexp_range(automat, current_status, c) )
			{
				pair = '\0';
				current_status++;
			}

			continue;
		}

		switch(c)
		{
			case '?' :
				automat->map[i] = current_status;

				for(j = 0; j < ALPHABET_COUNT; j++)
				{
					automat->matrix[current_status][1+j] = current_status+1;
				}

				current_status++;
			break;

			case '*' :
				automat->map[i] = current_status;
				last = current_status;

				if( i == len-1 )
				{
					int j;

					for(j = 0; j < ALPHABET_COUNT; j++)
					{
						automat->matrix[current_status-1][1+j] = current_status;
					}
				}
			break;

			case '[' :
				pair = c;
				automat->map[i] = current_status;
				regexp_range(automat, current_status, c);
			break;

			case '\"' :
			case '\'' :
				pair = c;
			break;

			case '\\' :
				automat->map[i] = current_status;
				c = str_regexp[++i];

			default :
				automat->map[i] = current_status;
				regexp_normal_char(automat, current_status, last, c);
				current_status++;
			break;
		}
	}
}

static void init_map(automat_t *automat)
{
	int i;

	automat->map = (int *)malloc(automat->len_regexp * sizeof(int) );

	for(i = 0; i < automat->len_regexp; i++)
	{
		automat->map[i] = -1;
	}
}

automat_t* automat_new(const char *str_regexp)
{
	automat_t *automat;

	automat = (automat_t *) malloc( sizeof(automat_t) );
	automat->str_regexp = strdup(str_regexp);
	automat->len_regexp = strlen(str_regexp);
	automat->count_status = get_count_status(str_regexp);
	automat->current_status = 0;
	automat->matrix = get_matrix(automat->count_status);

	init_map(automat);
	gen_matrix(automat, str_regexp);

	return automat;
}

int automat_get_current_status(const automat_t *automat)
{
	return automat->current_status;
}

void automat_set_current_status(automat_t *automat, const int status)
{
	automat->current_status = status;
}

void automat_reset(automat_t *automat)
{
	automat->current_status = 0;
}

char* automat_get_regexp(automat_t *automat)
{
	return automat->str_regexp;
}

char* automat_get_curent_regexp(const automat_t *automat)
{
	int len;
	int i;

	if( automat->current_status == -1 )
	{
		return NULL;
	}

	for(i = 0; i < automat->len_regexp; i++)
	{
		if( automat->map[i] == automat->current_status )
		{
			return automat->str_regexp+i;
		}
	}

	return NULL;
}

void automat_print(const automat_t *automat)
{
	int i;
	int j;

	printf("regexp: %s\n", automat->str_regexp);

	printf("map: ");

	for(i = 0; i < automat->len_regexp; i++)
	{
		if( automat->map[i] != -1 )
		{
			printf("[%d -> \'%c\'] ", automat->map[i], automat->str_regexp[i]);
		}
	}

	putchar('\n');

	putchar(' ');

	for(i = 0; i < ALPHABET_COUNT; i++)
	{
		printf("%c", ' '+i);
	}

	putchar('\n');

	for(i = 0; i < automat->count_status; i++)
	{
		for(j = 0; j < ALPHABET_COUNT+1; j++)
		{
			if( automat->matrix[i][j] != -1 )
			{
				printf("%d", automat->matrix[i][j]);
			}
			else
			{
				putchar(' ');
			}
		}

		putchar('\n');
	}
}

int automat_is_final_status(const automat_t *automat)
{
	if( automat->current_status == automat->count_status )
	{
		return 1;
	}

	return 0;
}

void automat_step(automat_t *automat, const char c)
{
	int id;

	id = get_alphabet_id(c);

	if( id == -1 )
	{
		automat->current_status = -1;
		return;
	}

	if( automat->current_status < 0 || automat->current_status >= automat->count_status )
	{
		automat->current_status = -1;
		return;
	}

	automat->current_status = automat->matrix[automat->current_status][1+id];
}

void automat_steps_for_string(automat_t *automat, const char *str)
{
	int len;
	int res;
	int i;

	len = strlen(str);

	for(i = 0; i < len; i++)
	{
		char c;

		c = str[i];
		automat_step(automat, c);
	}
}

int automat_destroy(automat_t *automat)
{
	int i;

	free(automat->str_regexp);

	for(i = 0; i < automat->count_status; i++)
	{
		free(automat->matrix[i]);
	}

	free(automat->matrix);
	free(automat->map);
	free(automat);
}

int regexp(automat_t *automat, const char *str)
{
	int len;
	int res;
	int i;

	len = strlen(str);

	for(i = 0; i < len; i++)
	{
		char c;

		c = str[i];
		automat_step(automat, c);

		res = automat_is_final_status(automat);

		if( res )
		{
			automat_reset(automat);
			return 1;
		}
	}

	automat_reset(automat);

	return res;
}

//#define TEST_REGEXP
#ifdef TEST_REGEXP
int main()
{
	automat_t *automat;

	//automat = automat_new("a?b[\\a-\\zABC0-9\\\\]d\\*e[^*?\"\'!^]g");
	automat = automat_new("a?b[\\a-\\zABC0-9\\X\\\\]d\\*e[^*?\"\'!^]g");
	//automat = automat_new("b[\\\\]d");
	automat_print(automat);
	printf("res = %d\n", regexp(automat, "axbcd*efg"));
	automat_destroy(automat);

	automat = automat_new("ab\"cd*?ef\"gh");
	automat_print(automat);
	printf("res = %d\n", regexp(automat, "abcd*?efgh"));
	automat_destroy(automat);

	automat = automat_new("ab*cdef");
	automat_print(automat);
	printf("res = %d\n", regexp(automat, "abcdxcdef"));
	automat_destroy(automat);

	automat = automat_new("abc*");
	automat_print(automat);
	printf("res = %d\n", regexp(automat, "abcd"));
	automat_destroy(automat);

	return 0;
}
#endif
