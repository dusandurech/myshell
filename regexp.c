
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "regexp.h"

#define RESIZE_ALIGMENT		8


#define FLAG_FIN_STATUS			1

#define ALPHA_TO_ID(c)			(c - ' ')
#define ID_TO_ALPHA(id)			(id + ' ')

static void* resize(void *p, int n, int *count, int *alloc)
{
	if( *count+1 > *alloc )
	{
		(*alloc) += RESIZE_ALIGMENT;
		p = realloc(p, (*alloc) * n);
	}

	return p;
}

#if 0
typedef struct trans_function_struct
{
	int len;
	int alloc;
	int *status;
} trans_function_t;
#endif

void trans_func_clean(trans_function_t *dst)
{
	dst->len = 0;
}

void trans_function_clone(trans_function_t *dst, trans_function_t *src)
{
	dst->status = NULL;
	dst->len = src->len;

	if( dst->len > 0 )
	{
		dst->status = malloc((dst->len) * sizeof(int));
		memcpy(dst->status, src->status, (dst->len) * sizeof(int));
	}
}

int trans_func_cmp(trans_function_t *trans1, trans_function_t *trans2)
{
	int i;

	if( trans1->len != trans2->len )
	{
		return 0;
	}

	for(i = 0; i < trans1->len; i++)
	{
		if( trans1->status[i] != trans2->status[i] )
		{
			return 0;
		}
	}

	return 1;
}

void trans_func_destroy(trans_function_t *trans)
{
	if( trans->status != NULL )
	{
		free(trans->status);
	}
}

int trans_func_is_in(trans_function_t *trans, int status)
{
	int len;
	int i;

	len = trans->len;

	for(i = 0; i < len; i++)
	{
		if( trans->status[i] == status )
		{
			return 1;
		}
	}

	return 0;
}

void trans_func_append(trans_function_t *trans, int status)
{
	int len;
	int i;

	len = trans->len;

	for(i = 0; i < len; i++)
	{
		if( trans->status[i] == status )
		{
			return;
		}
	}

	trans->status = resize(trans->status, sizeof(int), &trans->len, &trans->alloc);
	trans->status[len] = status;
	trans->len++;
}

void trans_func_append_copy(trans_function_t *dst, trans_function_t *src)
{
	int len;
	int i;

	len = src->len;

	for(i = 0; i < len; i++)
	{
		trans_func_append(dst, src->status[i]);
	}
}

void trans_func_print(trans_function_t *trans)
{
	int len;
	int i;

	len = trans->len;

	printf("{ ");
	for(i = 0; i < len; i++)
	{
		printf("%d ", trans->status[i]);
	}

	printf("} ");
}

#if 0
typedef struct nsa_status_struct
{
	int status;
	int flag;
	trans_function_t trans[ALPHABET_COUNT];
} nsa_status_t;
#endif

nsa_status_t* nsa_status_new(int status)
{
	nsa_status_t *nsa_status;

	nsa_status = (nsa_status_t *) malloc(sizeof(nsa_status_t));
	memset(nsa_status, 0, sizeof(nsa_status_t));
	nsa_status->status = status;

	return nsa_status;
}

nsa_status_t* nsa_status_clone(nsa_status_t *nsa_status)
{
	nsa_status_t *clone;
	int i;

	clone = nsa_status_new(nsa_status->status);
	clone->flag = nsa_status->flag;

	for(i = 0; i < ALPHABET_COUNT; i++)
	{
		trans_function_clone(&clone->trans[i], &nsa_status->trans[i]);
	}

	return clone;
}

void nsa_status_fin(nsa_status_t *nsa_status)
{
	nsa_status->flag |= FLAG_FIN_STATUS;
}

void nsa_status_append(nsa_status_t *nsa_status, char c, int status)
{
	trans_func_append(&nsa_status->trans[ALPHA_TO_ID(c)], status);
}

void nsa_status_print(nsa_status_t *nsa_status)
{
	int i;

	printf("%d: ", nsa_status->status);

	for(i = 0; i < ALPHABET_COUNT; i++)
	{
		if( nsa_status->trans[i].len > 0 )
		{
			int j;

			printf("\'%c\' -> ", ID_TO_ALPHA(i));

			trans_func_print(&nsa_status->trans[i]);
		}
	}

	putchar('\n');
}

