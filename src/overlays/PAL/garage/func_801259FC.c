#include "rr/garage_image_stroke.h"

/* Address of the byte holding 4bpp pixel (x, y); rows are 32 bytes. */
#define PIXEL_BYTE(x, y) (&((u8 *)((y) * 32 + (s32)image))[(x) / 2])

/* Paint a size x size block of the color nibble at (x, y). Each instance
 * owns its block-local temporaries. */
#define PLOT_BLOCK(x, y)                                              \
    for (i = 0; i < size; i++) {                                      \
        for (j = 0; j < size; j++) {                                  \
            u8 *p = PIXEL_BYTE((x) + j, (y) + i);                     \
            u8 v = *p;                                                \
            u8 *q;                                                    \
            u8 w;                                                     \
            *p = (((x) + j) & 1) ? (v & 0x0F) : (v & 0xF0);           \
            q = PIXEL_BYTE((x) + j, (y) + i);                         \
            w = *q;                                                   \
            *q = w | ((((x) + j) & 1) ? (color << 4) : color);        \
        }                                                             \
    }

/* Bresenham line from (x0, y0) to (x1, y1) with a square brush. */
void garage_draw_brush_line(u32 *image, s32 x0, s32 y0, s32 x1, s32 y1, u8 color, u8 size) {
    s32 dx, dy, step, err;
    s32 i, j;

    if (x0 < x1) {
        dx = x1 - x0;
    } else {
        dx = x0 - x1;
    }
    if (y0 < y1) {
        dy = y1 - y0;
    } else {
        dy = y0 - y1;
    }
    if (dy < dx) {
        if (x1 < x0) {
            if (y1 < y0) {
                step = 1;
            } else {
                step = -1;
            }
            err = x0;
            x0 = x1;
            x1 = err;
            y0 = y1;
        } else {
            if (y0 < y1) {
                step = 1;
            } else {
                step = -1;
            }
        }
        PLOT_BLOCK(x0, y0);
        err = dx >> 1;
        for (x0++; x0 <= x1; x0++) {
            err -= dy;
            if (err < 0) {
                err += dx;
                y0 += step;
            }
            PLOT_BLOCK(x0, y0);
        }
    } else {
        if (y1 < y0) {
            if (x1 < x0) {
                step = 1;
            } else {
                step = -1;
            }
            err = y0;
            y0 = y1;
            y1 = err;
            x0 = x1;
        } else {
            if (x0 < x1) {
                step = 1;
            } else {
                step = -1;
            }
        }
        PLOT_BLOCK(x0, y0);
        err = dy >> 1;
        for (y0++; y0 <= y1; y0++) {
            err -= dx;
            if (err < 0) {
                err += dy;
                x0 += step;
            }
            PLOT_BLOCK(x0, y0);
        }
    }
}
