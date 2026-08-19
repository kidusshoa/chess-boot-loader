#include "boot_log.h"
#include "vga.h"

void boot_log_run(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write_line("chess-boot-loader kernel v0.3.0");
    vga_write_line("POST complete - switching to graphics...");
}
