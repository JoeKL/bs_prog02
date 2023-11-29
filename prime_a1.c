#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

unsigned int latest_prime = 0;

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
    while(true){
        if(latest_prime != 0){
            printf("CLIENT: %d is prime!\n", latest_prime);
            latest_prime = 0;
            //der client soll für eine Sekunde warten, wenn prime geprintet wurde
            sleep(1);
        }
    }
    return NULL;
}

// 
void *find_primes(){

    // zähle von 1 bis unendlich in dauerschleife
    for (unsigned int i = 1; i; i++)
    {
        // überprüfe ob i Prime ist
        if(isPrime(i)){
            //wenn ja, speichere i als latest_prime, sodass sie vom Clienten geprintet werden kann
            printf("SERVER: saving %d\n", i);

            latest_prime = i;
            //nach dem beschreiben von latest_prime wird eine Sekunde gesleept
            //durch das sleep werden race-conditions unterbunden, da der Client so immer schneller ist als der Server.
            sleep(1);
        }
    }    
    return NULL;
}

// int main(int argc, char const *argv[])
int main()
{
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

    return EXIT_SUCCESS;
}
