#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "GrilleSDL.h"
#include "Ressources.h"

// Dimensions de la grille de jeu
#define NB_LIGNES   12
#define NB_COLONNES 19

// Nombre de cases maximum par piece
#define NB_CASES    4

// Macros utilisees dans le tableau tab
#define VIDE        0
#define BRIQUE      1
#define DIAMANT     2

int tab[NB_LIGNES][NB_COLONNES]
={ {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
   {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}};

typedef struct
{
  int ligne;
  int colonne;
} CASE;

typedef struct
{
  CASE cases[NB_CASES];
  int  nbCases;
  int  couleur;
} PIECE;

PIECE pieces[12] = { 0,0,0,1,1,0,1,1,4,0,       // carre 4
                     0,0,1,0,2,0,2,1,4,0,       // L 4
                     0,1,1,1,2,0,2,1,4,0,       // J 4
                     0,0,0,1,1,1,1,2,4,0,       // Z 4
                     0,1,0,2,1,0,1,1,4,0,       // S 4
                     0,0,0,1,0,2,1,1,4,0,       // T 4
                     0,0,0,1,0,2,0,3,4,0,       // I 4
                     0,0,0,1,0,2,0,0,3,0,       // I 3
                     0,1,1,0,1,1,0,0,3,0,       // J 3
                     0,0,1,0,1,1,0,0,3,0,       // L 3
                     0,0,0,1,0,0,0,0,2,0,       // I 2
                     0,0,0,0,0,0,0,0,1,0 };     // carre 1

void DessinePiece(PIECE piece);
int  CompareCases(CASE case1,CASE case2);
void TriCases(CASE *vecteur,int indiceDebut,int indiceFin);
void* threadDefileMessage(void* p);
void* threadPiece(void* p);
void HandlerSIGINT(int sig);
void HandlerSIGALRM(int sig);
void setMessage(const char* texte, bool signalOn);
void RotationPiece(PIECE* pPiece);

pthread_mutex_t mutexMessage = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutexCasesInserees;
pthread_cond_t condCasesInserees;
char* message;
int tailleMessage;
int indiceCourant = 0;
PIECE pieceEnCours;

CASE casesInserees[NB_CASES];  // cases insérées par le joueur 
int  nbCasesInserees;  // nombre de cases actuellement insérées par le joueur.

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char* argv[])
{
  pthread_t thMsg, thPiece;
  EVENT_GRILLE_SDL event;
 
  srand((unsigned)time(NULL));

  // Ouverture de la fenetre graphique
  printf("(MAIN %p) Ouverture de la fenetre graphique\n",pthread_self()); fflush(stdout);
  if (OuvertureFenetreGraphique() < 0)
  {
    printf("Erreur de OuvrirGrilleSDL\n");
    fflush(stdout);
    exit(1);
  }

  struct sigaction A;

  A.sa_handler = HandlerSIGINT;
  sigemptyset(&A.sa_mask);
  A.sa_flags = 0;

  if ((sigaction(SIGINT, &A, NULL)) == -1)
  {
    perror("Erreur lors de l'armement de SIGINT\n");
    exit(1);
  }
  printf("Le signal SIGINT à bien été armé \n");

  struct sigaction B;
  B.sa_handler = HandlerSIGALRM;
  sigemptyset(&B.sa_mask);
  B.sa_flags = 0;

  if ((sigaction(SIGALRM, &B, NULL)) == -1)
  {
    perror("Erreur lors de l'armement de SIGALRM\n");
    exit(1);
  }
  printf("Le signal SIGALRM à bien été armé \n");

  pthread_mutex_init(&mutexCasesInserees,NULL);
  pthread_cond_init(&condCasesInserees,NULL);

  setMessage("Bienvenue dans Blockudoku ", true);

  pthread_create(&thMsg, NULL, threadDefileMessage, NULL);
  pthread_create(&thPiece, NULL, threadPiece, NULL);



  // Exemples d'utilisation du module Ressources --> a supprimer
  DessineChiffre(1,15,7);
  //char buffer[40];
  //sprintf(buffer,"coucou");
  //for (int i=0 ; i<strlen(buffer) ; i++) DessineLettre(10,2+i,buffer[i]);
  DessineBrique(7,3,false);
  DessineBrique(7,5,true);


  printf("(MAIN %p) Attente du clic sur la croix\n",pthread_self());  
  bool ok = false;
  while(!ok)
  {
    event = ReadEvent();
    if (event.type == CROIX) ok = true;
    if (event.type == CLIC_GAUCHE)
    {
      DessineDiamant(event.ligne,event.colonne,ROUGE);
      tab[event.ligne][event.colonne] = DIAMANT;
    }
  }
  // Fermeture de la fenetre
  printf("(MAIN %p) Fermeture de la fenetre graphique...",pthread_self()); fflush(stdout);
  FermetureFenetreGraphique();
  printf("OK\n");


  exit(0);
}

void* threadDefileMessage(void* p)
{
  // Masquer tout les signaux sauf SIGALRM
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask, SIGALRM);
  pthread_sigmask(SIG_SETMASK, &mask, NULL);

  while (1)
  {
    pthread_mutex_lock(&mutexMessage);

    if (tailleMessage > 0)
    {
      for (int i = 0; i<17;i++)
      {
        int index = (indiceCourant + i) % tailleMessage;
        DessineLettre(10,i+1,message[index]);
      }

      indiceCourant++;

      if (indiceCourant >= tailleMessage) indiceCourant = 0;
    }

    pthread_mutex_unlock(&mutexMessage);

    usleep(400000);
  }
  return NULL;
}

