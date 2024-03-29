#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#define ARRAY_SIZE 5

unsigned int prime_array[ARRAY_SIZE];
int server_array_index = 0;
int client_array_index = 0;

pthread_mutex_t array_mutexes[ARRAY_SIZE];
pthread_cond_t prime_written_cond[ARRAY_SIZE]; 
pthread_cond_t prime_read_cond[ARRAY_SIZE]; 

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

//client läuft in dauerschleife und prüft ob latest_prime gesetzt ist, wenn ja dann print und lösche latest_prime
void *print_prime(){    
    while(true) {
        pthread_mutex_lock(&array_mutexes[client_array_index]); // locke resource[index]
        while (prime_array[client_array_index] == 0) {
            pthread_cond_wait(&prime_written_cond[client_array_index], &array_mutexes[client_array_index]); // warte bis resource[index] beschrieben wurde
        }
        printf("CLIENT: %d is prime in [%d]!\n", prime_array[client_array_index], client_array_index);
        prime_array[client_array_index] = 0;

        pthread_mutex_unlock(&array_mutexes[client_array_index]); // unlocke resource[index]
        pthread_cond_signal(&prime_read_cond[client_array_index]); // Signal an den Server-Thread, dass die Zahl gelesen wurde

        client_array_index = (client_array_index + 1 >= ARRAY_SIZE) ? 0 : client_array_index + 1; // wenn next nächstes element größer ARRAY_SIZE -> 0 
        sleep(2);
    }
    return NULL;
}


void *find_primes(){

    // zähle von 1 bis unendlich in dauerschleife
    for (unsigned int i = 1;; i++)
    {
        if(isPrime(i)) {
            pthread_mutex_lock(&array_mutexes[server_array_index]); //locke resource[index] 

            while (prime_array[server_array_index] != 0) {
                pthread_cond_wait(&prime_read_cond[server_array_index], &array_mutexes[server_array_index]); // warte bis resource[index] ausgelesen wurde
            }

            prime_array[server_array_index] = i;
            printf("SERVER: saving %d in [%d]\n", i, server_array_index); 

            pthread_mutex_unlock(&array_mutexes[server_array_index]); // unlocke resource[index]
            pthread_cond_signal(&prime_written_cond[server_array_index]); // Signal an den Client-Thread, dass eine neue Zahl geschrieben wurde

            server_array_index = (server_array_index + 1 >= ARRAY_SIZE) ? 0 : server_array_index + 1; // wenn next nächstes element größer ARRAY_SIZE -> 0 
            sleep(1);
        }
    }    
    return NULL;
}

// int main(int argc, char const *argv[])
int main()
{

    // Initialisieren von Mutexes und Bedingungsvariablen
    int err;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        if ((err = pthread_mutex_init(&array_mutexes[i], NULL)) != 0) {
            perror("Failed to init mutex");
            return EXIT_FAILURE;
        }
        if ((err = pthread_cond_init(&prime_written_cond[i], NULL)) != 0) {
            perror("Failed to init condition variable");
            return EXIT_FAILURE;
        }
        if ((err = pthread_cond_init(&prime_read_cond[i], NULL)) != 0) {
            perror("Failed to init condition variable");
            return EXIT_FAILURE;
        }
    }

    // thread identifier für server und client
    pthread_t server, client; 
    
    
    // Erstellen des Server-Threads
    if (pthread_create(&server, NULL, find_primes, NULL) != 0) {
        perror("Failed to create server thread");
        return EXIT_FAILURE;
    }

    // Erstellen des Client-Threads
    if (pthread_create(&client, NULL, print_prime, NULL) != 0) {
        perror("Failed to create client thread");
        return EXIT_FAILURE;
    }
    
    // Code ab hier ist effektiv nutzlos, da server und client in dauerschleife sind.
    // Warten auf die Beendigung des Client-Threads
    if (pthread_join(client, NULL) != 0) {
        perror("Failed to join client thread");
        return EXIT_FAILURE;
    }

    // Warten auf die Beendigung des Server-Threads
    if (pthread_join(server, NULL) != 0) {
        perror("Failed to join server thread");
        return EXIT_FAILURE;
    }


    // aufräumen der cond und mutex
    for (int i = 0; i < ARRAY_SIZE; i++) 
    {
        pthread_mutex_destroy(&array_mutexes[i]);
        pthread_cond_destroy(&prime_written_cond[i]);
        pthread_cond_destroy(&prime_read_cond[i]);    
    }  

    return EXIT_SUCCESS;
}