void nsa_status_print_graphviz(nsa_status_t *nsa_status)
{
	int i;

	if( nsa_status->flag & FLAG_FIN_STATUS )
	{
		printf("%d [style=filled, fillcolor=\"red\"];\n", nsa_status->status);
	}

	for(i = 0; i < ALPHABET_COUNT; i++)
	{
		if( nsa_status->trans[i].len > 0 )
		{
			int j;

			for(j = 0; j < nsa_status->trans[i].len; j++)
			{
				printf("%d->%d [label=\"%c\"];\n", nsa_status->status, nsa_status->trans[i].status[j], ID_TO_ALPHA(i));
			}
		}
	}
}

void nsa_status_destroy(nsa_status_t *nsa_status)
{
	int i;

	for(i = 0; i < ALPHABET_COUNT; i++)
	{
		trans_func_destroy(&nsa_status->trans[i]);
	}

	free(nsa_status);
}

#if 0
typedef struct nsa_struct
{
	int count;
	int alloc;
	nsa_status_t **status;
} nsa_t;
#endif

nsa_t* nsa_new()
{
	nsa_t *nsa;

	nsa = (nsa_t *) malloc(sizeof(nsa_t));
	memset(nsa, 0, sizeof(nsa_t));

	return nsa;
}

void nsa_append_status(nsa_t *nsa, nsa_status_t *status)
{
	nsa->status = resize(nsa->status, sizeof(nsa_status_t *), &nsa->count, &nsa->alloc);
	nsa->status[nsa->count++] = status;
}

int nsa_step(nsa_t *nsa, char *str)
{
	char *current_status;
	char *next_status;
	int len;
	int res;
	int i, j, k;

	len = strlen(str);

	current_status = (char *) malloc( nsa->count * sizeof(char) );
	next_status = (char *) malloc( nsa->count * sizeof(char) );

	memset(current_status, 0, nsa->count * sizeof(char) );
	memset(next_status, 0, nsa->count * sizeof(char) );

	current_status[0] = 1;

	for(i = 0; i < len; i++)
	{
		nsa_status_t *nsa_status;
		char c;

		memset(next_status, 0, nsa->count * sizeof(char) );
		c = str[i];

		for(j = 0; j < nsa->count; j++)
		{
			if( current_status[j] == 1 )
			{
				//printf("%d ", j+1);

				nsa_status = nsa->status[j];
		
				for(k = 0; k < nsa_status->trans[ALPHA_TO_ID(c)].len; k++)
				{
					int n;

					n = nsa_status->trans[ALPHA_TO_ID(c)].status[k];
					next_status[n-1] = 1;
					//printf("n = %d\n", n);
				}
			}
		}

		//putchar('\n');
		memcpy(current_status, next_status, nsa->count * sizeof(char) );
	}

	res = 0;

	if( current_status[nsa->count-1] == 1 )
	{
		res = 1;
	}

	free(current_status);
	free(next_status);

	return res;
}

nsa_t* nsa_clone(nsa_t *nsa)
{
	nsa_t *clone;
	int i;

	clone = nsa_new();

	for(i = 0; i < nsa->count; i++)
	{
		nsa_status_t *nsa_status;

		nsa_status = nsa_status_clone(nsa->status[i]);
		nsa_append_status(clone, nsa_status);
	}

	return clone;
}

void nsa_print(nsa_t *nsa)
{
	int i;

	for(i = 0 ; i < nsa->count; i++)
	{
		nsa_status_print(nsa->status[i]);
	}
}

void nsa_print_graphviz(nsa_t *nsa)
{
	int i;

	printf("digraph automat {\n");

	for(i = 0 ; i < nsa->count; i++)
	{
		nsa_status_print_graphviz(nsa->status[i]);
	}

	printf("}\n");
}

