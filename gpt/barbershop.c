#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int MAX_CUSTOMERS;
int NUM_BARBERS;
int MAX_PEOPLE_IN_SHOP;
int SOFA_CAPACITY;

// Definindo as variáveis globais
int sofa_count = 0, num_people_in_shop = 0, standing_count = 0;  // Contadores
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex principal
pthread_mutex_t payment_mutex = PTHREAD_MUTEX_INITIALIZER;  // Mutex para pagamento
pthread_cond_t work_available = PTHREAD_COND_INITIALIZER;  // Condição para sinalizar o trabalho
int work_done = 0;  // Flag para finalizar o trabalho

// Estrutura de cliente
typedef struct {
    int id;
    pthread_cond_t waiting_for_service;
    pthread_cond_t waiting_for_sofa;
} Customer;

// Estrutura de fila
typedef struct {
    Customer **items;
    int front;
    int rear;
    int count;
    int capacity;
} Queue;

// Variáveis globais para as filas de clientes
Customer *customer_data;
Queue *standing_queue, *sofa_queue;

// Função para inicializar a fila
void initialize_queue(Queue *q, int size) {
    q->capacity = size;
    q->items = (Customer **)malloc(sizeof(Customer *) * size);  // Aloca a fila
    q->front = q->rear = q->count = 0;  // Inicializa os índices e o contador
}

// Função para verificar se a fila está vazia
int is_empty(Queue *q) {
    return q->count == 0;  // Se o contador for 0, a fila está vazia
}

// Função para verificar se a fila está cheia
int is_full(Queue *q) {
    return q->count == q->capacity;  // Se o contador for igual à capacidade, a fila está cheia
}

// Função para adicionar um cliente à fila
void enqueue(Queue *q, Customer *customer) {
    if (!is_full(q)) {
        q->items[q->rear] = customer;  // Adiciona o cliente ao final da fila
        q->rear = (q->rear + 1) % q->capacity;  // Atualiza o índice do final da fila
        q->count++;  // Incrementa o contador de elementos na fila
    }
}

// Função para remover e retornar o cliente da frente da fila
Customer* dequeue(Queue *q) {
    if (!is_empty(q)) {
        Customer *customer = q->items[q->front];  // Pega o cliente da frente
        q->front = (q->front + 1) % q->capacity;  // Atualiza o índice do início da fila
        q->count--;  // Decrementa o contador de elementos na fila
        return customer;  // Retorna o cliente removido
    }
    return NULL;  // Retorna NULL se a fila estiver vazia
}

// Função para gerar um número aleatório dentro de um intervalo
int random_time(int min, int max) {
    return rand() % (max - min + 1) + min;  // Retorna um número aleatório entre min e max
}

// Função que simula a ação do cliente
void *customer_func(void *arg) {
    Customer *customer = (Customer *)arg;
    printf("Cliente %d chegou na loja.\n", customer->id);

    pthread_mutex_lock(&mutex);  // Adquire o mutex para acesso exclusivo

    if (num_people_in_shop >= MAX_PEOPLE_IN_SHOP) {  // Se a loja estiver cheia
        printf("Cliente %d foi embora, loja cheia.\n", customer->id);
        pthread_mutex_unlock(&mutex);  // Libera o mutex
        return NULL;  // Cliente vai embora
    }

    num_people_in_shop++;  // Incrementa o número de pessoas na loja

    // Verifica se há espaço no sofá para o cliente
    if (sofa_count < SOFA_CAPACITY) {
        printf("Cliente %d sentou no sofá. Pessoas no sofá: %d\n", customer->id, sofa_count + 1);
        sofa_count++;  // Incrementa o número de pessoas no sofá
        enqueue(sofa_queue, customer);  // Coloca o cliente na fila do sofá
        pthread_cond_signal(&work_available);  // Sinaliza um barbeiro
    } else {  // Caso o sofá esteja cheio, o cliente fica em pé
        printf("Cliente %d ficou em pé.\n", customer->id);
        standing_count++;  // Incrementa o número de clientes em pé
        enqueue(standing_queue, customer);  // Coloca o cliente na fila dos clientes em pé
        pthread_cond_wait(&customer->waiting_for_sofa, &mutex);  // Espera até que o sofá tenha espaço
    }

    pthread_cond_wait(&customer->waiting_for_service, &mutex);  // Espera o serviço ser concluído

    num_people_in_shop--;  // Decrementa o número de pessoas na loja
    printf("Cliente %d saiu.\n", customer->id);
    pthread_mutex_unlock(&mutex);  // Libera o mutex

    return NULL;  // Fim da execução da thread do cliente
}

