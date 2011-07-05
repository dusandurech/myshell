
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "array.h"

array_t* array_new()
{
	array_t *new;

	new = malloc( sizeof(array_t) );
	memset(new, 0, sizeof(array_t) );

	return new;
}

array_t* array_clone(array_t *p)
{
	array_t *new;

	assert( p != NULL );

	new = malloc( sizeof(array_t) );
	memcpy(new, p, sizeof(array_t) );

	if( p->list != NULL )
	{
		new->list = malloc( p->alloc * sizeof(void *) );
		memcpy(new->list, p->list, p->alloc * sizeof(void *));
	}

	return new;
}

array_t* array_clone_item(array_t *p, void *f)
{
	array_t *new;
	void* (*fce)(void *);
	int i;

	assert( p != NULL );
	assert( f != NULL );

	new = array_clone(p);
	fce = f;

	for(i = 0 ; i < p->count ; i++)
		new->list[i] = fce(p->list[i]);

	return new;
}

void array_add(array_t *p, void *item)
{
	assert( p != NULL );

	if( p->alloc == 0 )
	{
		p->alloc = ARRAY_ALLOC_LIMIT;
		p->count = 1;
		p->list = malloc(p->alloc * sizeof(void *) );
		memset(p->list, 0, p->alloc * sizeof(void *));
		p->list[0] = item;

		return;
	}

	if( p->count + 1 <= p->alloc )
	{
		p->list[p->count] = item;
		p->count += 1;

		return;
	}

	if( p->count + 1 > p->alloc )
	{
		void **new;

#ifdef DEBUG_LIST
		printf("realokujem z %d na %d (count=%d)\n",
		p->alloc, p->alloc+LIST_ALLOC_LIMIT, p->count);
#endif

		p->alloc += ARRAY_ALLOC_LIMIT;
		new = malloc(p->alloc * sizeof(void *) );
		memcpy(new, p->list, p->count * sizeof(void *) );
		free(p->list);
		p->list = new;
		p->list[p->count] = item;
		p->count++;

		return;
	}
}

void array_insert(array_t *p, int n, void *item)
{
	assert( p != NULL );

	array_add(p, NULL); // :)

	assert( n >= 0 || n < p->count );

	memmove(p->list+n+1, p->list+n, ( (p->count-1) - n ) * sizeof(void *));

	p->list[n] = item;
}

void *array_get(array_t *p, int n)
{
	assert( p != NULL );
	assert( n >= 0 || n < p->count );

	return p->list[n];
}

int array_search_item(array_t *p, void *n)
{
	int i;

	assert( p != NULL );

	for( i = 0 ; i < p->count ; i++ )
		if( p->list[i] == n )return i;

	return -1;
}

void array_del(array_t *p, int n)
{
	assert( p != NULL );
	assert( n >= 0 || n < p->count );

	memmove(p->list+n, p->list+n+1,  ( (p->count-1) - n ) * sizeof(void *));
	
	p->count--;

	if( p->count + ARRAY_ALLOC_LIMIT < p->alloc )
	{
		void **new;

#ifdef DEBUG_ARRAY
		printf("realokujem z %d na %d (count=%d)\n",
		p->alloc, p->alloc-ARRAY_ALLOC_LIMIT, p->count);
#endif

		p->alloc -= ARRAY_ALLOC_LIMIT;
		new = malloc(p->alloc * sizeof(void *) );
		memcpy(new, p->list, p->count * sizeof(void *) );
		free(p->list);
		p->list = new;
	}
}

void array_del_item(array_t *p, int n, void *f)
{
	int (*fce)(void *);

	assert( p != NULL );
	assert( n >= 0 || n < p->count );

	fce = f;
	if( fce != NULL )fce(p->list[n]);

	array_del(p, n);
}

void arrray_do_empty(array_t *p)
{
	assert( p != NULL );

	p->count = 0;
}

void arrray_do_empty_item(array_t *p, void *f)
{
	void (*fce)(void *);
	int i;

	assert( p != NULL );
	assert( f != NULL );

	fce = f;

	for(i = 0 ; i < p->count ; i++)
		fce(p->list[i]);

	arrray_do_empty(p);
}

void array_print_string(array_t *p)
{
	int i;

	assert( p != NULL );

	for(i = 0 ; i < p->count ; i++)
	{
		char *s;

		s = (char *) p->list[i];

		printf(">%s< ", s);
	}

	putchar('\n');
}

void** array_get_clone_array(array_t *array, void *f)
{
	void **res;
	void* (*fce)(void *);
	int i;

	assert( array != NULL );
	assert( f != NULL );

	fce = f;

	res = (void **) malloc( (array->count+1) * sizeof(void *) );

	for(i = 0; i < array->count; i++ )
	{
		void *item;

		item = (void *) array->list[i];
		res[i] = fce(item);
	}

	res[array->count] = NULL;

	return res;
}

void array_destroy(array_t *p)
{
	assert( p != NULL );

	if( p->list != NULL )free(p->list);
	free(p);
}

void array_destroy_item(array_t *p, void *f)
{
	void (*fce)(void *);
	int i;

	assert( p != NULL );
	assert( f != NULL );

	fce = f;

	for(i = 0 ; i < p->count ; i++)
		fce(p->list[i]);

	array_destroy(p);
}

