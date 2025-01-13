#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "serveur.h"
#include "client.h"

#define DEBBUG


static void app(void) {
   SOCKET sock = init_connection();
   char buffer[BUF_SIZE];
   /* the index for the array */
   int actual = 0;
   int nbCompte = 0;
   int max = sock;
   /* an array for all clients */
   Client clients[MAX_CLIENTS];
   fd_set rdfs;

   while(1)
   {
      int i = 0;
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      /* add socket of each client */
      for(i = 0; i < actual; i++){
         FD_SET(clients[i].sock, &rdfs); //le sock de chaque client déclaré dans l'ens des descripteur à surveiller plus tard en lecture
      }

      if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1){
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs)){
         /* stop process when type on keyboard */
         break;
      }
      else if(FD_ISSET(sock, &rdfs)){
      //Si un client se connecte : Si on lit qc sur le sock du serveur
         /* new client */
         SOCKADDR_IN csin = { 0 };
         socklen_t sinsize = sizeof csin; //origin size_t
         //Nouveau socket pour communiquer avec le client 
         int csock = accept(sock, (SOCKADDR *)&csin, &sinsize); //accepte la connexion d'un socket sur le socket sock
         if(csock == SOCKET_ERROR){
            perror("accept()");
            continue;
         }         

         /* after connecting the client sends its name */
         if(read_client(csock, buffer) == -1){
            /* disconnected */
            continue;
         }

         /* what is the new maximum fd ? */
         max = csock > max ? csock : max;

         FD_SET(csock, &rdfs);

         char buffercopy[BUF_SIZE];
         Client c = nouveau_client(nbCompte, csock, buffer, buffercopy, actual);
         clients[actual] = c;
         actual++;
      }
      else {
      //Pas de nouvelles connexions : rien sur socket sock du serveur
         int i = 0;

         for(i = 0; i < actual; i++) {
            /* a client is talking */

            if(FD_ISSET(clients[i].sock, &rdfs)) {
               Client client = clients[i];
               int c = read_client(clients[i].sock, buffer);
               /* client disconnected */

               if(c == 0) { //pas d'octet reçu du client c 
                  closesocket(clients[i].sock);
                  remove_client(clients, i, &actual);
                  strncpy(buffer, client.name, BUF_SIZE - 1);
                  strncat(buffer, " disconnected !", BUF_SIZE - strlen(buffer) - 1);
                  // send_message_to_all_clients(clients, client, actual, buffer, 1);
               }
               else {
               
                  // send_message_to_all_clients(clients, client, actual, buffer, 0);
                  char buffercopy[BUF_SIZE];
                  char* tabRequete[5];
                  requete_type(clients[i], buffer, buffercopy, tabRequete);
                  #ifdef DEBBUG
                        for (int j = 0; j < 5; j++) {
                           if (tabRequete[j] != NULL) printf("Dans debug %s \n", tabRequete[j]);
                        }
                  #endif
                  reponseServeur(buffer, clients[i], tabRequete);
               }
               break;
            }
         }
      }
   }
   clear_clients(clients, actual);
   end_connection(sock);
}


static void clear_clients(Client *clients, int actual){
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      closesocket(clients[i].sock);
   }
}


static void remove_client(Client *clients, int to_remove, int *actual){
   /* we remove the client in the array */
   memmove(clients + to_remove, clients + to_remove + 1, (*actual - to_remove - 1) * sizeof(Client));
   /* number client - 1 */
   (*actual)--;
}


static void reponseServeur(const char *buffer, Client c, char* tabRequete[]) {
   // int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   int id_compte = atoi(tabRequete[2]); //conversion char* à int
    
   if (tabRequete == NULL || tabRequete[0] == NULL || tabRequete[3] == NULL || tabRequete[4] == NULL) {
      printf("Erreur : tabRequete invalide.\n");
   }

   if(strcmp(c.password, tabRequete[3]) == 0) {//le mdp correspond toujours au 3ème argument
          // Vérifier si l'id_compte correspond à un compte du client
      int compte_trouve = 0;
      for (int i = 0; i<100; i++) { //Supposons qu'il y a moins que 500 comptes crées dans la base de données
         if (c.comptes[i].idCompte == id_compte) {
            compte_trouve = 1;
            break;
         }
      }
      if (!compte_trouve) snprintf(message, BUF_SIZE, "Erreur : compte %d non trouvé.\n", id_compte);
      else{
         if (strcmp(tabRequete[0], "AJOUT") == 0){
            int somme = atoi(tabRequete[4]);
            c.comptes[id_compte].montant += somme; //On ajoute la somme voulue sur le compte identifié
            snprintf(message, BUF_SIZE, "AJOUT de %d€ sur votre compte\n", somme);
         } 
         else if(strcmp(tabRequete[0], "OPERATIONS")){
            printf("dans reponseServeur OPERATIONS\n");
            snprintf(message, BUF_SIZE, "Liste des 10 dernières opérations :\n");
            write_client(c.sock, message);
            for(int i=0; i<10; i++){
               snprintf(message, BUF_SIZE, "%s\n",c.comptes[id_compte].operations[i]);
               write_client(c.sock, message);
            }
            snprintf(message, BUF_SIZE, "\n");
         }
         else snprintf(message, BUF_SIZE, "on verra ça plus tard\n");
      } 
   } else snprintf(message, BUF_SIZE, "Mode de passe érroné\n");

   //Décaler les opérations existantes vers la droite
   for (int j = 9; j > 0; j--) { //operations est un tab de 10 str
      c.comptes[id_compte].operations[j] = c.comptes[id_compte].operations[j - 1];
   }
   // Ajouter la nouvelle opération en première position
   c.comptes[id_compte].operations[0] = strdup(message);
   if (c.comptes[id_compte].operations[0] == NULL) {
      perror("Erreur d'allocation mémoire pour operations[0]");
      return;
   }

   write_client(c.sock, message);
}

