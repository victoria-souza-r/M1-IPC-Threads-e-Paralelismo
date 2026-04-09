#include <stdio.h>
#include <stdlib.h>
#include "filters.h"


// FILTRO NEGATIVO (por bloco)
void apply_negative_block(const PGM* in, PGM* out, int row_start, int row_end) {
    if (!in || !out || !in->data || !out->data) return;

    for (int y = row_start; y < row_end; y++) {
        int row_offset = y * in->w;
        for (int x = 0; x < in->w; x++) {
            int idx = row_offset + x;
            out->data[idx] = (uchar)(in->maxv - in->data[idx]);
        }
    }
}


// FILTRO SLICE (por bloco)
void apply_slice_block(const PGM* in, PGM* out, int row_start, int row_end, int t1, int t2) {
    if (!in || !out || !in->data || !out->data) return;

    for (int y = row_start; y < row_end; y++) {
        int row_offset = y * in->w;
        for (int x = 0; x < in->w; x++) {
            int idx = row_offset + x;
            uchar pixel = in->data[idx];

            // Fora do intervalo [t1, t2] → branco
            if (pixel <= t1 || pixel >= t2) {
                out->data[idx] = in->maxv;
            } else {
                out->data[idx] = pixel;
            }
        }
    }
}


// FUNÇÕES COMPLETAS (imagem inteira)
void apply_negative_full(const PGM* in, PGM* out) {
    if (!in || !out) return;
    apply_negative_block(in, out, 0, in->h);
}

void apply_slice_full(const PGM* in, PGM* out, int t1, int t2) {
    if (!in || !out) return;
    apply_slice_block(in, out, 0, in->h, t1, t2);
}
