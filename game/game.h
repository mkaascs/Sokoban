#ifndef GAME_H
#define GAME_H

#include "levels.h"

#include <stdbool.h>

#define ROWS 8
#define COLS 8

typedef struct GameState {
    long long start_time;
    long long end_time;
    int move_count;
    int player_x, player_y;
    bool is_level_completed;
} GameState;

typedef struct Game {
    Level level;
    GameState state;
} Game;

typedef enum EntityType {
    ENTITY_NONE,
    ENTITY_WALL,
    ENTITY_BOX,
    ENTITY_GOAL,
    ENTITY_PLAYER,
    ENTITY_PLAYER_ON_GOAL,
    ENTITY_BOX_ON_GOAL
} EntityType;

bool start_game(int level);
bool move_player(int dx, int dy);

Level get_current_level();
GameState get_game_state();
EntityType get_entity_type(int row, int column);

long long get_time_ms();

bool check_win();

#endif //GAME_H
