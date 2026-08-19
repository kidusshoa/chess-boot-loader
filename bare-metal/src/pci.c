#include "pci.h"

#include "io.h"

static uint32_t pci_config_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    const uint32_t address =
        0x80000000u | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) | ((uint32_t)func << 8) | (offset & 0xFCu);

    outl(0xCF8, address);
    return inl(0xCFC);
}

uint32_t pci_find_vga_framebuffer(void) {
    for (uint8_t slot = 0; slot < 32; ++slot) {
        const uint32_t vendor_device = pci_config_read32(0, slot, 0, 0x00);
        if (vendor_device == 0xFFFFFFFFu) {
            continue;
        }

        const uint32_t class_rev = pci_config_read32(0, slot, 0, 0x08);
        const uint32_t class_code = class_rev >> 8;

        if ((class_code & 0xFFFF00u) != 0x030000u) {
            continue;
        }

        for (uint8_t bar_offset = 0x10; bar_offset <= 0x18; bar_offset += 4) {
            const uint32_t bar = pci_config_read32(0, slot, 0, bar_offset);
            if ((bar & 0x1u) == 0 && bar != 0) {
                return bar & ~0xFu;
            }
        }
    }

    return 0;
}
