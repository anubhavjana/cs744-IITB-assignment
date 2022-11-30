all: load server

load: load_gen.c
	gcc -g -pthread load_gen.c -o load

server: server.c
	gcc -g -pthread server.c -o server -w

clean:
	rm -f load server *.log *.png
