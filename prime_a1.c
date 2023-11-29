#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define NUM_THREADS 5
#define ARRAY_SIZE 4096

int *PrimeArray = NULL; // Global pointer initialized to NULL
int array_index = 0;


bool isPrime(unsigned int number)
{
    if(number < 2)
        return false;

    for(int i = 2; i * i <= number; i++)
    {
        if((number % i) == 0)
        {
            return false;
        }
    }
    return true;
}


void *serve_number(){
    printf("starting server thread\n");

    for (int i = 0; i < ARRAY_SIZE; i++)
    {
        printf("checking %d\n", i);
        if(isPrime(i)){
            PrimeArray[array_index] = i;
            array_index++;
            printf("\t%d is prime!\n", i);
        }

    }    
    return NULL;
}

// int main(int argc, char const *argv[])
int main()
{
    PrimeArray = malloc(ARRAY_SIZE * sizeof(int));
    pthread_t server; // Thread Array mit Max Num_THREAD Threads
    // pthread_t threads[NUM_THREADS]; // Thread Array mit Max Num_THREAD Threads
    
    printf("creating server thread\n");
    pthread_create(&server, NULL, serve_number, NULL);
    // int server_id = pthread_create(&server, NULL, serve_number, NULL);
    
    printf("server thread finished\n");
    return EXIT_SUCCESS;
}
