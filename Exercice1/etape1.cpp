#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <string.h>

int i = 0;
int n;


int compteur = 0;
#define ApplicGarage "./1.cpp"


void* FctThread(void *p); 


int main() { 
    pthread_t th; 
 
    printf("Thread principal (%d,%p) demarre...\n",getpid(),pthread_self()); 
 
    int err; 
 
    err = pthread_create(&th, NULL, FctThread, NULL); 
 
    void* resultat;

    pthread_join(th, &resultat);

    printf("Thread principal : résultat = %d\n", (long)resultat); 
} 
 
void* FctThread(void* p) { 
    printf("Thread secondaire (%d,%p) demarre...\n",getpid(),pthread_self()); 

    char cible[] = "printf";
    int len = strlen(cible);
    char buf[len];

    while(1)
    {
        int fd = open(ApplicGarage, O_RDONLY);
        printf("*");

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

        if (memcmp(buf, cible, len) == 0)
        {
            compteur++;
        }

        i++;

        nanosleep(2);
    }

    printf("\n");
 
    return (void*)(long)compteur;
} 