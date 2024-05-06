main:
	make http-server
http-server:
	gcc -c -Wall src/main.c -o main.o
	gcc -c -Wall src/parser.c -o  parser.o
	gcc main.o parser.o -o server
	rm -rf main.o parser.o
