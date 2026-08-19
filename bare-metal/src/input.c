#include "input.h"

#include "io.h"

#define PS2_DATA 0x60
#define PS2_STATUS 0x64
#define PS2_COMMAND 0x64

static bool extended_prefix;

static void ps2_wait_write(void) {
    for (int i = 0; i < 100000; ++i) {
        if ((inb(PS2_STATUS) & 0x02) == 0) {
            return;
        }
    }
}

static void ps2_wait_read(void) {
    for (int i = 0; i < 100000; ++i) {
        if (inb(PS2_STATUS) & 0x01) {
            return;
        }
    }
}

static void ps2_write_command(uint8_t value) {
    ps2_wait_write();
    outb(PS2_COMMAND, value);
}

static void ps2_write_data(uint8_t value) {
    ps2_wait_write();
    outb(PS2_DATA, value);
}

static uint8_t ps2_read_data(void) {
    ps2_wait_read();
    return inb(PS2_DATA);
}

void keyboard_init(void) {
    extended_prefix = false;

    ps2_write_command(0xAE);
    io_wait();

    ps2_write_data(0xF4);
    ps2_read_data();
}

static key_t scancode_to_key(uint8_t scancode, bool extended) {
    if (extended) {
        switch (scancode) {
            case 0x48:
                return KEY_UP;
            case 0x50:
                return KEY_DOWN;
            case 0x4B:
                return KEY_LEFT;
            case 0x4D:
                return KEY_RIGHT;
            default:
                return KEY_NONE;
        }
    }

    switch (scancode) {
        case 0x39:
            return KEY_SELECT;
        case 0x1C:
            return KEY_SELECT;
        case 0x13:
            return KEY_RESTART;
        case 0x48:
            return KEY_UP;
        case 0x50:
            return KEY_DOWN;
        case 0x4B:
            return KEY_LEFT;
        case 0x4D:
            return KEY_RIGHT;
        default:
            return KEY_NONE;
    }
}

key_t keyboard_poll(void) {
    if ((inb(PS2_STATUS) & 0x01) == 0) {
        return KEY_NONE;
    }

    const uint8_t scancode = inb(PS2_DATA);

    if (scancode == 0xE0) {
        extended_prefix = true;
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
