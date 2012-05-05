
#ifndef UTIL_H

#define UTIL_H

extern char* get_nodename();
extern char* get_username();
extern char* get_home_dir();
extern char* get_current_dir();
extern int set_current_dir(const char *path);
extern void append_file_to_path(char *path, char *filename);
extern int str_append_to_char(char *str, int maxlen, char c);

#endif
