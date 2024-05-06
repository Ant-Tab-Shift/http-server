main:
	make server
server:
	gcc -c -Wall main.c -o main.o
	gcc -c -Wall parser.c -o  parser.o
	gcc main.o parser.o -o resources/server
	rm -rf main.o parser.o
