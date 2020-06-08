#include <ArtnetWifi.h>
#include <Arduino.h>
#include <NeoPixelBus.h>
#include <Ticker.h>
#include "credentials.h"

#define LED_PIN     3
#define NUM_LEDS    120
#define BRIGHTNESS  255
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB

Ticker blinker;

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(NUM_LEDS, LED_PIN);

WiFiUDP UdpSend;
ArtnetWifi artnet;
bool packageReceived = true;

void checkIdle() {
  if (!packageReceived) {
    for (int i = 0; i < NUM_LEDS; i++)
    {
      strip.SetPixelColor(i, RgbColor(0, 0, 0));
    }
    strip.Show();
  }
  else {
    packageReceived = false;
  }
}

// connect to wifi – returns true if successful or false if not
boolean ConnectWifi(void)
{
  boolean state = true;
  int i = 0;

  WiFi.begin(ssid, password);
  Serial.println("");
  Serial.println("Connecting to WiFi");
  
  // Wait for connection
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (i > 20){
      state = false;
      break;
    }
    i++;
  }
  if (state) {
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(IPAddress(WiFi.localIP()));
  } else {
    Serial.println("");
    Serial.println("Connection failed.");
    delay(5000);
    ESP.restart();
  }
  
  return state;
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  packageReceived = true;
  for (int i = 0; i < length/3; i++)
  {
    if (i > NUM_LEDS) {
      break;
    }
    strip.SetPixelColor(i, RgbColor(data[i * 3], data[i * 3 + 1], data[i * 3 + 2]));
  }
  strip.Show();
}

void setup()
{
  Serial.begin(115200);
  ConnectWifi();
  Serial.flush();

  // this resets all the neopixels to an off state
  strip.Begin();
  strip.Show();

  // this will be called for each packet received
  artnet.setArtDmxCallback(onDmxFrame);
  artnet.begin();
  blinker.attach(5, checkIdle);
}

void loop()
{
  artnet.read();
}