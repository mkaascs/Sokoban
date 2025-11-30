#include "leaderboard.h"

#include <stdio.h>
#include <string.h>
#include <secure/_string.h>

#include "crypto.h"

#define DEFAULT_CAPACITY 4

#define BUFFER_SIZE 1024
#define USERS_FILE "users.bin"

static UserArray users;
static int array_capacity = DEFAULT_CAPACITY;

static int parse_plain_text(const char*, User*);
static int binary_search_insert_position(float);
static bool add_user(User);

bool load_leaderboard() {
    FILE* file = fopen(USERS_FILE, "rb+");
    if (file == NULL) {
        file = fopen(USERS_FILE, "wb+");
        if (file == NULL)
            return false;
    }

    users.users = (User*)malloc(DEFAULT_CAPACITY * sizeof(User));
    users.length = 0;

    int cipher_text_length = 0;

    while (fread(&cipher_text_length, sizeof(int), 1, file)) {
        unsigned char buffer[BUFFER_SIZE];
        if (cipher_text_length > BUFFER_SIZE || cipher_text_length <= 0) {
            fseek(file, cipher_text_length + 1, SEEK_CUR);
            continue;
        }

        fread(buffer, 1, cipher_text_length, file);
        char newline;
        fread(&newline, 1, 1, file);

        unsigned char plain_text[BUFFER_SIZE];
        const int plain_text_length = decrypt_data(buffer, cipher_text_length, plain_text);
        if (plain_text_length < 0) continue;
        plain_text[plain_text_length] = '\0';

        User parsed_user;
        parse_plain_text(plain_text, &parsed_user);
        add_user(parsed_user);
    }

    fclose(file);
    return true;
}

bool save_leaderboard() {
    FILE* file = fopen(USERS_FILE, "wb");
    if (file == NULL)
        return false;

    for (int index = 0; index < users.length; index++) {
        User* user = &users.users[index];

        unsigned char data[BUFFER_SIZE];
        int length = snprintf(data, sizeof(data), "%s %s %d",
            user->username, user->password, user->completed_levels);

        for (int level = 0; level < LEVEL_COUNT; level++)
            length += snprintf(data + length, sizeof(data), " %d %d",
                user->level_times[index], user->level_moves[index]);

        length += snprintf(data + length, sizeof(data), " %.2f",
            user->total_score);

        unsigned char cipher_text[BUFFER_SIZE];
        const int cipher_text_length = encrypt_data(data, length, cipher_text);
        if (cipher_text_length < 0) {
            fclose(file);
            return false;
        }

        fwrite(&cipher_text_length, sizeof(int), 1, file);
        fwrite(cipher_text, 1, cipher_text_length, file);

        char newline = '\n';
        fwrite(&newline, 1, 1, file);
    }

    fclose(file);
    return true;
}

UserArray get_users() {
    return users;
}

int find_user(const char* username) {
    for (int index = 0; index < users.length; index++)
        if (strcmp(users.users[index].username, username) == 0)
            return index;

    return -1;
}

bool register_user(const char* username, const char* password) {
    User new_user = {0};

    strncpy(new_user.username, username, sizeof(new_user.username) - 1);
    strncpy(new_user.password, password, sizeof(new_user.password) - 1);

    return add_user(new_user);
}

bool update_score(const char* username, const float new_score) {
    const int index = find_user(username);
    if (index == -1)
        return false;

    if (new_score == users.users[index].total_score)
        return true;

    User temp = users.users[index];
    temp.total_score = new_score;

    if (index < users.length - 1)
        memmove(&users.users[index], &users.users[index + 1], sizeof(User) * (users.length - index - 1));

    users.length--;

    const int position = binary_search_insert_position(new_score);
    if (position < users.length)
        memmove(&users.users[position + 1], &users.users[position], sizeof(User) * (users.length - position));

    users.users[position] = temp;
    users.length++;
    return true;
}

void free_leaderboard() {
    if (users.users == NULL)
        return;

    free(users.users);
}

static bool add_user(const User user) {
    if (find_user(user.username) != -1)
        return false;

    if (users.length == array_capacity) {
        array_capacity *= 2;
        users.users = (User*)realloc(users.users, array_capacity * sizeof(User));
    }

    const int position = binary_search_insert_position(user.total_score);
    if (position < users.length)
        memmove(&users.users[position + 1], &users.users[position], sizeof(User) * (users.length - position));

    users.users[position] = user;
    users.length++;

    return true;
}

static int binary_search_insert_position(const float score) {
    int left = 0;
    int right = users.length;

    while (left < right) {
        const int mid = (left + right) / 2;

        if (users.users[mid].total_score > score)
            left = mid + 1;

        else right = mid;
    }

    return left;
}

static int parse_plain_text(const char* plain_text, User* user) {
    const int items = sscanf(plain_text, "%s %s %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %f",
                             user->username, user->password, &user->completed_levels,
                             &user->level_times[0], &user->level_moves[0],
                             &user->level_times[1], &user->level_moves[1],
                             &user->level_times[2], &user->level_moves[2],
                             &user->level_times[3], &user->level_moves[3],
                             &user->level_times[4], &user->level_moves[4],
                             &user->level_times[5], &user->level_moves[5],
                             &user->level_times[6], &user->level_moves[6],
                             &user->level_times[7], &user->level_moves[7],
                             &user->level_times[8], &user->level_moves[8],
                             &user->level_times[9], &user->level_moves[9],
                             &user->total_score);

    return items;
}
