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

pthread_mutex_t mutexParam = PTHREAD_MUTEX_INITIALIZER;

int main() { 
    pthread_t thsec[4];
    int i, nbth = 0;

    for (i = 0; data[i].nom[0] != '\0'; i++)
    {
        pthread_mutex_lock(&mutexParam);

        memcpy(&Param, &data[i], sizeof(DONNEE));
        pthread_create(&thsec[nbth], NULL, threadsecond, &Param);
        

        nbth++;
    }
    

    for (int j = 0; j < nbth; j++)
        pthread_join(thsec[j], NULL);

    pthread_mutex_destroy(&mutexParam);

    return 0;
} 
 
void* threadsecond(void* p) {

    DONNEE parm;

    parm = *(DONNEE*)p;
    pthread_mutex_unlock(&mutexParam);

    char txt[120];
    sprintf(txt, "Thread (%d,%p) %s demarre...\n",getpid(),pthread_self(), parm.nom); 
    TRACE(txt);

    struct timespec ts;

    ts.tv_sec = parm.nbSecondes;
    ts.tv_nsec = 0;

    nanosleep(&ts, NULL);

    sprintf(txt, "Thread (%d,%p) %s termine...\n",getpid(),pthread_self(), parm.nom); 
    TRACE(txt);
    return NULL;
}