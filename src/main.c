#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

/* Constantes definidas no roteiro do trabalho */
#define NUMBER_OF_CUSTOMERS 5  
#define NUMBER_OF_RESOURCES 3

 // Montante disponível de cada recurso 
int available[NUMBER_OF_RESOURCES];  
// Demanda máxima de cada cliente                    
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];  
// Montante correntemente alocado a cada cliente        
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];   
// Necessidade remanescente de cada cliente    
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];  

/* Mutex para evitar condição de corrida */
pthread_mutex_t banco_mutex;

/* Protótipos exigidos pelo trabalho */
int request_resources(int customer_num, int request[]); 
int release_resources(int customer_num, int release[]); 

/* Protótipos auxiliares */

// Para isolar a lógica do algoritmo do banqueiro
bool is_safe(); 
// Comportamento das thread
void* customer_thread(void* customer_id); 

int main(int argc, char *argv[]){

    // TODO: Validar e ler os argumentos da linha de comando (argv) para preencher 'available'
    
    // TODO: Inicializar matrizes maximum, allocation e need
    
    // TODO: Inicializar o mutex
    
    // TODO: Criar as N threads

    return 0;
}

