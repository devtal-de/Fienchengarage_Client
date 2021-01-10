#include <ESP8266WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include "defines.h"
#include "config.h"


WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

void setup() {

    Serial.begin(9600);
    Serial.println("Starting");

    // Tür / Relais init
    pinMode(gpioTuer1, OUTPUT);
    digitalWrite(gpioTuer1, LOW);
    pinMode(gpioTuer2, OUTPUT);
    digitalWrite(gpioTuer2, LOW);
    pinMode(gpioTuer3, OUTPUT);
    digitalWrite(gpioTuer3, LOW);
    pinMode(gpioTuer4, OUTPUT);
    digitalWrite(gpioTuer4, LOW);

    pinMode(gpioLED, OUTPUT);
    //digitalWrite(gpioLED, LOW);

    // Beginn WiFi
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WLAN_SSID);

    WiFi.mode(WIFI_STA);
    WiFi.begin(WLAN_SSID, WLAN_PSK);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    // Ende WiFi

    // NTP Client
    timeClient.begin();


}

void loop() {

    // NTP Client
    timeClient.update();
    Serial.println(timeClient.getFormattedTime());
    Serial.println(timeClient.getEpochTime());

    digitalWrite(gpioTuer4, LOW);
    digitalWrite(gpioTuer1, HIGH);
    Serial.println("Tor 1");
    delay(5000);
    digitalWrite(gpioTuer1, LOW);
    digitalWrite(gpioTuer2, HIGH);
    Serial.println("Tor 2");
    delay(5000);
    digitalWrite(gpioTuer2, LOW);
    digitalWrite(gpioTuer3, HIGH);
    Serial.println("Tor 3");
    delay(5000);
    digitalWrite(gpioTuer3, LOW);
    digitalWrite(gpioTuer4, HIGH);
    Serial.println("Tor 4");
    delay(5000);
    
}