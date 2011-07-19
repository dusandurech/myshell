
#ifndef REGEXP_H

#define REGEXP_H

typedef struct automat_struct
{
	char *str_regexp;
	int len_regexp;
	int *map;
	int count_status;
	int current_status;
	int **matrix;
} automat_t;

extern automat_t* automat_new(const char *str_regexp);
extern int automat_get_current_status(const automat_t *automat);
extern void automat_set_current_status(automat_t *automat, const int status);
extern void automat_reset(automat_t *automat);
extern char* automat_get_regexp(automat_t *automat);
extern char* automat_get_curent_regexp(const automat_t *automat);
extern void automat_print(const automat_t *automat);
extern int automat_is_final_status(const automat_t *automat);
extern void automat_step(automat_t *automat, const char c);
extern void automat_steps_for_string(automat_t *automat, const char *str);
extern int automat_destroy(automat_t *automat);
extern int regexp(automat_t *automat, const char *str);

#endif
