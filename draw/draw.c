#include "draw.h"

#include "../game/game.h"
#include "../game/levels.h"
#include "../game/bot/bot.h"
#include "../leaderboard/crypto.h"
#include "../profile/profile.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <_time.h>

#include "../leaderboard/leaderboard.h"

#include <GL/freeglut_std.h>
#include <openssl/sha.h>

#include "batching/batching.h"
#include "batching/font.h"

#define WINDOW_WIDTH 1400
#define WINDOW_HEIGHT 800

#define GAME_RECT_X -0.75
#define GAME_RECT_Y -0.75
#define GAME_RECT_WIDTH 0.8
#define GAME_RECT_HEIGHT 1.25

#define MENU_RECT_X 0.1
#define MENU_RECT_Y -0.75
#define MENU_RECT_WIDTH 0.4
#define MENU_RECT_HEIGHT 1.25

#define BUTTON_WIDTH 0.3
#define BUTTON_HEIGHT 0.1
#define BOT_BUTTON_X MENU_RECT_X + 0.05
#define BOT_BUTTON_Y MENU_RECT_Y + 1.05
#define NEXT_BUTTON_X MENU_RECT_X + 0.05
#define NEXT_BUTTON_Y MENU_RECT_Y + 0.85
#define PREV_BUTTON_X MENU_RECT_X + 0.05
#define PREV_BUTTON_Y MENU_RECT_Y + 0.65
#define LEVEL_BUTTON_X MENU_RECT_X + 0.05
#define LEVEL_BUTTON_Y MENU_RECT_Y + 0.45
#define RESTART_BUTTON_X MENU_RECT_X + 0.05
#define RESTART_BUTTON_Y MENU_RECT_Y + 0.25
#define LEADERBOARD_BUTTON_X MENU_RECT_X + 0.05
#define LEADERBOARD_BUTTON_Y MENU_RECT_Y + 0.15
#define CONTROLS_BUTTON_X MENU_RECT_X + 0.05
#define CONTROLS_BUTTON_Y MENU_RECT_Y + 0.05
#define QUIT_BUTTON_X MENU_RECT_X + 0.05
#define QUIT_BUTTON_Y MENU_RECT_Y - 0.05

#define LEVEL_SELECT_BUTTON_WIDTH 0.1
#define LEVEL_SELECT_BUTTON_HEIGHT 0.1
#define LEVEL_SELECT_BASE_X -0.8
#define LEVEL_SELECT_BASE_Y 0.7

#define PASSWORD_STRENGTH_X -0.4
#define PASSWORD_STRENGTH_Y -0.1
#define PASSWORD_STRENGTH_WIDTH 0.4
#define PASSWORD_STRENGTH_HEIGHT 0.05

#define TOGGLE_BUTTON_X -0.4
#define TOGGLE_BUTTON_Y -0.2
#define TOGGLE_BUTTON_WIDTH 0.4
#define TOGGLE_BUTTON_HEIGHT 0.1

#define QUIT_LOGIN_BUTTON_X -0.4
#define QUIT_LOGIN_BUTTON_Y -0.4
#define QUIT_LOGIN_BUTTON_WIDTH 0.2
#define QUIT_LOGIN_BUTTON_HEIGHT 0.1

#define TARGET_FRAME_TIME_MS 16

typedef enum {
    LOGIN,
    GAME,
    LEVEL_SELECTOR,
    LEADERBOARD,
    TUTORIAL,
    CONTROLS
} DisplayMode;

const char tutorialMoves[] = {'d', 'd', 's'};
const int tutorialMoveCount = 3;

char current_username[MAX_USERNAME + 1] = "";
char current_password[MAX_PASSWORD + 1] = "";
char errorMessage[100] = "";
bool is_login_screen = true;
bool is_register_mode = false;
bool is_username_entered = false;
bool is_first_game = false;
int password_strength = 0;
DisplayMode current_mode = LOGIN;
User* current_user = NULL;

static char* bot_path = NULL;
static int bot_step = 0;
static bool bot_running = false;

void draw();
void reshape(int width, int height);
void mouse(int button, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void special_keys(int key, int x, int y);
void timer_redisplay(int value);

void init_draw(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutCreateWindow("Sokoban by NP");

    batch_reset();
    batch_init();

    font_init();

    glutTimerFunc(TARGET_FRAME_TIME_MS, timer_redisplay, 0);
    glutDisplayFunc(draw);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special_keys);
    glutMouseFunc(mouse);
}

