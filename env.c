
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern char **environ;

char** env_import()
{
	return environ;
}

char* env_get(const char *name)
{
	return getenv(name);
}

int env_set(const char *name, const char *value)
{
	return setenv(name, value, 1);
}

int env_unset(const char *name)
{
	return unsetenv(name);
}

int env_print(const char *name)
{
	char *value;

	value = env_get(name);

	if( value != NULL )
	{
		printf("%s=%s\n", name, value);
		return 1;
	}

	return 0;
}

void env_print_all()
{
	char **array;
	int i;

	array = env_import();

	for(i = 0; array[i] != NULL; i++)
	{
		char *str;

		str = array[i];
		printf("%s\n", str);
	}
}

#ifdef TEST_ENV
int main()
{
	env_set("A", "1");
	env_set("A", "2");
	env_set("B", "X");
	env_unset("B");

	env_print("A");
	env_print("B");

	printf("---\n");
	env_print_all();

	return 0;
}
#endif