// Função que simula a ação do barbeiro
void *barber_func(void *arg) {
    int barber_id = *(int *)arg;  // Obtém o identificador do barbeiro
    while (!work_done) {  // Enquanto o trabalho não for concluído
        pthread_mutex_lock(&mutex);  // Adquire o mutex para acesso exclusivo
        if (sofa_count == 0) {  // Se não houver clientes no sofá
            printf("Barbeiro %d está dormindo, esperando trabalho.\n", barber_id);
            pthread_cond_wait(&work_available, &mutex);  // O barbeiro dorme até que haja trabalho
        }

        Customer *customer = dequeue(sofa_queue);  // Remove o próximo cliente da fila do sofá
        if (customer) {
            sofa_count--;  // Decrementa o número de pessoas no sofá
            printf("Barbeiro %d está cortando o cabelo do cliente %d.\n", barber_id, customer->id);

            // Tenta mover um cliente em pé para o sofá assim que um cliente é selecionado
            if (!is_empty(standing_queue) && sofa_count < SOFA_CAPACITY) {
                Customer *next_customer = dequeue(standing_queue);  // Remove o próximo cliente em pé
                sofa_count++;  // Incrementa o número de pessoas no sofá
                enqueue(sofa_queue, next_customer);  // Coloca o cliente no sofá
                printf("Barbeiro %d moveu o cliente %d da fila de pé para o sofá. Pessoas no sofá: %d\n", barber_id, next_customer->id, sofa_count);
                pthread_cond_signal(&next_customer->waiting_for_sofa);  // Sinaliza para o cliente sentar no sofá
            }

            printf("Pessoas no sofá agora: %d\n", sofa_count);

            pthread_mutex_unlock(&mutex);  // Libera o mutex
            sleep(random_time(10, 15));  // Aumenta o tempo de corte para 10-15 segundos
            pthread_mutex_lock(&payment_mutex);  // Adquire o mutex de pagamento
            printf("Barbeiro %d está realizando o pagamento do cliente %d.\n", barber_id, customer->id);
            sleep(random_time(1, 2));  // Simula o tempo de espera pelo pagamento
            pthread_cond_signal(&customer->waiting_for_service);  // Sinaliza para o cliente que pode sair
            pthread_mutex_unlock(&payment_mutex);  // Libera o mutex de pagamento
        }
        pthread_mutex_unlock(&mutex);  // Libera o mutex
    }
    return NULL;  // Fim da execução da thread do barbeiro
}

int main(int argc, char *argv[]) {
    if (argc != 5) {  // Verifica se o número de parâmetros passados é correto
        printf("Uso incorreto. Passe os parâmetros: MAX_CUSTOMERS NUM_BARBERS MAX_PEOPLE_IN_SHOP SOFA_CAPACITY\n");
        return 1;
    }

    // Lê os parâmetros passados via linha de comando
    MAX_CUSTOMERS = atoi(argv[1]);
    NUM_BARBERS = atoi(argv[2]);
    MAX_PEOPLE_IN_SHOP = atoi(argv[3]);
    SOFA_CAPACITY = atoi(argv[4]);

    srand(time(NULL));  // Inicializa o gerador de números aleatórios

    // Aloca memória para os dados dos clientes
    customer_data = (Customer *)malloc(sizeof(Customer) * MAX_CUSTOMERS);
    standing_queue = (Queue *)malloc(sizeof(Queue));
    sofa_queue = (Queue *)malloc(sizeof(Queue));

    initialize_queue(standing_queue, MAX_PEOPLE_IN_SHOP);  // Inicializa a fila dos clientes em pé
    initialize_queue(sofa_queue, SOFA_CAPACITY);  // Inicializa a fila dos clientes no sofá

    // Cria as threads dos barbeiros
    pthread_t barber_threads[NUM_BARBERS];
    int barber_ids[NUM_BARBERS];
    for (int i = 0; i < NUM_BARBERS; i++) {
        barber_ids[i] = i + 1;
        pthread_create(&barber_threads[i], NULL, barber_func, &barber_ids[i]);
    }

    // Cria as threads dos clientes com tempo aleatório
    pthread_t customer_threads[MAX_CUSTOMERS];
    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        customer_data[i].id = i + 1;
        sleep(random_time(1, 2));  // Simula o tempo de chegada dos clientes
        pthread_create(&customer_threads[i], NULL, customer_func, &customer_data[i]);
    }

    // Espera as threads dos clientes terminarem
    for (int i = 0; i < MAX_CUSTOMERS; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    // Sinaliza que o trabalho foi concluído
    work_done = 1;
    pthread_cond_broadcast(&work_available);  // Sinaliza todos os barbeiros

    // Cancela as threads dos barbeiros
    for (int i = 0; i < NUM_BARBERS; i++) {
        pthread_join(barber_threads[i], NULL);
    }

    // Libera os recursos
    free(customer_data);
    free(standing_queue->items);
    free(sofa_queue->items);
    free(standing_queue);
    free(sofa_queue);

    printf("Todos os clientes foram atendidos.\n");

    return 0;
}
