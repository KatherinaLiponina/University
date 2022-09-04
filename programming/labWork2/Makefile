all: server
server: main.o linkedlist.o
	gcc -o server main.o linkedlist.o -lpthread
main.o: main.c
	gcc -c main.c
linkedlist.o: linkedlist.c
	gcc -c linkedlist.c
