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

int main() { 
    pthread_t thsec[10];
    int i, nbth = 0;

    for (i = 0; data[i].nom[0] != '\0'; i++)
    {
        memcpy(&Param, &data[i], sizeof(DONNEE));
        pthread_create(&thsec[nbth], NULL, threadsecond, &Param);
    
        nbth++;
    }
    

    for (int j = 0; j < i; j++)
        pthread_join(thsec[j], NULL);

    return 0;
} 
 
void* threadsecond(void* p) { 
    char txt[120];
    DONNEE* d = (DONNEE*)p;
    sprintf(txt, "Thread (%d,%p) %s demarre...\n",getpid(),pthread_self(), d->nom); 
    TRACE(txt);

    struct timespec ts;

    ts.tv_sec = d->nbSecondes;
    ts.tv_nsec = 0;

    nanosleep(&ts, NULL);

    sprintf(txt, "Thread (%d,%p) %s termine...\n",getpid(),pthread_self(), d->nom); 
    TRACE(txt);

    return NULL;
}