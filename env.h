
#ifndef ENV_H

#define ENV_H

extern char** env_import();
extern char* env_get(const char *name);
extern int env_set(const char *name, const char *value);
extern int env_is_set(const char *name);
extern int env_unset(const char *name);
extern int env_print(const char *name);
extern void env_print_all();

#endif
