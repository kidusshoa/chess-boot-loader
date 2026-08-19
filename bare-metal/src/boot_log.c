#include "boot_log.h"
#include "vga.h"

static void delay_ticks(volatile uint32_t count) {
    while (count-- > 0) {
        __asm__ volatile("pause");
    }
}

static void write_ok(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_writestring(" OK");
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_writestring("\n");
}

void boot_log_run(void) {
    vga_set_color(VGA_COLOR_LIGHT_GREEN, VGA_COLOR_BLACK);
    vga_write_line("chess-boot-loader kernel v0.2.0");
    vga_write_line("");

    vga_writestring("Real mode handoff complete");
    write_ok();

    vga_writestring("POST: Memory map from Multiboot");
    write_ok();

    vga_writestring("POST: CPU in protected mode");
    write_ok();

    vga_writestring("Loading chess engine...");
    delay_ticks(500000);
    write_ok();

    vga_writestring("Initializing C king...");
    delay_ticks(200000);
    write_ok();

    vga_writestring("Initializing Java queen...");
    delay_ticks(200000);
    write_ok();

    vga_writestring("Initializing Python bishop...");
    delay_ticks(200000);
    write_ok();

    vga_writestring("Initializing JavaScript knight...");
    delay_ticks(200000);
    write_ok();

    vga_writestring("Initializing Rust rook...");
    delay_ticks(200000);
    write_ok();

    vga_writestring("Initializing Go pawn...");
    delay_ticks(200000);
    write_ok();

    vga_write_line("");
    vga_set_color(VGA_COLOR_YELLOW, VGA_COLOR_BLACK);
    vga_write_line("Boot complete. Starting chess UI...");
    vga_set_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
    delay_ticks(1000000);
}