void* threadPiece(void* p)
{
  while (1)
  {
    // 1
    int index = rand() % 12;
    pieceEnCours = pieces[index];

    // 2
    int c = rand() % 4;

    switch(c)
    {
      case 0: pieceEnCours.couleur = JAUNE; break;
      case 1: pieceEnCours.couleur = ROUGE; break;
      case 2: pieceEnCours.couleur = VERT; break;
      case 3: pieceEnCours.couleur = VIOLET; break;
    }

    int rot = rand() % 4;
    for (int i = 0; i < rot; i++)
      RotationPiece(&pieceEnCours);

    DessinePiece(pieceEnCours);

    // 3

    pthread_mutex_lock(&mutexCasesInserees);
    while(nbCasesInserees < pieceEnCours.nbCases)
      pthread_cond_wait(&condCasesInserees, &mutexCasesInserees);
    pthread_mutex_unlock(&mutexCasesInserees);

    // 4, étape 3 à faire en premier

  }


  return NULL;
}

void RotationPiece(PIECE* pPiece)
{
  int Lmin, Cmin;
  int L, C;

  // Rotation
  for (int i = 0; i < pPiece->nbCases; i++)
  {
    L = pPiece->cases[i].ligne;
    C = pPiece->cases[i].colonne;

    pPiece->cases[i].ligne = -C;
    pPiece->cases[i].colonne = L;
  }

  // Minimum
  Lmin = pPiece->cases[0].ligne;
  Cmin = pPiece->cases[0].colonne;

  for (int i = 1; i < pPiece->nbCases; i++)
  {
    if (pPiece->cases[i].ligne < Lmin)
      Lmin = pPiece->cases[i].ligne;

    if (pPiece->cases[i].colonne < Cmin)
      Cmin = pPiece->cases[i].colonne;  
  }

  // Translation
  for (int i = 0; i < pPiece->nbCases; i++)
  {
    pPiece->cases[i].ligne -= Lmin;
    pPiece->cases[i].colonne -= Cmin;
  }

  // Tri
  TriCases(pPiece->cases, 0, pPiece->nbCases - 1);
}

void setMessage(const char *texte, bool signalOn)
{
  pthread_mutex_lock(&mutexMessage);

  if (message != NULL)
    free(message);

  message = (char*) malloc(strlen(texte) + 1);

  strcpy(message, texte);

  tailleMessage = strlen(message);

  if (signalOn)
    alarm(10);

  pthread_mutex_unlock(&mutexMessage);
}

void HandlerSIGINT(int sig)
{
  printf("CTRL+C Activé, le programme se ferme");
  FermetureFenetreGraphique();
  exit(0);
}

void HandlerSIGALRM(int sig)
{
  setMessage("Jeu en cours ", false);
}


///////////////////////////////////////////////////////////////////////////////////////////////////
/////// Fonctions fournies ////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
void DessinePiece(PIECE piece)
{
  int Lmin,Lmax,Cmin,Cmax;
  int largeur,hauteur,Lref,Cref;

  Lmin = piece.cases[0].ligne;
  Lmax = piece.cases[0].ligne;
  Cmin = piece.cases[0].colonne;
  Cmax = piece.cases[0].colonne;

  for (int i=1 ; i<=(piece.nbCases-1) ; i++)
  {
    if (piece.cases[i].ligne > Lmax) Lmax = piece.cases[i].ligne;
    if (piece.cases[i].ligne < Lmin) Lmin = piece.cases[i].ligne;
    if (piece.cases[i].colonne > Cmax) Cmax = piece.cases[i].colonne;
    if (piece.cases[i].colonne < Cmin) Cmin = piece.cases[i].colonne;
  }

  largeur = Cmax - Cmin + 1;
  hauteur = Lmax - Lmin + 1;

  switch(largeur)
  {
    case 1 : Cref = 15; break;
    case 2 : Cref = 15; break;
    case 3 : Cref = 14; break;
    case 4 : Cref = 14; break;  
  }

  switch(hauteur)
  {
    case 1 : Lref = 4; break;
    case 2 : Lref = 4; break;
    case 3 : Lref = 3; break;
    case 4 : Lref = 3; break;
  }

  for (int L=3 ; L<=6 ; L++) for (int C=14 ; C<=17 ; C++) EffaceCarre(L,C);
  for (int i=0 ; i<piece.nbCases ; i++) DessineDiamant(Lref + piece.cases[i].ligne,Cref + piece.cases[i].colonne,piece.couleur);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int CompareCases(CASE case1,CASE case2)
{
  if (case1.ligne < case2.ligne) return -1;
  if (case1.ligne > case2.ligne) return +1;
  if (case1.colonne < case2.colonne) return -1;
  if (case1.colonne > case2.colonne) return +1;
  return 0;
}

void TriCases(CASE *vecteur,int indiceDebut,int indiceFin)
{ // trie les cases de vecteur entre les indices indiceDebut et indiceFin compris
  // selon le critere impose par la fonction CompareCases()
  // Exemple : pour trier un vecteur v de 4 cases, il faut appeler TriCases(v,0,3); 
  int  i,iMin;
  CASE tmp;

  if (indiceDebut >= indiceFin) return;

  // Recherche du minimum
  iMin = indiceDebut;
  for (i=indiceDebut ; i<=indiceFin ; i++)
    if (CompareCases(vecteur[i],vecteur[iMin]) < 0) iMin = i;

  // On place le minimum a l'indiceDebut par permutation
  tmp = vecteur[indiceDebut];
  vecteur[indiceDebut] = vecteur[iMin];
  vecteur[iMin] = tmp;

  // Tri du reste du vecteur par recursivite
  TriCases(vecteur,indiceDebut+1,indiceFin); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////