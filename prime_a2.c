#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

unsigned int latest_prime = 0;
pthread_mutex_t latest_prime_mutex;

pthread_cond_t prime_written_cond;
pthread_cond_t prime_read_cond;

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
        pthread_mutex_lock(&latest_prime_mutex); // locke resource
        while (latest_prime == 0) {
            pthread_cond_wait(&prime_written_cond, &latest_prime_mutex); // warte bis resource beschrieben wurde
        }
        printf("CLIENT: %d is prime!\n", latest_prime);
        latest_prime = 0;
        
        pthread_cond_signal(&prime_read_cond); // Signal an den Server-Thread, dass die Zahl gelesen wurde
        pthread_mutex_unlock(&latest_prime_mutex); // unlocke resource
        sleep(1);
    }
    return NULL;
}


void *find_primes(){

    // zähle von 1 bis unendlich in dauerschleife
    for (unsigned int i = 1;; i++)
    {
        if(isPrime(i)) {
            pthread_mutex_lock(&latest_prime_mutex); //locke resource
            while (latest_prime != 0) {
                pthread_cond_wait(&prime_read_cond, &latest_prime_mutex); // warte bis resource ausgelesen wurde
            }
            latest_prime = i;
            printf("SERVER: saving %d\n", i); 

            pthread_cond_signal(&prime_written_cond); // Signal an den Client-Thread, dass eine neue Zahl geschrieben wurde
            pthread_mutex_unlock(&latest_prime_mutex); // Freigeben des Mutex
            sleep(2);
        }
    }    
    return NULL;
}

// int main(int argc, char const *argv[])
int main()
{
    // init von cond und mutex
    pthread_mutex_init(&latest_prime_mutex, NULL);
    pthread_cond_init(&prime_written_cond, NULL);
    pthread_cond_init(&prime_read_cond, NULL);    

    // thread identifier für server und client
    pthread_t server, client; 
    
    //create thread server mit funktion "find_primes"
    pthread_create(&server, NULL, find_primes, NULL);

    //create thread client mit funktion "print_prime"
    pthread_create(&client, NULL, print_prime, NULL);
    
    // Code ab hier ist effektiv nutzlos, da server und client in dauerschleife sind.
    pthread_join(client, NULL);
    pthread_join(server, NULL);


    // aufräumen der cond und mutex
    pthread_mutex_destroy(&latest_prime_mutex);
    pthread_cond_destroy(&prime_written_cond);
    pthread_cond_destroy(&prime_read_cond);    

    return EXIT_SUCCESS;
}
