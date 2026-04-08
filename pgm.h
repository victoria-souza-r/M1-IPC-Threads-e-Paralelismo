#ifndef PGM_H
#define PGM_H
 
#define MODE_NEG 0    // Representa o Filtro Negativo
#define MODE_SLICE 1  // Representa o Filtro de Fatiamento (Slice)

#include <stdlib.h>


// Atalho para 'unsigned char' (8 bits, 0-255), ideal para tons de cinza
typedef unsigned char uchar;

// Estrutura principal que armazena os dados da imagem PGM
typedef struct {
    int w;          // Largura (Width) da imagem em pixels
    int h;          // Altura (Height) da imagem em pixels
    int maxv;       // Valor máximo de intensidade (geralmente 255)
    uchar* data;    // Ponteiro para o vetor linear de pixels (alocação dinâmica)
} PGM;

/* 
 * Estrutura de Metadados (Header):
 * Utilizada para empacotar as informações que serão enviadas via FIFO.
 * Garante que o Worker saiba as dimensões e o filtro a ser aplicado
 * antes de receber a massa de pixels.
 */
typedef struct {
    int w, h, maxv; // Metadados da imagem
    int mode;       // Define qual filtro o Worker deve aplicar (0 ou 1)
    int t1, t2;     // Parâmetros de limiarização usados apenas no modo SLICE
} Header;


 // PROTÓTIPOS DAS FUNÇÕES DE I/O E MEMÓRIA

/**
 * Lê uma imagem PGM (formato binário P5) do disco.
 * @param path Caminho do arquivo de entrada.
 * @param img  Ponteiro para a estrutura onde os dados serão carregados.
 * @return Retorna 1 em caso de sucesso ou 0 em caso de falha.
 */
int read_pgm(const char* path, PGM* img);

/**
 * Grava uma imagem PGM no disco.
 * @param path Caminho onde o arquivo será salvo.
 * @param img  Ponteiro para a estrutura contendo os dados processados.
 * @return Retorna 1 em caso de sucesso ou 0 em caso de falha.
 */
int write_pgm(const char* path, const PGM* img);

/**
 * Libera a memória alocada para o vetor de pixels da imagem.
 * Essencial para evitar vazamentos de memória (memory leaks).
 * @param img Ponteiro para a estrutura PGM a ser limpa.
 */
void free_pgm(PGM* img);

#endif // PGM_H
