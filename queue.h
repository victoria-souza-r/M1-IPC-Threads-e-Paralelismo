#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>

// Define uma tarefa (intervalo de linhas)
typedef struct {
    int row_start;
    int row_end;
} Task;

#define QMAX 128

// Fila circular FIFO com sincronização
typedef struct {
    Task tasks[QMAX];
    size_t head;
    size_t tail;
    size_t count;

    int closed; // indica encerramento da fila

    pthread_mutex_t lock;
    sem_t sem_items; // itens disponíveis
    sem_t sem_space; // espaço disponível
} Queue;

// Protótipos
void queue_init(Queue* q);
int  queue_push(Queue* q, Task t);
int  queue_pop(Queue* q, Task* out);
void queue_close(Queue* q);
void queue_destroy(Queue* q);

#endif
