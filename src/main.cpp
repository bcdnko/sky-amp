#include <Arduino.h>
#include <BLEMidi.h>

#define NOISE_THRESHOLD 2

#define SWITCH_PIN 19
#define BUTTON_PIN 18

int knobPins[] = { 36, 39, 34, 35, 32, 33, 25, 26 };
int leds[] = { 4, 16 };
bool pushButtonState = false;
bool switchState = false;

int controllers[sizeof(knobPins)] = { 
  16, 18, 19, 20, 21, 22, 23, 24
};

int oldValue[sizeof(knobPins)] = { 0, 0, 0, 0, 0, 0, 0, 0 };

void setup() {
  Serial.begin(115200);
  
  Serial.println("Initializing bluetooth");
  BLEMidiServer.begin("SkyAmp");
  Serial.println("Waiting for connections...");
  //BLEMidiServer.enableDebugging();

  for (int i = 0; i < (sizeof(leds) / sizeof(leds[0])); i++) {
    pinMode(leds[i], OUTPUT);
    digitalWrite(leds[i], HIGH);
  }

  pinMode(SWITCH_PIN, INPUT_PULLUP);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
  if(!BLEMidiServer.isConnected()) {
    // return;
  }

  for (int i = 0; i < (sizeof(knobPins) / sizeof(knobPins[0])); i++) {
    int v = analogRead(knobPins[i]);
    int value = map(v, 0, 4095, 0, 127);
    if (abs(oldValue[i] - value) <= NOISE_THRESHOLD) {
      continue;
    }

    oldValue[i] = value;
    Serial.print(controllers[i]);
    Serial.print(' ');
    Serial.println(value);
    BLEMidiServer.controlChange(1, controllers[i], value);
  }

  int newSwitchState = digitalRead(SWITCH_PIN);
  if (newSwitchState != switchState) {
    digitalWrite(leds[0], newSwitchState ? HIGH : LOW);
    if (newSwitchState) {
      BLEMidiServer.controlChange(1, 30, 127);
    } else {
      BLEMidiServer.controlChange(1, 30, 0);
    }
    switchState = newSwitchState;
  }

  if (digitalRead(BUTTON_PIN) == LOW) {
    pushButtonState = !pushButtonState;
    digitalWrite(leds[1], pushButtonState ? HIGH : LOW);
    if (pushButtonState) {
      BLEMidiServer.controlChange(1, 31, 127);
    } else {
      BLEMidiServer.controlChange(1, 31, 0);
    }
    delay(100);
  }

  delay(100);
}