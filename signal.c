
#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <signal.h>

#include "main.h"

#include "signal.h"

void handler_sig_int(int signo)
{
	fprintf(stdout, "Signal INT\n");
	fprintf(stdout, "get my control term: %d\n", tcgetpgrp(0) );
}

void handler_sig_stop(int signo)
{
	fprintf(stdout, "Signal SIGTSTP\n");
	fprintf(stdout, "get my control term: %d\n", tcgetpgrp(0) );
}

void handler_sig_ttou(int signo)
{
	//fprintf(stderr, "Signal SIGTTOU\n");
	//fprintf(stderr, "get my control term: %d\n", tcgetpgrp(0) );
}

int signal_set_for_process()
{
	signal(SIGINT, SIG_DFL);
	signal(SIGTTIN, SIG_DFL);
	signal(SIGTTOU, SIG_DFL);
	signal(SIGTSTP, SIG_DFL);

	return 0;
}

int signal_set_for_shell()
{
	signal(SIGINT, handler_sig_int);

	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTSTP, SIG_IGN);

	return 0;
}

int signal_send_to_session(pid_t sessin_pid, int signum)
{
	return kill(-sessin_pid, signum);
}

int signal_send_to_process(pid_t process_pid, int signum)
{
	return kill(process_pid, signum);
}

int signal_init()
{
	signal_set_for_shell();

	return 0;
}
