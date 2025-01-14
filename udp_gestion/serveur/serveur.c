#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "serveur.h"
#include "client.h"

// #define DEBBUG

static void app(void){
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
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the connection socket */
      FD_SET(sock, &rdfs);

      if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
      {
      perror("select()");
      exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs))
      {
      /* stop process when type on keyboard */
      break;
      }
      else if(FD_ISSET(sock, &rdfs))
      {
         /* new client */
         SOCKADDR_IN csin = { 0 }; //le client donne l'adresse du sock sur lequel ils vont comm

         /* a client is talking */
         read_client(sock, &csin, buffer);
         
         if(check_if_client_exists(clients, &csin, actual) == 0)
         {
         //si le client communique via une adresse et un port déjà connu
            if(actual != MAX_CLIENTS)
            {
               // Client c = { csin };
               //elsa
               char buffercopy[BUF_SIZE];
               Client c = nouveau_client(nbCompte, sock, &csin, buffer, buffercopy, actual);
               // strncpy(c.name, buffer, BUF_SIZE - 1);
               clients[actual] = c;
               nbCompte++;
               actual++;
            }
         }
         else
         {
               Client *client = get_client(clients, &csin, actual);
               if(client == NULL) continue;
               //elsa
               char buffercopy[BUF_SIZE];
               char* tabRequete[5];
               requete_type(client, buffer, buffercopy, tabRequete, sock);
               #ifdef DEBBUG
                     for (int j = 0; j < 5; j++) {
                        if (tabRequete[j] != NULL) printf("Dans debug %s \n", tabRequete[j]);
                     }
               #endif
               reponseServeur(buffer, client, tabRequete, nbCompte, sock);
         }
      }
   }

   end_connection(sock);
}

static int check_if_client_exists(Client *clients, SOCKADDR_IN *csin, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      if(clients[i].sin.sin_addr.s_addr == csin->sin_addr.s_addr
        && clients[i].sin.sin_port == csin->sin_port)
      {
     return 1;
      }
   }

   return 0;
}

static Client* get_client(Client *clients, SOCKADDR_IN *csin, int actual)
{
   int i = 0;
   for(i = 0; i < actual; i++)
   {
      if(clients[i].sin.sin_addr.s_addr == csin->sin_addr.s_addr
        && clients[i].sin.sin_port == csin->sin_port)
      {
     return &clients[i];
      }
   }

   return NULL;
}


