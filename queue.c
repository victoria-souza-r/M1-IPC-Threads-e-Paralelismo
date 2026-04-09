#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include "queue.h"


// Inicialização 
void queue_init(Queue* q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
    q->closed = 0;

    pthread_mutex_init(&q->lock, NULL);
    sem_init(&q->sem_items, 0, 0);
    sem_init(&q->sem_space, 0, QMAX);
}


// Inserção (Produtor)
int queue_push(Queue* q, Task t) {
    // espera espaço
    sem_wait(&q->sem_space);

    pthread_mutex_lock(&q->lock);

    if (q->closed) {
        pthread_mutex_unlock(&q->lock);
        sem_post(&q->sem_space); // devolve espaço
        return -1;
    }

    // insere
    q->tasks[q->tail] = t;
    q->tail = (q->tail + 1) % QMAX;
    q->count++;

    pthread_mutex_unlock(&q->lock);

    // sinaliza item disponível
    sem_post(&q->sem_items);

    return 0;
}

// Remoção (Consumidor)
int queue_pop(Queue* q, Task* out) {
    // espera item
    sem_wait(&q->sem_items);

    pthread_mutex_lock(&q->lock);

    // fila fechada e vazia → fim
    if (q->count == 0 && q->closed) {
        pthread_mutex_unlock(&q->lock);
        return -1;
    }

    // remove
    *out = q->tasks[q->head];
    q->head = (q->head + 1) % QMAX;
    q->count--;

    pthread_mutex_unlock(&q->lock);

    // libera espaço
    sem_post(&q->sem_space);

    return 0;
}


// Fechamento da fila
void queue_close(Queue* q) {
    pthread_mutex_lock(&q->lock);
    q->closed = 1;
    pthread_mutex_unlock(&q->lock);

    // acorda TODAS as threads bloqueadas
    for (int i = 0; i < QMAX; i++) {
        sem_post(&q->sem_items);
    }
}


// Destruição
void queue_destroy(Queue* q) {
    pthread_mutex_destroy(&q->lock);
    sem_destroy(&q->sem_items);
    sem_destroy(&q->sem_space);
}