void timer_redisplay(int value) {
    glutPostRedisplay();
    glutTimerFunc(TARGET_FRAME_TIME_MS, timer_redisplay, 0);
}

void start_draw() {
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glutMainLoop();
}

void exit_game() {
    save_leaderboard();
    free_leaderboard();
    levels_free();
    batch_free();
    profile_print();
    exit(0);
}

static void draw_rectangle(const float x, const float y, const float width, const float height, const float r, const float g, const float b) {
    profile_start("draw_rectangle()");
    batch_add_rect(x, y, width, height, r, g, b);
    profile_end();
}

void draw_text(float x_ndc, float y_ndc, const char* text, float r, float g, float b) {
    profile_start("draw_text()");
    draw_text_batch(x_ndc,y_ndc,text,r,g,b);
    profile_end();
}

bool move_player_prf(const int dx, const int dy) {
    profile_start("move_player()");
    bool result = move_player(dx, dy);
    profile_end();
    return result;
}

static void draw_cell(const float x, const float y, const float cellWidth, const float cellHeight, const EntityType type) {
    profile_start("draw_cell()");

    float r, g, b;
    switch (type) {
        case ENTITY_WALL: r = 153.0f/255.0f; g = 153.0f/255.0f; b = 153.0f/255.0f; break;
        case ENTITY_PLAYER: r = 1.0f; g = 204.0f/255.0f; b = 0.0f; break;
        case ENTITY_GOAL: r = 153.0f/255.0f; g = 204.0f/255.0f; b = 153.0f/255.0f; break;
        case ENTITY_BOX: r = 204.0f/255.0f; g = 153.0f/255.0f; b = 102.0f/255.0f; break;
        case ENTITY_BOX_ON_GOAL: r = 1.0f; g = 153.0f/255.0f; b = 51.0f/255.0f; break;
        case ENTITY_PLAYER_ON_GOAL: r = 153.0f/255.0f; g = 1.0f; b = 1.0f; break;
        default: r = 102.0f/255.0f; g = 76.0f/255.0f; b = 51.0f/255.0f; break;
    }

    batch_add_rect(x, y, cellWidth, cellHeight, r, g, b);
    profile_end();
}

static int evaluate_password_strength(const char* password) {
    const int length = strlen(password);
    int score = 0;
    bool hasLower = false, hasUpper = false, hasDigit = false, hasSpecial = false;
    char uniqueChars[256] = {0};
    int uniqueCount = 0;

    for (int i = 0; i < length; i++) {
        if (!uniqueChars[(unsigned char)password[i]]) {
            uniqueChars[(unsigned char)password[i]] = 1;
            uniqueCount++;
        }
    }

    if (length >= 20) score += 4;
    else if (length >= 16) score += 3;
    else if (length >= 12) score += 2;
    else if (length >= 8) score += 1;

    if (uniqueCount >= 12) score += 2;
    else if (uniqueCount >= 8) score += 1;

    for (int i = 0; i < length; i++) {
        if (islower(password[i])) hasLower = true;
        else if (isupper(password[i])) hasUpper = true;
        else if (isdigit(password[i])) hasDigit = true;
        else hasSpecial = true;
    }

    if (hasLower) score++;
    if (hasUpper) score++;
    if (hasDigit) score++;
    if (hasSpecial) score++;

    return (score > 5) ? 5 : score;
}

static void get_password_strength_feedback(const int strength, char* feedback) {
    switch (strength) {
        case 0:
        case 1:
            strcpy(feedback, "Weak password! Use more characters and variety.");
            break;
        case 2:
            strcpy(feedback, "Fair password. Add unique symbols or length.");
            break;
        case 3:
            strcpy(feedback, "Moderate password. More variety or length helps.");
            break;
        case 4:
            strcpy(feedback, "Good password. Add a few more unique chars.");
            break;
        case 5:
            strcpy(feedback, "Strong password!");
            break;
        default: ;
    }
}

// Прототипы функций
void keyboard(unsigned char key, int x, int y);
void special_keys(int key, int x, int y);
void mouse(int button, int state, int x, int y);
void start_sokoban();
float recalculate_total_score(User user);
void update_tutorial_animation();
void draw_tutorial();

float recalculate_total_score(const User user) {
    const Level* level = get_current_level();
    float totalScore = 0.0;
    for (int i = 1; i < LEVEL_COUNT; i++) {
        if (user.level_moves[i] > 0 && user.level_times[i] > 0) {
            totalScore += ((100.0 / 8 * level->walls.columns / (float)user.level_moves[i]) +
                          (100.0 / 8 * level->walls.columns / (float)user.level_times[i])) * (i + 1);
        }
    }

    return totalScore;
}

