#define NTHREADS 10
#define NMUTEX 65536
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
  
pthread_t tid[NTHREAD];
int counter;
pthread_mutex_t lock[NMUTEX];
  
void* trythis(void* arg)
{
    int fnum = 0xabcd ;
    pthread_mutex_lock(&lock[fnum]);
  
    unsigned long i = 0;
    counter += 1;
    printf("\n Job %d has started\n", counter);
  
    system("sleep 5") ;
  
    printf("\n Job %d has finished\n", counter);
  
    pthread_mutex_unlock(&lock[fnum]);
  
    return NULL;
}
  
int main(void)
{
    int i = 0;
    int error;
  
    for  (i = 0 ; i < NMUTEX ; i++) {
        if (pthread_mutex_init(&lock[i], NULL) != 0) {
            printf("\n mutex init has failed\n");
            return 1;
        }
    }
  
    i = 0 ;
    while (i < 2) {
        error = pthread_create(&(tid[i]), NULL, &trythis, NULL);
        if (error != 0)
            printf("\nThread can't be created :[%s]",
                   strerror(error));
        i++;
    }
  
    pthread_join(tid[0], NULL);
    pthread_join(tid[1], NULL);
    for  (i = 0 ; i < NMUTEX ; i++)
        pthread_mutex_destroy(&lock[i]);
  
    return 0;
}
