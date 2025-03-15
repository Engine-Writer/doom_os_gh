#ifndef HAL_H
#define HAL_H
#include "multiboot2.h"

extern multiboot_tag_framebuffer_t *fbo_tag_gb;
extern multiboot_tag_framebuffer_common_t fbo_com_gb;
extern multiboot_tag_bootdev_t *bootdev_tag;

uint32_t HAL_Initialize(multiboot_info_t *multiboot_info_addr);

#endif // HAL_H