#include <stdint.h>

#include "display.h"
#include "draw.h"

static uint8_t *_framebuffer;

void draw_init(uint8_t *framebuffer)
{
    _framebuffer = framebuffer;
}

bool draw_pixel(int x, int y, uint8_t v)
{
    if ((x < 0) || (x >= DISPLAY_WIDTH) || (y < 0) || (y >= DISPLAY_HEIGHT)) {
        return false;
    }
    int i = x + (y * DISPLAY_WIDTH);
    _framebuffer[i] = v;
    return true;
}

