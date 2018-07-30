#include <Arduino.h>

#include "ESP8266DebounceButtons.h"

ESP8266DebounceButtons debounce;
uint8_t button7PressCount = 10;
void setup(){
  Serial.begin(115200);
  pinMode(D4,OUTPUT);

  debounce.addButtonPressPin(D3, [](bool pressed){
    digitalWrite(D4,!digitalRead(D4));
    Serial.println(F("Button D3 Pressed"));
    
  } );

  debounce.addButtonReleasePin(D7, [](bool pressed){
    Serial.println(F("Button D7 Released"));
    button7PressCount--;
    if(button7PressCount == 0){
      debounce.removeButtonReleasePin(D7);
      Serial.println(F("Button D7 removed from debouncer"));
    }
  } );

  debounce.enable();

}

void loop(){
  debounce.update();
}