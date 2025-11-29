#ifndef FONT_H
#define FONT_H

#include <stdbool.h>
#include <GL/freeglut_std.h>

#define FIRST_CHAR 32
#define LAST_CHAR 126
#define CHAR_COUNT (LAST_CHAR - FIRST_CHAR + 1)

typedef struct {
    float u1, v1, u2, v2;
    int width, height;
    int advance;
    int x_offset;
    int y_offset;
} Glyph;

typedef struct {
    GLuint texture_id;
    int text_width;
    int text_height;
    Glyph glyphs[CHAR_COUNT];
} FontAtlas;

extern FontAtlas font_atlas;

bool font_init();
void draw_text_batch(float x, float y, const char* text, float r, float g, float b);

#endif //FONT_H
