#makefile
CC= gcc

all: tcpserveur tcpclient

tcpserveur : ./serveur/serveur.c
	$(CC) -Wall -o serveurRUN ./serveur/serveur.c 

tcpclient : ./client/client.c
	$(CC) -Wall -o clientRUN ./client/client.c 

# règle nettoyage .o
clean :
	rm -f ./serveurRUN ./clientRUN *.o

