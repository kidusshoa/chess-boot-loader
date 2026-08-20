#include "input.h"

#include "io.h"

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

static bool extended_prefix;
static bool set2_break_prefix;

static int screen_width = 640;
static int screen_height = 480;

static int mouse_x = 320;
static int mouse_y = 240;
static bool mouse_left_down;
static bool mouse_click_pending;

static uint8_t mouse_packet[3];
static int mouse_packet_index;

static void ps2_wait_write(void) {
    for (int i = 0; i < 100000; ++i) {
        if ((inb(PS2_STATUS) & 0x02) == 0) {
            return;
        }
        io_wait();
    }
}

static bool ps2_wait_read(uint8_t* value) {
    for (int i = 0; i < 100000; ++i) {
        if (inb(PS2_STATUS) & 0x01) {
            *value = inb(PS2_DATA);
            return true;
        }
        io_wait();
    }
    return false;
}

static void ps2_flush_output(void) {
    for (int i = 0; i < 32; ++i) {
        if ((inb(PS2_STATUS) & 0x01) == 0) {
            return;
        }
        (void)inb(PS2_DATA);
    }
}

static void ps2_write_command(uint8_t value) {
    ps2_wait_write();
    outb(PS2_COMMAND, value);
}

static void ps2_write_keyboard(uint8_t value) {
    ps2_wait_write();
    outb(PS2_COMMAND, 0xD4);
    ps2_wait_write();
    outb(PS2_DATA, value);
}

static void ps2_write_mouse(uint8_t value) {
    ps2_wait_write();
    outb(PS2_COMMAND, 0xD4);
    ps2_wait_write();
    outb(PS2_DATA, value);
}

static bool ps2_read_ack(void) {
    uint8_t response = 0;
    if (!ps2_wait_read(&response)) {
        return false;
    }
    return response == 0xFA;
}

static void ps2_configure_controller(void) {
    ps2_write_command(0x20);
    uint8_t config = 0;
    if (!ps2_wait_read(&config)) {
        return;
    }

    config &= ~(uint8_t)0x30;
    config |= 0x40;

    ps2_write_command(0x60);
    ps2_wait_write();
    outb(PS2_DATA, config);
}

static void keyboard_enable_scanning(void) {
    ps2_write_command(0xAE);
    io_wait();

    ps2_write_keyboard(0xF4);
    (void)ps2_read_ack();
}

static void mouse_clamp_position(void) {
    if (mouse_x < 0) {
        mouse_x = 0;
    }
    if (mouse_y < 0) {
        mouse_y = 0;
    }
    if (mouse_x >= screen_width) {
        mouse_x = screen_width - 1;
    }
    if (mouse_y >= screen_height) {
        mouse_y = screen_height - 1;
    }
}

static void mouse_handle_packet(void) {
    const uint8_t flags = mouse_packet[0];
    int dx = (int)(int8_t)mouse_packet[1];
    int dy = (int)(int8_t)mouse_packet[2];

    if (flags & 0x10) {
        dx -= 256;
    }
    if (flags & 0x20) {
        dy -= 256;
    }

    mouse_x += dx;
    mouse_y -= dy;
    mouse_clamp_position();

    const bool left = (flags & 0x01) != 0;
    if (left && !mouse_left_down) {
        mouse_click_pending = true;
    }
    mouse_left_down = left;
}

static void mouse_handle_byte(uint8_t byte) {
    if (mouse_packet_index == 0) {
        if ((byte & 0x08) == 0) {
            return;
        }
        mouse_packet[0] = byte;
        mouse_packet_index = 1;
        return;
    }

    mouse_packet[mouse_packet_index++] = byte;
    if (mouse_packet_index < 3) {
        return;
    }

    mouse_packet_index = 0;
    mouse_handle_packet();
}

static void mouse_enable_streaming(void) {
    ps2_write_command(0xA8);
    io_wait();

    ps2_write_mouse(0xFF);
    (void)ps2_read_ack();
    uint8_t response = 0;
    (void)ps2_wait_read(&response);
    (void)ps2_wait_read(&response);

    ps2_write_mouse(0xF6);
    (void)ps2_read_ack();

    ps2_write_mouse(0xF4);
    (void)ps2_read_ack();
}

static key_t scancode_to_key(uint8_t scancode, bool extended) {
    if (extended) {
        switch (scancode) {
            case 0x48:
            case 0x75:
                return KEY_UP;
            case 0x50:
            case 0x72:
                return KEY_DOWN;
            case 0x4B:
            case 0x6B:
                return KEY_LEFT;
            case 0x4D:
            case 0x74:
                return KEY_RIGHT;
            default:
                return KEY_NONE;
        }
    }

    switch (scancode) {
        case 0x11:
        case 0x1D:
            return KEY_UP;
        case 0x1F:
        case 0x1B:
            return KEY_DOWN;
        case 0x1E:
            return KEY_LEFT;
        case 0x20:
        case 0x23:
            return KEY_RIGHT;
        case 0x39:
        case 0x29:
            return KEY_SELECT;
        case 0x1C:
        case 0x5A:
            return KEY_SELECT;
        case 0x13:
        case 0x15:
            return KEY_RESTART;
        default:
            return KEY_NONE;
    }
}

static key_t keyboard_handle_byte(uint8_t scancode) {
    if (scancode == 0xE0) {
        extended_prefix = true;
        return KEY_NONE;
    }

    if (scancode == 0xF0) {
        set2_break_prefix = true;
        return KEY_NONE;
    }

    if (set2_break_prefix) {
        set2_break_prefix = false;
        return KEY_NONE;
    }

    if (scancode & 0x80) {
        extended_prefix = false;
        return KEY_NONE;
    }

    const bool extended = extended_prefix;
    extended_prefix = false;

    return scancode_to_key(scancode, extended);
}

void input_init(void) {
    extended_prefix = false;
    set2_break_prefix = false;
    mouse_packet_index = 0;
    mouse_left_down = false;
    mouse_click_pending = false;

    ps2_flush_output();
    ps2_configure_controller();
    ps2_flush_output();

    keyboard_enable_scanning();
    mouse_enable_streaming();

    mouse_x = screen_width / 2;
    mouse_y = screen_height / 2;
    mouse_clamp_position();
}

void input_set_screen_size(int width, int height) {
    if (width > 0) {
        screen_width = width;
    }
    if (height > 0) {
        screen_height = height;
    }

    mouse_x = screen_width / 2;
    mouse_y = screen_height / 2;
    mouse_clamp_position();
}

input_event_t input_poll(void) {
    input_event_t event = {0};

    if (mouse_click_pending) {
        event.mouse_click = true;
        event.mouse_x = mouse_x;
        event.mouse_y = mouse_y;
        mouse_click_pending = false;
    }

    if ((inb(PS2_STATUS) & 0x01) == 0) {
        return event;
    }

    const uint8_t status = inb(PS2_STATUS);
    const uint8_t data = inb(PS2_DATA);

    if (status & 0x20) {
        mouse_handle_byte(data);
        if (mouse_click_pending && !event.mouse_click) {
            event.mouse_click = true;
            event.mouse_x = mouse_x;
            event.mouse_y = mouse_y;
            mouse_click_pending = false;
        }
        return event;
    }

    event.key = keyboard_handle_byte(data);
    return event;
}
