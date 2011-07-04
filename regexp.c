
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "regexp.h"

#define ALPHABET_COUNT	(126-32+1)

static int get_count_status(const char *str_regexp)
{
	int len;
	int count;
	int i;
	int flag;

	char c;
	char c_prev;

	count = 0;
	len = strlen(str_regexp);
	c_prev = '\0';
	flag = 1;

	for(i = 0; i < len; i++)
	{
		c = str_regexp[i];

		if( c_prev != '\\' &&  c == '[' )
		{
			flag = 0;
		}

		if( c_prev != '\\' &&  c == ']' )
		{
			flag = 1;
		}

		if( flag == 1 && c_prev != '\\' &&  c != '*' )
		{
			count++;
		}

		c_prev = c;
	}

	return count+1;
}

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

static void gen_matrix(automat_t *automat, const char *str_regexp)
{
	int last;
	int len;
	int i;
	int current_status;
	int in_range;
	char range_first;

	len = strlen(str_regexp);
	last = -1;
	current_status  = 0;
	in_range = 0;
	range_first = '\0';

	for(i = 0; i < len; i++)
	{
		char c;
		int j;
		int id;

		c = str_regexp[i];

		//printf("c = %c\n", c);

		if( in_range == 1 )
		{
			if( range_first == '\0' )
			{
				range_first = c;

				if( range_first == '!' || range_first == '^' )
				{
					continue;
				}
			}

			if( c == ']' )
			{
				if( range_first == '!' || range_first == '^' )
				{
					for(j = 0; j < ALPHABET_COUNT; j++)
					{
						if( automat->matrix[current_status][1+j] == -1 )
						{
							automat->matrix[current_status][1+j] = current_status+1;
						}
						else
						{
							automat->matrix[current_status][1+j] = -1;
						}
					}
				}

				current_status++;
				in_range = 0;

				continue;
			}

			if( c == '-' )
			{
				char prev_c;
				char next_c;

				int prev_id;
				int next_id;

				prev_c = str_regexp[i-1];
				next_c = str_regexp[++i];

				if( next_c == '\\' )
				{
					next_c = str_regexp[++i];
				}

				prev_id = get_alphabet_id(prev_c);
				next_id = get_alphabet_id(next_c);

				for(j = prev_id; j <= next_id; j++)
				{
					automat->matrix[current_status][1+j] = current_status+1;
				}

				continue;
			}

			if( c == '\\' )
			{
				c = str_regexp[++i];
			}

			id = get_alphabet_id(c);
			automat->matrix[current_status][1+id] = current_status+1;

			continue;
		}

		switch(c)
		{
			case '?' :
				for(j = 0; j < ALPHABET_COUNT; j++)
				{
					automat->matrix[current_status][1+j] = current_status+1;
				}

				current_status++;
			break;

			case '*' :
				last = current_status;
			break;

			case '[' :
				in_range = 1;
			break;

			case '\\' :

			default :
				for(j = 0; j < ALPHABET_COUNT; j++)
				{
					automat->matrix[current_status][1+j] = last;
				}

				id = get_alphabet_id(c);
				automat->matrix[current_status][1+id] = current_status+1;
				current_status++;
			break;
		}
	}

	for(i = 0; i < ALPHABET_COUNT; i++)
	{
		automat->matrix[current_status][1+i] = current_status;
	}
}

automat_t* automat_new(const char *str_regexp)
{
	automat_t *automat;

	automat = (automat_t *) malloc( sizeof(automat_t) );
	automat->count_status = get_count_status(str_regexp);
	automat->current_status = 0;
	automat->matrix = get_matrix(automat->count_status);
	gen_matrix(automat, str_regexp);

	return automat;
}

void automat_reset(automat_t *automat)
{
	automat->current_status = 0;
}

void automat_print(const automat_t *automat)
{
	int i;
	int j;

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
	if( automat->current_status == automat->count_status-1 )
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

int automat_destroy(automat_t *automat)
{
	int i;

	for(i = 0; i < automat->count_status; i++)
	{
		free(automat->matrix[i]);
	}

	free(automat->matrix);
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
	}

	res = automat_is_final_status(automat);

	automat_reset(automat);

	return res;
}

#ifdef TEST_REGEXP
int main()
{
	automat_t *automat;

	automat = automat_new("a[a-zABC0-9]d*e?f");
	automat_print(automat);

	printf("res = %d\n", regexp(automat, "aAdbexf"));

	automat_destroy(automat);

	return 0;
}
#endif
