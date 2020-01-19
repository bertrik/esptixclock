#include <Arduino.h>
#include <WiFiManager.h>
#include <NTPClient.h>

const char *NTP_SERVER = "nl.pool.ntp.org";
// enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
const char *TZ_INFO = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";   // Europe/Amsterdam

static WiFiManager wifiManager;
static WiFiUDP ntpUDP;
static NTPClient ntpClient(ntpUDP, NTP_SERVER);

void setup(void)
{
    Serial.begin(115200);
    Serial.println("\nESP-TIXCLOCK");

    // NTP setup
    setenv("TZ", TZ_INFO, 1);
    ntpClient.begin();

    // get IP address
    wifiManager.autoConnect("ESP-TIXCLOCK");
}

void loop(void)
{
    static unsigned long prev_second = 0;

    // new second?
    unsigned long ms = millis();
    unsigned long second = ms / 1000;
    if (second != prev_second) {
        // do something, e.g. update display
        prev_second = second;
    }

    // NTP update
    ntpClient.update();
}

