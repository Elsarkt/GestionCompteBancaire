//client.c
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include "client.h"


static void app(const char *address, const char *name)
{
   SOCKET sock = init_connection(address); //adress (du socket déclaré) : localhost
   char buffer[BUF_SIZE];

   fd_set rdfs; //rdfs est un Ensemble de descripteurs 

   /* send our name */
   write_server(sock, name);

   while(1)
   {
      FD_ZERO(&rdfs);

      /* add STDIN_FILENO -> l'entrée du clavier récupérée dans le terminal */
      FD_SET(STDIN_FILENO, &rdfs);

      /* add the socket */
      FD_SET(sock, &rdfs); 

      //select attend un changement d'état des descripteurs contenus dans différents ensembles
      if(select(sock + 1, &rdfs, NULL, NULL, NULL) == -1) //sock+1 appartient à un ens de desc. SURVEILLé en LECTURE (rdfs 2ème param)
      {
         perror("select()");
         exit(errno);
      }

      /* something from standard input : i.e keyboard */
      if(FD_ISSET(STDIN_FILENO, &rdfs)) //Le select en lecture a fait changé d'état le descript sock dans l'ens de desc rdfs
      {
         //Si on a une entrée au clavier sur le termninal client
         fgets(buffer, BUF_SIZE - 1, stdin); //place l'entrée dans le tab de char buffer avec place reservée pour '\0'
         {
            char *p = NULL;
            p = strstr(buffer, "\n");
            if(p != NULL)
            {
               *p = 0;
            }
            else
            {
               /* fclean */
               buffer[BUF_SIZE - 1] = 0;
            }
         }
         write_server(sock, buffer); 
      }
      //si on a un changement de lecture sur le socket -> lecture d'une réponse serveur
      else if(FD_ISSET(sock, &rdfs))
      {
         int n = read_server(sock, buffer);
         /* server down */
         if(n == 0)
         {
            printf("Server disconnected !\n");
            break;
         }
         puts(buffer); //on l'inscrit sur le tableau de char buffer du client
         // printf("Lecture d'une réponse : puts(buffer) : %s\n",buffer);
      }
   }

   end_connection(sock);
}

static int init_connection(const char *address)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };
   struct hostent *hostinfo; //info de l'hôte auquel on veut se connecter -> le serveur

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   hostinfo = gethostbyname(address); /* on récupère les informations de l'hôte */
   if (hostinfo == NULL) 
   {
      fprintf (stderr, "Unknown host %s.\n", address);
      exit(EXIT_FAILURE);
   }

   //
   sin.sin_addr = *(IN_ADDR *) hostinfo->h_addr; /* l'adresse se trouve dans le champ h_addr de la structure hostinfo */
   sin.sin_port = htons(PORT); /* on utilise htons pour lla conversion du port en byte  */
   sin.sin_family = AF_INET;

   if(connect(sock,(SOCKADDR *) &sin, sizeof(SOCKADDR)) == SOCKET_ERROR) //Connexion du socket client à l'adresse du socket serveur
   {
      perror("connect()");
      exit(errno);
   }

   return sock;
}

static void end_connection(int sock)
{
   closesocket(sock);
}

static int read_server(SOCKET sock, char *buffer)
{
   int n = 0; //nombre d'octets reçus

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0) //recv stock les données pointé à l'adresse buffer
   {
      perror("recv()");
      exit(errno);
   }

   buffer[n] = '\0'; //Dernier caractère systématique après le dernier octet reçu

   return n;
}

static void write_server(SOCKET sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}

int main(int argc, char **argv)
{
   if(argc < 2)
   {
      printf("Usage : %s [address] [pseudo]\n", argv[0]);
      return EXIT_FAILURE;
   }


   app(argv[1], argv[2]); //argv[0]:nom du prog, argv[1]:adresse serveur, argv[2]:nom du client


   return EXIT_SUCCESS;
}