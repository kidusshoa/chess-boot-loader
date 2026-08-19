#pragma once

#include <stdint.h>

void serial_init(void);
void serial_write(const char* text);
void serial_write_hex32(uint32_t value);
void serial_write_line(const char* text);
