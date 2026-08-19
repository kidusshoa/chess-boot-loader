#pragma once

#include <stdint.h>

#include "multiboot2.h"

void kernel_main(uint32_t magic, struct multiboot_boot_info* boot_info);
