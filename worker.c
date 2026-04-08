#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "pgm.h"
#include "filters.h"
#include "queue.h"

// Variáveis Globais (Compartilhadas entre as threads)
PGM g_in, g_out;      // Estruturas para as imagens de entrada e saída
Queue g_queue;        // Fila de tarefas (monitorada por mutex/semáforo)
Header g_header;      // Metadados da imagem (largura, altura, etc)
int g_nthreads = 4;   // Quantidade de threads no pool


// Função da Thread (Consumidor)
void* worker_thread(void* arg) {
    Task task;

    while (1) {
        // Tenta retirar uma tarefa da fila. Se a fila estiver vazia e fechada, retorna != 0
        if (queue_pop(&g_queue, &task) != 0)
            break;

        // Aplica o filtro selecionado apenas no bloco de linhas designado pela tarefa
        if (g_header.mode == MODE_NEG) {
            apply_negative_block(&g_in, &g_out, task.row_start, task.row_end);

        } else if (g_header.mode == MODE_SLICE) {
            apply_slice_block(&g_in, &g_out, task.row_start, task.row_end,
                              g_header.t1, g_header.t2);
        }
    }

    return NULL;
}


// MAIN WORKER
int main(int argc, char** argv) {
    // Validação básica dos argumentos de linha de comando
    if (argc < 5) {
        fprintf(stderr, "Uso: %s <fifo> <threads> <neg|slice> [t1 t2] <saida.pgm>\n", argv[0]);
        return 1;
    }

    // Captura parâmetros iniciais
    const char* fifo_path = argv[1];
    g_nthreads = atoi(argv[2]);
    if (g_nthreads <= 0) g_nthreads = 4;

    const char* filter_type = argv[3];
    const char* out_path;

    int mode;
    int t1 = 0, t2 = 0;

  
    // Parsing dos argumentos (Decide o filtro e captura t1/t2)
    if (strcmp(filter_type, "neg") == 0) {
        mode = MODE_NEG;
        out_path = argv[4];

    } else if (strcmp(filter_type, "slice") == 0) {
        if (argc < 7) {
            fprintf(stderr, "Erro: slice requer t1 e t2\n");
            return 1;
        }
        mode = MODE_SLICE;
        t1 = atoi(argv[4]);
        t2 = atoi(argv[5]);
        out_path = argv[6];

    } else {
        fprintf(stderr, "Filtro inválido\n");
        return 1;
    }

   
    // 1. Abre FIFO (IPC - Inter-Process Communication)
    printf("[Worker] Abrindo FIFO %s...\n", fifo_path);

    // O open() bloqueia até que o Sender abra o FIFO para escrita
    int fd = open(fifo_path, O_RDONLY);
    if (fd == -1) {
        perror("Erro ao abrir FIFO");
        return 1;
    }

   
    // 2. Lê Metadados (Header) enviando pelo Sender
    size_t header_size = sizeof(Header);
    size_t total = 0;

    // Garante a leitura total do struct Header independente da fragmentação do duto
    while (total < header_size) {
        ssize_t n = read(fd, ((char*)&g_header) + total, header_size - total);
        if (n <= 0) {
            perror("Erro ao ler header");
            close(fd);
            return 1;
        }
        total += n;
    }

    printf("[Worker] Imagem recebida: %dx%d\n", g_header.w, g_header.h);

    // Configura o modo de processamento baseado nos argumentos do terminal
    g_header.mode = mode;
    g_header.t1 = t1;
    g_header.t2 = t2;

    
    // 3. Aloca memória para matrizes de pixels
    g_in.w = g_out.w = g_header.w;
    g_in.h = g_out.h = g_header.h;
    g_in.maxv = g_out.maxv = g_header.maxv;

    size_t img_size = (size_t)g_in.w * g_in.h;

    g_in.data = malloc(img_size);
    g_out.data = malloc(img_size);

    if (!g_in.data || !g_out.data) {
        fprintf(stderr, "Erro de memória\n");
        close(fd);
        return 1;
    }

  
    // 4. Lê Pixels (Dados da Imagem) do FIFO
    size_t total_read = 0;
    while (total_read < img_size) {
        ssize_t n = read(fd, g_in.data + total_read, img_size - total_read);
        if (n <= 0) {
            perror("Erro ao ler pixels");
            close(fd);
            return 1;
        }
        total_read += n;
    }

    close(fd); // Fecha o FIFO após a leitura completa

    
    // 5. Inicializa Fila e Pool de Threads
    queue_init(&g_queue);

    pthread_t* threads = malloc(sizeof(pthread_t) * g_nthreads);
    
    // Cria as threads consumidoras
    for (int i = 0; i < g_nthreads; i++) {
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }

   
    // 6. Produtor: Cria tarefas dividindo a imagem em blocos
    int chunk_size = 10; // Cada thread processa 10 linhas por vez

    for (int i = 0; i < g_in.h; i += chunk_size) {
        Task t;
        t.row_start = i;
        // Garante que o último bloco não ultrapasse a altura da imagem
        t.row_end = (i + chunk_size > g_in.h) ? g_in.h : i + chunk_size;

        queue_push(&g_queue, t); // Insere na fila (bloqueia se estiver cheia)
    }

   
    // 7. Sincroniza Finalização
    queue_close(&g_queue); // Sinaliza para as threads que não haverá mais tarefas

    for (int i = 0; i < g_nthreads; i++) {
        pthread_join(threads[i], NULL); // Aguarda cada thread terminar seu bloco
    }

   
    // 8. Salva o resultado final em disco (PGM P5)
    printf("[Worker] Salvando em %s...\n", out_path);
    write_pgm(out_path, &g_out);

    
    // 9. Limpeza de Recursos
    free(threads);
    free(g_in.data);
    free(g_out.data);
    queue_destroy(&g_queue);

    printf("[Worker] Finalizado com sucesso.\n");

    return 0;
}
