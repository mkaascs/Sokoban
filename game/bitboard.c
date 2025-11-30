#include "bitboard.h"

void bb_init(BitBoard* bb, const int rows, const int columns) {
    bb->rows = rows;
    bb->columns = columns;

    const int bits = rows * columns;
    bb->words = (bits + 63) / 64;
    bb->data = calloc(bb->words, sizeof(uint64_t));
}

void bb_free(const BitBoard* bb) {
    if (bb != NULL)
        free(bb->data);
}

bool bb_find_first_bit(const BitBoard* bb, int* out_r, int* out_c) {
    for (int w = 0; w < bb->words; w++) {
        uint64_t word = bb->data[w];
        if (word == 0) continue;

        int bit_index = __builtin_ctzll(word);
        int global_index = w * 64 + bit_index;

        if (global_index >= bb->rows * bb->columns)
            return false;

        *out_r = global_index / bb->columns;
        *out_c = global_index % bb->columns;
        return true;
    }

    return false;
}