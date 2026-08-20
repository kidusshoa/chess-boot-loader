#include "framebuffer.h"

#include "multiboot2.h"
#include "serial.h"
#include "vbe.h"
#include "vga_mode13.h"

static int framebuffer_from_multiboot_tag(struct framebuffer* fb, struct multiboot_boot_info* boot_info) {
    if (!boot_info || boot_info->total_size < sizeof(struct multiboot_boot_info)) {
        return 0;
    }

    uint8_t* tag_ptr = (uint8_t*)boot_info + sizeof(struct multiboot_boot_info);
    uint8_t* end = (uint8_t*)boot_info + boot_info->total_size;

    while (tag_ptr + sizeof(struct multiboot_tag) <= end) {
        struct multiboot_tag* tag = (struct multiboot_tag*)tag_ptr;

        if (tag->type == MULTIBOOT_TAG_TYPE_END) {
            break;
        }

            if (tag->type == MULTIBOOT_TAG_TYPE_FRAMEBUFFER &&
                tag->size >= sizeof(struct multiboot_tag_framebuffer)) {
                struct multiboot_tag_framebuffer* fb_tag = (struct multiboot_tag_framebuffer*)tag;

                if (fb_tag->framebuffer_type == 2 || fb_tag->framebuffer_bpp < 24) {
                    continue;
                }

                fb->address = (uint32_t*)(uintptr_t)fb_tag->framebuffer_addr;
                fb->width = fb_tag->framebuffer_width;
                fb->height = fb_tag->framebuffer_height;
                fb->pitch = fb_tag->framebuffer_pitch;
                fb->bpp = fb_tag->framebuffer_bpp;

                if (fb->pitch == 0 && fb->width > 0 && fb->bpp > 0) {
                    fb->pitch = fb->width * ((uint32_t)fb->bpp / 8u);
                }

                if (fb->address && fb->width >= 320 && fb->height >= 200 && fb->bpp >= 24) {
                serial_write("framebuffer: multiboot ");
                serial_write_hex32((uint32_t)(uintptr_t)fb->address);
                serial_write(" ");
                serial_write_hex32(fb->width);
                serial_write("x");
                serial_write_hex32(fb->height);
                serial_write("x");
                serial_write_hex32(fb->bpp);
                serial_write_line("");
                return 1;
            }
        }

        if (tag->size < 8) {
            break;
        }

        tag_ptr += (tag->size + 7u) & ~7u;
    }

    return 0;
}

int framebuffer_init(struct framebuffer* fb, struct multiboot_boot_info* boot_info) {
    if (!fb) {
        return 0;
    }

    fb->address = 0;
    fb->width = 0;
    fb->height = 0;
    fb->pitch = 0;
    fb->bpp = 0;

    if (vga_mode13_init(fb)) {
        return 1;
    }

    if (bochs_vbe_init(fb, 640, 480, 32)) {
        serial_write_line("framebuffer: bochs vbe 640x480x32");
        return 1;
    }

    if (framebuffer_from_multiboot_tag(fb, boot_info)) {
        return 1;
    }

    serial_write_line("framebuffer: init failed");
    return 0;
}

uint32_t* framebuffer_at(struct framebuffer* fb, int x, int y) {
    if (!fb || !fb->address || x < 0 || y < 0) {
        return 0;
    }

    if ((uint32_t)x >= fb->width || (uint32_t)y >= fb->height) {
        return 0;
    }

    const uint32_t bytes_per_pixel = fb->bpp / 8u;
    if (bytes_per_pixel == 0) {
        return 0;
    }

    uint8_t* row = (uint8_t*)fb->address + (uint32_t)y * fb->pitch;

    if (bytes_per_pixel == 4) {
        return (uint32_t*)(row + (uint32_t)x * 4u);
    }

    if (bytes_per_pixel == 1) {
        return (uint32_t*)(row + (uint32_t)x);
    }

    if (bytes_per_pixel == 3) {
        return (uint32_t*)(row + (uint32_t)x * 3u);
    }

    return 0;
}