// Обновление уровня void update_level(int dx, int dy) {
void update_level(const int dx, const int dy) {
    profile_start("update_level()");
    if (!move_player_prf(dx, dy)) {
        profile_end();
        return;
    }

    const GameState* state = get_game_state();
    const Level* level = get_current_level();

    if (!state->is_level_completed) {
        profile_end();
        return;
    }

    float time_taken = state->end_time - state->start_time / 1000;
    time_taken = time_taken < 1 ? 1 : time_taken;

    const float level_score =
        (100.0 / 8 * level->walls.columns / (float)state->move_count +
        100.0 / 8 * level->walls.columns / time_taken) * (level->number + 1);

    bool need_update = (current_user->completed_levels & 1u << level->number) == 0
        || state->move_count < current_user->level_moves[level->number]
        || (state->move_count == current_user->level_moves[level->number] && time_taken < current_user->level_times[level->number]);

    if (need_update) {
        current_user->level_moves[level->number] = state->move_count;
        current_user->level_times[level->number] = time_taken;

        current_user->total_score = recalculate_total_score(*current_user);
        update_score(current_username, current_user->total_score);
    }

    current_user->completed_levels |= 1u << level->number;

    profile_end();
}

void bot_play_step(int value) {
    if (!bot_running || !bot_path) return;

    char move = bot_path[bot_step++];

    switch (move) {
        case 'u': case 'U': update_level(0, 1); break;
        case 'd': case 'D': update_level(0, -1); break;
        case 'l': case 'L': update_level(-1, 0); break;
        case 'r': case 'R': update_level(1, 0); break;
        default: ;
    }

    const GameState* state = get_game_state();
    if (state->is_level_completed || bot_path[bot_step] == '\0') {
        bot_running = false;
        bot_step = 0;
        free(bot_path);
        bot_path = NULL;
        return;
    }

    glutTimerFunc(250, bot_play_step, 0);
}

void start_bot() {
    if (bot_running) return;

    profile_start("solve_path()");
    char* path = solve_astar(get_current_level());
    profile_end();

    if (!path) return;

    bot_path = path;
    bot_running = true;

    glutTimerFunc(500, bot_play_step, 0);
}

bool start_game_prf(const int level) {
    profile_start("start_game()");
    bool result = start_game(level);
    profile_end();
    return result;
}

