CC = gcc
FLAGS = -g -O0
OBJ = array.o command.o dir.o main.o process.o regexp.o terminal.o util.o
OUT_BIN = myshell

$(OUT_BIN): $(OBJ)
	$(CC) $(FLAGS) -o $(OUT_BIN) $(OBJ)

array.o: array.c array.h
	$(CC) $(FLAGS) -c array.c

command.o: command.c main.h array.h dir.h process.h command.h
	$(CC) $(FLAGS) -c command.c

dir.o: dir.c dir.h array.h
	$(CC) $(FLAGS) -c dir.c

main.o: main.c main.h process.h command.h terminal.h
	$(CC) $(FLAGS) -c main.c

process.o: process.c main.h array.h dir.h process.h
	$(CC) $(FLAGS) -c process.c

regexp.o: regexp.c main.h regexp.h
	$(CC) $(FLAGS) -c regexp.c

terminal.o: terminal.c main.h util.h terminal.h
	$(CC) $(FLAGS) -c terminal.c

util.o: util.c main.h util.h
	$(CC) $(FLAGS) -c util.c

clean:
	rm -rf *.o $(OUT_BIN)
