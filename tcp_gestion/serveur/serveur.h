//serveur.h
#ifndef SERVEUR_H
#define SERVEUR_H


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> /* close */
#include <netdb.h> /* gethostbyname */
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;


#define CRLF        "\r\n"
#define PORT         1977
#define MAX_CLIENTS     100

#define BUF_SIZE    1024

#include "client.h"

static void app(void);
static int init_connection(void);
static void end_connection(int sock);
static int read_client(SOCKET sock, char *buffer);
static void write_client(SOCKET sock, const char *buffer);
// static void send_message_to_all_clients(Client *clients, Client client, int actual, const char *buffer, char from_server);
static void remove_client(Client *clients, int to_remove, int *actual);
static void clear_clients(Client *clients, int actual);
void requete_type(Client client, const char* buffer, char buffercopy[], char* param[]);
Client nouveau_client(int nbCompte, SOCKET csock, const char* buffer,char* buffercopy, int actual);
static void reponseServeur(const char *buffer, Client c, char* tabRequete[], int nbCompte);



#endif /* guard */


  // Décaler les opérations existantes vers la droite
   // for (int j = 9; j > 0; j--) {
   //    if (c.comptes[compte_trouve].operations[j] != NULL) {
   //       // free(c.comptes[compte_trouve].operations[j]); // Libère la mémoire de l'opération qui sera écrasée
   //       c.comptes[compte_trouve].operations[j] = NULL;
   //    }
   //    c.comptes[compte_trouve].operations[j] = c.comptes[compte_trouve].operations[j - 1];
   // }
   // if (c.comptes[compte_trouve].operations[0] != NULL) {
   //    // free(c.comptes[compte_trouve].operations[0]); // Libère l'ancienne opération en position 0
   //    c.comptes[compte_trouve].operations[0] = NULL;
   // }
   // char* copieMsg = strdup(message);
   // if (copieMsg == NULL) {
   //    perror("Erreur d'allocation mémoire pour operations[0]");
   // }
   // c.comptes[compte_trouve].operations[0] = copieMsg; // Ajoute la nouvelle opération en première position
   // char *temp;