static int init_connection(void){
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == SOCKET_ERROR)
   {
      perror("socket()");
      exit(errno);
   }

   //permet la réutilisation de l'adresse directement après la fermeture de la précédente conenxion
   int optval = 1;
   if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1)
   {
      perror("setsockopt()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR) //associe le socket déclaré à l'adresse et le port que le serveur s'est défini
   {
      perror("bind()");
      exit(errno);
   }

   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR) { //Ecoute les connexions sur con cock délcaré : Queue max = 100
      perror("listen()");
      exit(errno);
   }

   return sock;
}


static void end_connection(int sock){
   closesocket(sock);
}


static int read_client(SOCKET sock, char *buffer){
   int n = 0;

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = '\0';

   return n;
}


static void write_client(SOCKET sock, const char *buffer){
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}


void requete_type(Client client, const char* buffer, char buffercopy[], char* param[]){
   strncpy(buffercopy, buffer, BUF_SIZE); // Copie du buffer pour éviter de modifier l'original

   int cptElem = 0;
   char *ptrElem = strtok(buffercopy, " "); //séparation des éléments du buffer lorsqu'on a " "

   if (strncmp(buffercopy, "AJOUT", 5) == 0) {
      //requête d'ajout d'une somme d'argent
      while ((ptrElem) != NULL && cptElem < 5) {
         param[cptElem] = ptrElem;
         printf("param[%d] : %s\n",cptElem,param[cptElem]);
         cptElem ++;
         ptrElem = strtok(NULL, " "); //demande du prochain param du buffer
      }

      if (cptElem != 5) {
         printf("Usage: AJOUT <id_client> <id_compte> <password> <somme>\n");
      }

   } else if (strncmp(buffercopy, "RETRAIT", 7) == 0){
      //requête d'ajout de retrait d'argent
      while ((ptrElem) != NULL && cptElem < 5) {
         param[cptElem] = ptrElem;
         cptElem ++;
         ptrElem = strtok(NULL, " "); //demande du prochain param du buffer
      }

      if (cptElem != 5) {
         printf("Usage: RETRAIT <id_client> <id_compte> <password> <somme>\n");
      }
   
   }else if (strncmp(buffercopy, "SOLDE", 5) == 0){
      //requête de consultation de solde
      while ((ptrElem) != NULL && cptElem < 4) {
         param[cptElem] = ptrElem;
         cptElem ++;
         ptrElem = strtok(NULL, " "); //demande du prochain param du buffer
      }
      param[5] = "0"; //La cinquième case du tableau inutile -> mise automatiquement à 0
      if (cptElem != 5) {
         printf("Usage: SOLDE <id_client> <id_compte> <password> \n");
      }
   }
   else if (strncmp(buffercopy, "OPERATIONS", 10) == 0){
      while ((ptrElem) != NULL && cptElem < 4) {
         printf("param[%d] : %s\n",cptElem,param[cptElem]);
         param[cptElem] = ptrElem;
         cptElem ++;
         ptrElem = strtok(NULL, " "); //demande du prochain param du buffer
      }
      param[5] = "0"; //La cinquième case du tableau inutile -> mise automatiquement à 0
      if (cptElem != 5) {
         printf("Usage: OPERATIONS <id_client> <id_compte> <password> \n");
      }
   }
   else{
      write_client(client.sock,"Erreur dans l'opération indiquée");
   }
   // return param;
}


Client nouveau_client(int nbCompte, SOCKET csock, const char* buffer, char buffercopy[], int actual){
   //création objet client avec socket et pseudo pour discuter avec lui
   // char buffercopy[BUF_SIZE]; // Copie locale du buffer
   strncpy(buffercopy, buffer, BUF_SIZE-1); // Copie du buffer pour éviter de modifier l'original
   buffercopy[BUF_SIZE - 1] = '\0'; // Assure la terminaison du buffer
   printf("buffercopy: %s\n", buffercopy);
   
   Client c = { csock };
   char* name = strtok(buffercopy, " "); // Récupère le nom
   char* password = strtok(NULL, " "); // Récupère le mot de passe

   if (name && password) {
      #ifdef DEBBUG
         printf("name et password : %s %s\n", name, password);
      #endif 
      strncpy(c.name, name, BUF_SIZE - 1);    // Copie du nom
      strncpy(c.password, password, BUF_SIZE - 1); // Copie du mot de passe
   }else{
      printf("nom ou mdp vides\n");
   }

   // Initialisation des comptes
   c.comptes = malloc(sizeof(Compte)); // Alloue un tableau de comptes
   if (c.comptes == NULL) {
      perror("Erreur d'allocation mémoire pour les comptes");
      closesocket(csock);
      exit(EXIT_FAILURE);
   }
   //Création d'un compte par client pour le moment
   c.comptes->idCompte = nbCompte; //premier compte initialisé à l'ordre dans lequel le client s'est co
   c.comptes->montant = 0;
   for (int i = 0; i < 5; i++) {
      c.comptes->operations[i] = NULL; //initialisation de toutes les cases de operation du compte à NULL
   }

   printf("Nouveau client dans notre banque : %s, id_client : %d, id_compte : %d, somme : %d\n", c.name, actual, c.comptes->idCompte, c.comptes->montant);
   //Réponse serveur nouveau client
   return c;
}


int main(int argc, char **argv){
   app();
   return EXIT_SUCCESS;
}
