#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define NUMBER_OF_CUSTOMERS 5  
#define NUMBER_OF_RESOURCES 3

int available[NUMBER_OF_RESOURCES];  
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];  
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];   
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];  

pthread_mutex_t banco_mutex;

int request_resources(int customer_num, int request[]); 
int release_resources(int customer_num, int release[]); 


bool is_safe(); 
void* customer_thread(void* customer_id); 

int main(int argc, char *argv[]){

    if (argc != NUMBER_OF_RESOURCES + 1) {
        printf("Uso incorreto. Exemplo: ./banqueiro 10 5 7\n");
        return -1;
    }

    srand(time(NULL));

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] = atoi(argv[i + 1]);
    }

    int max_test_data[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = {
        {7, 5, 3}, 
        {3, 2, 2}, 
        {9, 0, 2}, 
        {2, 2, 2}, 
        {4, 3, 3}  
    };

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            maximum[i][j] = max_test_data[i][j];
            allocation[i][j] = 0; 
            need[i][j] = maximum[i][j];  
        }
    }

    pthread_mutex_init(&banco_mutex, NULL);
    printf("Banco aberto com recursos iniciais: [%d, %d, %d]\n\n", available[0], available[1], available[2]);

    pthread_t threads[NUMBER_OF_CUSTOMERS];
    int thread_ids[NUMBER_OF_CUSTOMERS]; 

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, customer_thread, &thread_ids[i]);
    }

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&banco_mutex);
    printf("\nTodos os clientes terminaram. Banco fechado.\n");

    return 0;
}

bool is_safe() {
    int work[NUMBER_OF_RESOURCES];
    bool finish[NUMBER_OF_CUSTOMERS];

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        work[i] = available[i];
    }

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        finish[i] = false;
    }

    int count = 0; 
    while (count < NUMBER_OF_CUSTOMERS) {
        bool found = false; 

        for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
            if (!finish[i]) {
                bool can_allocate = true;
                for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                    if (need[i][j] > work[j]) {
                        can_allocate = false;
                        break; 
                    }
                }

                if (can_allocate) {
                    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                        work[j] += allocation[i][j]; 
                    }
                    finish[i] = true; 
                    found = true;
                    count++;
                }
            }
        }

        if (!found) {
            break;
        }
    }

    if (count == NUMBER_OF_CUSTOMERS) {
        return true;  
    } else {
        return false; 
    }
}

int request_resources(int customer_num, int request[]){
    pthread_mutex_lock(&banco_mutex);

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (request[i] > need[customer_num][i]) {
            pthread_mutex_unlock(&banco_mutex);
            return -1; 
        }
    }

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (request[i] > available[i]) {
            pthread_mutex_unlock(&banco_mutex);
            return -1; 
        }
    }

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] -= request[i];
        allocation[customer_num][i] += request[i];
        need[customer_num][i] -= request[i];
    }

    if (is_safe()) {
        pthread_mutex_unlock(&banco_mutex);
        return 0; 
    } else {
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            available[i] += request[i];
            allocation[customer_num][i] -= request[i];
            need[customer_num][i] += request[i];
        }
        
        pthread_mutex_unlock(&banco_mutex);
        return -1; 
    }
}

int release_resources(int customer_num, int release[]){

    pthread_mutex_lock(&banco_mutex);

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (release[i] > allocation[customer_num][i]) {
            pthread_mutex_unlock(&banco_mutex);
            return -1;
        }
    }

    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] += release[i];
        allocation[customer_num][i] -= release[i];
        need[customer_num][i] += release[i]; 
    }

    pthread_mutex_unlock(&banco_mutex);

    return 0;
}

void* customer_thread(void* param) {
    int customer_num = *(int*)param;
    int request[NUMBER_OF_RESOURCES];
    int release[NUMBER_OF_RESOURCES];

    while(true) { 
        
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            if (need[customer_num][i] > 0) {
                request[i] = rand() % (need[customer_num][i] + 1);
            } else {
                request[i] = 0;
            }
        }

        printf("Cliente %d tentando pedir: [%d, %d, %d]\n", customer_num, request[0], request[1], request[2]);

        if (request_resources(customer_num, request) == 0) {
            printf("--> APROVADO: Cliente %d pegou os recursos!\n", customer_num);

            for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
                if (allocation[customer_num][i] > 0) {
                    release[i] = rand() % (allocation[customer_num][i] + 1);
                } else {
                    release[i] = 0;
                }
            }

            printf("Cliente %d devolvendo: [%d, %d, %d]\n", customer_num, release[0], release[1], release[2]);
            release_resources(customer_num, release);

        } else {
            printf("--> NEGADO: Pedido do Cliente %d deixaria o banco inseguro (Aguardando...)\n", customer_num);
            sleep(1); 
        }

        sleep(1);
    }
    return NULL;
}

