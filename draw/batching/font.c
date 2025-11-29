#include "font.h"

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "batching.h"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"
#include "../../profile/profile.h"

FontAtlas font_atlas;

#define PIXEL_HEIGHT 24.0f
#define FONT_FILENAME "Lato-Regular.ttf"

static unsigned char ttf_buffer[1 << 20];
static stbtt_fontinfo font_info;

bool font_init() {
    FILE* file = fopen(FONT_FILENAME, "rb");
    if (file == NULL)
        return false;

    fread(ttf_buffer, 1, sizeof(ttf_buffer), file);
    fclose(file);

    if (!stbtt_InitFont(&font_info, ttf_buffer, 0)) {
        return false;
    }

    float scale = stbtt_ScaleForPixelHeight(&font_info, PIXEL_HEIGHT);

    const int atlas_w = 512;
    const int atlas_h = 512;
    unsigned char* atlas_pixels = calloc(atlas_w * atlas_h, 1);

    int x = 0, y = 0, row_h = 0;

    for (int c = FIRST_CHAR; c <= LAST_CHAR; ++c) {
        Glyph* glyph = &font_atlas.glyphs[c - FIRST_CHAR];
        memset(glyph, 0, sizeof(Glyph));

        if (c == ' ') {
            int advance, lsb;
            stbtt_GetCodepointHMetrics(&font_info, c, &advance, &lsb);
            glyph->advance = (int)(scale * advance);
            continue;
        }

        int x0, y0, g_w, g_h;
        unsigned char* bitmap = stbtt_GetCodepointBitmap(
            &font_info, scale, scale, c, &g_w, &g_h, &x0, &y0
        );

        if (g_w == 0 || g_h == 0) {
            stbtt_FreeBitmap(bitmap, NULL);
            int advance, lsb;
            stbtt_GetCodepointHMetrics(&font_info, c, &advance, &lsb);
            glyph->advance = (int)(scale * advance);
            continue;
        }

        if (x + g_w > atlas_w) {
            x = 0;
            y += row_h;
            row_h = 0;
            if (y + g_h > atlas_h) {
                stbtt_FreeBitmap(bitmap, NULL);
                break;
            }
        }

        if (g_h > row_h) row_h = g_h;

        glyph->width = g_w;
        glyph->height = g_h;
        glyph->x_offset = x0;
        glyph->y_offset = y0;

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&font_info, c, &advance, &lsb);
        glyph->advance = (int)(scale * advance);

        glyph->u1 = (float)x / atlas_w;
        glyph->v2 = (float)y / atlas_h;
        glyph->u2 = (float)(x + g_w) / atlas_w;
        glyph->v1 = (float)(y + g_h) / atlas_h;

        for (int iy = 0; iy < g_h; ++iy) {
            memcpy(
                atlas_pixels + (y + iy) * atlas_w + x,
                bitmap + iy * g_w,
                g_w
            );
        }

        stbtt_FreeBitmap(bitmap, NULL);
        x += g_w;
    }

    font_atlas.text_width = atlas_w;
    font_atlas.text_height = atlas_h;

    glGenTextures(1, &font_atlas.texture_id);
    glBindTexture(GL_TEXTURE_2D, font_atlas.texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, atlas_w, atlas_h, 0, GL_ALPHA, GL_UNSIGNED_BYTE, atlas_pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    free(atlas_pixels);
    return true;
}

void draw_text_batch(float x_ndc, float y_ndc, const char* text, float r, float g, float b) {
    profile_start();
    if (!text || !*text) return;

    const int win_w = glutGet(GLUT_WINDOW_WIDTH);
    const int win_h = glutGet(GLUT_WINDOW_HEIGHT);
    if (win_w <= 0 || win_h <= 0) return;

    float px_per_ndc_x = win_w / 2.0f;
    float px_per_ndc_y = win_h / 2.0f;

    float pen_x = (x_ndc + 1.0f) * px_per_ndc_x;
    float pen_y = (1.0f - y_ndc) * px_per_ndc_y;

    for (int i = 0; text[i]; ++i) {
        const unsigned char c = (unsigned char)text[i];
        if (c < FIRST_CHAR || c > LAST_CHAR) {
            const Glyph *space = &font_atlas.glyphs[' ' - FIRST_CHAR];
            pen_x += space->advance;
            continue;
        }

        const Glyph *glyph = &font_atlas.glyphs[c - FIRST_CHAR];
        if (glyph->width == 0 || glyph->height == 0) {
            pen_x += glyph->advance;
            continue;
        }

        const float glyph_x = pen_x + glyph->x_offset;
        const float glyph_y = pen_y + glyph->y_offset;

        const float x1 = (glyph_x / win_w) * 2.0f - 1.0f;
        const float y1 = 1.0f - (glyph_y / win_h) * 2.0f;
        const float x2 = ((glyph_x + glyph->width) / win_w) * 2.0f - 1.0f;
        const float y2 = 1.0f - ((glyph_y + glyph->height) / win_h) * 2.0f;

        batch_add_textured_quad(x1, y1, x2, y2, glyph->u1, glyph->v2, glyph->u2, glyph->v1, r, g, b, 1.0f);
        pen_x += glyph->advance;
    }

    profile_end("draw_text_batch()");
}