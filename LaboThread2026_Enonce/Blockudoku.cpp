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
void* threadEvent(void* p);
void* threadScore(void* p);
void* threadCases(void* p);
void* threadNettoyeur(void* p);
void LiberationCase(void* p);
void HandlerSIGINT(int sig);
void HandlerSIGUSR1(int sig);
void HandlerSIGALRM(int sig);
void setMessage(const char* texte, bool signalOn);
void RotationPiece(PIECE* pPiece);
int VerifierPiece();

//////////////////////////////////////////////////////////////////
pthread_mutex_t mutexMessage = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t mutexCasesInserees;
pthread_cond_t condCasesInserees;

pthread_mutex_t mutexScore;
pthread_cond_t condScore;

pthread_key_t cle;
pthread_mutex_t mutexAnalyse;
pthread_cond_t condAnalyse;

//////////////////////////////////////////////////////////////////

char* message;
int tailleMessage;
int indiceCourant = 0;
PIECE pieceEnCours;

CASE casesInserees[NB_CASES];  // cases insérées par le joueur 
int  nbCasesInserees = 0;  // nombre de cases actuellement insérées par le joueur.
int score = 0;
bool MAJScore = false;
bool MAJCombos = false;

//////////////////////////////////////////////////////////////////

pthread_t tabThreadCase[9][9];

int lignesCompletes[9];
int nbLignesCompletes = 0;    
int colonnesCompletes[9];    
int nbColonnesCompletes = 0;  
int carresComplets[9];    
int nbCarresComplets = 0;  
int nbAnalyses = 0;


int combos = 0;
int c = 0;

///////////////////////////////////////////////////////////////////////////////////////////////////
int main(int argc,char* argv[])
{
  pthread_t thMsg, thPiece, thEvent, thScore, thNettoyeur;
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

  

  pthread_key_create(&cle, LiberationCase);

  pthread_mutex_init(&mutexCasesInserees,NULL);
  pthread_cond_init(&condCasesInserees,NULL);

  setMessage("Bienvenue dans Blockudoku ", true);

  pthread_create(&thMsg, NULL, threadDefileMessage, NULL);
  pthread_create(&thPiece, NULL, threadPiece, NULL);
  pthread_create(&thEvent, NULL, threadEvent, NULL);
  pthread_create(&thScore, NULL, threadScore, NULL);

  for (int L = 0; L < 9; L++)
  {
    for (int C = 0; C < 9; C++)
    {
      CASE* nouvelleCase = (CASE*) malloc(sizeof(CASE));

      nouvelleCase->ligne = L;
      nouvelleCase->colonne = C;

      pthread_create(&tabThreadCase[L][C], NULL, threadCases, nouvelleCase);
    }
  }

  pthread_create(&thNettoyeur, NULL, threadNettoyeur, NULL);

  // Exemples d'utilisation du module Ressources --> a supprimer
  //DessineChiffre(1,15,7);
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
  }
  // Fermeture de la fenetre
  printf("(MAIN %p) Fermeture de la fenetre graphique...",pthread_self()); fflush(stdout);
  printf("OK\n");


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

    int pieceValidee = 0;

    while(!pieceValidee)
    {
      pthread_mutex_lock(&mutexCasesInserees);

      while(nbCasesInserees < pieceEnCours.nbCases)
        pthread_cond_wait(&condCasesInserees, &mutexCasesInserees);

      // 4
      if (VerifierPiece())
      {
        for (int i = 0; i < nbCasesInserees; i++)
        {
          int L = casesInserees[i].ligne;
          int C = casesInserees[i].colonne;

          tab[L][C] = BRIQUE;
          DessineBrique(L, C, false);

          pthread_kill(tabThreadCase[L][C], SIGUSR1);
        }

        for (int i = 0; i < nbCasesInserees; i++)
        {
          score++;
        }
        MAJScore = true;
        pthread_cond_signal(&condScore);

        nbCasesInserees = 0;
        pieceValidee = 1;
      }
      else
      {
        for (int i = 0; i < nbCasesInserees; i++)
        {
          int L = casesInserees[i].ligne;
          int C = casesInserees[i].colonne;

          tab[L][C] = VIDE;
          EffaceCarre(L,C);
        }

        nbCasesInserees = 0;
      }

      pthread_mutex_unlock(&mutexCasesInserees);
    }

  }

  return NULL;
}

