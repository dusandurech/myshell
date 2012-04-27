
#ifndef REGEXP_H

#define REGEXP_H

#define ALPHABET_COUNT		(126-32+1)
//#define ALPHABET_COUNT		(8)

typedef struct trans_function_struct
{
	int len;
	int alloc;
	int *status;
} trans_function_t;

extern void trans_func_clean(trans_function_t *dst);
extern void trans_function_clone(trans_function_t *dst, trans_function_t *src);
extern int trans_func_cmp(trans_function_t *trans1, trans_function_t *trans2);
extern void trans_func_destroy(trans_function_t *trans);
extern int trans_func_is_in(trans_function_t *trans, int status);
extern void trans_func_append(trans_function_t *trans, int status);
extern void trans_func_append_copy(trans_function_t *dst, trans_function_t *src);
extern void trans_func_print(trans_function_t *trans);
 
typedef struct nsa_status_struct
{
	int status;
	int flag;
	trans_function_t trans[ALPHABET_COUNT];
} nsa_status_t;

extern nsa_status_t* nsa_status_new(int status);
extern nsa_status_t* nsa_status_clone(nsa_status_t *nsa_status);
extern void nsa_status_fin(nsa_status_t *nsa_status);
extern void nsa_status_append(nsa_status_t *nsa_status, char c, int status);
extern void nsa_status_print(nsa_status_t *nsa_status);
extern void nsa_status_print_graphviz(nsa_status_t *nsa_status);
extern void nsa_status_destroy(nsa_status_t *nsa_status);

typedef struct nsa_struct
{
	int count;
	int alloc;
	nsa_status_t **status;
} nsa_t;

extern nsa_t* nsa_new();
extern void nsa_append_status(nsa_t *nsa, nsa_status_t *status);
extern int nsa_step(nsa_t *nsa, char *str);
extern nsa_t* nsa_clone(nsa_t *nsa);
extern void nsa_print(nsa_t *nsa);
extern void nsa_print_graphviz(nsa_t *nsa);
extern int nsa_regexp(nsa_t *nsa, char *str_regexp);
extern void nsa_destroy(nsa_t *nsa);

typedef struct dsa_status_struct
{
	int status;
	int flag;
	int trans_function[ALPHABET_COUNT];
} dsa_status_t;

extern dsa_status_t* dsa_status_new(int status);
extern void dsa_status_fin(dsa_status_t *dsa_status);
extern void dsa_status_append(dsa_status_t *dsa_status, char c, int status);
extern void dsa_status_print(dsa_status_t *dsa_status);
extern void dsa_status_print_graphviz(dsa_status_t *dsa_status);
extern void dsa_status_destroy(dsa_status_t *dsa_status);

typedef struct dsa_struct
{
	int count;
	int alloc;
	dsa_status_t **status;
} dsa_t;

extern dsa_t* dsa_new();
extern void dsa_print(dsa_t *dsa);
extern void dsa_print_graphviz(dsa_t *dsa);
extern void dsa_regexp(dsa_t *dsa, char *str_regexp);
extern int dsa_one_step(dsa_t *dsa, int status, char c);
extern int dsa_is_stat_fin(dsa_t *dsa, int status);
extern int dsa_is_stat_blackhole(dsa_t *dsa, int status);
extern int dsa_step(dsa_t *dsa, char *str);
extern void dsa_append_status(dsa_t *dsa, dsa_status_t *status);
extern void dsa_destroy(dsa_t *dsa);
extern void convert_nsa_to_dsa(nsa_t *nsa, dsa_t *dsa);
extern int regexp(char *str_trgexp, char *str_in);

#endif
