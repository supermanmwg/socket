all:server client timer
server: server.c
	gcc server.c -o server -g
client: client.c
	gcc client.c -o client -g
timer: time.c
	gcc time.c -o timer -g -lrt

clean:
	rm server client timer

