#ifndef PGM_H
#define PGM_H
#define MODE_NEG 0
#define MODE SLICE 1
#include <stdlib.h>

// Facilitador para não precisar digitar 'unsigned char' toda vez
typedef unsigned char uchar;

// Estrutura da Imagem
typedef struct {
    int w, h, maxv;
    uchar* data; // O asterisco indica que é um vetor de pixels
} PGM;

// Estrutura de Metadados para o FIFO (Conforme Seção 2 do PDF)
typedef struct {
    int w, h, maxv;
    int mode;   // 0=NEGATIVO, 1=SLICE
    int t1, t2; // Limites para o SLICE (válido se mode=SLICE)
} Header;

// Protótipos das funções de I/O
int read_pgm(const char* path, PGM* img);
int write_pgm(const char* path, const PGM* img);

// Função para liberar a memória alocada dinamicamente
void free_pgm(PGM* img);

#endif