// Отрисовка меню игры
void batch_menu() {
    draw_rectangle(MENU_RECT_X, MENU_RECT_Y, MENU_RECT_WIDTH, MENU_RECT_HEIGHT, 0.0, 0.0, 0.0);
    draw_rectangle(BOT_BUTTON_X, BOT_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    draw_rectangle(NEXT_BUTTON_X, NEXT_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    draw_rectangle(PREV_BUTTON_X, PREV_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    draw_rectangle(LEVEL_BUTTON_X, LEVEL_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    draw_rectangle(RESTART_BUTTON_X, RESTART_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    draw_rectangle(LEADERBOARD_BUTTON_X, LEADERBOARD_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    draw_rectangle(CONTROLS_BUTTON_X, CONTROLS_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    draw_rectangle(QUIT_BUTTON_X, QUIT_BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, 0.5, 0.5, 0.5);

    draw_text(BOT_BUTTON_X + 0.05, BOT_BUTTON_Y + 0.05, "Start Bot", 1.0, 1.0, 1.0);
    draw_text(NEXT_BUTTON_X + 0.05, NEXT_BUTTON_Y + 0.05, "Next Level", 1.0, 1.0, 1.0);
    draw_text(PREV_BUTTON_X + 0.05, PREV_BUTTON_Y + 0.05, "Previous Level", 1.0, 1.0, 1.0);
    draw_text(LEVEL_BUTTON_X + 0.05, LEVEL_BUTTON_Y + 0.05, "Choose Level", 1.0, 1.0, 1.0);
    draw_text(RESTART_BUTTON_X + 0.05, RESTART_BUTTON_Y + 0.05, "Restart Level", 1.0, 1.0, 1.0);
    draw_text(LEADERBOARD_BUTTON_X + 0.05, LEADERBOARD_BUTTON_Y + 0.05, "Leaderboard", 1.0, 1.0, 1.0);
    draw_text(CONTROLS_BUTTON_X + 0.05, CONTROLS_BUTTON_Y + 0.05, "Controls", 1.0, 1.0, 1.0);
    draw_text(QUIT_BUTTON_X + 0.05, QUIT_BUTTON_Y + 0.05, "Quit", 1.0, 1.0, 1.0);
}

// Отрисовка игры
void draw_game() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    const GameState* state = get_game_state();
    batch_reset();

    draw_rectangle(GAME_RECT_X, GAME_RECT_Y, GAME_RECT_WIDTH, GAME_RECT_HEIGHT, 0.999, 0.844, 0.600);

    const Level* level = get_current_level();
    const int rows = level->walls.rows;
    const int columns = level->walls.columns;

    const float cellWidth = GAME_RECT_WIDTH / columns;
    const float cellHeight = GAME_RECT_HEIGHT / rows;

    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            const float x = GAME_RECT_X + j * cellWidth;
            const float y = GAME_RECT_Y + i * cellHeight;
            draw_cell(x, y, cellWidth, cellHeight, get_entity_type(i, j));
        }
    }

    if (state->is_level_completed) {
        draw_text(-0.2, 0.0, "LEVEL COMPLETE!", 0.0, 1.0, 0.0);
    }

    const int displayTime = (state->is_level_completed && state->end_time != 0)
        ? (state->end_time - state->start_time) / 1000
        : (get_time_ms() - state->start_time) / 1000;

    const int hours = displayTime / 3600;
    const int minutes = (displayTime % 3600) / 60;
    const int seconds = displayTime % 60;
    char timerText[20];
    snprintf(timerText, sizeof(timerText), "Time: %02d:%02d:%02d", hours, minutes, seconds);
    draw_text(-0.2, 0.8, timerText, 1.0, 1.0, 1.0);

    char movesText[20];
    snprintf(movesText, sizeof(movesText), "Moves: %d", state->move_count);
    draw_text(-0.2, 0.75, movesText, 1.0, 1.0, 1.0);

    if (strlen(current_username) > 0) {
        char scoreText[30];
        snprintf(scoreText, sizeof(scoreText), "Score: %.2f", current_user->total_score);
        draw_text(-0.2, 0.70, scoreText, 1.0, 1.0, 1.0);
    }

    draw_text(0.6, 0.8, current_username, 1.0, 1.0, 0.0);

    batch_menu();
    batch_flush();

    glutSwapBuffers();
}

// Отрисовка окна авторизации
void draw_login() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    batch_reset();

    draw_rectangle(-0.5, -0.5, 1.0, 1.0, 0.2, 0.2, 0.2);

    if (is_username_entered && is_register_mode) {
        float r = 0, g = 0;
        switch (password_strength) {
            case 0:
            case 1: r = 1.0; g = 0.0; break;
            case 2: r = 1.0; g = 0.5; break;
            case 3: r = 1.0; g = 1.0; break;
            case 4:
            case 5: r = 0.0; g = 1.0; break;
            default: ;
        }

        draw_rectangle(PASSWORD_STRENGTH_X, PASSWORD_STRENGTH_Y, PASSWORD_STRENGTH_WIDTH, PASSWORD_STRENGTH_HEIGHT, r, g, 0.0);

        char strengthText[20];
        snprintf(strengthText, sizeof(strengthText), "Strength: %d/5", password_strength);
        draw_text(PASSWORD_STRENGTH_X + 0.05, PASSWORD_STRENGTH_Y + 0.03, strengthText, 1.0, 1.0, 1.0);
    }

    draw_rectangle(TOGGLE_BUTTON_X, TOGGLE_BUTTON_Y, TOGGLE_BUTTON_WIDTH, TOGGLE_BUTTON_HEIGHT, 0.5, 0.5, 0.5);
    draw_rectangle(QUIT_LOGIN_BUTTON_X, QUIT_LOGIN_BUTTON_Y, QUIT_LOGIN_BUTTON_WIDTH, QUIT_LOGIN_BUTTON_HEIGHT, 0.5, 0.5, 0.5);

    draw_text(-0.4, 0.4, is_register_mode ? "Register" : "Login", 1.0, 1.0, 1.0);
    draw_text(-0.4, 0.2, "ENTER YOUR USERNAME (just enter):", 1.0, 1.0, 1.0);
    draw_text(-0.1, 0.1, current_username, 1.0, 1.0, 1.0);
    draw_text(-0.4, 0.0, is_username_entered ? "ENTER YOUR PASSWORD (just enter):" : "", 1.0, 1.0, 1.0);

    draw_text(TOGGLE_BUTTON_X + 0.05, TOGGLE_BUTTON_Y + 0.05, "Toggle Mode", 1.0, 1.0, 1.0);
    draw_text(QUIT_LOGIN_BUTTON_X + 0.05, QUIT_LOGIN_BUTTON_Y + 0.05, "Quit", 1.0, 1.0, 1.0);
    draw_text(-0.4, -0.3, "Enter: Next/Submit", 0.8, 0.8, 0.8);
    draw_text(-0.4, -0.6, "Username: 4-20 chars, Password: 8-20 chars", 0.8, 0.8, 0.8);

    char maskedPassword[MAX_PASSWORD];
    const int password_len = strlen(current_password);
    memset(maskedPassword, '*', password_len);
    maskedPassword[password_len] = '\0';
    draw_text(-0.1, -0.1, is_username_entered ? maskedPassword : "", 1.0, 1.0, 1.0);

    if (strlen(errorMessage) > 0) {
        draw_text(-0.4, -0.5, errorMessage, 1.0, 0.0, 0.0);
    }

    batch_flush();
    glutSwapBuffers();
}

// Отрисовка выбора уровня
void draw_level_selector() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    batch_reset();

    for (int i = 0; i < LEVEL_COUNT; i++) {
        float x = LEVEL_SELECT_BASE_X + (i % 5) * (LEVEL_SELECT_BUTTON_WIDTH + 0.05);
        float y = LEVEL_SELECT_BASE_Y - (i / 5) * (LEVEL_SELECT_BUTTON_HEIGHT + 0.05);
        draw_rectangle(x, y, LEVEL_SELECT_BUTTON_WIDTH, LEVEL_SELECT_BUTTON_HEIGHT, 0.5, 0.5, 0.5);
        char levelText[4];
        snprintf(levelText, sizeof(levelText), "%d", i + 1);
        draw_text(x + 0.03f, y + 0.05f, levelText, 1.0, 1.0, 1.0);
    }

    draw_rectangle(-0.1, -0.8, 0.2, 0.1, 0.5, 0.5, 0.5);

    draw_text(-0.9, 0.9, "SELECT LEVEL:", 1.0, 1.0, 1.0);
    draw_text(-0.05, -0.75, "Back", 1.0, 1.0, 1.0);

    batch_flush();
    glutSwapBuffers();
}

// Отрисовка таблицы лидеров
void draw_leaderboard() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION); glLoadIdentity(); gluOrtho2D(-1.0,1.0,-1.0,1.0);
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();

    batch_reset();

    draw_rectangle(-1.0f, -1.0f, 2.0f, 2.0f, 0.0f, 0.0f, 0.0f);
    draw_rectangle(-0.1f, -0.8f, 0.2f, 0.1f, 0.5f, 0.5f, 0.5f);

    draw_text(-0.9, 0.9, "LEADERBOARD:", 1.0, 1.0, 1.0);
    profile_start("get_users()");
    const UserArray* users = get_users();
    profile_end();

    const int count = users->length > 10 ? 10 : users->length;
    for (int index = 0; index < count; index++) {
        char entry[50];
        snprintf(entry, sizeof(entry), "%d. %s - %.2f", index + 1, users->users[index].username, users->users[index].total_score);
        draw_text(-0.8, 0.7 - index * 0.1, entry, 1.0, 1.0, 1.0);
    }

    draw_text(-0.05, -0.75, "Back", 1.0, 1.0, 1.0);

    batch_flush();
    glutSwapBuffers();
}

