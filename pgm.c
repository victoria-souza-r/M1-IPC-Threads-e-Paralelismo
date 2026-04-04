#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pgm.h"

// Função auxiliar: pular espaços e comentários (# ...)
static void skip_comments(FILE* f) {
    int ch;
    while ((ch = fgetc(f)) != EOF) {
        if (isspace(ch)) {
            continue;
        }
        if (ch == '#') {
            while ((ch = fgetc(f)) != EOF && ch != '\n');
        } else {
            ungetc(ch, f);
            break;
        }
    }
}

// LEITURA PGM (P5 - Binário)
int read_pgm(const char* path, PGM* img) {
    FILE* f = fopen(path, "rb");
    if (!f) {
        perror("Erro ao abrir arquivo para leitura");
        return 0;
    }

    char format[3];

    // 1. Lê o tipo (P5)
    if (fscanf(f, "%2s", format) != 1) {
        fclose(f);
        return 0;
    }

    if (strcmp(format, "P5") != 0) {
        fprintf(stderr, "Formato inválido (%s)! Use PGM P5.\n", format);
        fclose(f);
        return 0;
    }

    // 2. Lê dimensões
    skip_comments(f);
    if (fscanf(f, "%d %d", &img->w, &img->h) != 2) {
        fclose(f);
        return 0;
    }

    // 3. Lê maxv
    skip_comments(f);
    if (fscanf(f, "%d", &img->maxv) != 1) {
        fclose(f);
        return 0;
    }

    // 4. Consumir qualquer whitespace antes dos dados binários
    int ch;
    do {
        ch = fgetc(f);
    } while (isspace(ch));
    ungetc(ch, f);

    // 5. Aloca memória com segurança
    size_t size = (size_t)img->w * img->h;
    img->data = (uchar*) malloc(size);
    if (!img->data) {
        fprintf(stderr, "Erro: falha ao alocar memória.\n");
        fclose(f);
        return 0;
    }

    // 6. Lê os pixels
    size_t n = fread(img->data, 1, size, f);
    if (n != size) {
        fprintf(stderr, "Erro: leitura incompleta (%zu de %zu bytes).\n", n, size);
        free(img->data);
        fclose(f);
        return 0;
    }

    fclose(f);
    return 1;
}

// ESCRITA PGM (P5 - Binário)
int write_pgm(const char* path, const PGM* img) {
    FILE* f = fopen(path, "wb");
    if (!f) {
        perror("Erro ao abrir arquivo para escrita");
        return 0;
    }

    // Cabeçalho padrão PGM
    fprintf(f, "P5\n%d %d\n%d\n", img->w, img->h, img->maxv);

    size_t size = (size_t)img->w * img->h;

    // Escreve pixels
    size_t n = fwrite(img->data, 1, size, f);
    if (n != size) {
        fprintf(stderr, "Erro ao escrever pixels (%zu de %zu bytes).\n", n, size);
        fclose(f);
        return 0;
    }

    fclose(f);
    return 1;
}
// LIBERAR MEMÓRIA
void free_pgm(PGM* img) {
    if (img && img->data) {
        free(img->data);
        img->data = NULL;
    }
}
