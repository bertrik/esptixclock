#include <stdint.h>

#define DISPLAY_WIDTH   9
#define DISPLAY_HEIGHT  3

void display_init(void);

uint32_t display_get_framecounter();
void *display_get_framebuffer();

void display_enable(void);
void display_disable(void);

