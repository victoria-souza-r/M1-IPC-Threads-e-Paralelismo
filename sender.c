#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      // Manipulação de controle de arquivos (O_WRONLY)
#include <sys/stat.h>   // Definições de estados de arquivos (mkfifo)
#include <sys/types.h>  // Tipos de dados de sistema
#include <unistd.h>     // Chamadas de sistema padrão (write, close)
#include <errno.h>      // Tratamento de erros do sistema
#include "pgm.h"

int main(int argc, char** argv) {
    // Validação de argumentos: espera o caminho do FIFO e a imagem PGM
    if (argc < 3) {
        fprintf(stderr, "Uso correto: %s <fifo_path> <entrada.pgm>\n", argv[0]);
        return 1;
    }

    const char* fifo_path = argv[1];
    const char* inpath = argv[2];

    // 1) CRIAÇÃO DO FIFO (IPC - Comunicação entre Processos)
    // mkfifo cria um "pipe nomeado" no sistema de arquivos com permissão 0666 (leitura/escrita)
    if (mkfifo(fifo_path, 0666) == -1) {
        // Se o erro for EEXIST, significa que o FIFO já existe
        if (errno != EEXIST) {
            perror("[Sender] Erro ao criar FIFO");
            return 1;
        }
    }

    // 2) LEITURA DA IMAGEM DO DISCO
    PGM img;
    printf("[Sender] Lendo a imagem %s...\n", inpath);
    if (!read_pgm(inpath, &img)) {
        fprintf(stderr, "[Sender] Falha ao ler imagem.\n");
        return 1;
    }

    // 3) PREPARAÇÃO DO CABEÇALHO (Metadados para o Worker)
    Header header;
    header.w = img.w;       // Largura
    header.h = img.h;       // Altura
    header.maxv = img.maxv; // Valor máximo de cinza (normalmente 255)
    header.mode = MODE_NEG; // Modo padrão enviado (será sobrescrito pelo Worker)
    header.t1 = 0;
    header.t2 = 0;

    // 4) ABERTURA DO FIFO PARA ESCRITA
    // O comando open() em um FIFO bloqueia a execução até que outro processo (Worker) abra para leitura
    printf("[Sender] Aguardando o Worker se conectar ao FIFO '%s'...\n", fifo_path);
    int fd = open(fifo_path, O_WRONLY); 
    if (fd == -1) {
        perror("[Sender] Erro ao abrir FIFO");
        free_pgm(&img);
        return 1;
    }

    printf("[Sender] Worker conectado! Enviando dados...\n");

    // 5) ENVIO DO CABEÇALHO (Header)
    // Usamos um loop while para garantir que todos os bytes do struct sejam transmitidos,
    // tratando possíveis interrupções no fluxo do pipe.
    size_t header_size = sizeof(Header);
    ssize_t sent = 0;

    while (sent < (ssize_t)header_size) {
        // Escreve no descritor de arquivo (fd) do FIFO
        ssize_t n = write(fd, ((char*)&header) + sent, header_size - sent);
        if (n <= 0) {
            perror("[Sender] Erro ao enviar cabeçalho");
            close(fd);
            free_pgm(&img);
            return 1;
        }
        sent += n;
    }

    // 6) ENVIO DOS PIXELS (Dados brutos da imagem)
    // Transmissão da matriz de pixels (largura * altura bytes)
    size_t data_size = (size_t)img.w * img.h;
    ssize_t total = 0;

    while (total < (ssize_t)data_size) {
        // Envia blocos de dados até que o tamanho total da imagem seja atingido
        ssize_t n = write(fd, img.data + total, data_size - total);
        if (n <= 0) {
            perror("[Sender] Erro ao enviar pixels");
            close(fd);
            free_pgm(&img);
            return 1;
        }
        total += n;
    }

    // 7) ENCERRAMENTO
    // Fecha o descritor do FIFO e libera a memória alocada para a imagem
    close(fd);
    free_pgm(&img);

    printf("[Sender] Envio concluído com sucesso.\n");
    return 0;
}
