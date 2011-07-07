
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

int signal_init()
{
	signal(SIGINT, handler_sig_int);
	signal(SIGTSTP, handler_sig_stop);

	signal(SIGTTIN, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);

	return 0;
}
