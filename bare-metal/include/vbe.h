#pragma once

#include "framebuffer.h"

int bochs_vbe_init(struct framebuffer* fb, uint32_t width, uint32_t height, uint8_t bpp);