// Отрисовка демонстрационного уровня
void draw_tutorial() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    batch_reset();
    draw_rectangle(GAME_RECT_X, GAME_RECT_Y, GAME_RECT_WIDTH, GAME_RECT_HEIGHT, 0.999, 0.844, 0.600);

    const Level* level = get_current_level();
    const int rows = level->walls.rows;
    const int columns = level->walls.columns;

    const float cellWidth = GAME_RECT_WIDTH / columns;
    const float cellHeight = GAME_RECT_HEIGHT / rows;

    const GameState* state = get_game_state();
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < columns; j++) {
            const float x = GAME_RECT_X + j * cellWidth;
            const float y = GAME_RECT_Y + i * cellHeight;
            draw_cell(x, y, cellWidth, cellHeight, get_entity_type(i, j));
        }
    }

    char hint[50];
    switch (state->move_count) {
        case 0:
            snprintf(hint, sizeof(hint), "Press D or Right Arrow to move right");
            break;
        case 1:
            snprintf(hint, sizeof(hint), "Press D or Right Arrow to push the box");
            break;
        case 2:
            snprintf(hint, sizeof(hint), "Press S or Down Arrow to push the box to the target");
            break;
        default:
            snprintf(hint, sizeof(hint), "Tutorial Complete!");
            break;
    }

    draw_text(-0.4, 0.8, "Welcome to Sokoban! Watch how to play:", 1.0, 1.0, 1.0);
    draw_text(-0.4, 0.7, hint, 0.0, 1.0, 0.0);

    batch_flush();

    glutSwapBuffers();
}

