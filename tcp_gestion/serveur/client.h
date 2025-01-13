//client.h côté serveur
#ifndef CLIENT_H
#define CLIENT_H

#include "serveur.h"

typedef struct
{
   int idCompte; 
   int montant; 
   char *operations[10]; //pointeur de tableaux de taille 5 : operations[0] contient les param de la première opération du compte
}Compte;

typedef struct
{
   SOCKET sock;
   char name[BUF_SIZE];
   Compte* comptes; //Tableau des comptes d'un client
   char password[BUF_SIZE];
   int nbCompteclient;
}Client;



#endif /* guard */