void* threadEvent(void* p)
{
  EVENT_GRILLE_SDL event;
  while(1)
  {
    event = ReadEvent();

    if (event.type == CLIC_GAUCHE)
    {
      int L = event.ligne;
      int C = event.colonne;
  
      if (tab[L][C] == VIDE)
      {
        DessineDiamant(L,C,ROUGE);
        tab[L][C] = DIAMANT;

        pthread_mutex_lock(&mutexCasesInserees);

        casesInserees[nbCasesInserees].ligne = L;
        casesInserees[nbCasesInserees].colonne = C;
        nbCasesInserees++;

        if (nbCasesInserees == pieceEnCours.nbCases)
        {
          pthread_cond_signal(&condCasesInserees);
        }

        pthread_mutex_unlock(&mutexCasesInserees);
      }      
    }

    if (event.type == CLIC_DROIT)
    {
      pthread_mutex_lock(&mutexCasesInserees);

      for (int i = 0; i < nbCasesInserees; i++)
      {
        int L = casesInserees[i].ligne;
        int C = casesInserees[i].colonne;

        EffaceCarre(L,C);
        tab[L][C] = VIDE;
      }

      nbCasesInserees = 0;

      pthread_mutex_unlock(&mutexCasesInserees);
    }

    if (event.type == CROIX)
    {
      for (int L = 0; L < 9; L++)
      {
        for (int C = 0; C < 9; C++)
        {
          pthread_join(tabThreadCase[L][C], NULL);
        }
      }


      FermetureFenetreGraphique();
      exit(0);
    }
  }

  return NULL;
}

void* threadScore(void* p)
{
  while (1)
  {
    pthread_mutex_lock(&mutexScore);

    while(!MAJScore && !MAJCombos)
      pthread_cond_wait(&condScore, &mutexScore);
    

    int s = score;

    int d1 = s / 1000;
    int d2 = (s / 100) % 10;
    int d3 = (s / 10) % 10;
    int d4 = s % 10;

    DessineChiffre(1,14,d1);
    DessineChiffre(1,15,d2);
    DessineChiffre(1,16,d3);
    DessineChiffre(1,17,d4);

    int combosLocal = combos;
    combos = 0;

    c += combosLocal;

    int c1 = c / 1000;
    int c2 = (c / 100) % 10;
    int c3 = (c / 10) % 10;
    int c4 = c % 10;

    DessineChiffre(8,14,c1);
    DessineChiffre(8,15,c2);
    DessineChiffre(8,16,c3);
    DessineChiffre(8,17,c4);   

    MAJScore = false;
    MAJCombos = false;

    pthread_mutex_unlock(&mutexScore);
  }

  return NULL;
}

void* threadCases(void* p)
{
  struct sigaction C;
  C.sa_handler = HandlerSIGUSR1;
  sigemptyset(&C.sa_mask);
  C.sa_flags = 0;

  if ((sigaction(SIGUSR1, &C, NULL)) == -1)
  {
    perror("Erreur lors de l'armement de SIGUSR1\n");
    exit(1);
  }

  CASE* nouvelleCase = (CASE*) p;


  pthread_setspecific(cle, nouvelleCase);

  while (1)
  {
    pause();
  }

  return NULL;
}

