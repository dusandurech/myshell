
#ifndef TERMINAL_H

#define TERMINAL_H

extern int term_init();
extern void term_print_status();
extern int term_readline(char *str_line);
extern int term_set_old();
extern int term_set_new();
extern int term_set_control(pid_t session_pid);
extern int term_get_file_fd();
extern int term_quit();

#endif
