#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <semaphore.h>
#include <stddef.h>

/**
 * Estrutura Task (Tarefa)
 * Define o subconjunto de dados que uma thread irá processar.
 * No contexto deste projeto, representa um bloco de linhas da imagem.
 */
typedef struct {
    int row_start; // Índice da linha inicial (inclusive)
    int row_end;   // Índice da linha final (exclusive)
} Task;

// Tamanho máximo da fila circular
#define QMAX 128

/**
 * Estrutura Queue (Fila Circular)
 * Gerencia a distribuição de tarefas entre a thread produtora (Main)
 * e as threads consumidoras (Pool de Threads).
 */
typedef struct {
    Task tasks[QMAX]; // Buffer estático para armazenar as tarefas
    size_t head;      // Aponta para o próximo item a ser removido (pop)
    size_t tail;      // Aponta para a próxima posição livre (push)
    size_t count;     // Quantidade atual de itens na fila

    int closed;       // Sinalizador: 1 se a fila foi encerrada, 0 caso contrário

    // Mecanismos de Sincronização
    pthread_mutex_t lock; // Garante exclusão mútua ao manipular head, tail e count
    sem_t sem_items;      // Semáforo contador: indica quantos itens estão prontos para consumo
    sem_t sem_space;      // Semáforo contador: indica quantos espaços estão livres para produção
} Queue;

/**
 * Inicializa a fila, zerando índices e configurando Mutex e Semáforos.
 */
void queue_init(Queue* q);

/**
 * Adiciona uma tarefa à fila (Produtor).
 * Bloqueia se a fila estiver cheia (aguarda sem_space).
 * Retorna 0 em caso de sucesso ou -1 se a fila estiver fechada.
 */
int  queue_push(Queue* q, Task t);

/**
 * Remove uma tarefa da fila para processamento (Consumidor).
 * Bloqueia se a fila estiver vazia (aguarda sem_items).
 * Retorna 1 se obteve uma tarefa, 0 se a fila fechou e está vazia.
 */
int  queue_pop(Queue* q, Task* out);

/**
 * Marca a fila como fechada. 
 * Usado para sinalizar às threads que não haverá novas tarefas.
 */
void queue_close(Queue* q);

/**
 * Libera os recursos de sincronização (Mutex e Semáforos) alocados no SO.
 */
void queue_destroy(Queue* q);

#endif
