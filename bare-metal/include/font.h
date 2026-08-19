#pragma once

#include <stdint.h>

void font_draw_char(int x, int y, char character, uint32_t color);
void font_draw_string(int x, int y, const char* text, uint32_t color);
int font_char_width(void);
int font_char_height(void);
