#ifndef LEVELS_H
#define LEVELS_H

#include <stdlib.h>

#define LEVEL_COUNT 11

// Структура для хранения уровней
// ОПТИМИЗАЦИЯ: Замена char[][] на uint64_t
typedef struct Level {
    int number;

    uint64_t walls;
    uint64_t boxes;
    uint64_t goals;
    uint64_t player;
} Level;

extern const Level originalLevels[];

#endif //LEVELS_H
