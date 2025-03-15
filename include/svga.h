#ifndef SVGA_H
#define SVGA_H

#include <stdint.h>
#include "util.h"
#include "multiboot2.h"

void RenderFrame0();
void RenderStuff(uint32_t delta_time);

void set_pixel(color_t color, uint16_Vector2_t pixel_position);
void draw_line(color_t color, uint16_Vector2_t start, uint16_Vector2_t end);
void draw_rect(color_t color, uint16_Vector2_t pos, uint16_Vector2_t size);
void draw_filled_quad(color_t color, uint16_Vector2_t p1, uint16_Vector2_t p2,
                        uint16_Vector2_t p3, uint16_Vector2_t p4);

void clear_screen(color_t color);
uint32_t convert_rgb_to_framebuffer(color_t color);

#endif // SVGA_H