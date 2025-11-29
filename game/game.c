#include <stdlib.h>

#include "game.h"

#include <time.h>

#include "../profile/profile.h"

Game game;

static uint64_t bit(int, int);

bool start_game(const int level) {
    profile_start();
    game.level.number = level;
    game.level.boxes = originalLevels[level].boxes;
    game.level.goals = originalLevels[level].goals;
    game.level.player = originalLevels[level].player;
    game.level.walls = originalLevels[level].walls;

    const uint64_t mask = game.level.player;
    if (mask != 0) {
        const int index = __builtin_ctzll(mask);
        game.state.player_x = index % COLS;
        game.state.player_y = index / COLS;
    }

    else return false;

    game.state.start_time = get_time_ms();
    game.state.move_count = 0;
    game.state.is_level_completed = false;
    profile_end("start_game");

    return true;
}

bool move_player(const int dx, const int dy) {
    profile_start();
    const int new_x = game.state.player_x + dx;
    const int new_y = game.state.player_y + dy;

    if (new_x < 0 || new_x >= COLS || new_y < 0 || new_y >= ROWS)
        return false;

    const uint64_t next_bit = bit(new_y, new_x);

    if (game.level.boxes & next_bit) {
        const int box_x = new_x + dx;
        const int box_y = new_y + dy;

        if (box_x < 0 || box_x >= COLS || box_y < 0 || box_y >= ROWS)
            return false;

        const uint64_t box_next_bit = bit(box_y, box_x);

        if (!(game.level.walls & box_next_bit) && !(game.level.boxes & box_next_bit)) {
            game.level.boxes &= ~next_bit;
            game.level.boxes |= box_next_bit;
        }

        else return false;
    }

    if (game.level.walls & next_bit)
        return false;

    game.level.player &= ~bit(game.state.player_y, game.state.player_x);
    game.level.player |= bit(new_y, new_x);

    game.state.player_x = new_x;
    game.state.player_y = new_y;

    game.state.move_count++;

    if (check_win()) {
        game.state.is_level_completed = true;
        game.state.end_time = get_time_ms();
    }

    profile_end("move_player");

    return true;
}

Level get_current_level() {
    return game.level;
}

GameState get_game_state() {
    return game.state;
}

bool check_win() {
    return (game.level.boxes & game.level.goals) == game.level.goals
        || (game.level.boxes & ~game.level.goals) == 0;
}

EntityType get_entity_type(const int row, const int column) {
    const uint64_t b = bit(row, column);

    if (game.level.walls & b) return ENTITY_WALL;
    if (game.level.player & b) return game.level.goals & b ? ENTITY_PLAYER_ON_GOAL : ENTITY_PLAYER;
    if (game.level.boxes & b)  return game.level.goals & b ? ENTITY_BOX_ON_GOAL : ENTITY_BOX;
    if (game.level.goals & b) return ENTITY_GOAL;
    return ENTITY_NONE;
}

uint64_t bit(const int row, const int column) {
    return 1ULL << row * COLS + column;
}

long long get_time_ms() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}