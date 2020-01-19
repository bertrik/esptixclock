#include <string.h>

#include <Arduino.h>

#include "framebuffer.h"
#include "tix.h"

#define PIN_COL_0   D0
#define PIN_COL_1   D1
#define PIN_COL_2   D2
#define PIN_COL_3   D3
#define PIN_COL_4   D4
#define PIN_COL_5   D5
#define PIN_COL_6   D6
#define PIN_COL_7   D7
#define PIN_COL_8   D8

// TODO
#define PIN_ROW_0   D1
#define PIN_ROW_1   D2
#define PIN_ROW_2   D3

static const vsync_fn_t *vsync_fn;
static bool framebuffer[PIX_HEIGHT][PIX_WIDTH];
static int row = 0;
static int frame = 0;

#define FAST_GPIO_WRITE(pin,val) if (val) GPOS = 1<<(pin); else GPOC = 1<<(pin)

// "horizontal" interrupt routine, displays one line
static void ICACHE_RAM_ATTR tix_hsync(void)
{
    // select the row
    FAST_GPIO_WRITE(PIN_ROW_0, row == 0);
    FAST_GPIO_WRITE(PIN_ROW_1, row == 1);
    FAST_GPIO_WRITE(PIN_ROW_2, row == 2);

    // write column data
    row = (row + 1) & 3;
    if (row == 3) {
        vsync_fn(frame++);
    } else {
#if 0    
        // write the column shift registers
        led_pixel_t *pwmrow = pwmstate[row];
        pixel_t *fb_row = framebuffer[row];
        for (int col = 0; col < LED_WIDTH; col++) {
            // dither
            led_pixel_t c1 = pwmrow[col];
            pixel_t c2 = fb_row[col];
            int r = c1.r + (c2.r & 0xF0);
            int g = c1.g + (c2.g & 0xF0);

            // write R and G data
//            FAST_GPIO_WRITE(PIN_SHIFT, 0);
//            FAST_GPIO_WRITE(PIN_DATA_R, r < 256);
//            FAST_GPIO_WRITE(PIN_DATA_G, g < 256);

            // write back
            pwmrow[col].r = r;
            pwmrow[col].g = g;

            // shift
//            FAST_GPIO_WRITE(PIN_SHIFT, 1);
        }
#endif
    }
}

void tix_write_framebuffer(const void *data)
{
    memcpy(framebuffer, data, sizeof(framebuffer));
}

void tix_init(const vsync_fn_t * vsync)
{
#if 0
    // set all pins to a defined state
    pinMode(PIN_ENABLE, OUTPUT);
    digitalWrite(PIN_ENABLE, 0);
    pinMode(PIN_LATCH, OUTPUT);
    digitalWrite(PIN_LATCH, 0);
    pinMode(PIN_SHIFT, OUTPUT);
    digitalWrite(PIN_SHIFT, 0);
    pinMode(PIN_DATA_R, OUTPUT);
    digitalWrite(PIN_DATA_R, 1);
    pinMode(PIN_DATA_G, OUTPUT);
    digitalWrite(PIN_DATA_G, 1);
    pinMode(PIN_MUX_0, OUTPUT);
    digitalWrite(PIN_MUX_0, 0);
    pinMode(PIN_MUX_1, OUTPUT);
    digitalWrite(PIN_MUX_1, 0);
    pinMode(PIN_MUX_2, OUTPUT);
    digitalWrite(PIN_MUX_2, 0);
#endif

    // copy vsync pointer
    vsync_fn = vsync;

#if 0
    // clear the frame buffer and initialise pwm state
    memset(framebuffer, 0, sizeof(framebuffer));
    for (int y = 0; y < LED_HEIGHT; y++) {
        for (int x = 0; x < LED_WIDTH; x++) {
            pwmstate[y][x].r = random(256);
            pwmstate[y][x].g = random(256);
        }
    }
#endif
    row = 0;

    // initialise timer
    timer1_isr_init();
}

void tix_enable(void)
{
    // set up timer interrupt
    timer1_disable();
    timer1_attachInterrupt(tix_hsync);
    timer1_write(500); // hsync rate = 5 MHz / number
    timer1_enable(TIM_DIV16, TIM_EDGE, TIM_LOOP);
}

void tix_disable(void)
{
    // detach the interrupt routine
    timer1_detachInterrupt();
    timer1_disable();

    // disable all rows
    FAST_GPIO_WRITE(PIN_ROW_0, 0);
    FAST_GPIO_WRITE(PIN_ROW_1, 0);
    FAST_GPIO_WRITE(PIN_ROW_2, 0);
}

