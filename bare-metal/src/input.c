#include "input.h"

#include "io.h"
#include "serial.h"

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64
#define COM1 0x3F8

static bool extended_prefix;
static bool set2_break_prefix;
static bool mouse_enabled;

static int screen_width = 640;
static int screen_height = 480;

static int mouse_x = 320;
static int mouse_y = 240;
static bool mouse_left_down;
static bool mouse_click_pending;

static uint8_t mouse_packet[3];
static int mouse_packet_index;

static bool ps2_input_ready(void) {
    return (inb(PS2_STATUS) & 0x02) == 0;
}

static bool ps2_output_ready(void) {
    return (inb(PS2_STATUS) & 0x01) != 0;
}

static bool ps2_wait_input(void) {
    for (int i = 0; i < 100000; ++i) {
        if (ps2_input_ready()) {
            return true;
        }
        io_wait();
    }
    return false;
}

static bool ps2_read_byte(uint8_t* value) {
    for (int i = 0; i < 100000; ++i) {
        if (ps2_output_ready()) {
            *value = inb(PS2_DATA);
            return true;
        }
        io_wait();
    }
    return false;
}

static void ps2_flush(void) {
    for (int i = 0; i < 64; ++i) {
        if (!ps2_output_ready()) {
            return;
        }
        (void)inb(PS2_DATA);
    }
}

static bool ps2_command(uint8_t command) {
    if (!ps2_wait_input()) {
        return false;
    }
    outb(PS2_COMMAND, command);
    return true;
}

static bool ps2_write_data(uint8_t value) {
    if (!ps2_wait_input()) {
        return false;
    }
    outb(PS2_DATA, value);
    return true;
}

static bool ps2_write_mouse(uint8_t value) {
    if (!ps2_command(0xD4)) {
        return false;
    }
    return ps2_write_data(value);
}

static bool ps2_write_keyboard(uint8_t value) {
    return ps2_write_data(value);
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

static bool mouse_try_init(void) {
    ps2_command(0xA8);
    io_wait();

    if (!ps2_write_mouse(0xFF)) {
        return false;
    }

    uint8_t response = 0;
    if (!ps2_read_byte(&response) || response != 0xFA) {
        return false;
    }
    if (!ps2_read_byte(&response) || response != 0xAA) {
        return false;
    }
    if (!ps2_read_byte(&response)) {
        return false;
    }

    if (!ps2_write_mouse(0xF6)) {
        return false;
    }
    if (!ps2_read_byte(&response) || response != 0xFA) {
        return false;
    }

    if (!ps2_write_mouse(0xF4)) {
        return false;
    }
    if (!ps2_read_byte(&response) || response != 0xFA) {
        return false;
    }

    return true;
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

static key_t serial_poll(void) {
    if ((inb(COM1 + 5) & 0x01) == 0) {
        return KEY_NONE;
    }

    const uint8_t character = inb(COM1);
    switch (character) {
        case 'w':
        case 'W':
            return KEY_UP;
        case 's':
        case 'S':
            return KEY_DOWN;
        case 'a':
        case 'A':
            return KEY_LEFT;
        case 'd':
        case 'D':
            return KEY_RIGHT;
        case ' ':
        case '\r':
        case '\n':
            return KEY_SELECT;
        case 'r':
        case 'R':
            return KEY_RESTART;
        default:
            return KEY_NONE;
    }
}

void input_init(void) {
    extended_prefix = false;
    set2_break_prefix = false;
    mouse_enabled = false;
    mouse_packet_index = 0;
    mouse_left_down = false;
    mouse_click_pending = false;

    serial_write_line("input: initializing ps/2");

    ps2_command(0xAD);
    ps2_command(0xA7);
    ps2_flush();

    if (!ps2_command(0x20)) {
        serial_write_line("input: failed to read config");
        ps2_command(0xAE);
        return;
    }

    uint8_t config = 0;
    if (!ps2_read_byte(&config)) {
        serial_write_line("input: config timeout");
        ps2_command(0xAE);
        return;
    }

    config &= (uint8_t)~0x33;
    config |= 0x40;

    if (!ps2_command(0x60) || !ps2_write_data(config)) {
        serial_write_line("input: failed to write config");
        ps2_command(0xAE);
        return;
    }

    ps2_flush();

    if (!ps2_command(0xAA)) {
        serial_write_line("input: controller self-test failed to start");
    } else {
        uint8_t test_result = 0;
        if (ps2_read_byte(&test_result) && test_result == 0x55) {
            serial_write_line("input: controller ok");
        } else {
            serial_write_line("input: controller self-test bad result");
        }
    }

    ps2_flush();
    ps2_command(0xAE);

    if (!ps2_write_keyboard(0xF4)) {
        serial_write_line("input: failed to enable keyboard scanning");
        return;
    }

    uint8_t ack = 0;
    if (ps2_read_byte(&ack) && ack == 0xFA) {
        serial_write_line("input: keyboard ready");
    } else {
        serial_write_line("input: keyboard ack missing");
    }

    ps2_flush();
    mouse_enabled = mouse_try_init();
    serial_write_line(mouse_enabled ? "input: mouse ready" : "input: mouse unavailable");

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

    const key_t serial_key = serial_poll();
    if (serial_key != KEY_NONE) {
        event.key = serial_key;
    }

    if (mouse_click_pending) {
        event.mouse_click = true;
        event.mouse_x = mouse_x;
        event.mouse_y = mouse_y;
        mouse_click_pending = false;
    }

    while (ps2_output_ready()) {
        const uint8_t status = inb(PS2_STATUS);
        if ((status & 0x01) == 0) {
            break;
        }

        const uint8_t data = inb(PS2_DATA);

        if (mouse_enabled && (status & 0x20)) {
            mouse_handle_byte(data);
            continue;
        }

        const key_t key = keyboard_handle_byte(data);
        if (key != KEY_NONE) {
            event.key = key;
        }
    }

    if (mouse_click_pending && !event.mouse_click) {
        event.mouse_click = true;
        event.mouse_x = mouse_x;
        event.mouse_y = mouse_y;
        mouse_click_pending = false;
    }

    return event;
}
