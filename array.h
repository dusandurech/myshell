
#ifndef ARRAY_H

#define ARRAY_H

#define ARRAY_ALLOC_LIMIT 256

typedef struct array_struct
{
	void **list;
	int count;
	int alloc;
} array_t;

extern array_t* array_new();
extern array_t* array_clone(array_t *p);
extern array_t* array_clone_item(array_t *p, void *f);

extern void array_add(array_t *p, void *item);
extern void array_insert(array_t *p, int n, void *item);
extern void *array_get(array_t *p,int n);
extern int array_search_item(array_t *p, void *n);

extern void array_del(array_t *p,int n);
extern void array_del_item(array_t *p,int n,void *f);
extern void arrray_do_empty(array_t *p);

extern void array_destroy(array_t *p);
extern void array_destroy_item(array_t *p,void *f);

#endif
