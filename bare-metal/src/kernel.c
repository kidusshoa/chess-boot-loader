#include "boot_log.h"
#include "kernel.h"
#include "vga.h"

void kernel_main(uint32_t magic, struct multiboot_info* info) {
    (void)info;

    vga_initialize();
    vga_clear();

    if (magic != 0x36d76289) {
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write_line("Invalid Multiboot2 magic.");
        vga_write_hex32(magic);
        vga_writestring("\n");
        while (1) {
            __asm__ volatile("hlt");
        }
    }

    boot_log_run();

    while (1) {
        __asm__ volatile("hlt");
    }
}
