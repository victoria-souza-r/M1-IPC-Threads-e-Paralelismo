#include <stdio.h>
#include <stdlib.h>
#include "filters.h"

// FILTRO NEGATIVO (Processamento por Bloco/Threads)
/**
 * Aplica a inversão de cores (negativo) em um intervalo de linhas.
 * Fórmula: s = (L - 1) - r
 */
void apply_negative_block(const PGM* in, PGM* out, int row_start, int row_end) {
    // Verificação de segurança para evitar acesso a ponteiros nulos
    if (!in || !out || !in->data || !out->data) return;

    // Percorre apenas o intervalo de linhas designado para esta thread
    for (int y = row_start; y < row_end; y++) {
        // Calcula o deslocamento da linha na matriz linearizada
        int row_offset = y * in->w;
        
        for (int x = 0; x < in->w; x++) {
            // Índice do pixel atual (y * largura + x)
            int idx = row_offset + x;
            
            // O novo valor é o máximo permitido (geralmente 255) menos o valor atual
            out->data[idx] = (uchar)(in->maxv - in->data[idx]);
        }
    }
}

// FILTRO SLICE / FATIAMENTO 
/**
 * Aplica o destaque de intervalo (Slicing).
 * Pixels fora do intervalo [t1, t2] ficam brancos (maxv).
 * Pixels dentro do intervalo preservam sua cor original.
 */
void apply_slice_block(const PGM* in, PGM* out, int row_start, int row_end, int t1, int t2) {
    if (!in || !out || !in->data || !out->data) return;

    for (int y = row_start; y < row_end; y++) {
        int row_offset = y * in->w;
        for (int x = 0; x < in->w; x++) {
            int idx = row_offset + x;
            uchar pixel = in->data[idx];

            // Lógica de Limiarização:
            // Se o pixel for menor/igual ao limite inferior OU maior/igual ao superior
            if (pixel <= t1 || pixel >= t2) {
                // Satura o pixel para branco (255)
                out->data[idx] = in->maxv;
            } else {
                // Mantém o pixel original para destacar a faixa de interesse
                out->data[idx] = pixel;
            }
        }
    }
}

// FUNÇÕES AUXILIARES (Processamento da Imagem Inteira)
// Chama a função de bloco cobrindo todas as linhas (de 0 até a altura H)
void apply_negative_full(const PGM* in, PGM* out) {
    if (!in || !out) return;
    apply_negative_block(in, out, 0, in->h);
}

// Chama a função de fatia cobrindo todas as linhas
void apply_slice_full(const PGM* in, PGM* out, int t1, int t2) {
    if (!in || !out) return;
    apply_slice_block(in, out, 0, in->h, t1, t2);
}
