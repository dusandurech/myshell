
#ifndef TERMINAL_H

#define TERMINAL_H

extern int term_init();
extern void term_cursor_right();
extern void term_cursor_left();
extern void term_putc(char c);
extern void term_puts(char *s);
extern int term_getc();
extern int term_set_old();
extern int term_set_new();
extern int term_set_control(pid_t session_pid);
extern int term_get_file_fd();
extern int term_quit();

#endif
