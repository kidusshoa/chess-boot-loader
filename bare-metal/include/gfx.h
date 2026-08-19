#pragma once

#include <stdint.h>

#include "bitmap.h"
#include "framebuffer.h"

void gfx_bind(struct framebuffer* fb);
struct framebuffer* gfx_framebuffer(void);
void gfx_clear(uint32_t color);
void gfx_fill_rect(int x, int y, int width, int height, uint32_t color);
void gfx_draw_rect(int x, int y, int width, int height, uint32_t color);
void gfx_fill_circle(int center_x, int center_y, int radius, uint32_t color);
void gfx_blit_rgba(int x, int y, const bitmap_t* bitmap);
void gfx_draw_text(int x, int y, const char* text, uint32_t color);
