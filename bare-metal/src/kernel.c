#include "boot_log.h"
#include "framebuffer.h"
#include "kernel.h"
#include "multiboot2.h"
#include "paging.h"
#include "serial.h"
#include "ui.h"
#include "vga.h"

#include <stdint.h>

void kernel_main(uint32_t magic, struct multiboot_boot_info* boot_info) {
    serial_init();
    serial_write_line("chess-boot-loader kernel starting");

    vga_initialize();
    vga_clear();

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        serial_write("invalid multiboot magic: ");
        serial_write_hex32(magic);
        serial_write_line("");
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write_line("Invalid Multiboot2 magic.");
        vga_write_hex32(magic);
        vga_writestring("\n");
        while (1) {
            __asm__ volatile("hlt");
        }
    }

    boot_log_run();

    paging_disable();

    struct framebuffer fb;
    if (!framebuffer_init(&fb, boot_info)) {
        serial_write_line("framebuffer unavailable");
        vga_set_color(VGA_COLOR_LIGHT_RED, VGA_COLOR_BLACK);
        vga_write_line("No graphics framebuffer available.");
        vga_write_line("Run QEMU with: -machine pc -vga std");
        while (1) {
            __asm__ volatile("hlt");
        }
    }

    paging_map_framebuffer((uint32_t)(uintptr_t)fb.address);

    serial_write("entering chess ui fb=");
    serial_write_hex32((uint32_t)(uintptr_t)fb.address);
    serial_write(" ");
    serial_write_hex32(fb.width);
    serial_write("x");
    serial_write_hex32(fb.height);
    serial_write_line("");

    ui_run(&fb);
}