int nsa_regexp(nsa_t *nsa, char *str_regexp)
{
	nsa_status_t* nsa_status;
	int current_status;
	int len;
	int i;

	len = strlen(str_regexp);
	current_status = 1;
	nsa_status = NULL;

	nsa_status = nsa_status_new(current_status);

	for(i = 0; i <= len; i++)
	{
		int c;
		int j;

		c = str_regexp[i];

		switch(c)
		{
			case '\0' :
				nsa_status_fin(nsa_status);
				nsa_append_status(nsa, nsa_status);
			break;

			case '?' :
				for(j = 0; j < ALPHABET_COUNT; j++)
				{
					nsa_status_append(nsa_status, ID_TO_ALPHA(j), current_status+1);
				}

				nsa_append_status(nsa, nsa_status);
				nsa_status = nsa_status_new(++current_status);

				//current_status++;
			break;

			case '*' :
				for(j = 0; j < ALPHABET_COUNT; j++)
				{
					nsa_status_append(nsa_status, ID_TO_ALPHA(j), current_status);
				}
			break;

			case '[' :
			{
				char alphabet[ALPHABET_COUNT];
				int neg = 0;

				memset(alphabet, 0, sizeof(char)*ALPHABET_COUNT );
				
				i++;

				if( str_regexp[i] == '!' || str_regexp[i] == '^' )
				{
					//printf("neg\n");
					neg = 1;
					i++;
				}

				while( str_regexp[i] != ']' )
				{
					//printf("str_regexp[%d] = %c\n", i, str_regexp[i]);

					if( str_regexp[i] == '\0' )
					{
						return -1;
					}

					if( str_regexp[i] == '\\' )
					{
						alphabet[ ALPHA_TO_ID(str_regexp[++i]) ] = 1;
						continue;
					}

					if( str_regexp[i] == '-' )
					{
						char prev;
						char next;

						prev = str_regexp[i-1];
						next = str_regexp[i+1];

						if( next == '\\' )
						{
							next = str_regexp[++i];
						}

						for(j = prev ; j <= next; j++)
						{
							alphabet[ ALPHA_TO_ID(j) ] = 1;
						}

						i += 2;
						continue;
					}

					alphabet[ ALPHA_TO_ID(str_regexp[i]) ] = 1;
					i++;
				}

				if( neg )
				{
					for(j = 0; j < ALPHABET_COUNT; j++)
					{
						alphabet[j] = ! alphabet[j];
					}
				}

				for(j = 0; j < ALPHABET_COUNT; j++)
				{
					if( alphabet[j] == 1 )
					{
						//printf("range %c\n", ID_TO_ALPHA(j));
						nsa_status_append(nsa_status, ID_TO_ALPHA(j), current_status+1);
					}
				}

				nsa_append_status(nsa, nsa_status);
				nsa_status = nsa_status_new(++current_status);
			}
			break;

			case '\\' :
				i++;

			default :
				nsa_status_append(nsa_status, str_regexp[i], current_status+1);
				nsa_append_status(nsa, nsa_status);
				nsa_status = nsa_status_new(++current_status);
			break;
		}
	}

	return 0;
}

int nsa_extra_regexp(nsa_t *nsa, char *str_regexp)
{
	nsa_status_t* nsa_status;
	int current_status;
	int len;
	int i;

	len = strlen(str_regexp);
	current_status = 1;
	nsa_status = NULL;

	nsa_status = nsa_status_new(current_status);

	for(i = 0; i <= len; i++)
	{
		int c;
		int j;

		c = str_regexp[i];

		switch(c)
		{
			case '\0' :
				nsa_status_fin(nsa_status);
				nsa_append_status(nsa, nsa_status);
			break;

			case '?' :
				for(j = 0; j < ALPHABET_COUNT; j++)
				{
					nsa_status_append(nsa_status, ID_TO_ALPHA(j), current_status+1);
				}

				nsa_append_status(nsa, nsa_status);
				nsa_status = nsa_status_new(++current_status);

				//current_status++;
			break;

			case '*' :
				for(j = 0; j < ALPHABET_COUNT; j++)
				{
					nsa_status_append(nsa_status, ID_TO_ALPHA(j), current_status);
				}
			break;

			case '+' :
			break;

			case '[' :
			{
				char alphabet[ALPHABET_COUNT];
				int neg = 0;

				memset(alphabet, 0, sizeof(char)*ALPHABET_COUNT );

				if( str_regexp[++i] == '!' )
				{
					neg = 1;
					i++;
				}

				while( str_regexp[i] != ']' )
				{
					if( str_regexp[i] == '-' )
					{
						for(j = str_regexp[i-1] ; j <= str_regexp[i+1]; j++)
						{
							alphabet[ ALPHA_TO_ID(j) ] = 1;
						}

						i += 2;
						continue;
					}

					alphabet[ ALPHA_TO_ID(str_regexp[i]) ] = 1;
					i++;
				}

				if( neg )
				{
					for(j = 0; j < ALPHABET_COUNT; j++)
					{
						alphabet[j] = ! alphabet[j];
					}
				}

				for(j = 0; j < ALPHABET_COUNT; j++)
				{
					if( alphabet[j] == 1 )
					{
						nsa_status_append(nsa_status, ID_TO_ALPHA(j), current_status+1);
					}
				}

				nsa_append_status(nsa, nsa_status);
				nsa_status = nsa_status_new(++current_status);
			}
			break;

			case '\\' :
				i++;

			default :
				nsa_status_append(nsa_status, str_regexp[i], current_status+1);
				nsa_append_status(nsa, nsa_status);
				nsa_status = nsa_status_new(++current_status);
			break;
		}
	}

	return 0;
}

