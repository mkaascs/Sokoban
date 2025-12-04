#ifndef LEVELS_H
#define LEVELS_H

#include "bitboard.h"

#define LEVEL_COUNT 10

// Структура для хранения уровней
// ОПТИМИЗАЦИЯ: Замена char[][] на uint64_t
typedef struct Level {
    int number;

    BitBoard walls;
    BitBoard boxes;
    BitBoard goals;
    BitBoard player;
} Level;

void levels_init();
void levels_free();
Level* get_levels();

#endif //LEVELS_H
