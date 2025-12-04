#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "game.h"

#include "../profile/profile.h"
#include "bot/bot.h"

Game game = {0};

bool start_game(const int level) {
    const Level* levels = get_levels();
    const Level* src = &levels[level];

    if (game.level == NULL)
        game.level = malloc(sizeof(Level));

    else {
        bb_free(&game.level->walls);
        bb_free(&game.level->boxes);
        bb_free(&game.level->goals);
        bb_free(&game.level->player);
    }

    if (game.state == NULL)
        game.state = malloc(sizeof(GameState));

    bb_init(&game.level->walls, src->walls.rows, src->walls.columns);
    bb_init(&game.level->boxes, src->boxes.rows, src->boxes.columns);
    bb_init(&game.level->goals, src->goals.rows, src->goals.columns);
    bb_init(&game.level->player, src->player.rows, src->player.columns);

    memcpy(game.level->walls.data, src->walls.data, src->walls.words * sizeof(uint64_t));
    memcpy(game.level->boxes.data, src->boxes.data, src->boxes.words * sizeof(uint64_t));
    memcpy(game.level->goals.data, src->goals.data, src->goals.words * sizeof(uint64_t));
    memcpy(game.level->player.data, src->player.data, src->player.words * sizeof(uint64_t));

    game.level->number = level;

    int pr, pc;
    if (!bb_find_first_bit(&game.level->player, &pr, &pc)) {
        bb_free(&game.level->walls);
        bb_free(&game.level->boxes);
        bb_free(&game.level->goals);
        bb_free(&game.level->player);
        free(game.level);
        free(game.state);
        game.level = NULL;
        game.state = NULL;
        return false;
    }

    game.state->player_y = pr;
    game.state->player_x = pc;
    game.state->start_time = get_time_ms();
    game.state->move_count = 0;
    game.state->is_level_completed = false;

    return true;
}

bool move_player(const int dx, const int dy) {
    const int rows = game.level->walls.rows;
    const int columns = game.level->walls.columns;

    const int new_x = game.state->player_x + dx;
    const int new_y = game.state->player_y + dy;

    if (new_x < 0 || new_x >= columns || new_y < 0 || new_y >= rows)
        return false;

    bool next_is_wall = bb_get(&game.level->walls, new_y, new_x);
    const bool next_is_box  = bb_get(&game.level->boxes, new_y, new_x);

    if (next_is_box) {
        const int box_x = new_x + dx;
        const int box_y = new_y + dy;

        if (box_x < 0 || box_x >= columns || box_y < 0 || box_y >= rows)
            return false;

        const bool wall_ahead = bb_get(&game.level->walls, box_y, box_x);
        const bool box_ahead  = bb_get(&game.level->boxes, box_y, box_x);

        if (wall_ahead || box_ahead)
            return false;

        bb_clear(&game.level->boxes, new_y, new_x);
        bb_set(&game.level->boxes,   box_y,    box_x);
    }

    if (next_is_wall)
        return false;

    bb_clear(&game.level->player, game.state->player_y, game.state->player_x);
    bb_set(&game.level->player, new_y, new_x);

    game.state->player_x = new_x;
    game.state->player_y = new_y;

    game.state->move_count++;

    if (check_win()) {
        game.state->is_level_completed = true;
        game.state->end_time = get_time_ms();
    }

    return true;
}

const Level* get_current_level() {
    return game.level;
}

const GameState* get_game_state() {
    return game.state;
}

bool check_win() {
    for (int i = 0; i < game.level->boxes.words; i++) {
        const uint64_t boxes = game.level->boxes.data[i];
        const uint64_t goals = game.level->goals.data[i];

        if (boxes & ~goals)
            return false;
    }

    return true;
}

EntityType get_entity_type(const int row, const int column) {
    if (row < 0 || row >= game.level->walls.rows || column < 0 || column >= game.level->walls.columns)
        return ENTITY_NONE;

    const bool is_wall = bb_get(&game.level->walls,  row, column);
    const bool is_player = bb_get(&game.level->player,row, column);
    const bool is_box = bb_get(&game.level->boxes,  row, column);
    const bool is_goal = bb_get(&game.level->goals,  row, column);

    if (is_wall) return ENTITY_WALL;
    if (is_player) return is_goal ? ENTITY_PLAYER_ON_GOAL : ENTITY_PLAYER;
    if (is_box)    return is_goal ? ENTITY_BOX_ON_GOAL    : ENTITY_BOX;
    if (is_goal)   return ENTITY_GOAL;
    return ENTITY_NONE;
}

long long get_time_ms() {
    struct timespec ts;
    timespec_get(&ts, TIME_UTC);
    return ts.tv_sec * 1000LL + ts.tv_nsec / 1000000LL;
}