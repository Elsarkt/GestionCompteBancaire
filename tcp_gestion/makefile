#makefile
CC= gcc

all: tcpserveur tcpclient

tcpserveur : ./serveur/serveur.c
	$(CC) -Wall -o ./serveur/serveur ./serveur/serveur.c 

tcpclient : ./client/client.c
	$(CC) -Wall -o ./client/client ./client/client.c 

# règle nettoyage .o
clean :
	rm -f ./serveur/* ./client/* *.o

