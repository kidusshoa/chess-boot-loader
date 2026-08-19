#include "serial.h"

#include "io.h"

#define COM1 0x3F8

static int serial_is_transmit_empty(void) {
    return inb(COM1 + 5) & 0x20;
}

void serial_init(void) {
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x80);
    outb(COM1 + 0, 0x03);
    outb(COM1 + 1, 0x00);
    outb(COM1 + 3, 0x03);
    outb(COM1 + 2, 0xC7);
    outb(COM1 + 4, 0x0B);
}

static void serial_write_char(char character) {
    while (!serial_is_transmit_empty()) {
    }
    outb(COM1, (uint8_t)character);
}

void serial_write(const char* text) {
    while (*text) {
        serial_write_char(*text++);
    }
}

void serial_write_line(const char* text) {
    serial_write(text);
    serial_write("\r\n");
}

void serial_write_hex32(uint32_t value) {
    static const char digits[] = "0123456789ABCDEF";
    serial_write("0x");

    for (int shift = 28; shift >= 0; shift -= 4) {
        serial_write_char(digits[(value >> shift) & 0xF]);
    }
}