void nsa_destroy(nsa_t *nsa)
{
	if( nsa->count > 0 )
	{
		int i;

		for(i = 0 ; i < nsa->count; i++)
		{
			nsa_status_destroy(nsa->status[i]);
		}

		free(nsa->status);
	}

	free(nsa);
}

#if 0
typedef struct dsa_status_struct
{
	int status;
	int flag;
	int trans_function[ALPHABET_COUNT];
} dsa_status_t;
#endif

dsa_status_t* dsa_status_new(int status)
{
	dsa_status_t *dsa_status;

	dsa_status = (dsa_status_t *) malloc(sizeof(dsa_status_t));
	memset(dsa_status, 0, sizeof(dsa_status_t));
	dsa_status->status = status;

	return dsa_status;
}

void dsa_status_fin(dsa_status_t *dsa_status)
{
	dsa_status->flag |= FLAG_FIN_STATUS;
}

void dsa_status_append(dsa_status_t *dsa_status, char c, int status)
{
	dsa_status->trans_function[ALPHA_TO_ID(c)] = status;
}

void dsa_status_print(dsa_status_t *dsa_status)
{
	int i;

	printf("%d: ", dsa_status->status);

	for(i = 0; i < ALPHABET_COUNT; i++)
	{
		if( dsa_status->trans_function[i] != 0 )
		{
			int j;

			printf("\'%c\' -> %d ", ID_TO_ALPHA(i), dsa_status->trans_function[i]);
		}
	}

	putchar('\n');
}

void dsa_status_print_graphviz(dsa_status_t *dsa_status)
{
	int i;

	if( dsa_status->flag & FLAG_FIN_STATUS )
	{
		printf("%d [style=filled, fillcolor=\"red\"];\n", dsa_status->status);
	}

	for(i = 0; i < ALPHABET_COUNT; i++)
	{
		if( dsa_status->trans_function[i] != 0 )
		{
			printf("%d->%d [label=\"%c\"];\n", dsa_status->status, dsa_status->trans_function[i], ID_TO_ALPHA(i));
		}
	}
}

void dsa_status_destroy(dsa_status_t *dsa_status)
{
	free(dsa_status);
}

#if 0
typedef struct dsa_struct
{
	int count;
	int alloc;
	dsa_status_t **status;
} dsa_t;
#endif

dsa_t* dsa_new()
{
	dsa_t *dsa;

	dsa = (dsa_t *) malloc(sizeof(dsa_t));
	memset(dsa, 0, sizeof(dsa_t));

	return dsa;
}

void dsa_print(dsa_t *dsa)
{
	int i;

	for(i = 0 ; i < dsa->count; i++)
	{
		dsa_status_print(dsa->status[i]);
	}
}

void dsa_print_graphviz(dsa_t *dsa)
{
	int i;

	printf("digraph automat {\n");

	for(i = 0 ; i < dsa->count; i++)
	{
		dsa_status_print_graphviz(dsa->status[i]);
	}

	printf("}\n");
}

void convert_nsa_to_dsa(nsa_t *nsa, dsa_t *dsa);

void dsa_regexp(dsa_t *dsa, char *str_regexp)
{
	nsa_t *nsa;

	nsa = nsa_new();
	nsa_regexp(nsa, str_regexp);
	//nsa_print_graphviz(nsa);
	//nsa_print(nsa);
	convert_nsa_to_dsa(nsa, dsa);
	nsa_destroy(nsa);
}

