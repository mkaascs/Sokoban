#include "levels.h"

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

Level load_8x8level() {
    Level level = {0};
    level.number = 1;
    bb_init(&level.walls, 8, 8);
    bb_init(&level.boxes, 8, 8);
    bb_init(&level.goals, 8, 8);
    bb_init(&level.player, 8, 8);

    const char* ascii[] = {
        "########",
        "#      #",
        "#  .$  #",
        "#   $  #",
        "#    . #",
        "#      #",
        "# @    #",
        "########"
    };

    set_walls_from_ascii(&level.walls, ascii, 8, 8);
    set_entities_from_ascii(&level.boxes, &level.goals, &level.player, ascii, 8, 8);

    return level;
}

Level load_12x12level() {
    Level level = {0};
    level.number = 2;
    bb_init(&level.walls, 12, 12);
    bb_init(&level.boxes, 12, 12);
    bb_init(&level.goals, 12, 12);
    bb_init(&level.player, 12, 12);

    const char* ascii[] = {
        "############",
        "#          #",
        "#  $       #",
        "#   $      #",
        "#    $     #",
        "#     .    #",
        "#      .   #",
        "#       .  #",
        "#        @ #",
        "#          #",
        "#          #",
        "############"
    };

    set_walls_from_ascii(&level.walls, ascii, 12, 12);
    set_entities_from_ascii(&level.boxes, &level.goals, &level.player, ascii, 12, 12);

    return level;
}

Level load_16x16level() {
    Level level = {0};
    level.number = 3;
    bb_init(&level.walls, 16, 16);
    bb_init(&level.boxes, 16, 16);
    bb_init(&level.goals, 16, 16);
    bb_init(&level.player, 16, 16);

    const char* ascii[] = {
        "################",
        "#         ###  #",
        "#         ###  #",
        "#  #   #  ###  #",
        "#  # $ #  ###  #",
        "#  ### #########",
        "#          #####",
        "#    #### ######",
        "#  ####    .   #",
        "#  #### $  . @ #",
        "#  #### $  .   #",
        "#  ######  #####",
        "#  ######      #",
        "#              #",
        "#              #",
        "################"
    };

    set_walls_from_ascii(&level.walls, ascii, 16, 16);
    set_entities_from_ascii(&level.boxes, &level.goals, &level.player, ascii, 16, 16);

    return level;
}

Level load_32x32level() {
    Level level;
    level.number = 4;
    bb_init(&level.walls, 32, 32);
    bb_init(&level.boxes, 32, 32);
    bb_init(&level.goals, 32, 32);
    bb_init(&level.player, 32, 32);

    const char* ascii[] = {
        "################################",
        "#                              #",
        "#  $ $ $ $ $ $ $ $             #",
        "#  . . . . . . . .             #",
        "#                              #",
        "#     ################         #",
        "#     #          #####         #",
        "#     #   #      #####         #",
        "#     #   #      #####         #",
        "#     #   # $    #####         #",
        "#     #   # .    #####         #",
        "#     #   #      #####         #",
        "#     #   ############         #",
        "#     #   ############         #",
        "#         ############         #",
        "#         ############         #",
        "#     #   ##   $ $ $ $         #",
        "#     #   ##    . . . .        #",
        "#     #   ##                   #",
        "#     #   ################     #",
        "#     #                        #",
        "#     #                        #",
        "#     #                        #",
        "#     #                        #",
        "#     #                        #",
        "#     #                        #",
        "#     #                 @      #",
        "#     ##########################",
        "#                              #",
        "#                              #",
        "#                              #",
        "################################"
    };

    set_walls_from_ascii(&level.walls, ascii, 32, 32);
    set_entities_from_ascii(&level.boxes, &level.goals, &level.player, ascii, 32, 32);

    return level;
}

Level load_64x64level() {
    Level level;
    level.number = 5;
    bb_init(&level.walls, 64, 64);
    bb_init(&level.boxes, 64, 64);
    bb_init(&level.goals, 64, 64);
    bb_init(&level.player, 64, 64);

    const char* ascii[] = {
    "################################################################",
    "################################################################",
    "##                  .                  ###                  #..#",
    "##                  .                  ###                  #..#",
    "##                  .                                       #..#",
    "##                 ##                  ###                  #..#",
    "#########          ##                  ###                  #..#",
    "##    $            ##    #########     ###                  #..#",
    "##            #######    # $           ###                  #..#",
    "##             .   ##    #             ###                  #..#",
    "##                 ##    #             ###               @  #..#",
    "##########################################            ##########",
    "##########################################            ##########",
    "#################                                     ##########",
    "##         ###  #                                     ##########",
    "##         ###  #                                     ##########",
    "##  #   #  ###  #          #####################################",
    "##  # $ #  ###  #          #####################################",
    "##  ### #########          #####################################",
    "##          #####          #####################################",
    "##    #### ######          #####################################",
    "##  ####    .              #####################################",
    "##  #### $  .              #####################################",
    "##  #### $  .              #####################################",
    "##  ######  #####          #####################################",
    "##  ######   ####          #####################################",
    "##           ####          #####################################",
    "##           ####          #####################################",
    "#################          #####################################",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                              $ $ $ $ $ $  #..#",
    "#################                             . . . . . .   #..#",
    "#################                                           #..#",
    "#################                            ################..#",
    "#################                            ################..#",
    "#################                            ################..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                           #..#",
    "#################                                    $      #..#",
    "################################################################",
    "################################################################",
    "################################################################",
    "################################################################",
    "################################################################",
    "################################################################",
    "################################################################"
    };

    set_walls_from_ascii(&level.walls, ascii, 64, 64);
    set_entities_from_ascii(&level.boxes, &level.goals, &level.player, ascii, 64, 64);

    return level;
}

Level* original_levels;

void levels_init() {
    original_levels = malloc(LEVEL_COUNT * sizeof(Level));
    original_levels[0] = load_tutorial();
    //original_levels[1] = load_8x8level();
    //original_levels[2] = load_12x12level();
    //original_levels[3] = load_16x16level();
    //original_levels[4] = load_32x32level();
    original_levels[1] = load_64x64level();
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