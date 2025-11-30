#ifndef BITBOARD_H
#define BITBOARD_H

#include <stdlib.h>
#include <stdbool.h>

typedef struct {
    uint64_t* data;
    int words;
    int rows, columns;
} BitBoard;

void bb_init(BitBoard* bb, int rows, int columns);
void bb_free(const BitBoard* bb);
bool bb_find_first_bit(const BitBoard* bb, int* out_r, int* out_c);

static inline int bb_index(const BitBoard* bb, const int row, const int columns) {
    return row * bb->columns + columns;
}

static inline void bb_set(const BitBoard* bb, const int row, const int columns) {
    const int index = bb_index(bb, row, columns);
    bb->data[index / 64] |= (1ULL << (index % 64));
}

static inline void bb_clear(const BitBoard* bb, const int row, const int columns) {
    const int index = bb_index(bb, row, columns);
    bb->data[index / 64] &= ~(1ULL << (index % 64));
}

static inline int bb_get(const BitBoard* bb, const int row, const int columns) {
    const int index = bb_index(bb, row, columns);
    return (bb->data[index / 64] >> (index % 64)) & 1;
}

static inline uint64_t bb_mask(const BitBoard* bb, const int row, const int columns) {
    const int index = bb_index(bb, row, columns);
    return 1ULL << (index % 64);
}

static inline uint64_t bb_bit(const BitBoard* bb, const int row, const int columns) {
    const int index = row * bb->columns + columns;
    int word = index / 64;
    const int bit  = index % 64;
    return (uint64_t)1 << bit;
}

#endif //BITBOARD_H
