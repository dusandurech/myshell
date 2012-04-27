CC = gcc
FLAGS = -g -O0
OBJ = array.o command.o dir.o env.o expand_var.o history.o inter_cmd.o jobs.o main.o process.o readline.o regexp.o signal.o terminal.o util.o
OUT_BIN = myshell
LIBS = -ltermcap

$(OUT_BIN): $(OBJ)
	$(CC) $(FLAGS) -o $(OUT_BIN) $(OBJ) $(LIBS)

array.o: array.c array.h
	$(CC) $(FLAGS) -c array.c

command.o: command.c main.h env.h array.h dir.h expand_var.h process.h \
  command.h
	$(CC) $(FLAGS) -c command.c

dir.o: dir.c dir.h array.h
	$(CC) $(FLAGS) -c dir.c

env.o: env.c
	$(CC) $(FLAGS) -c env.c

expand_var.o: expand_var.c main.h env.h array.h expand_var.h
	$(CC) $(FLAGS) -c expand_var.c

history.o: history.c main.h util.h array.h history.h
	$(CC) $(FLAGS) -c history.c

inter_cmd.o: inter_cmd.c main.h jobs.h util.h inter_cmd.h
	$(CC) $(FLAGS) -c inter_cmd.c

jobs.o: jobs.c main.h array.h signal.h jobs.h
	$(CC) $(FLAGS) -c jobs.c

main.o: main.c main.h process.h command.h inter_cmd.h terminal.h \
  readline.h history.h signal.h jobs.h
	$(CC) $(FLAGS) -c main.c

process.o: process.c main.h array.h dir.h jobs.h terminal.h process.h
	$(CC) $(FLAGS) -c process.c

readline.o: readline.c main.h util.h history.h terminal.h readline.h
	$(CC) $(FLAGS) -c readline.c

regexp.o: regexp.c regexp.h
	$(CC) $(FLAGS) -c regexp.c

signal.o: signal.c main.h signal.h
	$(CC) $(FLAGS) -c signal.c

terminal.o: terminal.c main.h util.h terminal.h
	$(CC) $(FLAGS) -c terminal.c

util.o: util.c main.h util.h
	$(CC) $(FLAGS) -c util.c

clean:
	rm -rf *.o $(OUT_BIN)
