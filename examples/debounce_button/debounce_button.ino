#include <Arduino.h>

#include "ESP8266DebounceButtons.h"

ESP8266DebounceButtons debounce;
uint8_t button7PressCount = 3;
void setup(){
  Serial.begin(115200);
  pinMode(D4,OUTPUT);

  debounce.addButtonPin(D3, [](uint16_t msPressed){
    digitalWrite(D4,!digitalRead(D4));
    Serial.printf("Button D3 Pressed for %d\n", msPressed);
    
  } );

  debounce.addButtonPin(D7, [](uint16_t msPressed){
    Serial.printf("Button D7 Pressed for %d\n", msPressed);
    button7PressCount--;
    if(button7PressCount == 0){
      debounce.removeButtonPin(D7);
      Serial.println("Button D7 removed from debouncer");
    }
  } );

  debounce.enable();

}

void loop(){
  debounce.update();
}