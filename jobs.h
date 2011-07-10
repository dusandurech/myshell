
#ifndef JOBS_H

#define JOBS_H

#define JOBS_RUN_IN_FG	1
#define JOBS_RUN_IN_BG	2

#define JOBS_STAT_RUN		1
#define JOBS_STAT_STOP		2
#define JOBS_STAT_DONE		3

extern int jobs_init();
extern int jobs_add_process(const char *str_cmd, const pid_t session_pid, const int stat);
extern int jobs_clean_process(const pid_t session_pid);
extern int jobs_run(const int id, const int type);
extern void jobs_print_all();
extern int jobs_print_process(const int id);
extern int jobs_quit();

#endif
