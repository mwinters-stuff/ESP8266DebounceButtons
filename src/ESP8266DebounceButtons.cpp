#include <Arduino.h>
#include "ESP8266DebounceButtons.h"

ESP8266DebounceButtons*  ESP8266DebounceButtons::instance;

ESP8266DebounceButtons::ESP8266DebounceButtons(uint8_t checkDelayMS): 
  checkDelayMS(checkDelayMS),
  buttonsMask(0),
  buttonsReleased(0),
  isEnabled(false){
  ESP8266DebounceButtons::instance = this;
  for(int pin = 0; pin < GPIO_PIN_COUNT; pin++){
    buttonCallbacks[pin] = [](uint16_t){};
  }
}


void ESP8266DebounceButtons::addButtonPin(uint8_t pin,  callbackFunction callback){
  if(isEnabled){
    _disable();
  }
  pinMode(pin, INPUT_PULLUP);
  buttonCallbacks[pin] = callback;
  bitSet(buttonsMask,pin);
  if(isEnabled){
    _enable();
  }
}

void ESP8266DebounceButtons::removeButtonPin(uint8_t pin){
  if(isEnabled){
    _disable();
  }
  buttonCallbacks[pin] = [](uint16_t){};
  bitClear(buttonsMask,pin);
  if(isEnabled){
    _enable();
  }
}

void ESP8266DebounceButtons::_enable(){
  theTicker.attach_ms(checkDelayMS, ESP8266DebounceButtons::handleTicker);
}

void ESP8266DebounceButtons::_disable(){
  theTicker.detach();
}

void ESP8266DebounceButtons::enable(){
  if(!isEnabled){
    isEnabled = true;
    _enable();
  }
}

void ESP8266DebounceButtons::disable(){
  if(isEnabled){
    isEnabled = false;
    _disable();
  }
}


void ESP8266DebounceButtons::update(){
  if(isEnabled && buttonsReleased > 0){
    for(int pin = 0; pin < GPIO_PIN_COUNT; pin++){
      if(bitRead(buttonsMask,pin) && bitRead(buttonsReleased,pin)){
        bitClear(buttonsReleased,pin);
        buttonCallbacks[pin](buttonPressMS);
      }
    }
    _enable();
  }
}

void ESP8266DebounceButtons::_checkButtons(){
  static uint8_t state = 0;
  static uint32_t prevTimeMillis = 0;
  static uint16_t mask = 0;
  
  uint32_t timeMillis = millis();
  
  uint16_t buttonsUp = GPIO_REG_READ(GPIO_IN_ADDRESS);
  switch (state)
  {
    case 0:
      if (~buttonsUp & buttonsMask)       // if one of the specified buttons is down
      {
        mask = ~buttonsUp & buttonsMask;    // mask becomes all of masked down buttons
        prevTimeMillis = timeMillis;
        state = 1;
      }
      break;
      
    case 1:
      if (timeMillis - prevTimeMillis >= 15)  // if 15 ms or longer has elapsed
      {
        if (~buttonsUp & mask)        // and if a masked button is still down
        {
          buttonPressMS = prevTimeMillis;
          state = 2;            // proceed to next state
          mask = ~buttonsUp & mask;   // new mask becomes all of masked down buttons
        }
        else
        {
          state = 0;            // go back to previous (initial) state
        }
      }
      break;
      
    case 2:
      if (buttonsUp & mask)         // if a masked button is now up
      {
        state = 3;              // proceed to next state
        mask = buttonsUp & mask;      // new mask becomes all of masked up buttons
        prevTimeMillis = timeMillis;
      }
      else if (mask != (~buttonsUp & buttonsMask))  // if our mask becomes inaccurate
      {
        state = 0;              // go back to the initial state
      }
      break;

    case 3:
      if (timeMillis - prevTimeMillis >= 15)  // if 15 ms or longer has elapsed
      {
        if (buttonsUp & mask)       // and if a masked button is still up
        {
          state = 0;            // next state becomes initial state
          buttonsReleased = buttonsUp & mask;    // return masked up buttons
          buttonPressMS = timeMillis - buttonPressMS;
          _disable();
        }
        else
        {
          state = 2;            // go back to previous state
        }
      }
      break;
  }

}
void ESP8266DebounceButtons::handleTicker(){
  ESP8266DebounceButtons::instance->_checkButtons();
}