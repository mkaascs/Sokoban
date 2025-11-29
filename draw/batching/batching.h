#ifndef BATCHING_H
#define BATCHING_H

#include <stdbool.h>

#define BATCH_MAX_VERTICES 65536  // ~10k квадов

void batch_init();
void batch_reset();
void batch_flush();
void batch_free();

void batch_add_rect(float x, float y, float w, float h, float r, float g, float b);

void batch_add_textured_quad(
    float x1, float y1,
    float x2, float y2,
    float u1, float v1,
    float u2, float v2,
    float r, float g, float b, float a
);

#endif //BATCHING_H
