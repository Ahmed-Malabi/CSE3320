#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#define NONSHARED 1
#define BUFFERSIZE 5
#define MAXSIZE 5000000

sem_t textConsumer, textProducer;
char buffer[BUFFERSIZE+1];
char* s;
int sl;

void* printText(void* arg)
{
    int i = 4, j;

    while(1)
    {
        sem_wait(&textConsumer);
        if(i == -1)
            i = 4;
            
        if(buffer[i] == EOF)
            break;
            
        printf("%c", buffer[i]);
        i--;
        sem_post(&textProducer);
    }
}

void* getText(void* arg)
{
    FILE* fp;
    int i = 0, j;
    if((fp = fopen("message.txt", "r")) == NULL)
    {
        printf("ERROR: can’t open %s!\n", "messgae.txt");
        return 0;
    }

    s=(char *)malloc(sizeof(char)*MAXSIZE);
    s = fgets(s, MAXSIZE, fp);
    sl = strlen(s);

    while(1)
    {
        sem_wait(&textProducer);
        for(j = 4; j > -1; j--)
            buffer[j+1] = buffer[j];
                
        buffer[0] = s[i];

        if(s[i] == EOF)
            break;

        i++;
        sem_post(&textConsumer);
    }
}

int main()
{
    pthread_t producer_tid;  
    pthread_t cashier_tid;  

    sem_init(&textProducer, NONSHARED, BUFFERSIZE);
    sem_init(&textConsumer, NONSHARED, 0);  
    
    pthread_create(&producer_tid, NULL, getText, NULL);
    pthread_create(&cashier_tid, NULL, printText, NULL);

    pthread_join(producer_tid, NULL);
    pthread_join(cashier_tid, NULL);

    return 0;
}
