CC = gcc
FLAGS = -g -O0
OBJ = array.o automat.o command.o dir.o env.o expand_regexp.o expand_var.o inter_cmd.o jobs.o main.o process.o regexp.o signal.o terminal.o util.o
OUT_BIN = myshell

$(OUT_BIN): $(OBJ)
	$(CC) $(FLAGS) -o $(OUT_BIN) $(OBJ)

array.o: array.c array.h
	$(CC) $(FLAGS) -c array.c

automat.o: automat.c main.h automat.h
	$(CC) $(FLAGS) -c automat.c

command.o: command.c main.h env.h array.h dir.h process.h command.h
	$(CC) $(FLAGS) -c command.c

dir.o: dir.c dir.h array.h
	$(CC) $(FLAGS) -c dir.c

env.o: env.c
	$(CC) $(FLAGS) -c env.c

expand_regexp.o: expand_regexp.c main.h array.h dir.h automat.h regexp.h
	$(CC) $(FLAGS) -c expand_regexp.c

expand_var.o: expand_var.c main.h env.h array.h expand_var.h
	$(CC) $(FLAGS) -c expand_var.c

inter_cmd.o: inter_cmd.c main.h jobs.h inter_cmd.h
	$(CC) $(FLAGS) -c inter_cmd.c

jobs.o: jobs.c main.h array.h signal.h jobs.h
	$(CC) $(FLAGS) -c jobs.c

main.o: main.c main.h process.h command.h inter_cmd.h terminal.h signal.h \
  jobs.h
	$(CC) $(FLAGS) -c main.c

process.o: process.c main.h array.h dir.h jobs.h process.h
	$(CC) $(FLAGS) -c process.c

regexp.o: regexp.c main.h automat.h regexp.h
	$(CC) $(FLAGS) -c regexp.c

signal.o: signal.c main.h signal.h
	$(CC) $(FLAGS) -c signal.c

terminal.o: terminal.c main.h util.h terminal.h
	$(CC) $(FLAGS) -c terminal.c

util.o: util.c main.h util.h
	$(CC) $(FLAGS) -c util.c

clean:
	rm -rf *.o $(OUT_BIN)
