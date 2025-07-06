#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>

int MAX_CUSTOMERS = 10;
int NUM_BARBERS = 3;
int MAX_PEOPLE_IN_SHOP = 10;
int SOFA_CAPACITY = 4;
int MAX_SIZE = 10;  // Default value, to be updated via command line

// Estrutura de cliente
typedef struct {
    int id;
    pthread_cond_t waiting_for_service;
    pthread_cond_t waiting_for_sofa;
} Customer;

// Estrutura de fila
typedef struct {
    int *items;  // Alocado dinamicamente
    int front;
    int rear;
    int count;
    int capacity;  // Armazenar o tamanho máximo da fila
} Queue;

// Variáveis globais para o número de clientes e barbeiros
Customer *customer_data;
Queue *standing_queue;
Queue *sofa_queue;
int sofa_count = 0;
int num_people_in_shop = 0;
int standing_count = 0;  // Contador de clientes em pé

// Mutex e variáveis de condição
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t payment_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex dedicado para pagamentos
pthread_cond_t work_available = PTHREAD_COND_INITIALIZER;  // Condição para qualquer trabalho

bool work_done = false;

// Função para inicializar a fila
void initializeQueue(Queue* q, int size) {
    q->capacity = size;  // Definindo a capacidade da fila
    q->items = (int *)malloc(size * sizeof(int));  // Aloca a fila dinamicamente com base no tamanho MAX_SIZE
    q->front = 0;
    q->rear = 0;
    q->count = 0;
}

// Função para verificar se a fila está vazia
bool isEmpty(Queue* q) {
    return (q->count == 0);
}

// Função para verificar se a fila está cheia
bool isFull(Queue* q) {
    return (q->count == q->capacity);  // Usando a capacidade dinâmica
}

// Função para adicionar um elemento à fila
void enqueue(Queue* q, int value) {
    if (isFull(q)) {
        printf("Queue is full\n");
        return;
    }
    q->items[q->rear] = value;
    q->rear = (q->rear + 1) % q->capacity;  // Usando a capacidade dinâmica
    q->count++;
}

// Função para remover um elemento da fila
int dequeue(Queue* q) {
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return -1;
    }
    int value = q->items[q->front];
    q->front = (q->front + 1) % q->capacity;  // Usando a capacidade dinâmica
    q->count--;
    return value;
}

// Função para obter o elemento na frente da fila
int peek(Queue* q) {
    if (isEmpty(q)) {
        return -1;
    }
    return q->items[q->front];
}

// Função para encontrar cliente por ID
Customer* find_customer_by_id(int id) {
    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        if (customer_data[i].id == id) {
            return &customer_data[i];
        }
    }
    return NULL;
}

// Função para gerar um tempo aleatório
int random_time(int min, int max) {
    return rand() % (max - min + 1) + min;
}

void *customer_func(void *arg) {
    Customer *customer = (Customer *)arg;
    printf("Cliente %d chegou na loja.\n", customer->id);

    pthread_mutex_lock(&mutex);

    if (num_people_in_shop >= MAX_PEOPLE_IN_SHOP) {
        printf("Cliente %d foi embora, loja cheia.\n", customer->id);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }

    num_people_in_shop++;

    // Se o sofá não estiver cheio, o cliente senta no sofá
    if (sofa_count < SOFA_CAPACITY) {
        sofa_count++;
        enqueue(sofa_queue, customer->id);
        printf("Cliente %d sentou no sofá. Número de clientes no sofá %d.\n", customer->id, sofa_count);
        pthread_cond_broadcast(&work_available);  // Acorda todos os barbeiros
    } else {
        // Cliente fica em pé e espera
        standing_count++;
        enqueue(standing_queue, customer->id);
        printf("Cliente %d ficou em pé.\n", customer->id);
        pthread_cond_wait(&customer->waiting_for_sofa, &mutex);
    }

    // Espera ser atendido pelo barbeiro
    pthread_cond_wait(&customer->waiting_for_service, &mutex);  // Espera pelo pagamento
    
    // Cliente vai sair da loja depois que o pagamento for realizado
  
    num_people_in_shop--;  // Cliente sai da loja após pagamento

    pthread_mutex_unlock(&mutex);

    // Confirmando que a thread do cliente terminou
    printf("Cliente %d terminou corte e pagamento e está saindo.\n", customer->id);
    
    return NULL;
}

