#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#define TRACE(message) printf("[THREAD %d:%p] %s",getpid(),pthread_self(),message);

typedef struct
{
    char nom[20];
    int nbSecondes;
} DONNEE;


DONNEE data[] = { "MATAGNE",15,
                  "WILVERS",10,
                  "WAGNER",17,
                  "QUETTIER",8,
                  "",0 };

void* threadsecond(void* p);
void HandlerSIGINT(int sig);
void LiberationCompteur(void* p);

DONNEE Param;
int compteur = 0;

pthread_mutex_t mutexCompteur;
pthread_cond_t condCompteur;
pthread_key_t cle;

int main() { 
    pthread_t thsec[4];
    int i, nbth = 0;

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

    sigset_t mask;
    sigfillset(&mask);
    pthread_sigmask(SIG_SETMASK, &mask, NULL);

    pthread_key_create(&cle, LiberationCompteur);

    pthread_mutex_init(&mutexCompteur, NULL);
    pthread_cond_init(&condCompteur, NULL);

    printf("\nCompteur: %d\n", compteur);

    for (i = 0; data[i].nom[0] != '\0'; i++)
    {
        pthread_mutex_lock(&mutexCompteur);

        memcpy(&Param, &data[i], sizeof(DONNEE));
        pthread_create(&thsec[nbth], NULL, threadsecond, &Param);
        

        nbth++;
        compteur++;
        printf("\nCompteur: %d\n", compteur);
    }
    

    pthread_mutex_lock(&mutexCompteur);
    while (compteur != 0)
    {
        pthread_cond_wait(&condCompteur, &mutexCompteur);

        printf("Compteur: %d\n", compteur);
    }
    pthread_mutex_unlock(&mutexCompteur);

    pthread_mutex_destroy(&mutexCompteur);

    return 0;
} 
 
void* threadsecond(void* p) {

    DONNEE parm;

    parm = *(DONNEE*)p;
    pthread_mutex_unlock(&mutexCompteur);

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGINT);
    pthread_sigmask(SIG_SETMASK, &mask, NULL);

    char *nomp = new char[strlen(parm.nom) + 1];
    strcpy(nomp, parm.nom);

    pthread_setspecific(cle, nomp);

    char txt[120];
    sprintf(txt, "Thread (%d,%p) %s demarre...\n",getpid(),pthread_self(), parm.nom); 
    TRACE(txt);

    struct timespec ts;

    ts.tv_sec = parm.nbSecondes;
    ts.tv_nsec = 0;

    nanosleep(&ts, NULL);

    sprintf(txt, "Thread (%d,%p) %s termine...\n",getpid(),pthread_self(), parm.nom); 
    TRACE(txt);

    pthread_mutex_lock(&mutexCompteur);
    compteur--;
    pthread_cond_signal(&condCompteur);
    pthread_mutex_unlock(&mutexCompteur);

    return NULL;
}

void LiberationCompteur(void* p)
{
    TRACE("Libération de ma variable spécifique Compteur.\n");
    delete[] (char*) p;
}

void HandlerSIGINT(int sig)
{
    char *nom = (char*) pthread_getspecific(cle);

    printf("\nThread %d.%lu s'occupe de \"%s\"\n",
           getpid(),
           pthread_self(),
           nom);
}