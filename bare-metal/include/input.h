#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum {
    KEY_NONE = 0,
    KEY_UP,
    KEY_DOWN,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_SELECT,
    KEY_RESTART,
} key_t;

typedef struct {
    key_t key;
    bool mouse_click;
    int mouse_x;
    int mouse_y;
} input_event_t;

void input_init(void);
void input_set_screen_size(int width, int height);
input_event_t input_poll(void);
