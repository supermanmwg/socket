all:server client
server: server.c
	gcc server.c -o server -g
client: client.c
	gcc client.c -o client -g -lpthread

clean:
	rm server client
