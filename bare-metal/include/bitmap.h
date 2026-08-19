#pragma once

#include <stdint.h>

typedef struct {
    uint32_t width;
    uint32_t height;
    const uint32_t* pixels;
} bitmap_t;
