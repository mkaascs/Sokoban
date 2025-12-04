cmake_minimum_required(VERSION 3.15)
project(sokoban C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Пути для Homebrew (важно для macOS)
execute_process(
        COMMAND brew --prefix openssl@3
        OUTPUT_VARIABLE OPENSSL_ROOT_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE
)
execute_process(
        COMMAND brew --prefix freeglut
        OUTPUT_VARIABLE FREEGLUT_ROOT
        OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Указываем CMake, где искать include/lib
include_directories(${FREEGLUT_ROOT}/include)
link_directories(${FREEGLUT_ROOT}/lib)

include_directories(${OPENSSL_ROOT_DIR}/include)
link_directories(${OPENSSL_ROOT_DIR}/lib)

# Подключаем OpenSSL
find_package(OpenSSL REQUIRED)
if (NOT OpenSSL_FOUND)
    message(FATAL_ERROR "OpenSSL not found! Make sure 'brew install openssl@3' is done.")
endif()

# Подключаем OpenGL / GLUT
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)

if (APPLE)
    # На macOS иногда нужно использовать фреймворки
    set(EXTRA_LIBS "-framework OpenGL -framework GLUT")
endif()

# Указываем исходники
add_executable(sokoban main.c
        leaderboard/leaderboard.h
        leaderboard/leaderboard.c
        leaderboard/crypto.h
        leaderboard/crypto.c
        game/levels.h
        game/game.h
        game/levels.c
        game/game.c
        draw/draw.h
        draw/draw.c
        draw/batching/batching.h
        draw/batching/batching.c
        profile/profile.h
        profile/profile.c
        draw/batching/font.h
        draw/batching/font.c
        game/bitboard.h
        game/bitboard.c
        game/bot/bot.h
        game/bot/bot.c
)

# Машинно-зависимая оптимизация
target_compile_options(sokoban PRIVATE -O3 -march=native -flto -ffast-math)
target_compile_options(sokoban PRIVATE -fstrict-aliasing -Wstrict-aliasing)
target_link_options(sokoban PRIVATE -flto)

# Линкуем всё
target_link_libraries(sokoban
        OpenSSL::SSL
        OpenSSL::Crypto
        ${OPENGL_LIBRARIES}
        ${GLUT_LIBRARIES}
        ${EXTRA_LIBS}
)