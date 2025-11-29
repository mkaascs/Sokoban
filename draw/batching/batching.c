#include "batching.h"
#include <string.h>

#include "font.h"

#define BATCH_INITIAL_CAPACITY 512

typedef struct {
    float x, y;
    float u, v;
    float r, g, b, a;
} BatchVertex;

static BatchVertex* untextured_vertices = NULL;
static int untextured_count = 0;
static int untextured_capacity = 0;

static BatchVertex* textured_vertices = NULL;
static int textured_count = 0;
static int textured_capacity = 0;

static void ensure_capacity(BatchVertex** buf, const int* count, int* cap, int need);

void batch_init(void) {
    untextured_count = textured_count = 0;
    untextured_capacity = textured_capacity = 0;
    untextured_vertices = textured_vertices = NULL;
    ensure_capacity(&untextured_vertices, &untextured_count, &untextured_capacity, 1);
    ensure_capacity(&textured_vertices, &textured_count, &textured_capacity, 1);
}

void batch_reset() {
    untextured_count = 0;
    textured_count = 0;
}

void batch_add_rect(const float x, const float y, const float w, const float h, const float r, const float g, const float b) {
    ensure_capacity(&untextured_vertices, &untextured_count, &untextured_capacity, 6);
    const BatchVertex quad[6] = {
        {x,     y,     0, 0, r, g, b, 1},
        {x + w, y,     0, 0, r, g, b, 1},
        {x + w, y + h, 0, 0, r, g, b, 1},
        {x,     y,     0, 0, r, g, b, 1},
        {x + w, y + h, 0, 0, r, g, b, 1},
        {x,     y + h, 0, 0, r, g, b, 1},
    };

    memcpy(untextured_vertices + untextured_count, quad, sizeof(quad));
    untextured_count += 6;
}

void batch_add_textured_quad(float x1, float y1, float x2, float y2, float u1, float v1, float u2, float v2, float r, float g, float b, float a) {
    ensure_capacity(&textured_vertices, &textured_count, &textured_capacity, 6);
    const BatchVertex quad[6] = {
        {x1, y1, u1, v1, r, g, b, a},
        {x2, y1, u2, v1, r, g, b, a},
        {x2, y2, u2, v2, r, g, b, a},
        {x1, y1, u1, v1, r, g, b, a},
        {x2, y2, u2, v2, r, g, b, a},
        {x1, y2, u1, v2, r, g, b, a},
    };
    memcpy(textured_vertices + textured_count, quad, sizeof(quad));
    textured_count += 6;
}

void batch_flush() {
    if (untextured_count > 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_TEXTURE_2D);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(2, GL_FLOAT, sizeof(BatchVertex), &untextured_vertices[0].x);
        glColorPointer(4, GL_FLOAT, sizeof(BatchVertex), &untextured_vertices[0].r);

        glDrawArrays(GL_TRIANGLES, 0, untextured_count);

        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisable(GL_BLEND);

        untextured_count = 0;
    }

    if (textured_count > 0) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, font_atlas.texture_id);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        glVertexPointer(2, GL_FLOAT, sizeof(BatchVertex), &textured_vertices[0].x);
        glColorPointer(4, GL_FLOAT, sizeof(BatchVertex), &textured_vertices[0].r);
        glTexCoordPointer(2, GL_FLOAT, sizeof(BatchVertex), &textured_vertices[0].u);

        glDrawArrays(GL_TRIANGLES, 0, textured_count);

        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_VERTEX_ARRAY);
        glDisable(GL_TEXTURE_2D);
        glDisable(GL_BLEND);

        textured_count = 0;
    }
}

void batch_free(void) {
    free(untextured_vertices);
    free(textured_vertices);
    untextured_vertices = textured_vertices = NULL;
    untextured_count = textured_count = 0;
    untextured_capacity = textured_capacity = 0;
}

static void ensure_capacity(BatchVertex** buf, const int* count, int* cap, const int need) {
    if (*count + need <= *cap) return;

    int new_cap = *cap > 0 ? *cap : BATCH_INITIAL_CAPACITY;
    while (new_cap < *count + need) new_cap *= 2;

    BatchVertex* new_buf = (BatchVertex*)realloc(*buf, new_cap * sizeof(BatchVertex));
    if (new_buf == NULL) return;
    *buf = new_buf;
    *cap = new_cap;
}