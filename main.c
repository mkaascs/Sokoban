#include <openssl/evp.h>

#include "leaderboard/leaderboard.h"
#include "draw/draw.h"

// Главная функция
int main(const int argc, char** argv) {
    load_leaderboard();
    printf("leaderboard was loaded\n");

    init_draw(argc, argv);
    start_draw();

    printf("application was closed\n");
    exit_game();

    return 0;
}
