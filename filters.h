#ifndef FILTERS_H
#define FILTERS_H
#include "pgm.h"

/* 
 * Filtro Negativo: s = 255 - r
 * Aplica em um bloco específico da imagem (de row_start até row_end).
 * Útil para ser chamado dentro das threads do Worker.
 */
void apply_negative_block(const PGM* in, PGM* out, int row_start, int row_end);

/* 
 * Filtro de Limiarização com Fatiamento (Slice):
 * Se o pixel está entre t1 e t2, mantém o valor original ou aplica destaque.
 * Caso contrário, define como 0 ou 255 conforme o pseudo-código do PDF.
 */
void apply_slice_block(const PGM* in, PGM* out, int row_start, int row_end, int t1, int t2);

/* 
 * Funções de conveniência para a imagem inteira.
 * Internamente, elas apenas chamam as funções de bloco passando 0 e img->h.
 */
void apply_negative_full(const PGM* in, PGM* out);
void apply_slice_full(const PGM* in, PGM* out, int t1, int t2);

#endif
