#include <Arduino.h>

#include "ESP8266DebounceSwitch.h"

typedef void (*voidFuncPtr)(void);
extern "C" void ICACHE_RAM_ATTR __attachInterruptArg(uint8_t pin, voidFuncPtr userFunc, void*fp , int mode);

ESP8266DebounceSwitch *ESP8266DebounceSwitch::instance;

ESP8266DebounceSwitch::ESP8266DebounceSwitch():
                                                                      pinsWaitOpenMask(0),
                                                                      pinsWaitClosedMask(0),
                                                                      pinsTriggeredClosed(0),
                                                                      pinsTriggeredOpen(0)                                                                       
{
  ESP8266DebounceSwitch::instance = this;
}

void _checkPinClosed(uint8_t pin)
{
  ESP8266DebounceSwitch::instance->checkPinClosed(pin);
}
void _checkPinOpen(uint8_t pin)
{
  ESP8266DebounceSwitch::instance->checkPinOpen(pin);
}

void _pinClosedISR(void *arg){
  ESP8266DebounceSwitch::instance->pinClosedISR((uint32_t) arg);
}

void ESP8266DebounceSwitch::pinClosedISR(uint8_t pin){
  pinTickers[pin].once_ms(20,_checkPinClosed,pin);
}

void _pinOpenISR(void *arg){
  ESP8266DebounceSwitch::instance->pinOpenISR((uint32_t) arg);
}

void ESP8266DebounceSwitch::pinOpenISR(uint8_t pin){
  pinTickers[pin].once_ms(20,_checkPinOpen,pin);
}

void ESP8266DebounceSwitch::addSwitchPin(uint8_t pin, bool waitClosed, callbackFunctionSwitch callback)
{
  pinMode(pin, INPUT_PULLUP);
  triggeredCallbacks[pin] = callback;
  bitClear(pinsWaitClosedMask, pin);
  bitClear(pinsWaitOpenMask, pin);
  bitSet(waitClosed ? pinsWaitClosedMask : pinsWaitOpenMask, pin);
  detachInterrupt(digitalPinToInterrupt(pin));
  
  if(waitClosed){
    __attachInterruptArg(digitalPinToInterrupt(pin),(voidFuncPtr) _pinClosedISR,(void*)(uint32_t) pin, FALLING);
  }else{
    __attachInterruptArg(digitalPinToInterrupt(pin),(voidFuncPtr) _pinOpenISR,(void*)(uint32_t) pin, RISING);
  }
}

void ESP8266DebounceSwitch::removeSwitchPin(uint8_t pin)
{
  bitClear(pinsWaitClosedMask, pin);
  bitClear(pinsWaitOpenMask, pin);
  detachInterrupt(pin);
  pinTickers[pin].detach();

}
void ESP8266DebounceSwitch::_buttonSwitchOpen(uint8_t pin, bool closed){
  ESP8266DebounceSwitch::instance->buttonCallbacks[pin](pin, millis() - ESP8266DebounceSwitch::instance->buttonClosedMillis[pin]);
  if(ESP8266DebounceSwitch::instance->buttonRepeating){
    ESP8266DebounceSwitch::instance->addSwitchPin(pin, true, ESP8266DebounceSwitch::_buttonSwitchClosed);
  }
}

void ESP8266DebounceSwitch::_buttonSwitchClosed(uint8_t pin, bool closed){
  ESP8266DebounceSwitch::instance->buttonClosedMillis[pin] = millis();
  ESP8266DebounceSwitch::instance->addSwitchPin(pin, false, ESP8266DebounceSwitch::_buttonSwitchOpen);
}

void ESP8266DebounceSwitch::addButtonPin(uint8_t pin, callbackFunctionButton callback, bool repeat)
{
  buttonCallbacks[pin] = callback;
  buttonRepeating[pin] = repeat;
  addSwitchPin(pin,true,_buttonSwitchClosed);
}

void ESP8266DebounceSwitch::removeButtonPin(uint8_t pin)
{
  removeSwitchPin(pin);
}

void ESP8266DebounceSwitch::checkPinClosed(uint8_t pin)
{
  if(digitalRead(pin) == LOW){
    bitSet(pinsTriggeredClosed,pin);
    bitClear(pinsTriggeredOpen,pin);
  }else{
    bitClear(pinsTriggeredClosed,pin);
    bitClear(pinsTriggeredOpen,pin);
  }
}



void ESP8266DebounceSwitch::checkPinOpen(uint8_t pin)
{
 if(digitalRead(pin) == HIGH){
    bitSet(pinsTriggeredOpen,pin);
    bitClear(pinsTriggeredClosed,pin);
  }else{
    bitClear(pinsTriggeredOpen,pin);
    bitClear(pinsTriggeredClosed,pin);
  }
}

void ESP8266DebounceSwitch::update()
{
  for (uint8_t pin = 0; pin < GPIO_PIN_COUNT; pin++)
  {
    if(bitRead(pinsTriggeredClosed,pin)){
      triggeredCallbacks[pin](pin, true);
      bitClear(pinsTriggeredClosed,pin);
    } 

    if(bitRead(pinsTriggeredOpen,pin)){
      triggeredCallbacks[pin](pin, false);
      bitClear(pinsTriggeredOpen,pin);
    }

  }
}