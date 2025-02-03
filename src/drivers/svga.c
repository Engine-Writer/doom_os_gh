#include <stdint.h>
#include <memory.h>
#include "multiboot2.h"
#include "util.h"
#include "math.h"
#include "svga.h"
#include "hal.h"

/*
struct VbeInfoBlock {
    char     VbeSignature[4];         // == "VESA"
    uint16_t VbeVersion;              // == 0x0300 for VBE 3.0
    uint16_t OemStringPtr[2];         // isa vbeFarPtr
    uint8_t  Capabilities[4];
    uint16_t VideoModePtr[2];         // isa vbeFarPtr
    uint16_t TotalMemory;             // as # of 64KB blocks
    uint8_t  Reserved[492];
} __attribute__((packed));
*/

uint32_t ALPHA_MASK;

void RenderFrame0() {
    if (fbo_com_gb.framebuffer_type == 0) { // INDEXED RGB

    } else if (fbo_com_gb.framebuffer_type == 1) { // DIRECT RGB  fbo_tag_gb->framebuffer_
        ALPHA_MASK = ~(convert_rgb_to_framebuffer(make_svga_color(0xFF, 0xFF, 0xFF)));
        uint32_t framebuffer_value = convert_rgb_to_framebuffer(make_svga_color(0x7F, 0x7F, 0x7F));

        memset_pattern(fbo_com_gb.framebuffer_addr, &framebuffer_value, (fbo_com_gb.framebuffer_bpp|7)>>3, 
        (fbo_com_gb.framebuffer_width * fbo_com_gb.framebuffer_width * ((fbo_com_gb.framebuffer_bpp|7)>>3)));
        draw_filled_quad(make_svga_color(255, 255, 0), 
        make_uint16_vector2(30, 20), 
        make_uint16_vector2(fbo_com_gb.framebuffer_width-60, 20), 
        make_uint16_vector2(fbo_com_gb.framebuffer_width-40, fbo_com_gb.framebuffer_height-40),
        make_uint16_vector2(20, fbo_com_gb.framebuffer_height-40));
    }
}

void set_pixel(color_t color, uint16_Vector2_t pixel_position) {
    uint32_t real_color = convert_rgb_to_framebuffer(color);
    if (pixel_position.x > fbo_com_gb.framebuffer_width | pixel_position.x > fbo_com_gb.framebuffer_height) {
        return;
    }
    memcpy(fbo_com_gb.framebuffer_addr + (pixel_position.y * fbo_com_gb.framebuffer_width) + pixel_position.x, 
    &real_color, (fbo_com_gb.framebuffer_bpp|7)>>3);
}

void draw_line(color_t color, uint16_Vector2_t start, uint16_Vector2_t end) {
    uint32_t dx = abs32(end.x - start.x);
    uint32_t dy = abs32(end.y - start.y);
    uint32_t sx = (start.x < end.x) ? 1 : -1;
    uint32_t sy = (start.y < end.y) ? 1 : -1;
    uint32_t err = dx - dy;

    while (1) {
        set_pixel(color, start);

        if (start.x == end.x && start.y == end.y) {
            break;
        }

        int e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            start.x += sx;
        }
        if (e2 < dx) {
            err += dx;
            start.y += sy;
        }
    }
}

void draw_rect(color_t color, uint16_Vector2_t pos, uint16_Vector2_t size) {
    // Ensure the rectangle is within framebuffer bounds
    if (pos.x + size.x > fbo_com_gb.framebuffer_width || pos.y + size.y > fbo_com_gb.framebuffer_height) {
        return; // Rectangle exceeds framebuffer bounds
    }

    uint32_t real_color = convert_rgb_to_framebuffer(color);
    size_t bytes_per_pixel = (fbo_com_gb.framebuffer_bpp + 7) >> 3;

    for (uint16_t y_in_c = pos.y; y_in_c < (pos.y + size.y); y_in_c++) {
        void *line_start = (uint8_t *)fbo_com_gb.framebuffer_addr + ((y_in_c * fbo_com_gb.framebuffer_width + pos.x) * bytes_per_pixel);
        memset_pattern(line_start, &real_color, bytes_per_pixel, size.x * bytes_per_pixel);
    }
}

uint32_t convert_rgb_to_framebuffer(color_t color) {
    uint32_t pixel = 0;

    // Scale 8-bit values to framebuffer's bit depth
    uint32_t scaled_red = (color.r * ((1 << fbo_tag_gb->framebuffer_red_mask_size) - 1)) / 255;
    uint32_t scaled_green = (color.g * ((1 << fbo_tag_gb->framebuffer_green_mask_size) - 1)) / 255;
    uint32_t scaled_blue = (color.b * ((1 << fbo_tag_gb->framebuffer_blue_mask_size) - 1)) / 255;

    // Shift and mask each component to its position
    pixel |= (scaled_red << fbo_tag_gb->framebuffer_red_field_position);
    pixel |= (scaled_green << fbo_tag_gb->framebuffer_green_field_position);
    pixel |= (scaled_blue << fbo_tag_gb->framebuffer_blue_field_position);

    return ALPHA_MASK|pixel;
}

// Draw a filled convex quadrilateral by scanline filling.
void draw_filled_quad(color_t color, uint16_Vector2_t p1, uint16_Vector2_t p2,
                        uint16_Vector2_t p3, uint16_Vector2_t p4) {
    // Compute the vertical bounds (bounding box) of the quad.
    int min_y = p1.y, max_y = p1.y;
    if (p2.y < min_y) min_y = p2.y; else if (p2.y > max_y) max_y = p2.y;
    if (p3.y < min_y) min_y = p3.y; else if (p3.y > max_y) max_y = p3.y;
    if (p4.y < min_y) min_y = p4.y; else if (p4.y > max_y) max_y = p4.y;

    // Convert the color and determine the number of bytes per pixel.
    uint32_t real_color = convert_rgb_to_framebuffer(color);
    size_t bytes_per_pixel = (fbo_com_gb.framebuffer_bpp + 7) >> 3;

    // Array of vertices for convenience.
    uint16_Vector2_t vertices[4] = { p1, p2, p3, p4 };

    // Loop over each scanline in the bounding box.
    for (int y = min_y; y <= max_y; y++) {
        int left = fbo_com_gb.framebuffer_width;  // Start with a large value.
        int right = 0;                            // Start with a small value.

        // Process each edge of the quad.
        for (int i = 0; i < 4; i++) {
            uint16_Vector2_t a = vertices[i];
            uint16_Vector2_t b = vertices[(i + 1) % 4];
            // Determine if the current scanline intersects this edge.
            int edge_min_y = min32(a.y, b.y);
            int edge_max_y = max32(a.y, b.y);
            if (y >= edge_min_y && y <= edge_max_y) {
                int x = compute_intersection_x(a, b, y);
                left = min32(left, x);
                right = max32(right, x);
            }
        }

        // If a valid horizontal span was found, fill between left and right.
        if (left < right) {
            // Optional: clip to framebuffer bounds.
            if (left < 0) left = 0;
            if (right > fbo_com_gb.framebuffer_width)
                right = fbo_com_gb.framebuffer_width;
            void *line_start = (uint8_t *)fbo_com_gb.framebuffer_addr +
                               ((y * fbo_com_gb.framebuffer_width + left) * bytes_per_pixel);
            int num_pixels = right - left;
            // Fill the line using memset_pattern (number of bytes = num_pixels * bytes_per_pixel).
            memset_pattern(line_start, &real_color, bytes_per_pixel, num_pixels * bytes_per_pixel);
        }
    }
}