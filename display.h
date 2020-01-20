#include <stdint.h>

#define DISPLAY_WIDTH   9
#define DISPLAY_HEIGHT  3

typedef void (vsync_fn_t) (uint32_t frame_nr);

void display_init(vsync_fn_t * vsync);
void display_write_framebuffer(const void *data);

void display_enable(void);
void display_disable(void);

