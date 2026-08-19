#pragma once

#include <stdbool.h>

typedef enum {
    KEY_NONE = 0,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_SELECT,
    KEY_RESTART,
} key_t;

void keyboard_init(void);
key_t keyboard_poll(void);
