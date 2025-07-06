#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

// Definições de constantes
int MAX_CUSTOMERS;
int NUM_BARBERS;
int MAX_PEOPLE_IN_SHOP;
int SOFA_CAPACITY;

// Estrutura do Cliente
typedef struct {
    int id;
    pthread_cond_t service_cond;
    pthread_cond_t sofa_cond;
} Customer;

// Estrutura da Fila
typedef struct {
    int *items;
    int front;
    int rear;
    int count;
    int capacity;
} Queue;

// Variáveis globais
Customer *customer_data;
Queue *standing_queue;
Queue *sofa_queue;
int sofa_count = 0;
int num_people_in_shop = 0;
int standing_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t payment_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t work_available = PTHREAD_COND_INITIALIZER;
int work_done = 0;

// Função para inicializar fila
void inicializarFila(Queue *q, int tamanho) {
    q->items = (int*)malloc(tamanho * sizeof(int));
    q->front = 0;
    q->rear = 0;
    q->count = 0;
    q->capacity = tamanho;
}

// Função para verificar se a fila está vazia
int isEmpty(Queue *q) {
    return q->count == 0;
}

// Função para verificar se a fila está cheia
int isFull(Queue *q) {
    return q->count == q->capacity;
}

// Função para enfileirar
void enqueue(Queue *q, int valor) {
    if (!isFull(q)) {
        q->items[q->rear] = valor;
        q->rear = (q->rear + 1) % q->capacity;
        q->count++;
    }
}

// Função para desenfileirar
int dequeue(Queue *q) {
    if (!isEmpty(q)) {
        int valor = q->items[q->front];
        q->front = (q->front + 1) % q->capacity;
        q->count--;
        return valor;
    }
    return -1;
}

