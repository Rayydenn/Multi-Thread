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

DONNEE Param;
int compteur = 0;

pthread_mutex_t mutexCompteur;
pthread_cond_t condCompteur;

int main() { 
    pthread_t thsec[4];
    int i, nbth = 0;

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