# GestionCompteBancaire

Ce projet permet à un ou plusieurs clients d'intéragir avec leur banque par l'intermédiaire de deux protocoles différents : TCP et UDP, via un terminal.  
Le choix du protocole est à faire par l'utilisateur.

## Requierements
Projet fonctionnel sous Linux.

Développé sous Ubuntu 24.04.1 LTS.

## TCP

Cette partie nécessite de se placer le repertoire ./tcp_gestion.  


## UDP

Cette partie nécessite de se placer le repertoire ./udp_gestion.  


## Lancement du projet
Compiler le projet :
```shell script
make
```

Lancer le serveur sur un terminal :
```shell script
./serveurRUN
```
Lancer chaque client sur des terminaux différents :
```shell script
./clientRUN localhost <nomClient> <motDePasse>
```

Lorsqu'un client se connecte, le serveur indique le <id_client> <id_compte> qu'il lui associe.

Ci-dessous la liste des requêtes qu'un client peut effectuer auprès de la banque via son terminal : 

- Ajouter <somme> euros à son compte : 
```shell script
AJOUT <id_client> <id_compte> <motDePasse> <somme>

```

- retirer <somme> euros de son compte : 
```shell script
RETRAIT <id_client> <id_compte> <motDePasse> <somme>

```

- Consulter le solde de son compte : 
```shell script
SOLDE <id_client> <id_compte> <motDePasse> 

```

- Consulter la liste des 10 dernières opérations effectuées sur son compte : 
```shell script
OPERATIONS <id_client> <id_compte> <motDePasse> 

```