void *barber_func(void *arg) {
    int barber_id = *((int *)arg);
    
    while (1) {
        pthread_mutex_lock(&mutex);

        // Verifica se há trabalho para fazer (corte ou se já terminou o atendimento de todos)
        while (sofa_count == 0) {
            printf("Barbeiro %d está dormindo, esperando trabalho...\n", barber_id);
            pthread_cond_wait(&work_available, &mutex);
        }

        // Se houver cliente no sofá, realiza o corte de cabelo
        if (sofa_count > 0) {
            int customer_id = dequeue(sofa_queue);
            sofa_count--;
            printf("Barbeiro %d selecionou cliente %d para cortar cabelo.\n", barber_id, customer_id);
            

             if (standing_count > 0 && sofa_count < SOFA_CAPACITY) {
                int standing_customer_id = dequeue(standing_queue);
                standing_count--;  // Decrementa o contador de clientes em pé
                sofa_count++;
                enqueue(sofa_queue, standing_customer_id);
                
                Customer* standing_customer = find_customer_by_id(standing_customer_id);
                if (standing_customer) {
                    printf("Cliente %d foi movido para o sofá.\n", standing_customer_id);
                    pthread_cond_signal(&standing_customer->waiting_for_sofa); // Acorda o cliente para que ele possa se sentar no sofá
                }
            }

            printf("Barbeiro %d está cortando cabelo do cliente %d.\n", barber_id, customer_id);
            pthread_mutex_unlock(&mutex);
            
            // Simula tempo de corte (tempo aleatório entre 3 e 6 segundos)
            sleep(random_time(5, 10));
            
            // Após o corte, realiza o pagamento
            pthread_mutex_lock(&payment_mutex);  // Garante que apenas um pagamento seja feito por vez

            // Simula tempo de pagamento (tempo aleatório entre 1 e 3 segundos)
            printf("Barbeiro %d está realizando o pagamento do cliente %d.\n", barber_id, customer_id);
            sleep(random_time(2, 3));

            // Sinaliza que o cliente pode sair
            Customer* served_customer = find_customer_by_id(customer_id);
            if (served_customer) {
                pthread_cond_signal(&served_customer->waiting_for_service);  // Libera o cliente para sair
            }

            printf("Barbeiro %d finalizou o pagamento do cliente %d.\n", barber_id, customer_id);
            pthread_mutex_unlock(&payment_mutex);  // Libera mutex do pagamento
        }


        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Usage: %s <MAX_CUSTOMERS> <NUM_BARBERS> <MAX_PEOPLE_IN_SHOP> <SOFA_CAPACITY> <MAX_SIZE>\n", argv[0]);
        return 1;
    }

    // Recebe os parâmetros via linha de comando
    MAX_CUSTOMERS = atoi(argv[1]);
    NUM_BARBERS = atoi(argv[2]);
    MAX_PEOPLE_IN_SHOP = atoi(argv[3]);
    SOFA_CAPACITY = atoi(argv[4]);

    // Aloca dinamicamente o espaço para os clientes e para as filas
    customer_data = (Customer *)malloc(MAX_CUSTOMERS * sizeof(Customer));
    standing_queue = (Queue *)malloc(sizeof(Queue));
    sofa_queue = (Queue *)malloc(sizeof(Queue));

    // Inicializa as filas
    initializeQueue(standing_queue, MAX_PEOPLE_IN_SHOP);
    initializeQueue(sofa_queue, SOFA_CAPACITY);

    pthread_t barbers[NUM_BARBERS];
    pthread_t customers[MAX_CUSTOMERS];

    // Semente para números aleatórios
    srand(time(NULL));

    // Cria as threads dos barbeiros
    for (int i = 0; i < NUM_BARBERS; i++) {
        int *barber_id = (int *)malloc(sizeof(int));
        *barber_id = i + 1;
        pthread_create(&barbers[i], NULL, barber_func, barber_id);
    }

    // Cria as threads dos clientes
    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        customer_data[i].id = i + 1;
        pthread_cond_init(&customer_data[i].waiting_for_service, NULL);
        pthread_cond_init(&customer_data[i].waiting_for_sofa, NULL);
        pthread_create(&customers[i], NULL, customer_func, &customer_data[i]);

        // Adiciona um intervalo aleatório de tempo entre as chegadas dos clientes
        sleep(random_time(1, 3));  // Tempo aleatório entre 1 e 3 segundos para a chegada do cliente
    }

    // Espera as threads dos clientes terminarem
    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        pthread_join(customers[i], NULL);
    }

    // Define que o trabalho foi concluído, permitindo que os barbeiros saiam
    work_done = true;
    pthread_cond_broadcast(&work_available);  // Acorda todos os barbeiros restantes

    printf("Todos os clientes foram atendidos.\n");

    // Cancela as threads dos barbeiros
    for (int i = 0; i < NUM_BARBERS; i++) {
        pthread_cancel(barbers[i]);
    }

    // Libera as variáveis de condição
    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        pthread_cond_destroy(&customer_data[i].waiting_for_service);
        pthread_cond_destroy(&customer_data[i].waiting_for_sofa);
    }
    
    // Libera os mutexes
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&payment_mutex);

    // Libera a memória alocada
    free(customer_data);
    free(standing_queue->items);
    free(standing_queue);
    free(sofa_queue->items);
    free(sofa_queue);

    return 0;
}
