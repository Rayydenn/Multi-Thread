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

void* thread1(void *p); 
void* thread2(void *p);
void* thread3(void *p); 
void* thread4(void *p);


int main() { 
    pthread_t th1, th2, th3, th4; 
 
    printf("Thread principal (%d,%p) demarre...\n",getpid(),pthread_self()); 
 
    pthread_create(&th1, NULL, thread1, NULL);
    pthread_create(&th2, NULL, thread2, NULL);
    pthread_create(&th3, NULL, thread3, NULL);
    pthread_create(&th4, NULL, thread4, NULL);

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
 
void* thread1(void* p) { 
	int i = 0;
	int n;
	int compteur = 0;
    printf("Thread 1 (%d,%p) demarre...\n",getpid(),pthread_self()); 

    char cible[] = "printf";
    int len = strlen(cible);
    char buf[len];

    while(1)
    {
        int fd = open(ApplicGarage1, O_RDONLY);
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

        if (memcmp(buf, cible, len) == 0)
        {
            compteur++;
        }

        i++;

        usleep(100);
    }

    printf("\n");
 
    return (void*)(long)compteur;
} 

void* thread2(void* p) { 
	int i = 0;
	int n;
	int compteur = 0;
    printf("Thread 2 (%d,%p) demarre...\n",getpid(),pthread_self()); 

    char cible[] = "cout";
    int len = strlen(cible);
    char buf[len];

    while(1)
    {
        int fd = open(ApplicGarage2, O_RDONLY);
        printf("\t*\n");

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

        usleep(100);
    }

    printf("\n");
 
    return (void*)(long)compteur;
} 

void* thread3(void* p) { 
	int i = 0;
	int n;
	int compteur = 0;
    printf("Thread 3 (%d,%p) demarre...\n",getpid(),pthread_self()); 

    char cible[] = "sprintf";
    int len = strlen(cible);
    char buf[len];

    while(1)
    {
        int fd = open(ApplicGarage3, O_RDONLY);
        printf("\t\t*\n");

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

        usleep(100);
    }

    printf("\n");
 
    return (void*)(long)compteur;
} 

void* thread4(void* p) { 
	int i = 0;
	int n;
	int compteur = 0;
    printf("Thread 4 (%d,%p) demarre...\n",getpid(),pthread_self()); 

    char cible[] = "void";
    int len = strlen(cible);
    char buf[len];

    while(1)
    {
        int fd = open(ApplicGarage4, O_RDONLY);
        printf("\t\t\t*\n");

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

        usleep(100);
    }

    printf("\n");
 
    return (void*)(long)compteur;
} 