// Обновление анимации
void update_tutorial_animation() {
    const GameState* game_state = get_game_state();
    if (game_state->move_count >= tutorialMoveCount) {
        current_mode = GAME;
        is_first_game = false;
        start_game_prf(1);
        return;
    }

    const char move = tutorialMoves[game_state->move_count];
    switch (move) {
        case 'w': case 'W': move_player_prf(0, 1); break;
        case 's': case 'S': move_player_prf(0, -1); break;
        case 'a': case 'A': move_player_prf(-1, 0); break;
        case 'd': case 'D': move_player_prf(1, 0); break;
        default: ;
    }

    glutTimerFunc(1500, update_tutorial_animation, 0);
}

// Отрисовка окна управления
void draw_controls() {
    glClear(GL_COLOR_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    batch_reset();
    draw_rectangle(-0.8, -0.8, 1.6, 1.6, 0.2, 0.2, 0.2);
    draw_rectangle(-0.6, 0.5, 0.2, 0.1, 0.0, 0.5, 1.0);
    draw_rectangle(-0.6, 0.3, 0.2, 0.1, 0.0, 0.5, 1.0);
    draw_rectangle(-0.6, 0.1, 0.2, 0.1, 0.0, 0.5, 1.0);
    draw_rectangle(-0.6, -0.1, 0.2, 0.1, 0.0, 0.5, 1.0);
    draw_rectangle(-0.6, -0.3, 0.2, 0.1, 0.0, 0.8, 0.0);
    draw_rectangle(-0.1, -0.8, 0.2, 0.1, 0.5, 0.5, 0.5);

    draw_text(-0.4, 0.8, "Controls", 1.0, 1.0, 1.0);

    draw_text(-0.55, 0.55, "W / Up Arrow", 1.0, 1.0, 1.0);
    draw_text(-0.3, 0.55, ": Move Up", 1.0, 1.0, 1.0);

    draw_text(-0.55, 0.35, "S / Down Arrow", 1.0, 1.0, 1.0);
    draw_text(-0.3, 0.35, ": Move Down", 1.0, 1.0, 1.0);

    draw_text(-0.55, 0.15, "A / Left Arrow", 1.0, 1.0, 1.0);
    draw_text(-0.3, 0.15, ": Move Left", 1.0, 1.0, 1.0);

    draw_text(-0.55, -0.05, "D / Right Arrow", 1.0, 1.0, 1.0);
    draw_text(-0.3, -0.05, ": Move Right", 1.0, 1.0, 1.0);

    draw_text(-0.55, -0.25, "Enter", 1.0, 1.0, 1.0);
    draw_text(-0.3, -0.25, ": Confirm / Submit", 1.0, 1.0, 1.0);

    draw_text(-0.05, -0.75, "Back", 1.0, 1.0, 1.0);


    batch_flush();
    glutSwapBuffers();
}

// Главная функция отображения
void draw() {
    glClear(GL_COLOR_BUFFER_BIT);
    switch (current_mode) {
        case LOGIN:
            profile_start("draw_login()");
            draw_login();
            profile_end();
            break;
        case GAME:
            profile_start("draw_game()");
            draw_game();
            profile_end();
            break;
        case LEVEL_SELECTOR:
            profile_start("draw_level_selector");
            draw_level_selector();
            profile_end();
            break;
        case LEADERBOARD:
            profile_start("draw_leaderboard()");
            draw_leaderboard();
            profile_end();
            break;
        case TUTORIAL:
            profile_start("draw_tutorial()");
            draw_tutorial();
            profile_end();
            break;
        case CONTROLS:
            profile_start("draw_controls()");
            draw_controls();
            profile_end();
            break;
    }
}

// Обработчик изменения размеров
void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

// Обработчик мыши
void mouse(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        float glX = (float)x / glutGet(GLUT_WINDOW_WIDTH) * 2.0 - 1.0;
        float glY = -((float)y / glutGet(GLUT_WINDOW_HEIGHT) * 2.0 - 1.0);

        const Level* level = get_current_level();

        if (current_mode == LOGIN) {
            if (glX >= TOGGLE_BUTTON_X && glX <= TOGGLE_BUTTON_X + TOGGLE_BUTTON_WIDTH &&
                glY >= TOGGLE_BUTTON_Y && glY <= TOGGLE_BUTTON_Y + TOGGLE_BUTTON_HEIGHT) {
                is_register_mode = !is_register_mode;
                strcpy(errorMessage, "");
            }
            else if (glX >= QUIT_LOGIN_BUTTON_X && glX <= QUIT_LOGIN_BUTTON_X + QUIT_LOGIN_BUTTON_WIDTH &&
                     glY >= QUIT_LOGIN_BUTTON_Y && glY <= QUIT_LOGIN_BUTTON_Y + QUIT_LOGIN_BUTTON_HEIGHT) {
                exit_game();
            }
        }

        else if (current_mode == GAME) {
            if (glX >= BOT_BUTTON_X && glX <= BOT_BUTTON_X + BUTTON_WIDTH &&
                glY >= BOT_BUTTON_Y && glY <= BOT_BUTTON_Y + BUTTON_HEIGHT) {
                start_bot();
                }

            if (glX >= NEXT_BUTTON_X && glX <= NEXT_BUTTON_X + BUTTON_WIDTH &&
                glY >= NEXT_BUTTON_Y && glY <= NEXT_BUTTON_Y + BUTTON_HEIGHT && level->number < LEVEL_COUNT) {
                start_game_prf(level->number + 1);
            }
            else if (glX >= PREV_BUTTON_X && glX <= PREV_BUTTON_X + BUTTON_WIDTH &&
                     glY >= PREV_BUTTON_Y && glY <= PREV_BUTTON_Y + BUTTON_HEIGHT && level->number > 1) {
                start_game_prf(level->number - 1);
            }
            else if (glX >= LEVEL_BUTTON_X && glX <= LEVEL_BUTTON_X + BUTTON_WIDTH &&
                     glY >= LEVEL_BUTTON_Y && glY <= LEVEL_BUTTON_Y + BUTTON_HEIGHT) {
                current_mode = LEVEL_SELECTOR;
            }
            else if (glX >= RESTART_BUTTON_X && glX <= RESTART_BUTTON_X + BUTTON_WIDTH &&
                     glY >= RESTART_BUTTON_Y && glY <= RESTART_BUTTON_Y + BUTTON_HEIGHT) {
                start_game_prf(level->number);
            }
            else if (glX >= LEADERBOARD_BUTTON_X && glX <= LEADERBOARD_BUTTON_X + BUTTON_WIDTH &&
                     glY >= LEADERBOARD_BUTTON_Y && glY <= LEADERBOARD_BUTTON_Y + BUTTON_HEIGHT) {
                current_mode = LEADERBOARD;
            }
            else if (glX >= CONTROLS_BUTTON_X && glX <= CONTROLS_BUTTON_X + BUTTON_WIDTH &&
                     glY >= CONTROLS_BUTTON_Y && glY <= CONTROLS_BUTTON_Y + BUTTON_HEIGHT) {
                current_mode = CONTROLS;
            }
            else if (glX >= QUIT_BUTTON_X && glX <= QUIT_BUTTON_X + BUTTON_WIDTH &&
                     glY >= QUIT_BUTTON_Y && glY <= QUIT_BUTTON_Y + BUTTON_HEIGHT) {
                exit_game();
            }
        }
        else if (current_mode == LEVEL_SELECTOR) {
            for (int i = 0; i < LEVEL_COUNT - 1; i++) {
                float btnX = LEVEL_SELECT_BASE_X + (i % 5) * (LEVEL_SELECT_BUTTON_WIDTH + 0.05);
                float btnY = LEVEL_SELECT_BASE_Y - (i / 5) * (LEVEL_SELECT_BUTTON_HEIGHT + 0.05);
                if (glX >= btnX && glX <= btnX + LEVEL_SELECT_BUTTON_WIDTH &&
                    glY >= btnY && glY <= btnY + LEVEL_SELECT_BUTTON_HEIGHT) {
                    start_game_prf(i + 1);
                    current_mode = GAME;
                    break;
                }
            }
            if (glX >= -0.1 && glX <= 0.1 && glY >= -0.8 && glY <= -0.7) {
                current_mode = GAME;
            }
        }
        else if (current_mode == LEADERBOARD) {
            if (glX >= -0.1 && glX <= 0.1 && glY >= -0.8 && glY <= -0.7) {
                current_mode = GAME;
            }
        }
        else if (current_mode == CONTROLS) {
            if (glX >= -0.1 && glX <= 0.1 && glY >= -0.8 && glY <= -0.7) {
                current_mode = GAME;
            }
        }

    }
}

