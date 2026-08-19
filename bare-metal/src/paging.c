#include "paging.h"

static int paging_is_enabled(void) {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));
    return (cr0 & 0x80000000u) != 0;
}

void paging_disable(void) {
    uint32_t cr0;
    __asm__ volatile("mov %%cr0, %0" : "=r"(cr0));

    if ((cr0 & 0x80000000u) == 0) {
        return;
    }

    cr0 &= ~0x80000000u;
    __asm__ volatile("mov %0, %%cr0" : : "r"(cr0) : "memory");
    __asm__ volatile("jmp 1f\n1:" ::: "memory");
}

static uint32_t paging_cr3(void) {
    uint32_t cr3;
    __asm__ volatile("mov %%cr3, %0" : "=r"(cr3));
    return cr3;
}

void paging_map_framebuffer(uint32_t physical_address) {
    if (!paging_is_enabled() || physical_address == 0) {
        return;
    }

    static uint32_t fb_page_table[1024] __attribute__((aligned(4096)));

    const uint32_t pd_index = (physical_address >> 22) & 0x3FFu;
    const uint32_t region_base = physical_address & 0xFFC00000u;

    for (uint32_t i = 0; i < 1024; ++i) {
        fb_page_table[i] = (region_base + i * 4096u) | 0x003u;
    }

    uint32_t* page_directory = (uint32_t*)paging_cr3();
    page_directory[pd_index] = ((uint32_t)fb_page_table) | 0x003u;

    __asm__ volatile("mov %0, %%cr3" : : "r"(paging_cr3()) : "memory");
}
