#include "vbe.h"

#include "io.h"
#include "pci.h"
#include "serial.h"

#define VBE_DISPI_IOPORT_INDEX 0x01CE
#define VBE_DISPI_IOPORT_DATA 0x01CF

#define VBE_DISPI_INDEX_ID 0x0000
#define VBE_DISPI_INDEX_XRES 0x0001
#define VBE_DISPI_INDEX_YRES 0x0002
#define VBE_DISPI_INDEX_BPP 0x0003
#define VBE_DISPI_INDEX_ENABLE 0x0004
#define VBE_DISPI_INDEX_VIRT_WIDTH 0x0006
#define VBE_DISPI_INDEX_VIRT_HEIGHT 0x0007

#define VBE_DISPI_ID 0xB0C5
#define VBE_DISPI_ENABLED 0x01
#define VBE_DISPI_LFB_ENABLED 0x40

static void vbe_write(uint16_t index, uint16_t value) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    outw(VBE_DISPI_IOPORT_DATA, value);
}

static uint16_t vbe_read(uint16_t index) {
    outw(VBE_DISPI_IOPORT_INDEX, index);
    return inw(VBE_DISPI_IOPORT_DATA);
}

static uint32_t vbe_framebuffer_address(void) {
    const uint32_t pci_address = pci_find_vga_framebuffer();
    if (pci_address != 0) {
        serial_write("vga pci bar ");
        serial_write_hex32(pci_address);
        serial_write_line("");
        return pci_address;
    }

    return 0xE0000000;
}

int bochs_vbe_init(struct framebuffer* fb, uint32_t width, uint32_t height, uint8_t bpp) {
    if (!fb || width == 0 || height == 0) {
        return 0;
    }

    if (vbe_read(VBE_DISPI_INDEX_ID) != VBE_DISPI_ID) {
        return 0;
    }

    vbe_write(VBE_DISPI_INDEX_ENABLE, 0);
    vbe_write(VBE_DISPI_INDEX_XRES, (uint16_t)width);
    vbe_write(VBE_DISPI_INDEX_YRES, (uint16_t)height);
    vbe_write(VBE_DISPI_INDEX_BPP, bpp);
    vbe_write(VBE_DISPI_INDEX_VIRT_WIDTH, (uint16_t)width);
    vbe_write(VBE_DISPI_INDEX_VIRT_HEIGHT, (uint16_t)height);
    vbe_write(VBE_DISPI_INDEX_ENABLE, VBE_DISPI_ENABLED | VBE_DISPI_LFB_ENABLED);

    fb->address = (uint32_t*)vbe_framebuffer_address();
    fb->width = width;
    fb->height = height;
    fb->pitch = width * ((uint32_t)bpp / 8u);
    fb->bpp = bpp;
    return fb->address != 0;
}
