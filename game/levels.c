#include "levels.h"
#include <stdio.h>

static void set_walls_from_ascii(const BitBoard* walls, const char* ascii[], int rows, int columns);
static void set_entities_from_ascii(
    const BitBoard* boxes, const BitBoard* goals, const BitBoard* player,
    const char* ascii[], int rows, int columns
);

Level load_tutorial() {
    const int ROWS = 8;
    const int COLUMNS = 8;

    Level level;
    level.number = 0;

    bb_init(&level.walls, ROWS, COLUMNS);
    bb_init(&level.boxes, ROWS, COLUMNS);
    bb_init(&level.goals, ROWS, COLUMNS);
    bb_init(&level.player, ROWS, COLUMNS);

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLUMNS; c++) {
            if (r == 0 || r == ROWS-1 || c == 0 || c == COLUMNS-1) {
                bb_set(&level.walls, r, c);
            }
        }
    }

    bb_set(&level.player, 2, 2);

    bb_set(&level.goals, 2, 4);

    bb_set(&level.boxes, 2, 6);

    return level;
}

static const char* level_templates[LEVEL_COUNT - 1][8] = {
    {"########", "#      #", "# @ $  #", "#   .  #", "#      #", "#      #", "#      #", "########"},
    {"########", "#      #", "# @ $  #", "#  $ . #", "#   .  #", "#      #", "#      #", "########"},
    {"########", "#      #", "# @ $. #", "#   $  #", "# .  . #", "#    $ #", "#      #", "########"},
    {"########", "#      #", "#  @$  #", "#  $$  #", "#  . . #", "#  .   #", "#      #", "########"},
    {"########", "#   #  #", "# @$$$ #", "#   .# #", "#  ..  #", "#      #", "#      #", "########"},
    {"########", "# @ #  #", "# $$.$ #", "#   .# #", "#  ..  #", "#    $ #", "#      #", "########"},
    {"########", "#  @#  #", "# $$   #", "#  #.# #", "# . .  #", "#   $  #", "#      #", "########"},
    {"########", "#  @   #", "# $ $ ##", "#  . . #", "# .#. ##", "#  $$  #", "#      #", "########"},
    {"########", "#  @   #", "# $$   #", "#  #.# #", "#  . . #", "# $    #", "#      #", "########"},
};

Level* original_levels;

Level load_8x8level_from_template(int index) {
    Level level;
    level.number = index;

    bb_init(&level.walls, 8, 8);
    bb_init(&level.boxes, 8, 8);
    bb_init(&level.goals, 8, 8);
    bb_init(&level.player, 8, 8);

    set_walls_from_ascii(&level.walls, level_templates[index], 8, 8);
    set_entities_from_ascii(&level.boxes, &level.goals, &level.player, level_templates[index], 8, 8);

    return level;
}

void levels_init() {
    original_levels = malloc(LEVEL_COUNT * sizeof(Level));
    original_levels[0] = load_tutorial();
    for (int index = 1; index < LEVEL_COUNT; index++) {
        original_levels[index] = load_8x8level_from_template(index - 1);
    }
}

void levels_free() {
    if (original_levels == NULL)
        return;

    for (int i = 0; i < LEVEL_COUNT; i++) {
        bb_free(&original_levels[i].walls);
        bb_free(&original_levels[i].boxes);
        bb_free(&original_levels[i].goals);
        bb_free(&original_levels[i].player);
    }

    free(original_levels);
}

Level* get_levels() {
    return original_levels;
}

static void set_walls_from_ascii(const BitBoard* walls, const char* ascii[], const int rows, const int columns) {
    for (int r = 0; r < rows; r++) {
        int game_row = rows - 1 - r;
        for (int c = 0; c < columns; c++)
            if (ascii[r][c] == '#')
                bb_set(walls, game_row, c);
    }
}

static void set_entities_from_ascii(
    const BitBoard* boxes, const BitBoard* goals, const BitBoard* player,
    const char* ascii[], const int rows, const int columns
) {
    for (int row = 0; row < rows; row++) {
        int game_row = rows - 1 - row;
        for (int column = 0; column < columns; column++) {
            const char symbol = ascii[row][column];
            if (symbol == '.' || symbol == '*') bb_set(boxes,  game_row, column);
            if (symbol == '$' || symbol == '*' || symbol == '+') bb_set(goals,  game_row, column);
            if (symbol == '@' || symbol == '+') bb_set(player, game_row, column);
        }
    }
}