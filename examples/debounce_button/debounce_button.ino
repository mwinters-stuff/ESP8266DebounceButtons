#include <Arduino.h>

#include "ESP8266DebounceSwitch.h"
#include "user_interface.h"

ESP8266DebounceSwitch switches;

uint32_t heap = 0;
void trackMem(String point){
  uint32_t h = system_get_free_heap_size();
  if(heap > 0){
    Serial.printf("Heap Delta at %s %d\n",point.c_str(), h - heap);
  }
  heap = h;
}

void bePin2(uint8_t pin, bool closed){
  Serial.print("D2 ");
  Serial.println(closed ? " CLOSED " : " OPEN");
  switches.addSwitchPin(D2, !closed, bePin2);
}

void bePin3(uint8_t pin, bool closed){
  Serial.print("D3 ");
  Serial.println(closed ? " CLOSED " : " OPEN");
  switches.addSwitchPin(D3, !closed, bePin3);
}

void buttonCB(uint8_t pin, uint32_t ms){
  Serial.print("Button pressed for ");
  Serial.println(ms);
  digitalWrite(D4,!digitalRead(D4));
}

void setup(){
  Serial.begin(115200);
  delay(1000);
  pinMode(D4,OUTPUT);

  switches.addSwitchPin(D2, true, bePin2);
  switches.addSwitchPin(D3, true, bePin3);
  switches.addButtonPin(D5,buttonCB,true);
  
}

uint32_t msx = 0;
void loop(){
  switches.update();
  if(millis() - msx > 10000){
    trackMem("Loop");
    msx = millis();
  }
}