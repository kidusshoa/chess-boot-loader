#include "vga.h"

static uint16_t* const VGA_MEMORY = (uint16_t*)0xB8000;
static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;

static inline uint8_t vga_entry_color(enum vga_color foreground, enum vga_color background) {
    return (uint8_t)(foreground | background << 4);
}

static inline uint16_t vga_entry(unsigned char character, uint8_t color) {
    return (uint16_t)((uint16_t)character | (uint16_t)color << 8);
}

void vga_initialize(void) {
    terminal_row = 0;
    terminal_column = 0;
    terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
}

void vga_set_color(enum vga_color foreground, enum vga_color background) {
    terminal_color = vga_entry_color(foreground, background);
}

void vga_clear(void) {
    for (size_t y = 0; y < VGA_HEIGHT; ++y) {
        for (size_t x = 0; x < VGA_WIDTH; ++x) {
            const size_t index = y * VGA_WIDTH + x;
            VGA_MEMORY[index] = vga_entry(' ', terminal_color);
        }
    }
    terminal_row = 0;
    terminal_column = 0;
}

static void vga_scroll(void) {
    for (size_t y = 1; y < VGA_HEIGHT; ++y) {
        for (size_t x = 0; x < VGA_WIDTH; ++x) {
            VGA_MEMORY[(y - 1) * VGA_WIDTH + x] = VGA_MEMORY[y * VGA_WIDTH + x];
        }
    }

    for (size_t x = 0; x < VGA_WIDTH; ++x) {
        VGA_MEMORY[(VGA_HEIGHT - 1) * VGA_WIDTH + x] = vga_entry(' ', terminal_color);
    }

    if (terminal_row > 0) {
        --terminal_row;
    }
}

static void vga_putentryat(char character, uint8_t color, size_t x, size_t y) {
    VGA_MEMORY[y * VGA_WIDTH + x] = vga_entry((unsigned char)character, color);
}

void vga_write(const char* data, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        if (data[i] == '\n') {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                vga_scroll();
            }
            continue;
        }

        vga_putentryat(data[i], terminal_color, terminal_column, terminal_row);
        if (++terminal_column == VGA_WIDTH) {
            terminal_column = 0;
            if (++terminal_row == VGA_HEIGHT) {
                vga_scroll();
            }
        }
    }
}

void vga_writestring(const char* data) {
    size_t length = 0;
    while (data[length] != '\0') {
        ++length;
    }
    vga_write(data, length);
}

void vga_write_line(const char* data) {
    vga_writestring(data);
    vga_writestring("\n");
}

void vga_write_hex32(uint32_t value) {
    static const char digits[] = "0123456789ABCDEF";
    char buffer[11];
    buffer[0] = '0';
    buffer[1] = 'x';

    for (int i = 7; i >= 0; --i) {
        buffer[2 + (7 - i)] = digits[(value >> (i * 4)) & 0xF];
    }

    buffer[10] = '\0';
    vga_writestring(buffer);
}