int dsa_one_step(dsa_t *dsa, int status, char c)
{
	if( status == 0 )
	{
		return 0;
	}

	return dsa->status[status-1]->trans_function[ALPHA_TO_ID(c)];
}

int dsa_is_stat_fin(dsa_t *dsa, int status)
{
	if( status == 0 )
	{
		return 0;
	}

	if( dsa->status[status-1]->flag & FLAG_FIN_STATUS )
	{
		return 1;
	}

	return 0;
}

int dsa_is_stat_blackhole(dsa_t *dsa, int status)
{
	if( status == 0 )
	{
		return 1;
	}
	
	return 0;
}

int dsa_step(dsa_t *dsa, char *str)
{
	int len;
	int current_status;
	int i;

	len = strlen(str);
	current_status = 1;

	for(i = 0; i < len; i++)
	{
		char c;

		c = str[i];
		current_status = dsa->status[current_status-1]->trans_function[ALPHA_TO_ID(c)];

		if( current_status == 0 )
		{
			return 0;
		}
	}

	if( dsa->status[current_status-1]->flag & FLAG_FIN_STATUS )
	{
		return 1;
	}

	return 0;
}

void dsa_append_status(dsa_t *dsa, dsa_status_t *status)
{
	dsa->status = resize(dsa->status, sizeof(dsa_status_t *), &dsa->count, &dsa->alloc);
	dsa->status[dsa->count++] = status;
}

void dsa_destroy(dsa_t *dsa)
{
	if( dsa->count > 0 )
	{
		int i;

		for(i = 0 ; i < dsa->count; i++)
		{
			dsa_status_destroy(dsa->status[i]);
		}

		free(dsa->status);
	}

	free(dsa);
}

typedef struct line_struct
{
	trans_function_t trans;
	int status;
} line_t;

typedef struct table_struct
{
	line_t *line;
	int count;
	int alloc;
} table_t;

static void table_init(table_t *table)
{
	memset(table, 0, sizeof(table_t));
}

static void table_print(table_t *table)
{
	int i;

	for(i = 0; i < table->count; i++)
	{
		printf("%d -> ", table->line[i].status);
		trans_func_print(&table->line[i].trans);
		putchar('\n');
	}
}

static int table_find(table_t *table, trans_function_t *trans)
{
	int i;

	for(i = 0; i < table->count; i++)
	{
		if( trans_func_cmp(&table->line[i].trans, trans) )
		{
			return i;
		}
	}

	return -1;
}

static void table_append(table_t *table, trans_function_t *trans, int status)
{
	table->line = resize(table->line, sizeof(line_t), &table->count, &table->alloc);
	trans_function_clone(&table->line[table->count].trans, trans);
	table->line[table->count].status = status;
	table->count++;
}

static void table_destroy(table_t *table)
{
	int i;

	for(i = 0; i < table->count; i++)
	{
		trans_func_destroy(&table->line[i].trans);
	}

	free(table->line);
}

static void create_new_status(nsa_t *nsa, trans_function_t *trans, int accept_status)
{
	nsa_status_t* nsa_status;
	int i;
	int j;

	//printf("nsa->count = %d\n", nsa->count);

	nsa_status = nsa_status_new(nsa->count+1);

	if( trans_func_is_in(trans, accept_status) )
	{
		nsa_status_fin(nsa_status);
	}

	for(i = 0; i < trans->len; i++)
	{
		int n;

		n = trans->status[i];

		//printf("n = %d\n", n);

		for(j = 0; j < ALPHABET_COUNT; j++)
		{
			trans_func_append_copy(&nsa_status->trans[j], &nsa->status[n-1]->trans[j]);
		}
	}

	nsa_append_status(nsa, nsa_status);
}

static void decode_from_nsa_to_dsa(nsa_t *nsa, dsa_t *dsa, table_t *table)
{
	dsa_status_t* dsa_status;
	int i;

	for(i = 0; i < nsa->count; i++)
	{
		int j;

		dsa_status = dsa_status_new(i+1);
		dsa_status->flag = nsa->status[i]->flag;

		for(j = 0; j < ALPHABET_COUNT; j++)
		{
			trans_function_t *trans;

			trans = &nsa->status[i]->trans[j];

			if( trans->len > 1 )
			{
				int n;

				n = table_find(table, trans);

				if(  n != -1 )
				{
					dsa_status_append(dsa_status, ID_TO_ALPHA(j), table->line[n].status);
				}
			}
			else
			{
				if( trans->len == 1 )
				{
					dsa_status_append(dsa_status, ID_TO_ALPHA(j), trans->status[0]);
				}
			}
		}

		dsa_append_status(dsa, dsa_status);
	}
}

