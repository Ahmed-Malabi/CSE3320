#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <semaphore.h>

#define NONSHARED 1
#define BUFFERSIZE 5
#define MAXSIZE 5000000

sem_t textConsumer, textProducer;
char buffer[BUFFERSIZE];
char* s;
int head = 0, tail = 0;
FILE* fp;

/*
    printText is a function that prints out
    the characters in buffer in a FIFO order
*/
void* printText(void* arg)
{
    int i = 4, j;

    while(1)
    {
        //  we need to wait until there are new characters to print
        sem_wait(&textConsumer);

        //  if it is a valid char print it out
        if(buffer[head] > 0)
            printf("%c", buffer[head++]);

        //  once we print we can let the producer know they can overwrite
        sem_post(&textProducer);

        if(head == 5)
            head = 0;

        /*
            once the producer has reached the end of the file we will be nearing
            the end of the program, but sometimes the consumer can be behind.
            This makes sure the consumer catches up to the newest elements.
        */
        if((feof(fp) && (head + 1) == tail) || (feof(fp) && (head + 1) == 5))
            break;
    }
}

/*
    getText() is a function that opens and reads
    from a file character by character into 
    a buffer / circular queue to be printed
*/
void* getText(void* arg)
{
    int i = 0, j;
    char c;

    //  opens the file message.txt as it was the requirement
    if((fp = fopen("message.txt", "r")) == NULL)
    {
        printf("ERROR: can’t open %s!\n", "message.txt");
        return 0;
    }

    while(1)
    {
        //  waits to make sure producers doesn't out
        //  run the consumer and overwrite text
        sem_wait(&textProducer);

        c = fgetc(fp);
        buffer[tail++] = c;

        if(tail == 5)
            tail = 0;

        //  tell the consumer a new char was added
        sem_post(&textConsumer);

        if(feof(fp))
            break;
    }
}

/*  
    main just initilized all threads and
    semaphores and then calls and waits for 
    the threads to return.
*/
int main()
{
    pthread_t producer_tid;  
    pthread_t consumer_tid;  

    sem_init(&textProducer, NONSHARED, BUFFERSIZE);
    sem_init(&textConsumer, NONSHARED, 0);  
    
    pthread_create(&producer_tid, NULL, getText, NULL);
    pthread_create(&consumer_tid, NULL, printText, NULL);

    pthread_join(producer_tid, NULL);
    pthread_join(consumer_tid, NULL);

    sem_close(&textProducer);
    sem_close(&textConsumer);

    fclose(fp);

    printf("\n");
    return 0;
}
