#include <stdint.h>

#include <WiFiUdp.h>
#include <NTPClient.h>
#include <WiFi.h>
#include <Arduino.h>

#include "cmdproc.h"
#include "editline.h"
#include "display.h"

#define print    Serial.printf

const char *NTP_SERVER = "nl.pool.ntp.org";
// enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
const char *TZ_INFO = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";   // Europe/Amsterdam

static WiFiUDP ntpUDP;
static NTPClient ntpClient(ntpUDP, NTP_SERVER);

static char editline[120];
static uint8_t fb[DISPLAY_HEIGHT][DISPLAY_WIDTH];

static int do_fps(int argc, char *argv[])
{
    print("Measuring ...");
    uint32_t count = display_get_framecounter();
    delay(1000);
    int fps = display_get_framecounter() - count;
    print("FPS = %d\n", fps);
    return CMD_OK;
}

static int do_datetime(int argc, char *argv[])
{
    time_t now = ntpClient.getEpochTime();
    tm *timeinfo = localtime(&now);

    char text[32];
    const char *cmd = argv[0];
    if (strcmp(cmd, "date") == 0) {
        strftime(text, sizeof(text), "%a %F", timeinfo);
        print("date = %s\n", text);
    } else if (strcmp(cmd, "time") == 0) {
        strftime(text, sizeof(text), "%T", timeinfo);
        print("time = %s\n", text);
    } else {
        return CMD_ARG;
    }
    return CMD_OK;
}

static int setpixel(int x, int y, int v)
{
    if ((x < 0) || (x >= DISPLAY_WIDTH) || (y < 0) || (y >= DISPLAY_HEIGHT)) {
        return CMD_ARG;
    }
    fb[y][x] = v;
    return CMD_OK;
}

static int do_pix(int argc, char *argv[])
{
    if (argc < 3) {
        return CMD_ARG;
    }
    int x = atoi(argv[1]);
    int y = atoi(argv[2]);
    int v = 255;
    if (argc > 3) {
        v = atoi(argv[3]);
    }
    return setpixel(x, y, v);
}

static int do_enable(int argc, char *argv[])
{
    char *cmd = argv[0];
    if (strcmp(cmd, "enable") == 0) {
        print("Enabling display\n");
        display_enable();
        return CMD_OK;
    }
    if (strcmp(cmd, "disable") == 0) {
        print("Disabling display\n");
        display_disable();
        return CMD_OK;
    }
    return CMD_ARG;
}

static int do_reboot(int argc, char *argv[])
{
    display_disable();
    ESP.restart();
    return CMD_OK;
}

static int do_help(int argc, char *argv[]);
const cmd_t commands[] = {
    { "fps", do_fps, "Show frames-per-second" },
    { "date", do_datetime, "show date" },
    { "time", do_datetime, "show time" },
    { "pix", do_pix, "<x> <y> <val> Set pixel at (x,y) to value (val)" },
    { "enable", do_enable, "Enable display" },
    { "disable", do_enable, "Disable display" },
    { "reboot", do_reboot, "Reboot" },
    { "help", do_help, "Show help" },
    { NULL, NULL, NULL }
};

static void show_help(const cmd_t * cmds)
{
    for (const cmd_t * cmd = cmds; cmd->cmd != NULL; cmd++) {
        print("%10s: %s\n", cmd->name, cmd->help);
    }
}

static int do_help(int argc, char *argv[])
{
    show_help(commands);
    return CMD_OK;
}

void setup(void)
{
    Serial.begin(115200);
    Serial.println("\nESP-TIXCLOCK");
    EditInit(editline, sizeof(editline));

    // display init
    memset(fb, 0, sizeof(fb));
    display_init();

    // get IP address
    WiFi.begin("revspace-pub-2.4ghz");
    while (WiFi.status() != WL_CONNECTED) {
        print(".");
        delay(500);
    }
    print("\n");

    // NTP setup
    setenv("TZ", TZ_INFO, 1);
    ntpClient.begin();

    // start running the display
    display_enable();
}

void loop(void)
{
    static int prev_second = -1;

    // parse command line
    bool haveLine = false;
    if (Serial.available()) {
        char c;
        haveLine = EditLine(Serial.read(), &c);
        Serial.write(c);
    }
    if (haveLine) {
        int result = cmd_process(commands, editline);
        switch (result) {
        case CMD_OK:
            print("OK\n");
            break;
        case CMD_NO_CMD:
            break;
        case CMD_UNKNOWN:
            print("Unknown command, available commands:\n");
            show_help(commands);
            break;
        case CMD_ARG:
            print("Invalid argument(s)\n");
            break;
        default:
            print("%d\n", result);
            break;
        }
        print(">");
    }
    // new second?
    unsigned long ms = millis();
    int second = ms / 1000;
    if (second != prev_second) {
        // do something, e.g. update display
        prev_second = second;
    }

    // NTP update
    if (WiFi.status() == WL_CONNECTED) {
        ntpClient.update();
    }
}


