#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <stdbool.h>

#include "../game/levels.h"

#define MAX_USERNAME 20
#define MIN_USERNAME 4
#define MAX_PASSWORD 65
#define MIN_PASSWORD 8

// Структура для хранения данных пользователя
// ОПТИМИЗАЦИЯ: Замена int[] на uint64_t (А)
// ОПТИМИЗАЦИЯ: Замена linked-list на динамический массив (А)
// ОПТИМИЗАЦИЯ: Хранение пользователей in-memory, убраны повторное чтение и парсинг файла (А)
// ОПТИМИЗАЦИЯ: Использование binary insertion вместо bubble sort (А)
typedef struct User {
    char username[MAX_USERNAME];
    char password[MAX_PASSWORD];
    uint16_t completed_levels;
    int level_times[LEVEL_COUNT];
    int level_moves[LEVEL_COUNT];
    float total_score;
} User;

typedef struct UserArray {
    User* users;
    int length;
} UserArray;

bool load_leaderboard();
bool save_leaderboard();

const UserArray* get_users();
int find_user(const char* username);

bool register_user(const char* username, const char* password);
bool update_score(const char* username, float new_score);

void free_leaderboard();

#endif //LEADERBOARD_H
