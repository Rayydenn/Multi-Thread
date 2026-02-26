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
	const char *fichier;
	const char *mot;
	int tabs;
} ParamThreads;


void FctThreadFin(void* p);
void HandlerSIGINT(int sig);
void HandlerSIGUSR1(int sig);
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
      perror("Erreur lors de l'armement de SIGINT\n");
      exit(1);
    }
    printf("Le signal SINGINT à bien été armé \n");

    struct sigaction B;

    B.sa_handler = HandlerSIGUSR1;
    sigemptyset(&B.sa_mask);
    B.sa_flags = 0;

    if ((sigaction(SIGUSR1, &B, NULL)) == -1)
    {
      perror("Erreur lors de l'armement de SIGUSR1\n");
      exit(1);
    }
    printf("Le signal SIGUSR1 à bien été armé \n");

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    char txt[120];
    sprintf(txt, "Thread principal (%d,%p) demarre...\n",getpid(),pthread_self()); 
    TRACE(txt);

    pthread_create(&th1, NULL, threadslave, NULL);
    pthread_create(&th2, NULL, threadslave, NULL);
    pthread_create(&th3, NULL, threadslave, NULL);
    pthread_create(&th4, NULL, threadslave, NULL);
    pthread_create(&threadm, NULL,threadmaster, NULL);

    pthread_join(th1, NULL);
    pthread_join(th2, NULL);
    pthread_join(th3, NULL);
    pthread_join(th4, NULL);

    pthread_cancel(threadm);

    pthread_exit(NULL);
} 
 
void* threadslave(void* p) { 
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_BLOCK, &set, NULL);


    char txt[120];
    sprintf(txt, "Thread (%d,%p) demarre...\n",getpid(),pthread_self()); 
    TRACE(txt);

    pause();

    return NULL;
}

void* threadmaster(void *p)
{
    char txt[80];
    sprintf(txt, "Thread Master %p demarre", pthread_self());
    TRACE(txt);

    pthread_cleanup_push(FctThreadFin, (void*)"Thread master");

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);
    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &set, NULL);

    while (1)
    {
        pause();
    }

    pthread_cleanup_pop(1);
    pthread_exit(NULL);
}

void FctThreadFin(void *p) { 
    const char* name = (const char*) p; 
 
    char txt[80]; 
    sprintf(txt,"%s: Je passe par ma fonction de terminaison\n",name); 
    TRACE(txt); 
}

void HandlerSIGINT(int sig)
{
    printf("\nSignal SIGINT reçu par thread %p\n", pthread_self());

    kill(getpid(), SIGUSR1);
}

void HandlerSIGUSR1(int sig)
{
    char txt[120];
    sprintf(txt, "Thread (%d,%p) a reçu le message...\n",getpid(),pthread_self());
    TRACE(txt);

    pthread_exit(NULL);
}