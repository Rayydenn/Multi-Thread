#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>

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

void* threadsecondaire(void *p); 


int main() { 
    pthread_t th1, th2, th3, th4; 
 
    printf("Thread principal (%d,%p) demarre...\n",getpid(),pthread_self()); 
 
    ParamThreads p1 = {ApplicGarage1, "printf", 0};
    ParamThreads p2 = {ApplicGarage2, "cout", 1};
    ParamThreads p3 = {ApplicGarage3, "sprintf", 2};
    ParamThreads p4 = {ApplicGarage4, "void", 3};

    pthread_create(&th1, NULL, threadsecondaire, &p1);
    pthread_create(&th2, NULL, threadsecondaire, &p2);
    pthread_create(&th3, NULL, threadsecondaire, &p3);
    pthread_create(&th4, NULL, threadsecondaire, &p4);

    void* resultat1;
    void* res2;
    void* res3;
    void* res4;

    pthread_join(th1, &resultat1);
    pthread_join(th2, &res2);
    pthread_join(th3, &res3);
    pthread_join(th4, &res4);

    printf("Thread principal : résultat 1 = %d\n résultat 2 = %d\n résultat 3 = %d\n résultat 4 = %d\n", (long)resultat1, (long)res2, (long)res3, (long)res4); 
} 
 
void* threadsecondaire(void* p) { 
	ParamThreads *prm = (ParamThreads*) p;
	int i = 0;
	int n;
	int compteur = 0;
    printf("Thread 1 (%d,%p) demarre...\n",getpid(),pthread_self()); 

    int len = strlen(prm->mot);
    char buf[len];

    while(1)
    {
        int fd = open(prm->fichier, O_RDONLY);
        for (int t = 0; t < prm->tabs; t++)
            printf("\t");
        printf("*\n");

        fflush(stdout); 

        if (fd == -1)
        {
            perror("open");
            pthread_exit(NULL);
        }

        if (lseek(fd, i, SEEK_SET) == -1)
        {
            close(fd);
            break;
        }

		int n = read(fd, buf, len);

        close(fd);

        if (n < len)
        {
            break;
        }

        if (memcmp(buf, prm->mot, len) == 0)
        {
            compteur++;
        }

        i++;

        usleep(10);
    }

    printf("\n");
 
    return (void*)(long)compteur;
}