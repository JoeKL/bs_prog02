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

bool stop_execution = false;
pthread_cond_t ending_cond;

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
void *print_prime(void *arg){    
    int thread_id = *(int *)arg;
    while(true) {
        pthread_mutex_lock(&array_mutexes[client_array_index]); // locke resource[index]

        while (prime_array[client_array_index] == 0) {
            pthread_cond_wait(&prime_written_cond[client_array_index], &array_mutexes[client_array_index]); // warte bis resource[index] beschrieben wurde
        }


        if(stop_execution){
            pthread_mutex_unlock(&array_mutexes[client_array_index]); // unlocke resource[index]
            pthread_exit(NULL);
        }

        printf("CLIENT(%d): %d is prime in [%d]!\n",thread_id, prime_array[client_array_index], client_array_index);
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

            if(stop_execution){
                pthread_mutex_unlock(&array_mutexes[server_array_index]); // unlocke resource[index]
                pthread_exit(NULL);
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

void *keyboard_listener(

) {
    char key;
    printf("Drücke ENTER zum Beenden des Programms...\n");
    
    while((key = getchar()) != '\n') {
        // Loop until the ENTER key is pressed
        // You might want to handle other keys or provide some exit condition
    }
    
    stop_execution = true;
    pthread_cond_broadcast(&ending_cond);

    return NULL;
}

int main(int argc, char const *argv[])
{
       if (argc != 2) {
        printf("Bitte genau ein Argument angeben!\n");
        return EXIT_FAILURE;
    }

    int thread_count = atoi(argv[1]); // Konvertiert das Argument in eine Zahl

    if (thread_count <= 0) {
        printf("Bitte eine positive Zahl angeben!\n");
        return EXIT_FAILURE;
    }

    int *thread_ids = malloc(sizeof(int) * thread_count);

    printf("Programm mit %d Clients starten:\n", thread_count);


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
    if ((err = pthread_cond_init(&ending_cond, NULL)) != 0) {
            perror("Failed to init ending_cond");
            return EXIT_FAILURE;
        }

    // thread identifier für server und client
    pthread_t server, listener_thread;
    pthread_t *clients = malloc(sizeof(pthread_t) * thread_count); // Dynamisches Array für Client-Threads

    // Erstellen des Listener-Threads
    if (pthread_create(&listener_thread, NULL, keyboard_listener, NULL) != 0) {
        perror("Failed to create listener thread");
        return EXIT_FAILURE;
    }
    
    // Erstellen des Server-Threads
    if (pthread_create(&server, NULL, find_primes, NULL) != 0) {
        perror("Failed to create server thread");
        return EXIT_FAILURE;
    }
    
    for (int i = 0; i < thread_count; i++) {
        thread_ids[i] = i;
        if(pthread_create(&clients[i], NULL, print_prime, &thread_ids[i]) != 0){ // Erstellt jeden Client-Thread
            perror("Failed to create client thread");
            return EXIT_FAILURE;
        }
    }

    // Warten auf die Beendigung des Client-Threads
    if (pthread_join(listener_thread, NULL) != 0) {
        perror("Failed to join client thread");
        return EXIT_FAILURE;
    }

    // Warten auf die Beendigung des Server-Threads
    if (pthread_join(server, NULL) != 0) {
        perror("Failed to join server thread");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < thread_count; i++) {
        if(pthread_join(clients[i], NULL) != 0){ // Wartet auf jeden Client-Thread
            perror("Failed to join client thread");
            return EXIT_FAILURE;
        }
    }

    free(clients); // Gibt das dynamische Array frei
    free(thread_ids);

    // aufräumen der cond und mutex
    for (int i = 0; i < ARRAY_SIZE; i++) 
    {
        pthread_mutex_destroy(&array_mutexes[i]);
        pthread_cond_destroy(&prime_written_cond[i]);
        pthread_cond_destroy(&prime_read_cond[i]);    
    }
    pthread_cond_destroy(&ending_cond); 

    printf("Programm beendet.\n");

    return EXIT_SUCCESS;
}
