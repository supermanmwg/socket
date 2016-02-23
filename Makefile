all:server client timer mthread client_all encry
server: server.c
	gcc server.c -o server -g
client: client.c
	gcc client.c -o client -g -lpthread
timer: time.c
	gcc time.c -o timer -g -lrt
mthread: mthread.c
	gcc mthread.c -o mthread -g -lpthread
client_all:
	gcc client_all.c -o client_all -g -lpthread -lrt
encry:
	gcc md5.c encry.c -o encry

clean:
	rm server client timer client_all encry

