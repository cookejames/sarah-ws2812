#define VERSION "3"
#include "Adafruit_NeoPixel.h"
#include "SarahHome.h"

SarahHome sarahHome("lights");

#define LED_PIN 15
#define NUMPIXELS 100
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
  if (length != 11) {
    Serial.println("Received invalid message");
    return;
  }

  char redBuffer[4];
  redBuffer[0] = (char)payload[0];
  redBuffer[1] = (char)payload[1];
  redBuffer[2] = (char)payload[2];
  redBuffer[3] = '\0';
  int red = atoi(redBuffer);
  char greenBuffer[4];
  greenBuffer[0] = (char)payload[4];
  greenBuffer[1] = (char)payload[5];
  greenBuffer[2] = (char)payload[6];
  greenBuffer[3] = '\0';
  int green = atoi(greenBuffer);
  char blueBuffer[4];
  blueBuffer[0] = (char)payload[8];
  blueBuffer[1] = (char)payload[9];
  blueBuffer[2] = (char)payload[10];
  blueBuffer[3] = '\0';
  int blue = atoi(blueBuffer);

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
