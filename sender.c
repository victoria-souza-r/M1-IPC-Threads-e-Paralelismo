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

    // 1) Garante a existência do FIFO
    if (mkfifo(fifo_path, 0666) == -1) {
        if (errno != EEXIST) {
            perror("[Sender] Erro ao criar FIFO");
            return 1;
        }
    }

    // 2) Lê a imagem
    PGM img;
    printf("[Sender] Lendo a imagem %s...\n", inpath);
    if (!read_pgm(inpath, &img)) {
        fprintf(stderr, "[Sender] Falha ao ler imagem.\n");
        return 1;
    }

    // 3) Prepara cabeçalho
    Header header;
    header.w = img.w;
    header.h = img.h;
    header.maxv = img.maxv;
    header.mode = MODE_NEG; // valor padrão (worker decide depois)
    header.t1 = 0;
    header.t2 = 0;

    // 4) Abre FIFO (bloqueia até worker abrir)
    printf("[Sender] Aguardando o Worker se conectar ao FIFO '%s'...\n", fifo_path);
    int fd = open(fifo_path, O_WRONLY);
    if (fd == -1) {
        perror("[Sender] Erro ao abrir FIFO");
        free_pgm(&img);
        return 1;
    }

    printf("[Sender] Worker conectado! Enviando dados...\n");

    // 5) Envia o header (garantindo envio completo)
    size_t header_size = sizeof(Header);
    ssize_t sent = 0;

    while (sent < (ssize_t)header_size) {
        ssize_t n = write(fd, ((char*)&header) + sent, header_size - sent);
        if (n <= 0) {
            perror("[Sender] Erro ao enviar cabeçalho");
            close(fd);
            free_pgm(&img);
            return 1;
        }
        sent += n;
    }

    // 6) Envia os pixels (garantindo envio completo)
    size_t data_size = (size_t)img.w * img.h;
    ssize_t total = 0;

    while (total < (ssize_t)data_size) {
        ssize_t n = write(fd, img.data + total, data_size - total);
        if (n <= 0) {
            perror("[Sender] Erro ao enviar pixels");
            close(fd);
            free_pgm(&img);
            return 1;
        }
        total += n;
    }

    // 7) Finaliza
    close(fd);
    free_pgm(&img);

    printf("[Sender] Envio concluído com sucesso.\n");
    return 0;
}