void* threadNettoyeur(void* p)
{
  while(1)
  {  
    pthread_mutex_lock(&mutexAnalyse);
  
    int nbcase = pieceEnCours.nbCases;

    while(nbAnalyses < nbcase)
      pthread_cond_wait(&condAnalyse, &mutexAnalyse);
      
  
    if (nbCarresComplets == 0 && nbLignesCompletes == 0 && nbColonnesCompletes == 0)
    {
      nbAnalyses = 0;
      pthread_mutex_unlock(&mutexAnalyse);
      continue;
    }

    struct timespec req = {2, 0}, rem;
    while (nanosleep(&req, &rem) == -1)
      req = rem;

    for (int i = 0; i < nbLignesCompletes; i++)
    {
      int L = lignesCompletes[i];
      for (int C = 0; C < 9; C++)
      {
        tab[L][C] = VIDE;
        EffaceCarre(L, C);
      }
    }

    for (int i = 0; i < nbColonnesCompletes; i++)
    {
      int C = colonnesCompletes[i];
      for (int L = 0; L < 9; L++)
      {
        tab[L][C] = VIDE;
        EffaceCarre(L, C);
      }
    }

    for (int i = 0; i < nbCarresComplets; i++)
    {
      int num = carresComplets[i];
      int Lstart = (num / 3) * 3;
      int Cstart = (num % 3) * 3;

      for (int L = Lstart; L < Lstart + 3; L++)
      {
        for (int C = Cstart; C < Cstart + 3; C++)
        {
          tab[L][C] = VIDE;
          EffaceCarre(L, C);
        }
      }
    }
  
    combos = nbLignesCompletes + nbColonnesCompletes + nbCarresComplets;

    nbLignesCompletes = 0;
    nbColonnesCompletes = 0;
    nbCarresComplets = 0;
    nbAnalyses = 0;

    pthread_mutex_unlock(&mutexAnalyse);
    
    pthread_mutex_lock(&mutexScore);

    int comboscycle = combos;
    if (comboscycle > 0)
    {
      if (comboscycle == 1)
      {
        setMessage("Simple Combo ", true);
        score += 10;
      }
      else if (comboscycle == 2)
      {
        setMessage("Double Combo ", true);
        score += 25;
      }
      else if (comboscycle == 3)
      {
        setMessage("Triple Combo ", true);
        score += 40;
      }
      else
      {
        setMessage("Quadruple Combo ", true);
        score += 55;
      }

      combos = comboscycle;
      MAJCombos = true;
      pthread_cond_signal(&condScore);
      pthread_mutex_unlock(&mutexScore);
    }
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
  indiceCourant = 0;

  if (signalOn)
  {
    alarm(0);
    alarm(10);
  }

  pthread_mutex_unlock(&mutexMessage);
}

int VerifierPiece()
{
  if (nbCasesInserees != pieceEnCours.nbCases)
    return 0;

  CASE temp[NB_CASES];

  for (int i = 0; i < nbCasesInserees; i++)
    temp[i] = casesInserees[i];

  TriCases(temp, 0, nbCasesInserees - 1);

  int Lmin = temp[0].ligne;
  int Cmin = temp[0].colonne;

  for (int i = 1; i < nbCasesInserees; i++)
  {
    if (temp[i].ligne < Lmin)
      Lmin = temp[i].ligne;

    if (temp[i].colonne < Cmin)
      Cmin = temp[i].colonne;
  }

  for (int i = 0; i < nbCasesInserees; i++)
  {
    temp[i].ligne -= Lmin;
    temp[i].colonne -= Cmin;
  }

  for (int i = 0; i < nbCasesInserees; i++)
  {
    if (temp[i].ligne != pieceEnCours.cases[i].ligne ||
        temp[i].colonne != pieceEnCours.cases[i].colonne)
    {
      return 0;
    }
  }

  return 1;
}

void LiberationCase(void* p)
{
  printf("Libération de la case\n");
  free(p);
}

void HandlerSIGINT(int sig)
{
  printf("CTRL+C Activé, le programme se ferme\n");
  FermetureFenetreGraphique();
  exit(0);
}

void HandlerSIGALRM(int sig)
{
  setMessage("Jeu en cours ", false);
}

void HandlerSIGUSR1(int sig)
{
  CASE *Case = (CASE*) pthread_getspecific(cle);
  int comp = 0;
  bool present = false;

  pthread_mutex_lock(&mutexAnalyse);
  for (int i = 0; i < nbLignesCompletes; i++)
  {
    if (lignesCompletes[i] == Case->ligne)
    {
      present = true;
    }
  }
  if (!present)
  {
    for (int C = 0; C < 9; C++)
    {
      if (tab[Case->ligne][C] == BRIQUE)
        comp++;
    }

    if (comp == 9)
    {
      lignesCompletes[nbLignesCompletes] = Case->ligne;
      nbLignesCompletes++;

      for (int C = 0; C < 9; C++)
        DessineBrique(Case->ligne, C, true);
    }
  }
  present = false;
  comp = 0;

  for (int i = 0; i < nbColonnesCompletes; i++)
  {
    if (colonnesCompletes[i] == Case->colonne)
    {
      present = true;
    }
  }

  if (!present)
  {
    for (int L = 0; L < 9; L++)
    {
      if (tab[L][Case->colonne] == BRIQUE)
        comp++;
    }
    if (comp == 9)
    {
      colonnesCompletes[nbColonnesCompletes] = Case->colonne;
      nbColonnesCompletes++;

      for (int L = 0; L < 9; L++)
        DessineBrique(L, Case->colonne, true);
    }
  }

  present = false;
  comp = 0;

  int numCarre = (Case->ligne / 3) * 3 + (Case->colonne / 3);

  for (int i = 0; i < nbCarresComplets;i++)
  {
    if (carresComplets[i] == numCarre)
      present = true;
  }
  if (!present)
  {
    int L = (numCarre / 3) * 3;
    int C = (numCarre % 3) * 3;
    for (int i = L; i < L + 3; i++)
    {
      for (int j = C; j < C + 3; j++)
      {
        if (tab[i][j] == BRIQUE)
          comp++;
      }
    }
    if (comp == 9)
    {
      carresComplets[nbCarresComplets] = numCarre;
      nbCarresComplets++;

      for (int i = L; i < L + 3; i++)
      {
        for (int j = C; j < C + 3; j++)
        {
          DessineBrique(i, j, true);
        }
      }
    }
  }

  nbAnalyses++;

  pthread_cond_signal(&condAnalyse);
  pthread_mutex_unlock(&mutexAnalyse);


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