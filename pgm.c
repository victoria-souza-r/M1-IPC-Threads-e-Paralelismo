#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pgm.h"

// FUNÇÃO AUXILIAR: skip_comments
// Finalidade: Ignorar espaços em branco e comentários que 
// começam com '#' no cabeçalho do arquivo PGM.

static void skip_comments(FILE* f) {
    int ch;
    // fgetc lê caractere por caractere do arquivo
    while ((ch = fgetc(f)) != EOF) {
        if (isspace(ch)) {
            continue; // Ignora espaços, tabs e quebras de linha
        }
        if (ch == '#') {
            // Se encontrar '#', lê até o final da linha e ignora tudo
            while ((ch = fgetc(f)) != EOF && ch != '\n');
        } else {
            // Se não for espaço nem comentário, devolve o caractere 
            // para o fluxo do arquivo para ser lido pela próxima função
            ungetc(ch, f);
            break;
        }
    }
}

// LEITURA PGM (P5 - Formato Binário em Tons de Cinza)
int read_pgm(const char* path, PGM* img) {
    // Abre o arquivo no modo de leitura binária ("rb")
    FILE* f = fopen(path, "rb");
    if (!f) {
        perror("Erro ao abrir arquivo para leitura");
        return 0;
    }

    char format[3];

    // 1. Lê a assinatura (Magic Number). Deve ser "P5" para binário.
    if (fscanf(f, "%2s", format) != 1) {
        fclose(f);
        return 0;
    }

    if (strcmp(format, "P5") != 0) {
        fprintf(stderr, "Formato inválido (%s)! Use PGM P5.\n", format);
        fclose(f);
        return 0;
    }

    // 2. Lê as dimensões (Largura e Altura) ignorando comentários
    skip_comments(f);
    if (fscanf(f, "%d %d", &img->w, &img->h) != 2) {
        fclose(f);
        return 0;
    }

    // 3. Lê o valor máximo de intensidade (maxv), geralmente 255
    skip_comments(f);
    if (fscanf(f, "%d", &img->maxv) != 1) {
        fclose(f);
        return 0;
    }

    // 4. Consumir o único caractere de espaço/quebra de linha 
    // que separa o cabeçalho (texto) dos dados (binários)
    int ch;
    do {
        ch = fgetc(f);
    } while (isspace(ch));
    ungetc(ch, f); // Devolve o primeiro byte de dados reais

    // 5. Aloca memória dinamicamente para os pixels
    // Cada pixel no P5 ocupa 1 byte (uchar)
    size_t size = (size_t)img->w * img->h;
    img->data = (uchar*) malloc(size);
    if (!img->data) {
        fprintf(stderr, "Erro: falha ao alocar memória.\n");
        fclose(f);
        return 0;
    }

    // 6. Lê o bloco de dados binários de uma única vez (eficiência)
    size_t n = fread(img->data, 1, size, f);
    if (n != size) {
        fprintf(stderr, "Erro: leitura incompleta (%zu de %zu bytes).\n", n, size);
        free(img->data); // Limpa memória em caso de erro
        fclose(f);
        return 0;
    }

    fclose(f);
    return 1; // Sucesso
}

// ESCRITA PGM (P5 - Formato Binário)
int write_pgm(const char* path, const PGM* img) {
    // Abre o arquivo no modo de escrita binária ("wb")
    FILE* f = fopen(path, "wb");
    if (!f) {
        perror("Erro ao abrir arquivo para escrita");
        return 0;
    }

    // Escreve o cabeçalho no formato padrão: Tipo, Dimensões e MaxV
    fprintf(f, "P5\n%d %d\n%d\n", img->w, img->h, img->maxv);

    size_t size = (size_t)img->w * img->h;

    // Escreve o array de dados binários diretamente da memória para o disco
    size_t n = fwrite(img->data, 1, size, f);
    if (n != size) {
        fprintf(stderr, "Erro ao escrever pixels (%zu de %zu bytes).\n", n, size);
        fclose(f);
        return 0;
    }

    fclose(f);
    return 1; // Sucesso
}

// LIBERAR MEMÓRIA
// Finalidade: Evitar vazamento de memória (Memory Leak)
void free_pgm(PGM* img) {
    if (img && img->data) {
        free(img->data); // Libera o ponteiro alocado no malloc
        img->data = NULL; // Evita ponteiro solto (dangling pointer)
    }
}
