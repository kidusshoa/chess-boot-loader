#pragma once

#include <stdint.h>

void paging_disable(void);
void paging_map_framebuffer(uint32_t physical_address);
