
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include <unistd.h>
#include <sys/utsname.h>
#include <pwd.h>

#include "main.h"
#include "util.h"

char* get_nodename()
{
	static int isFirst = 1;
	static char str_nodename[STR_SIZE];

	if( isFirst == 1 )
	{
		struct utsname buf;

		uname(&buf);
		strcpy(str_nodename, buf.nodename);

		isFirst = 0;
	}

	return str_nodename;
}

char* get_username()
{
	static int isFirst = 1;
	static char str_username[STR_SIZE];

	if( isFirst == 1 )
	{
		struct passwd *pass;
		
		pass = getpwuid(getuid());
		strcpy(str_username, pass->pw_name);

		isFirst = 0;
	}

	return str_username;
}

char* get_home_dir()
{
	return getenv("HOME");
}

char* get_current_dir()
{
	static char path[STR_PATH_SIZE];

	getcwd(path, STR_PATH_SIZE);

	return path;
}
