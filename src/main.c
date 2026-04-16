#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

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

    // Validando argumentos da linha de comando
    if (argc != NUMBER_OF_RESOURCES + 1) {
        printf("Uso incorreto. Exemplo: ./banqueiro 10 5 7\n");
        return -1;
    }

    // Inicializando o gerador de números aleatórios
    srand(time(NULL));

    // Inicializando o array 'available' com os argumentos 
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] = atoi(argv[i + 1]);
    }

    // Inicializando os dados de teste clássicos do livro do Silberschatz para 'maximum'
    int max_test_data[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES] = {
        {7, 5, 3}, // Cliente 0
        {3, 2, 2}, // Cliente 1
        {9, 0, 2}, // Cliente 2
        {2, 2, 2}, // Cliente 3
        {4, 3, 3}  // Cliente 4
    };

    // Inicializando matrizes maximum, allocation e need
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
            maximum[i][j] = max_test_data[i][j];
            allocation[i][j] = 0; // Começa com nada alocado
            need[i][j] = maximum[i][j]; // Necessidade inicial é a máxima
        }
    }

    // Inicializando o Mutex
    pthread_mutex_init(&banco_mutex, NULL);
    printf("Banco aberto com recursos iniciais: [%d, %d, %d]\n\n", available[0], available[1], available[2]);

    // Criando as Threads dos clientes
    pthread_t threads[NUMBER_OF_CUSTOMERS];
    int thread_ids[NUMBER_OF_CUSTOMERS]; // Array para passar o ID corretamente

    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, customer_thread, &thread_ids[i]);
    }

    // Aguardando todas as threads terminarem (Join)
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        pthread_join(threads[i], NULL);
    }

    // Limpando o mutex ao finalizar
    pthread_mutex_destroy(&banco_mutex);
    printf("\nTodos os clientes terminaram. Banco fechado.\n");

    return 0;
}

bool is_safe() {
    //Inicializando os vetores Trabalho (work) e Término (finish)
    int work[NUMBER_OF_RESOURCES];
    bool finish[NUMBER_OF_CUSTOMERS];

    // Trabalho = Disponível
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        work[i] = available[i];
    }

    // Término[i] = false para todo i
    for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
        finish[i] = false;
    }

    // Encontrando uma sequência segura
    int count = 0; // Conta quantos clientes conseguiram terminar
    while (count < NUMBER_OF_CUSTOMERS) {
        bool found = false; // Flag para verificar se foi encontrado um cliente nesta rodada

        for (int i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
            if (!finish[i]) {
                // Necessidade_i <= Trabalho
                bool can_allocate = true;
                for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                    if (need[i][j] > work[j]) {
                        can_allocate = false;
                        break; // Se um recurso não for suficiente, já quebra o laço
                    }
                }

                // Se puder alocar (Passo 3)
                if (can_allocate) {
                    for (int j = 0; j < NUMBER_OF_RESOURCES; j++) {
                        work[j] += allocation[i][j]; // Trabalho = Trabalho + Alocação_i
                    }
                    finish[i] = true; // Término[i] = true
                    found = true;
                    count++;
                }
            }
        }

        // Se passar por todos os clientes e não achar ninguém que possa terminar, o laço é quebrado.
        if (!found) {
            break;
        }
    }

    // Se Término[i] == true para todo i, então o estado é seguro
    // Como usamos a variável 'count', basta checar se ela chegou ao total de clientes
    if (count == NUMBER_OF_CUSTOMERS) {
        return true;  // Estado Seguro
    } else {
        return false; // Estado Inseguro
    }
}

int request_resources(int customer_num, int request[]){
    pthread_mutex_lock(&banco_mutex);

    // Verificar se o cliente está pedindo mais do que ele disse que precisaria (need)
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (request[i] > need[customer_num][i]) {
            // O cliente excedeu sua demanda máxima
            pthread_mutex_unlock(&banco_mutex);
            return -1; // Nega o pedido
        }
    }

    // Verificar se o banco tem os recursos disponíveis no momento
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (request[i] > available[i]) {
            // Os recursos não estão disponíveis; o cliente deve esperar
            pthread_mutex_unlock(&banco_mutex);
            return -1; // Nega o pedido
        }
    }

    // Fingir a alocação dos recursos para testar a segurança
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] -= request[i];
        allocation[customer_num][i] += request[i];
        need[customer_num][i] -= request[i];
    }

    // Rodar o Algoritmo de Segurança com o estado alterado
    if (is_safe()) {
        // O estado é seguro.
        pthread_mutex_unlock(&banco_mutex);
        return 0; 
    } else {
        // O estado é inseguro. 
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            available[i] += request[i];
            allocation[customer_num][i] -= request[i];
            need[customer_num][i] += request[i];
        }
        
        // Desbloqueamos o mutex e negamos o pedido
        pthread_mutex_unlock(&banco_mutex);
        return -1; // Falha [cite: 33]
    }
}

int release_resources(int customer_num, int release[]){
    // Bloquear o mutex para evitar condição de corrida
    pthread_mutex_lock(&banco_mutex);

    // Verificar se o cliente está tentando devolver mais do que ele realmente tem alocado 
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        if (release[i] > allocation[customer_num][i]) {
            pthread_mutex_unlock(&banco_mutex);
            return -1; // O cliente está tentando devolver mais do que tem alocado, o que é inválido
        }
    }

    // Devolver os recursos ao banco e atualizar as matrizes do cliente
    for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] += release[i];
        allocation[customer_num][i] -= release[i];
        need[customer_num][i] += release[i]; 
    }

    // Desbloquear o mutex para que outras threads possam usar o banco
    pthread_mutex_unlock(&banco_mutex);

    return 0;
}

void* customer_thread(void* param) {
    int customer_num = *(int*)param;
    int request[NUMBER_OF_RESOURCES];
    int release[NUMBER_OF_RESOURCES];

    for(int iter = 0; iter < 5; iter++) { 
        
        // Gerando requisição aleatória limitada pelo 'need'
        for (int i = 0; i < NUMBER_OF_RESOURCES; i++) {
            if (need[customer_num][i] > 0) {
                request[i] = rand() % (need[customer_num][i] + 1);
            } else {
                request[i] = 0;
            }
        }

        // Mostrando o que o cliente quer pedir
        printf("Cliente %d tentando pedir: [%d, %d, %d]\n", customer_num, request[0], request[1], request[2]);

        // Tentando solicitar os recursos
        if (request_resources(customer_num, request) == 0) {
            printf("--> APROVADO: Cliente %d pegou os recursos!\n", customer_num);

            // Gerando liberação aleatória limitada pelo que está alocado 
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
            sleep(1); // Espera um pouco antes da próxima rodada
        }
    }
    return NULL;
}

