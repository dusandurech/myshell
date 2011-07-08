
#ifndef SIGNAL_H

#define SIGNAL_H

extern int signal_set_for_process();
extern int signal_set_for_shell();
extern int signal_send_to_session(pid_t sessin_pid, int signum);
extern int signal_send_to_process(pid_t process_pid, int signum);
extern int signal_init();

#endif
