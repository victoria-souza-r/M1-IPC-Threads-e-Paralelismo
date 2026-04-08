#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "queue.h"

// Inicialização da Fila Circular
void queue_init(Queue* q) {
    q->head = 0;   // Índice de saída (consumo)
    q->tail = 0;   // Índice de entrada (produção)
    q->count = 0;  // Total de tarefas atuais na fila
    q->closed = 0; // Flag para indicar fim da transmissão

    // Inicializa o Mutex para garantir que apenas uma thread mexa nos índices por vez
    pthread_mutex_init(&q->lock, NULL);

    // Semáforo de itens: começa em 0 pois a fila inicia vazia
    sem_init(&q->sem_items, 0, 0);

    // Semáforo de espaço: começa com QMAX (tamanho da fila), indicando que todos os espaços estão livres
    sem_init(&q->sem_space, 0, QMAX);
}

// Inserção (Produtor - Processo Principal)
int queue_push(Queue* q, Task t) {
    // 1. Controle de Fluxo: Espera haver espaço livre na fila.
    // Se a fila estiver cheia, o produtor dorme aqui.
    sem_wait(&q->sem_space);

    // 2. Exclusão Mútua: Bloqueia o acesso para alterar a estrutura da fila.
    pthread_mutex_lock(&q->lock);

    // Verifica se a fila foi fechada antes da inserção
    if (q->closed) {
        pthread_mutex_unlock(&q->lock);
        sem_post(&q->sem_space); // Devolve o espaço que "tentou" ocupar
        return -1;
    }

    // 3. Inserção: Coloca a tarefa na posição 'tail' e atualiza o índice circular
    q->tasks[q->tail] = t;
    q->tail = (q->tail + 1) % QMAX;
    q->count++;

    // 4. Libera o Mutex: Outra thread agora pode acessar a fila.
    pthread_mutex_unlock(&q->lock);

    // 5. Sinalização: Avisa as threads trabalhadoras que há um novo item pronto para consumo.
    sem_post(&q->sem_items);

    return 0;
}

// Remoção (Consumidor - Threads Trabalhadoras)
int queue_pop(Queue* q, Task* out) {
    // 1. Controle de Fluxo: Espera haver pelo menos um item na fila.
    // Se a fila estiver vazia, a thread trabalhadora dorme aqui.
    sem_wait(&q->sem_items);

    // 2. Exclusão Mútua: Garante que apenas uma thread consuma o item da vez.
    pthread_mutex_lock(&q->lock);

    // Condição de saída: Se a fila estiver vazia e marcada como fechada, encerra a thread.
    if (q->count == 0 && q->closed) {
        pthread_mutex_unlock(&q->lock);
        return -1;
    }

    // 3. Remoção: Copia a tarefa da posição 'head' para o ponteiro de saída.
    *out = q->tasks[q->head];
    q->head = (q->head + 1) % QMAX;
    q->count--;

    // 4. Libera o Mutex: Permite acesso das outras threads.
    pthread_mutex_unlock(&q->lock);

    // 5. Sinalização de Espaço: Avisa ao produtor que um espaço foi liberado na fila.
    sem_post(&q->sem_space);

    return 0;
}


// Fechamento da Fila (Sinalização de Término)
void queue_close(Queue* q) {
    pthread_mutex_lock(&q->lock);
    q->closed = 1; // Marca que não haverá mais novas tarefas
    pthread_mutex_unlock(&q->lock);

    // Acorda TODAS as threads que possam estar dormindo no sem_wait(&q->sem_items).
    // Isso evita que threads fiquem travadas esperando itens que nunca virão.
    for (int i = 0; i < QMAX; i++) {