// Запуск игры
void start_sokoban() {
    levels_init();
    printf("Запуск игры для пользователя: %s\n", current_username);
    if (is_first_game) {
        current_mode = TUTORIAL;
        start_game_prf(0);
        glutTimerFunc(1500, update_tutorial_animation, 0);
    } else {
        current_mode = GAME;
        start_game_prf(1);
    }
}

// Обработчик клавиш для авторизации
void login_keyboard(unsigned char key, int x, int y) {
    if (key == 13) {
        if (!is_username_entered) {
            int len = strlen(current_username);
            if (len < MIN_USERNAME || len > MAX_USERNAME - 1) {
                snprintf(errorMessage, sizeof(errorMessage), "Username must be %d-%d characters!", MIN_USERNAME, MAX_USERNAME - 1);
            } else {
                is_username_entered = true;
                strcpy(errorMessage, "");
            }
        } else {
            int passLen = strlen(current_password);
            if (passLen < MIN_PASSWORD || passLen > MAX_PASSWORD - 1) {
                snprintf(errorMessage, sizeof(errorMessage), "Password must be %d-%d characters!", MIN_PASSWORD, MAX_PASSWORD - 1);
            }

            else if (is_register_mode) {
                password_strength = evaluate_password_strength(current_password);
                bool user_exists = find_user(current_username) != -1;
                if (password_strength <= 2) {
                    char feedback[100];
                    get_password_strength_feedback(password_strength, feedback);
                    snprintf(errorMessage, sizeof(errorMessage), "%s", feedback);
                }

                else if (user_exists)
                    snprintf(errorMessage, sizeof(errorMessage), "Username already exists!");

                else {
                    char hashed_password[SHA256_DIGEST_LENGTH * 2 + 1];
                    hash_password(current_password, hashed_password);
                    register_user(current_username, hashed_password);
                    is_login_screen = false;
                    is_first_game = true;
                    const int index = find_user(current_username);
                    current_user = &get_users()->users[index];

                    start_sokoban();
                }
            }

            else {
                const int index = find_user(current_username);
                if (index == -1) {
                    snprintf(errorMessage, sizeof(errorMessage), "Username does not exist!");
                    return;
                }

                User* user = &get_users()->users[index];
                char hashed_password[SHA256_DIGEST_LENGTH * 2 + 1];
                hash_password(current_password, hashed_password);
                if (strcmp(user->password, hashed_password) != 0) {
                    snprintf(errorMessage, sizeof(errorMessage), "Incorrect password!");
                    return;
                }

                current_user = user;
                is_login_screen = false;
                start_sokoban();
            }
        }
    }

    else if (key == 8) {
        const int password_len = strlen(current_password);
        if (is_username_entered && password_len > 0) {
            current_password[password_len - 1] = '\0';
            password_strength = evaluate_password_strength(current_password);
        } else if (!is_username_entered && password_len > 0) {
            current_username[password_len - 1] = '\0';
        }
        strcpy(errorMessage, "");
    } else if (key >= 32 && key <= 126) {
        int username_len = strlen(current_username);
        int password_len = strlen(current_password);
        if (!is_username_entered && username_len < MAX_USERNAME - 1) {
            current_username[username_len] = key;
            current_username[username_len+1] = '\0';
            strcpy(errorMessage, "");
        } else if (is_username_entered && password_len < MAX_PASSWORD - 1) {
            current_password[password_len] = key;
            current_password[password_len+1] = '\0';
            password_strength = evaluate_password_strength(current_password);
            strcpy(errorMessage, "");
        }
    }
}

// Обработчик клавиш для игры
void keyboard(unsigned char key, int x, int y) {
    if (current_mode == LOGIN) {
        login_keyboard(key, x, y);
    } else if (current_mode == GAME && !bot_running && !get_game_state()->is_level_completed) {
        switch (key) {
            case 'w': case 'W': update_level(0, 1); break;
            case 's': case 'S': update_level(0, -1); break;
            case 'a': case 'A': update_level(-1, 0); break;
            case 'd': case 'D': update_level(1, 0); break;
            default: ;
        }
    }

}

// Обработчик специальных клавиш
void special_keys(int key, int x, int y) {
    if (current_mode == GAME) {
        switch (key) {
            case GLUT_KEY_UP:    update_level(0, 1); break;
            case GLUT_KEY_DOWN:  update_level(0, -1); break;
            case GLUT_KEY_LEFT:  update_level(-1, 0); break;
            case GLUT_KEY_RIGHT: update_level(1, 0); break;
            default: ;
        }
    }
}