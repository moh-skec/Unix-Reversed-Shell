all: client server paraserver

client: client.o
	gcc client.o -o client
client.o: client.c
	gcc -c client.c
server: server.o
	gcc server.o -o server
server.o: server.c
	gcc -c server.c
paraserver: paraserver.o
	gcc parserver.o -o paraserver
paraserver.o: paraserver.c
	gcc -c paraserver.c
