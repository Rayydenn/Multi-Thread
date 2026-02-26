#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#define TRACE(message) printf("[THREAD %d:%p] %s",getpid(),pthread_self(),message);

#define ApplicGarage1 "./1.cpp"
#define ApplicGarage2 "./2.cpp"
#define ApplicGarage3 "./3.cpp"
#define ApplicGarage4 "./4.cpp"

typedef struct
{
	const char *fichier;
	const char *mot;
	int tabs;
} ParamThreads;


void HandlerSIGINT(int sig);
void* threadslave(void *p); 
void* threadmaster(void *p);

int main() { 
    pthread_t th1, th2, th3, th4;
    pthread_t threadm;
 
    struct sigaction A;

    A.sa_handler = HandlerSIGINT;
    sigemptyset(&A.sa_mask);
    A.sa_flags = 0;

    if ((sigaction(SIGINT, &A, NULL)) == -1)
    {
      perror("(SERVEUR) Erreur lors de l'armement de SIGINT\n");
      exit(1);
    }
    printf("(SERVEUR) Le signal SINGINT à bien été armé \n");

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    char txt[120];
    sprintf(txt, "Thread principal (%d,%p) demarre...\n",getpid(),pthread_self()); 
    TRACE(txt);

    pthread_create(&th1, NULL, threadslave, NULL);
    pthread_create(&th2, NULL, threadslave, NULL);
    pthread_create(&th3, NULL, threadslave, NULL);
    pthread_create(&th4, NULL, threadslave, NULL);
    pthread_create(&threadm, NULL,threadmaster, NULL);

    pthread_join(threadm, NULL);

    /*sprintf(txt, "Thread principal en pause (Ctrl+C)...\n");
    TRACE(txt);
    pause();

    pthread_exit(NULL);*/
} 
 
void* threadslave(void* p) { 
    TRACE("Je masque SIGINT\n");
    char txt[120];
    sprintf(txt, "Thread (%d,%p) demarre...\n",getpid(),pthread_self()); 
    TRACE(txt);

    while(1) sleep(1);

    printf("\n");

    return NULL;
}

void* threadmaster(void *p)
{
    char txt[80];
    sprintf(txt, "Thread Master %p demarre", pthread_self());
    TRACE(txt);

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    while (1)
    {
        pause();
    }
}

void HandlerSIGINT(int sig)
{
    printf("\nSignal SIGINT reçu par thread %p\n", pthread_self());

    pthread_exit(NULL);
}