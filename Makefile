
CC=gcc
OBJ=main.o regexp.o util.o array.o dir.o terminal.o process.o command.o

myshell: $(OBJ)
	$(CC) $(FLAGS) -o myshell $(OBJ)

main.o: main.c main.h
	$(CC) $(FLAGS) -c main.c

regexp.o: regexp.c regexp.h
	$(CC) $(FLAGS) -c regexp.c

util.o: util.c util.h
	$(CC) $(FLAGS) -c util.c

array.o: array.c array.h
	$(CC) $(FLAGS) -c array.c

dir.o: dir.c dir.h
	$(CC) $(FLAGS) -c dir.c

terminal.o: terminal.c terminal.h
	$(CC) $(FLAGS) -c terminal.c

process.o: process.c process.h
	$(CC) $(FLAGS) -c process.c

command.o: command.c command.h
	$(CC) $(FLAGS) -c command.c

clean:
	rm -rf *.o myshell
