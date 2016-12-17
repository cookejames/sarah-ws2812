#define VERSION "5"
#include <WS2812FX.h>
#include "SarahHome.h"
#include <ArduinoJson.h>

SarahHome sarahHome("lights");

#define LED_PIN 15
#define NUMPIXELS 100
#define BUTTON_PIN 0
#define START_SPEED 200
#define START_BRIGHTNESS 255
WS2812FX ws2812fx = WS2812FX(NUMPIXELS, LED_PIN, NEO_GRB + NEO_KHZ800);
volatile int lastTime = 0;
boolean ledsOn = false;
int lastRed = 255;
int lastGreen = 100;
int lastBlue = 30;

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

  if (root.containsKey("r") && root.containsKey("g") && root.containsKey("b")) {
    int red = root["r"];
    int green = root["g"];
    int blue = root["b"];
    ws2812fx.setColor(red, green, blue);
    if (red == 0 && green == 0 && blue == 0) {
      ledsOn = false;
    } else {
      ledsOn = true;
      lastRed = red;
      lastGreen = green;
      lastBlue = blue;
    }
  }

  if (root.containsKey("mode")) {
    int mode = root["mode"];
    ws2812fx.setMode(mode);
  }

  if (root.containsKey("speed")) {
    int speed = root["speed"];
    ws2812fx.setSpeed(speed);
  }

  if (root.containsKey("brightness")) {
    int brightness = root["brightness"];
    ws2812fx.setBrightness(brightness);
  }
}

void pinChanged()
{
  int time = millis();
  int duration = time - lastTime;

  if (duration > 500) {
    if (ledsOn) {
      ws2812fx.setColor(0, 0, 0);
      ledsOn = false;
    } else {
      ws2812fx.setColor(lastRed, lastGreen, lastBlue);
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

  ws2812fx.init(); // This initializes the NeoPixel library.
  ws2812fx.setBrightness(START_BRIGHTNESS);
  ws2812fx.setSpeed(START_SPEED);
  ws2812fx.setMode(FX_MODE_STATIC);
  ws2812fx.setColor(lastRed, lastGreen, lastBlue);
  ws2812fx.start();
  pinMode(BUTTON_PIN, INPUT);
  attachInterrupt(BUTTON_PIN, pinChanged, RISING);
}

void loop() {
  sarahHome.loop();
  ws2812fx.service();
}