void convert_nsa_to_dsa(nsa_t *nsa, dsa_t *dsa)
{
	table_t table;
	nsa_t *nsa_temp;
	int accept_status;
	int next;
	int i;

	table_init(&table);

	accept_status = nsa->count;
	nsa_temp = nsa_clone(nsa);

	do{
		next = 0;

		for(i = 0; i < nsa_temp->count; i++)
		{
			int j;

			for(j = 0; j < ALPHABET_COUNT; j++)
			{
				trans_function_t *trans;
				int n;

				trans = &nsa_temp->status[i]->trans[j];
	
				n = table_find(&table, trans);
	
				if( n == -1 && trans->len > 1 )
				{
					table_append(&table, trans, nsa_temp->count+1);
					create_new_status(nsa_temp, trans, accept_status);
					next = 1;
				}
			}
		}
	}while( next );

	decode_from_nsa_to_dsa(nsa_temp, dsa, &table);

#if 0
	printf("------------\n");
	nsa_print(nsa);
	printf("------------\n");
	table_print(&table);
	printf("------------\n");
	nsa_print(nsa_temp);
	printf("------------\n");
	dsa_print(dsa);
#endif

	//nsa_print_graphviz(nsa);
	//dsa_print_graphviz(dsa);

	table_destroy(&table);
	nsa_destroy(nsa_temp);
}

static void test_dsa_regexp(char *str_regexp)
{
	nsa_t *nsa;
	dsa_t *dsa;

	dsa = dsa_new();
	nsa = nsa_new();

	nsa_regexp(nsa, str_regexp);
	convert_nsa_to_dsa(nsa, dsa);
	
	//nsa_print_graphviz(nsa);
	//nsa_print(nsa);

	dsa_print_graphviz(dsa);
	//dsa_print(dsa);

	//printf("res = %d\n", nsa_step(nsa, "ABACD") );
	//printf("res = %d\n", dsa_step(dsa, "ABACD") );

	nsa_destroy(nsa);
	dsa_destroy(dsa);
}

int regexp(char *str_trgexp, char *str_in)
{
	nsa_t *nsa;
	dsa_t *dsa;
	int res_nsa;
	int res_dsa;

	dsa = dsa_new();
	nsa = nsa_new();

	if( nsa_regexp(nsa, str_trgexp) == -1 )
	{
		nsa_destroy(nsa);
		dsa_destroy(dsa);

		return -1;
	}

	convert_nsa_to_dsa(nsa, dsa);
	
	res_nsa = nsa_step(nsa, str_in);
	res_dsa = dsa_step(dsa, str_in);

	nsa_destroy(nsa);
	dsa_destroy(dsa);

	return res_dsa;
}

//#define TEST_REGEXP

#ifdef TEST_REGEXP

#define STR_SIZE		256

static void cresh_test()
{
	char str_regexp[STR_SIZE];
	char in[STR_SIZE];
	int len;
	int i;

	len = random() % 10 + 4;

	memset(str_regexp, 0, STR_SIZE);
	memset(in, 0, STR_SIZE);

	for(i = 0; i < len; i++)
	{
		int n;

		n = rand() % 3;

		if( n == 0 )
		{
			str_regexp[i] = '*';
		}
		else if( n == 1 )
		{
			str_regexp[i] = '?';
		}
		else
		{
			n = rand() % 8;
			str_regexp[i] = 'A'+n;
		}
	}

	len = random() % 20 + 4;

	for(i = 0; i < len; i++)
	{
		int n;

		n = rand() % 8;
		in[i] = 'A'+n;
	}

	printf("%s %s %d\n", str_regexp, in, regexp(str_regexp, in));
}

int main(int argc, char **argv)
{
#if 0
	int i;

	srand( (unsigned int) time(NULL) );

	for(i = 0; i < 1000; i++)
	{
		cresh_test();
	}
#endif

	test_dsa_regexp("*ABABACA*");

	return 0;
}
#endif
