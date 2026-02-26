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
int pause(void);
void* threadslave(void *p); 


int main() { 
    pthread_t th1, th2, th3, th4; 
 
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

    char txt[120];
    sprintf(txt, "Thread principal (%d,%p) demarre...\n",getpid(),pthread_self()); 
    TRACE(txt);

    pthread_create(&th1, NULL, threadslave, NULL);
    pthread_create(&th2, NULL, threadslave, NULL);
    pthread_create(&th3, NULL, threadslave, NULL);
    pthread_create(&th4, NULL, threadslave, NULL);


    sprintf(txt, "Thread principal en pause (Ctrl+C)...\n");
    TRACE(txt);
    pause();

    pthread_exit(NULL);
} 
 
void* threadslave(void* p) { 
	int i = 0;
	int n;
	int compteur = 0;
    char txt[120];
    sprintf(txt, "Thread (%d,%p) demarre...\n",getpid(),pthread_self()); 
    TRACE(txt);

    pause();

    printf("\n");

    pthread_exit(NULL);
 
    return (void*)(long)compteur;
}

void HandlerSIGINT(int sig)
{
    printf("\nSignal SIGINT reçu par thread %p\n", pthread_self());
}