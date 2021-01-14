#include <ESP8266WiFi.h>        // Board + WIFI + SSL
#include <ESP8266HTTPClient.h>  // HTTPClient
#include <NTPClient.h>          // Time Server
#include <WiFiUdp.h>            // Time Server
#include <Wiegand.h>            // KeyPad



#include "defines.h"
#include "config.h"
#include "sha256.h"

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

Wiegand wiegand;
String Code;

unsigned long openDoor[4] = {0,0,0,0};

bool getConfig() {

  WiFiClientSecure httpsClient;
  httpsClient.setInsecure();
  BearSSL::X509List cert;
  cert.append(lets_encrypt_r3); // Active
  cert.append(lets_encrypt_e1); // Upcoming
  httpsClient.setTrustAnchors(&cert);
  
  if (!httpsClient.connect(CONFIG_HOSTNAME, CONFIG_PORT)) {
    Serial.println("connection failed ");
    return false;
  }
  Serial.println("connected");

  HTTPClient http;

  // use hostname as user-agent
  http.setUserAgent(HOSTNAME);

  http.begin(httpsClient, CONFIG_HOSTNAME, CONFIG_PORT, CONFIG_FILE, true);
  int httpCode = http.GET();
  Serial.printf("[HTTP] GET... code: %d\n", httpCode);
  if(httpCode != 200)
    return false;
  
  String content = http.getString();
  Serial.printf("[HTTP] GET... : %s\n",  content.c_str());
  
  Serial.printf("[HTTP] SHA256... : %s\n",  sha256(content).c_str());
  

  return true;
}


// Notifies when a reader has been connected or disconnected.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onStateChange()`
void stateChanged(bool plugged, const char *message)
{
  Serial.print(message);
  Serial.println(plugged ? "CONNECTED" : "DISCONNECTED");
}

// Notifies when a card was read.
// Instead of a message, the seconds parameter can be anything you want -- Whatever you specify on `wiegand.onReceive()`
void receivedData(uint8_t *data, uint8_t bits, const char *message)
{
  //char input[10];
  String input;
  char buf[12];

  Serial.print(message);
  Serial.print(bits);
  Serial.print("bits / ");
  //Print value in HEX
  uint8_t bytes = (bits + 7) / 8;
  for (int i = 0; i < bytes; i++)
  {
    // byte to hex
    sprintf(buf, "%x", data[i]);
    input.concat(buf);
  }

  // * pressed / clear session
  if (bits == 4 && input == "a")
  {
    Code.remove(0);
  }
  // new card
  else if (bits == 32)
  {
    Code = input;
  }
  // append Key
  else
  {
    Code.concat(input);
  }

  Serial.printf("input: %s\n", input.c_str());
  Serial.printf("Code: %s\n" , Code.c_str());
  
  // # pressed
  if (input == "b")
  {
    if (Code == "b913cbc21234b")
    {
      Serial.println("OK let's go");
      openDoor[0]=timeClient.getEpochTime()+10;
    }
    else if (Code == "ea2814cb")
    {
      openDoor[0]=timeClient.getEpochTime()+DOORS_TIMEOUT;
      openDoor[1]=timeClient.getEpochTime()+DOORS_TIMEOUT*2;
      openDoor[2]=timeClient.getEpochTime()+DOORS_TIMEOUT*3;
      openDoor[3]=timeClient.getEpochTime()+DOORS_TIMEOUT*4;
    }
    else
    {
      Serial.println("Nope");
      Code.remove(0);
    }
  }
}

// Notifies when an invalid transmission is detected
void receivedDataError(Wiegand::DataError error, uint8_t *rawData, uint8_t rawBits, const char *message)
{
  Serial.print(message);
  Serial.print(Wiegand::DataErrorStr(error));
  Serial.print(" - Raw data: ");
  Serial.print(rawBits);
  Serial.print("bits / ");

  //Print value in HEX
  uint8_t bytes = (rawBits + 7) / 8;
  for (int i = 0; i < bytes; i++)
  {
    Serial.print(rawData[i] >> 4, 16);
    Serial.print(rawData[i] & 0xF, 16);
  }
  Serial.println();
}

ICACHE_RAM_ATTR void pinStateChanged() {
  wiegand.setPin0State(digitalRead(WIEGAND_PIN_0));
  wiegand.setPin1State(digitalRead(WIEGAND_PIN_1));
}

void setup()
{

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
#if defined WLAN_SSID && defined WLAN_PSK
  wifi_station_set_hostname(HOSTNAME);
  
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.begin(WLAN_SSID, WLAN_PSK);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  WiFi.printDiag(Serial);
#endif
  // Ende WiFi

  // NTP Client
  timeClient.begin();

  // Init KeyPad
  pinMode(WIEGAND_PIN_0, INPUT);
  pinMode(WIEGAND_PIN_1, INPUT);
  pinMode(WIEGAND_PIN_LED, OUTPUT);
  digitalWrite(WIEGAND_PIN_LED, HIGH);
  wiegand.onReceive(receivedData, "Card readed: ");
  wiegand.onReceiveError(receivedDataError, "Card read error: ");
  wiegand.onStateChange(stateChanged, "State changed: ");
  wiegand.begin(Wiegand::LENGTH_ANY, true);
  attachInterrupt(digitalPinToInterrupt(WIEGAND_PIN_0), pinStateChanged, CHANGE);
  attachInterrupt(digitalPinToInterrupt(WIEGAND_PIN_1), pinStateChanged, CHANGE);
  //Sends the initial pin state to the Wiegand library
  pinStateChanged();

  getConfig();

  //All OK, LED off
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

}

void loop()
{

  // // NTP Client
  //timeClient.update();

  // Wiegand
  noInterrupts();
  wiegand.flush();
  interrupts();
  //Sleep a little -- this doesn't have to run very often.
  delay(100);

  if(openDoor[0] > timeClient.getEpochTime() ) {
    digitalWrite(gpioTuer1, HIGH);
  } else {
    digitalWrite(gpioTuer1, LOW);
  }
  if(openDoor[1] > timeClient.getEpochTime() ) {
    digitalWrite(gpioTuer2, HIGH);
  } else {
    digitalWrite(gpioTuer2, LOW);
  }
  if(openDoor[2] > timeClient.getEpochTime() ) {
    digitalWrite(gpioTuer3, HIGH);
  } else {
    digitalWrite(gpioTuer3, LOW);
  }
  if(openDoor[3] > timeClient.getEpochTime() ) {
    digitalWrite(gpioTuer4, HIGH);
  } else {
    digitalWrite(gpioTuer4, LOW);
  }

}
