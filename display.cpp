#include <string.h>

#include <Arduino.h>

#include "display.h"

// TODO
#define PIN_COL0   D1
#define PIN_COL1   D1
#define PIN_COL2   D1
#define PIN_COL3   D1
#define PIN_COL4   D1
#define PIN_COL5   D1
#define PIN_COL6   D1
#define PIN_COL7   D1
#define PIN_COL8   D1

// TODO
#define PIN_ROW0   D2
#define PIN_ROW1   D3
#define PIN_ROW2   D4

static const int cols[DISPLAY_WIDTH] = 
    {PIN_COL0, PIN_COL1, PIN_COL2, PIN_COL3, PIN_COL4, PIN_COL5, PIN_COL6, PIN_COL7, PIN_COL8};
static const int rows[DISPLAY_HEIGHT] = {PIN_ROW0, PIN_ROW1, PIN_ROW2};

static volatile uint32_t framecounter = 0;
static uint8_t framebuffer[DISPLAY_HEIGHT][DISPLAY_WIDTH];
static int row = 0;
static hw_timer_t *timer = NULL;

// "horizontal" interrupt routine, displays one line
static void IRAM_ATTR display_hsync(void)
{
    // disable all rows
    digitalWrite(PIN_ROW0, 1);
    digitalWrite(PIN_ROW1, 1);
    digitalWrite(PIN_ROW2, 1);

    // next row
    row = (row + 1) % 3;
    if (row == 0) {
        framecounter++;
    }

    // write column data
    uint8_t *fb_row = framebuffer[row];
    for (int i = 0; i < DISPLAY_WIDTH; i++) {
        digitalWrite(cols[i], fb_row[i] != 0);
    }
    // enable row until the next interrupt
    digitalWrite(rows[row], 0);
}

void display_init(void)
{
    // configure rows and columns
    for (int i = 0; i < DISPLAY_HEIGHT; i++) {
        int pin = rows[i];
        pinMode(pin, OUTPUT);
        digitalWrite(pin, 1);
    }
    for (int i = 0; i < DISPLAY_WIDTH; i++) {
        int pin  = cols[i];
        pinMode(pin, OUTPUT);
        digitalWrite(pin, 0);
    }

    // clear the frame buffer
    memset(framebuffer, 0, sizeof(framebuffer));
    row = 0;

    // timer 0 counts up in microseconds
    timer = timerBegin(0, 80, true);
    // interrupt on edge
    timerAttachInterrupt(timer, display_hsync, true);
    // set timer value
    timerAlarmWrite(timer, 100, true);
}

uint32_t display_get_framecounter()
{
    return framecounter;
}

void *display_get_framebuffer()
{
    return framebuffer;
}

void display_enable(void)
{
    timerAlarmEnable(timer);
}

void display_disable(void)
{
    timerAlarmDisable(timer);

    // disable all rows
    for (int i = 0; i < DISPLAY_HEIGHT; i++) {
        digitalWrite(rows[i], 1);
    }
    
    // disable all columns
    for (int i = 0; i < DISPLAY_WIDTH; i++) {
        digitalWrite(cols[i], 0);
    }
}