// Função para ver o primeiro elemento da fila
int peek(Queue *q) {
    if (!isEmpty(q)) {
        return q->items[q->front];
    }
    return -1;
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

// Função para gerar tempo aleatório
int random_time(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Função do cliente
void* customer_func(void* arg) {
    Customer *cliente = (Customer*)arg;
    
    printf("Cliente %d chegou\n", cliente->id);
    
    pthread_mutex_lock(&mutex);
    
    // Verificar se a barbearia está lotada
    if (num_people_in_shop >= MAX_PEOPLE_IN_SHOP) {
        printf("Cliente %d foi embora (barbearia lotada)\n", cliente->id);
        pthread_mutex_unlock(&mutex);
        return NULL;
    }
    
    num_people_in_shop++;
    
    // Verificar se há espaço no sofá
    if (sofa_count < SOFA_CAPACITY) {
        // Cliente senta no sofá
        sofa_count++;
        enqueue(sofa_queue, cliente->id);
        printf("Cliente %d sentou no sofá\n", cliente->id);
        pthread_cond_broadcast(&work_available);
    } else {
        // Cliente fica em pé
        standing_count++;
        enqueue(standing_queue, cliente->id);
        printf("Cliente %d ficou em pé\n", cliente->id);
        // Espera por espaço no sofá
        pthread_cond_wait(&cliente->sofa_cond, &mutex);
    }
    
    // Espera pelo serviço
    pthread_cond_wait(&cliente->service_cond, &mutex);
    
    num_people_in_shop--;
    pthread_mutex_unlock(&mutex);
    
    printf("Cliente %d saiu\n", cliente->id);
    return NULL;
}

// Função do barbeiro
void* barber_func(void* arg) {
    int barber_id = *(int*)arg;
    
    while (!work_done) {
        pthread_mutex_lock(&mutex);
        
        // Verificar se há clientes no sofá
        while (isEmpty(sofa_queue) && !work_done) {
            printf("Barbeiro %d está dormindo\n", barber_id);
            pthread_cond_wait(&work_available, &mutex);
        }
        
        if (work_done) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        // Há cliente no sofá
        if (!isEmpty(sofa_queue)) {
            int customer_id = dequeue(sofa_queue);
            sofa_count--;
            
            Customer *cliente = find_customer_by_id(customer_id);
            printf("Barbeiro %d está cortando cabelo do cliente %d\n", barber_id, customer_id);
            
            // Verificar se há clientes em pé e espaço no sofá
            if (!isEmpty(standing_queue) && sofa_count < SOFA_CAPACITY) {
                int waiting_customer_id = dequeue(standing_queue);
                standing_count--;
                sofa_count++;
                enqueue(sofa_queue, waiting_customer_id);
                
                Customer *waiting_customer = find_customer_by_id(waiting_customer_id);
                printf("Cliente %d se moveu para o sofá\n", waiting_customer_id);
                pthread_cond_signal(&waiting_customer->sofa_cond);
            }
            
            pthread_mutex_unlock(&mutex);
            
            // Simular corte de cabelo
            int cut_time = random_time(5, 10);
            sleep(cut_time);
            
            // Processo de pagamento
            pthread_mutex_lock(&payment_mutex);
            printf("Barbeiro %d recebendo pagamento do cliente %d\n", barber_id, customer_id);
            sleep(2); // Simular tempo de pagamento
            pthread_cond_signal(&cliente->service_cond);
            pthread_mutex_unlock(&payment_mutex);
        } else {
            pthread_mutex_unlock(&mutex);
        }
    }
    
    printf("Barbeiro %d terminou o trabalho\n", barber_id);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Uso: %s <MAX_CUSTOMERS> <NUM_BARBERS> <MAX_PEOPLE_IN_SHOP> <SOFA_CAPACITY>\n", argv[0]);
        return 1;
    }
    
    // Receber parâmetros via linha de comando
    MAX_CUSTOMERS = atoi(argv[1]);
    NUM_BARBERS = atoi(argv[2]);
    MAX_PEOPLE_IN_SHOP = atoi(argv[3]);
    SOFA_CAPACITY = atoi(argv[4]);
    
    // Inicializar gerador de números aleatórios
    srand(time(NULL));
    
    // Alocar memória para estruturas
    customer_data = (Customer*)malloc(MAX_CUSTOMERS * sizeof(Customer));
    standing_queue = (Queue*)malloc(sizeof(Queue));
    sofa_queue = (Queue*)malloc(sizeof(Queue));
    
    // Inicializar filas
    inicializarFila(standing_queue, MAX_PEOPLE_IN_SHOP);
    inicializarFila(sofa_queue, SOFA_CAPACITY);
    
    // Inicializar dados dos clientes
    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        customer_data[i].id = i + 1;
        pthread_cond_init(&customer_data[i].service_cond, NULL);
        pthread_cond_init(&customer_data[i].sofa_cond, NULL);
    }
    
    // Criar threads dos barbeiros
    pthread_t *barber_threads = (pthread_t*)malloc(NUM_BARBERS * sizeof(pthread_t));
    int *barber_ids = (int*)malloc(NUM_BARBERS * sizeof(int));
    
    for (int i = 0; i < NUM_BARBERS; i++) {
        barber_ids[i] = i + 1;
        pthread_create(&barber_threads[i], NULL, barber_func, &barber_ids[i]);
    }
    
    // Criar threads dos clientes
    pthread_t *customer_threads = (pthread_t*)malloc(MAX_CUSTOMERS * sizeof(pthread_t));
    
    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        pthread_create(&customer_threads[i], NULL, customer_func, &customer_data[i]);
        sleep(random_time(1, 3)); // Intervalo entre chegadas dos clientes
    }
    
    // Esperar as threads dos clientes terminarem
    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        pthread_join(customer_threads[i], NULL);
    }
    
    // Sinalizar que o trabalho foi concluído
    work_done = 1;
    pthread_cond_broadcast(&work_available);
    
    printf("Todos os clientes foram atendidos\n");
    
    // Esperar barbeiros terminarem
    for (int i = 0; i < NUM_BARBERS; i++) {
        pthread_join(barber_threads[i], NULL);
    }
    
    // Liberar recursos
    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        pthread_cond_destroy(&customer_data[i].service_cond);
        pthread_cond_destroy(&customer_data[i].sofa_cond);
    }
    
    free(customer_data);
    free(standing_queue->items);
    free(sofa_queue->items);
    free(standing_queue);
    free(sofa_queue);
    free(barber_threads);
    free(barber_ids);
    free(customer_threads);
    
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&payment_mutex);
    pthread_cond_destroy(&work_available);
    
    return 0;
}