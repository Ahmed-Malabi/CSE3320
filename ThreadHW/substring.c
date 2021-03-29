#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include <pthread.h>

#define MAX 5000000
#define THREADS 1

int total = 0;
int n1,n2 ; 
char *s1,*s2;
pthread_mutex_t mutex;

FILE *fp;

int readf(char* filename)
{
    if((fp=fopen(filename, "r"))==NULL)
    {
        printf("ERROR: can’t open %s!\n", filename);
        return 0;
    }

    s1=(char *)malloc(sizeof(char)*MAX);

    if (s1==NULL)
    {
        printf ("ERROR: Out of memory!\n");
        return -1;
    }

    s2=(char *)malloc(sizeof(char)*MAX);

    if (s1==NULL)
    {
    printf ("ERROR: Out of memory\n");
        return -1;
    }

    /*read s1 s2 from the file*/

    s1=fgets(s1, MAX, fp);
    s2=fgets(s2, MAX, fp);
    n1=strlen(s1); /*length of s1*/
    n2=strlen(s2); /*length of s2*/

    if( s1==NULL || s2==NULL || n1 < n2 ) /*when error exit*/
    {
        return -1;
    }
}

void* num_substring ( void* temp )
{
    int tid = *(int*)temp;
    int i,j,k, start, end;

    //  modified start and end to be based
    //  off thread id instead of entire string
    start = (tid) * (n1/THREADS);
    end = ((tid + 1) * (n1/THREADS));
    int count;
    int extra = 0;


    for (i = start; i <= end + extra; i++)
    {
        count = 0;

        //  if we are not on the last thread we
        //  overlap by n2-1 incase a word would
        //  be split by the threads.
        for(j = i,k = 0; k < n2; j++,k++)
        { /*search for the next string of size of n2*/
            if (*(s1+j) != *(s2+k))
            {
                break;
            }
            else
            {
                count++;
            }
            if (count == n2)
            {
                //  lock the total when it is being incremented
                //  to prevent back tracking if a thread is 
                //  interupted
                pthread_mutex_lock( &mutex );
                total++; /*find a substring in this step*/
                pthread_mutex_unlock( &mutex );
            }
        }
    }
}

int main(int argc, char *argv[])
{
    pthread_t threads[THREADS];
    pthread_mutex_init( & mutex, NULL );
    int id[THREADS];

    int i;

    if( argc < 2 )
    {
        printf("Error: You must pass in the datafile as a commandline parameter\n");
    }

    readf ( argv[1] );

    struct timeval start, end;
    float mtime; 
    int secs, usecs;    

    gettimeofday(&start, NULL);

    //  calls num_substring for each thread passing in its id
    for(i = 0; i < THREADS; i++)
    {
        id[i] = i;
        if(pthread_create(&threads[i], NULL, num_substring, (void*)&id[i]))
        {
            perror("Error creating thread: ");
            exit( EXIT_FAILURE ); 
        }
    }

    //  rejoins all the threads into an array
    for(i = 0; i < THREADS; i++)
    {
        if(pthread_join(threads[i], NULL)) 
        {
            perror("Problem with pthread_join: ");
        }
    }

    gettimeofday(&end, NULL);

    secs  = end.tv_sec  - start.tv_sec;
    usecs = end.tv_usec - start.tv_usec;
    mtime = ((secs) * 1000 + usecs/1000.0) + 0.5;

    printf ("The number of substrings is : %d\n" , total);
    printf ("Elapsed time is : %f milliseconds\n", mtime);

    if( s1 )
    {
        free( s1 );
    }

    if( s2 )
    {
        free( s2 );
    }

    return 0; 
}