#pragma once

#include <stdint.h>

#include "multiboot2.h"

struct framebuffer {
    uint32_t* address;
    uint32_t width;
    uint32_t height;
    uint32_t pitch;
    uint8_t bpp;
};

int framebuffer_init(struct framebuffer* fb, struct multiboot_boot_info* boot_info);
uint32_t* framebuffer_at(struct framebuffer* fb, int x, int y);