static int init_connection(void)
{
   /* UDP so SOCK_DGRAM */
   SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_client(SOCKET sock, SOCKADDR_IN *sin, char *buffer)
{
   int n = 0;
   socklen_t sinsize = sizeof *sin;

   if((n = recvfrom(sock, buffer, BUF_SIZE - 1, 0, (SOCKADDR *) sin, &sinsize)) < 0)
   {
      perror("recvfrom()");
      exit(errno);
   }

   buffer[n] = 0;

   return n;
}

static void write_client(SOCKET sock, SOCKADDR_IN *sin, const char *buffer)
{
   if(sendto(sock, buffer, strlen(buffer), 0, (SOCKADDR *) sin, sizeof *sin) < 0)
   {
      perror("send()");
      exit(errno);
   }
}


void requete_type(Client *client, const char* buffer, char buffercopy[], char* param[], SOCKET sock){
   strncpy(buffercopy, buffer, BUF_SIZE); // Copie du buffer pour éviter de modifier l'original

   int cptElem = 0;
   char *ptrElem = strtok(buffercopy, " "); //séparation des éléments du buffer lorsqu'on a " "

   if (strncmp(buffercopy, "AJOUT", 5) == 0) {
      //requête d'ajout d'une somme d'argent
      while ((ptrElem) != NULL && cptElem < 5) {
         param[cptElem] = ptrElem;
         #ifdef DEBUGG
            printf("param[%d] : %s\n",cptElem,param[cptElem]);
         #endif
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
      cptElem ++;
      if (cptElem != 5) {
         printf("Usage: SOLDE <id_client> <id_compte> <password> \n");
      }
   }
   else if (strncmp(buffercopy, "OPERATIONS", 10) == 0){
      while ((ptrElem) != NULL && cptElem < 4) {
         #ifdef DEBUGG
            printf("param[%d] : %s\n",cptElem,param[cptElem]);
         #endif
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
      write_client(sock, &client->sin, "Erreur dans l'opération indiquée");
   }
   // return param;
}


static void reponseServeur(const char *buffer, Client *c, char* tabRequete[], int nbCompte, SOCKET sock) {
   // int i = 0;
   char message[BUF_SIZE];
   message[0] = 0;
   int compte_trouve = -1;

   int id_compte = atoi(tabRequete[2]); //conversion char* à int
   if (id_compte < 0 || id_compte >= nbCompte) {
      snprintf(message, BUF_SIZE, "Erreur : compte %d non trouvé.\n", id_compte);
      write_client(sock, &c->sin, message);
   }

   if (tabRequete == NULL || tabRequete[0] == NULL || tabRequete[3] == NULL) {
      printf("Erreur : tabRequete invalide.\n");
   }

   if(strcmp(c->password, tabRequete[3]) == 0) {//le mdp correspond toujours au 3ème argument
          // Vérifier si l'id_compte correspond à un compte du client
      for (int i = 0; i<100; i++) { //Supposons qu'il y a moins que 500 comptes crées dans la base de données
         if (c->comptes[i].idCompte == id_compte) {
            compte_trouve = i;
            break;
         }
      }
      if (compte_trouve == -1) snprintf(message, BUF_SIZE, "Erreur : compte %d non trouvé.\n", id_compte);
      else{
         if (strcmp(tabRequete[0], "AJOUT") == 0){
            if(!tabRequete[4]){
               snprintf(message, BUF_SIZE, "somme manquante\n");
            }else{
               int somme = atoi(tabRequete[4]);
               c->comptes[compte_trouve].montant += somme; //On ajoute la somme voulue sur le compte identifié
               snprintf(message, BUF_SIZE, "AJOUT de %d€ sur votre compte\n", somme);
            }
         } 
         else if(strcmp(tabRequete[0], "OPERATIONS")==0){
            snprintf(message, BUF_SIZE, "Liste des 10 dernières opérations, de la plus ancienne à la plus récente:\n");
            write_client(sock, &c->sin, message);
            for(int h=0; h<10; h++){
               if (c->comptes[compte_trouve].operations[h] != NULL) {
                  snprintf(message, BUF_SIZE, "%s\n",c->comptes[compte_trouve].operations[h]);
                  write_client(sock, &c->sin, message);
               }
            }
            snprintf(message, BUF_SIZE, "\n");
         }
         else if(strcmp(tabRequete[0], "SOLDE")==0){
            snprintf(message, BUF_SIZE, "Votre SOLDE est de %d€\n", c->comptes[compte_trouve].montant );
         }
            else  if(strcmp(tabRequete[0], "RETRAIT")==0){
               if(!tabRequete[4]){
                  snprintf(message, BUF_SIZE, "somme manquant\n");
               }else{
                  int somme = atoi(tabRequete[4]);
                  c->comptes[compte_trouve].montant -= somme; //On ajoute la somme voulue sur le compte identifié
                  snprintf(message, BUF_SIZE, "RETRAIT de %d€ de votre compte\n", somme);
               }
         }
         else snprintf(message, BUF_SIZE, "Instruction inexistante");
      } 
   } else snprintf(message, BUF_SIZE, "Mode de passe érroné\n");

   //partie qui met à jour le tableau contenant la liste des 10 dernières opérations d'un client pour son compte compte_touve
   char* nouvelOp = malloc(BUF_SIZE);
   if(nouvelOp == NULL){
      perror("erreur allocation");
      exit(EXIT_FAILURE);
   }
   snprintf(nouvelOp, BUF_SIZE, "%s %s", tabRequete[0], tabRequete[4]);
   //ON décale les élément du tab operations vers la gauche
   if(c->comptes[compte_trouve].operations[9]!=NULL){
      free(c->comptes[compte_trouve].operations[0]);
   }
   for(int j=0; j<9; j++){
      c->comptes[compte_trouve].operations[j] = c->comptes[compte_trouve].operations[j+1];
   }
   //Le neuvième élément est l'opération dont l'appel est en train d'être réalisé
   c->comptes[compte_trouve].operations[9] = nouvelOp;

   //Ecrire le message de retour au client
   write_client(sock, &c->sin, message);
}


Client nouveau_client(int nbCompte, SOCKET csock, SOCKADDR_IN *sin, const char* buffer, char buffercopy[], int actual){
   //création objet client avec socket et pseudo pour discuter avec lui
   // char buffercopy[BUF_SIZE]; // Copie locale du buffer
   strncpy(buffercopy, buffer, BUF_SIZE-1); // Copie du buffer pour éviter de modifier l'original
   buffercopy[BUF_SIZE - 1] = '\0'; // Assure la terminaison du buffer
   #ifdef DEBUGG
      printf("buffercopy: %s\n", buffercopy);
   #endif
   
   Client c ;
   c.sin = *sin;
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
   for (int i = 0; i < 10; i++) {
      c.comptes->operations[i] = malloc(BUF_SIZE); //initialisation de toutes les cases de operation du compte à NULL
   }
   c.nbCompteclient = 1;
   printf("Nouveau client dans notre banque : %s, id_client : %d, id_compte : %d, somme : %d\n", c.name, actual, c.comptes->idCompte, c.comptes->montant);
   //Réponse serveur nouveau client
   return c;
}


int main(int argc, char **argv)
{

   app();


   return EXIT_SUCCESS;
}