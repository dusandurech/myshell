
#ifndef PROCESS_H

#define PROCESS_H

#define PROCESS_STDOUT_FILE		4
#define PROCESS_STDIN_FILE		8
#define PROCESS_STDOUT_APPEND_FILE	16
#define PROCESS_STDIN_DOC_HERE		32
#define PROCESS_NO_WAIT			64
#define PROCESS_AND			128
#define PROCESS_OR			256

typedef struct doument_here_struct
{
	char *text;
	char *eof;
	unsigned int length;
} doument_here_t;

typedef struct process_struct
{
	char *filename_exec;
	char **param;
	char **env;

	char *stdin_filename;
	char *stdout_filename;

	unsigned int flag;
	
	doument_here_t *doument_here;

	struct process_struct *pipe_process;
	struct process_struct *next_process;
} process_t;

extern process_t* process_new();
extern void process_set(process_t *process, char *filename_exec, char **param, char **env);
extern void process_print(const process_t *process);
extern int process_run(process_t *process);
extern void process_destroy(process_t *process);

#endif
