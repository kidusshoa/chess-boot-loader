#include "gfx.h"

#include "font.h"

static struct framebuffer* active_fb;

static void gfx_put_pixel(int x, int y, uint32_t color) {
    if (!active_fb || !active_fb->address) {
        return;
    }

    if (active_fb->bpp == 8) {
        uint8_t* pixels = (uint8_t*)active_fb->address;
        pixels[(uint32_t)y * active_fb->pitch + (uint32_t)x] = (uint8_t)color;
        return;
    }

    uint32_t* pixel = framebuffer_at(active_fb, x, y);
    if (pixel) {
        *pixel = color;
    }
}

void gfx_bind(struct framebuffer* fb) {
    active_fb = fb;
}

struct framebuffer* gfx_framebuffer(void) {
    return active_fb;
}

void gfx_clear(uint32_t color) {
    if (!active_fb || !active_fb->address) {
        return;
    }

    for (uint32_t y = 0; y < active_fb->height; ++y) {
        for (uint32_t x = 0; x < active_fb->width; ++x) {
            gfx_put_pixel((int)x, (int)y, color);
        }
    }
}

void gfx_fill_rect(int x, int y, int width, int height, uint32_t color) {
    if (!active_fb || !active_fb->address || width <= 0 || height <= 0) {
        return;
    }

    for (int row = y; row < y + height; ++row) {
        for (int col = x; col < x + width; ++col) {
            gfx_put_pixel(col, row, color);
        }
    }
}

void gfx_draw_rect(int x, int y, int width, int height, uint32_t color) {
    gfx_fill_rect(x, y, width, 1, color);
    gfx_fill_rect(x, y + height - 1, width, 1, color);
    gfx_fill_rect(x, y, 1, height, color);
    gfx_fill_rect(x + width - 1, y, 1, height, color);
}

void gfx_fill_circle(int center_x, int center_y, int radius, uint32_t color) {
    const int radius_sq = radius * radius;

    for (int py = -radius; py <= radius; ++py) {
        for (int px = -radius; px <= radius; ++px) {
            if (px * px + py * py <= radius_sq) {
                gfx_put_pixel(center_x + px, center_y + py, color);
            }
        }
    }
}

void gfx_blit_rgba(int x, int y, const bitmap_t* bitmap) {
    if (!active_fb || !active_fb->address || !bitmap || !bitmap->pixels || active_fb->bpp == 8) {
        return;
    }

    for (uint32_t row = 0; row < bitmap->height; ++row) {
        for (uint32_t col = 0; col < bitmap->width; ++col) {
            const uint32_t source = bitmap->pixels[row * bitmap->width + col];
            const uint8_t alpha = (uint8_t)(source >> 24);

            if (alpha == 0) {
                continue;
            }

            if (alpha == 255) {
                gfx_put_pixel(x + (int)col, y + (int)row, source & 0x00FFFFFFu);
                continue;
            }

            uint32_t* destination_ptr = framebuffer_at(active_fb, x + (int)col, y + (int)row);
            if (!destination_ptr) {
                continue;
            }

            const uint32_t destination = *destination_ptr;
            const uint8_t dst_b = (uint8_t)(destination & 0xFF);
            const uint8_t dst_g = (uint8_t)((destination >> 8) & 0xFF);
            const uint8_t dst_r = (uint8_t)((destination >> 16) & 0xFF);

            const uint8_t src_b = (uint8_t)(source & 0xFF);
            const uint8_t src_g = (uint8_t)((source >> 8) & 0xFF);
            const uint8_t src_r = (uint8_t)((source >> 16) & 0xFF);

            const uint8_t inv_alpha = (uint8_t)(255 - alpha);
            const uint8_t out_r = (uint8_t)((src_r * alpha + dst_r * inv_alpha) / 255);
            const uint8_t out_g = (uint8_t)((src_g * alpha + dst_g * inv_alpha) / 255);
            const uint8_t out_b = (uint8_t)((src_b * alpha + dst_b * inv_alpha) / 255);

            gfx_put_pixel(x + (int)col, y + (int)row, GFX_RGB(out_r, out_g, out_b));
        }
    }
}

void gfx_draw_text(int x, int y, const char* text, uint32_t color) {
    font_draw_string(x, y, text, color);
}

int gfx_is_8bpp(void) {
    return active_fb && active_fb->bpp == 8;
}
