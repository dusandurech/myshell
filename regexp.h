
#ifndef REGEXP_H

#define REGEXP_H

typedef struct automat_struct
{
	char *input_alphabet;
	int count_status;
	int current_status;
	int **matrix;
} automat_t;

extern automat_t* automat_new(const char *str_regexp);
extern void automat_reset(automat_t *automat);
extern void automat_print(const automat_t *automat);
extern int automat_is_final_status(const automat_t *automat);
extern void automat_step(automat_t *automat, const char c);
extern int automat_destroy(automat_t *automat);
extern int regexp(automat_t *automat, const char *str);

#endif
