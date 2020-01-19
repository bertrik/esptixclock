#include <stdint.h>

typedef void (vsync_fn_t) (int frame_nr);

void tix_init(vsync_fn_t * vsync);
void tix_write_framebuffer(const void *data);

void tix_enable(void);
void tix_disable(void);


