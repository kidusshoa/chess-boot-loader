#include "framebuffer.h"
#include "gfx.h"
#include "kernel.h"
#include "multiboot2.h"
#include "serial.h"
#include "ui.h"

#include <stdint.h>

void kernel_main(uint32_t magic, struct multiboot_boot_info* boot_info) {
    serial_init();
    serial_write_line("kernel: started");

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC) {
        serial_write("kernel: bad magic ");
        serial_write_hex32(magic);
        serial_write_line("");
        while (1) {
            __asm__ volatile("hlt");
        }
    }

    struct framebuffer fb;
    if (!framebuffer_init(&fb, boot_info)) {
        serial_write_line("kernel: no framebuffer");
        while (1) {
            __asm__ volatile("hlt");
        }
    }

    serial_write_line("kernel: drawing ui");

    gfx_bind(&fb);
    gfx_clear(GFX_RGB(0x1A, 0x1A, 0x2E));
    ui_run(&fb);
}
