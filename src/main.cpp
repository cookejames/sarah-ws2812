#define VERSION "4"
#include "Adafruit_NeoPixel.h"
#include "SarahHome.h"
#include <ArduinoJson.h>

SarahHome sarahHome("lights");

#define LED_PIN 15
#define NUMPIXELS 10
#define BUTTON_PIN 0
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
volatile int lastTime = 0;
boolean ledsOn = false;
int lastRed = 255;
int lastGreen = 100;
int lastBlue = 30;

void setStrip(int red, int green, int blue) {
  for(int i=0;i<NUMPIXELS;i++){
    pixels.setPixelColor(i, pixels.Color(red,green,blue));
    pixels.show();
    delay(100);
  }
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  char json[length];
  for (int i = 0; i < length; i++) {
    json[i] = (char) payload[i];
  }

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(json);
  if (!root.success())
  {
    Serial.println("Could not parse payload:");
    Serial.println(json);
    return;
  }

  int red = root["r"];
  int green = root["g"];
  int blue = root["b"];

  Serial.print(red);
  Serial.print(", ");
  Serial.print(green);
  Serial.print(", ");
  Serial.println(blue);

  setStrip(red, green, blue);
  if (red == 0 && green == 0 && blue == 0) {
    ledsOn = false;
  } else {
    ledsOn = true;
    lastRed = red;
    lastGreen = green;
    lastBlue = blue;
  }
}

void pinChanged()
{
  int time = millis();
  int duration = time - lastTime;

  if (duration > 500) {
    if (ledsOn) {
      setStrip(0, 0, 0);
      ledsOn = false;
    } else {
      setStrip(lastRed, lastGreen, lastBlue);
      ledsOn = true;
    }
  }
}

void setup() {
  Serial.begin(9600);
  sarahHome.setup(VERSION);

  char subscribeTopic[100];
  sprintf(subscribeTopic, "%s/%s", sarahHome.getDeviceType().c_str(), sarahHome.getNodeId().c_str());
  sarahHome.subscribe(subscribeTopic);
  sarahHome.mqttClient.setCallback(mqttCallback);

  pixels.begin(); // This initializes the NeoPixel library.
  setStrip(lastRed, lastGreen, lastBlue);
  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(BUTTON_PIN, pinChanged, RISING);
}

void loop() {
  sarahHome.loop();
}
