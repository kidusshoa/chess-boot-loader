#include "vga_mode13.h"

#include "io.h"
#include "serial.h"

static void vga_write_reg(uint16_t index_port, uint8_t index, uint8_t value) {
    outb(index_port, index);
    outb(index_port + 1u, value);
}

static void vga_set_palette_rgb(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    outb(0x3C8, index);
    outb(0x3C9, r >> 2);
    outb(0x3C9, g >> 2);
    outb(0x3C9, b >> 2);
}

static void vga_setup_palette(void) {
    vga_set_palette_rgb(0, 0x1A, 0x1A, 0x2E);
    vga_set_palette_rgb(1, 0xEE, 0xEE, 0xD2);
    vga_set_palette_rgb(2, 0x76, 0x96, 0x56);
    vga_set_palette_rgb(3, 0xFF, 0xFF, 0xFF);
    vga_set_palette_rgb(4, 0xFF, 0xD1, 0x66);
    vga_set_palette_rgb(5, 0xBA, 0xCA, 0x44);
    vga_set_palette_rgb(6, 0x82, 0x97, 0x69);
    vga_set_palette_rgb(7, 0xF5, 0xF5, 0xF5);
    vga_set_palette_rgb(8, 0x22, 0x22, 0x22);
    vga_set_palette_rgb(9, 0xCC, 0xCC, 0xCC);
    vga_set_palette_rgb(10, 0x11, 0x11, 0x11);
}

static void vga_set_mode_13h(void) {
    outb(0x3C2, 0x63);

    vga_write_reg(0x3C4, 0x00, 0x03);
    vga_write_reg(0x3C4, 0x01, 0x00);
    vga_write_reg(0x3C4, 0x02, 0x0F);
    vga_write_reg(0x3C4, 0x04, 0x06);

    vga_write_reg(0x3CE, 0x04, 0x00);
    vga_write_reg(0x3CE, 0x05, 0x40);
    vga_write_reg(0x3CE, 0x06, 0x05);

    outb(0x3D4, 0x11);
    outb(0x3D5, inb(0x3D5) & 0x7F);

    static const uint8_t crtc_13h[] = {
        0x5F, 0x4F, 0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x41, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0xEA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00,
    };

    for (uint8_t index = 0; index < sizeof(crtc_13h); ++index) {
        outb(0x3D4, index);
        outb(0x3D5, crtc_13h[index]);
    }

    inb(0x3DA);
    for (uint8_t index = 0; index < 16; ++index) {
        inb(0x3DA);
        outb(0x3C0, index);
        outb(0x3C0, index);
    }

    inb(0x3DA);
    outb(0x3C0, 0x20);
}

int vga_mode13_init(struct framebuffer* fb) {
    if (!fb) {
        return 0;
    }

    vga_set_mode_13h();
    vga_setup_palette();

    fb->address = (uint32_t*)0xA0000;
    fb->width = 320;
    fb->height = 200;
    fb->pitch = 320;
    fb->bpp = 8;

    serial_write_line("framebuffer: vga mode 13h 320x200x8");
    return 1;
}
