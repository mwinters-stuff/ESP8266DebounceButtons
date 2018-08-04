#ifndef _ESP8266_DEBOUNCE_SWITCH_H
#define _ESP8266_DEBOUNCE_SWITCH_H

#include <Ticker.h>
#include <eagle_soc.h>
#include <functional>

class ESP8266DebounceSwitch
{
  typedef std::function<void(uint8_t pin, bool closed)> callbackFunctionSwitch;
  typedef std::function<void(uint8_t pin, uint32_t millisDown)> callbackFunctionButton;

  private:
    uint16_t pinsWaitClosedMask;
    uint16_t pinsWaitOpenMask;
    uint16_t pinsTriggeredClosed;
    uint16_t pinsTriggeredOpen;

    callbackFunctionSwitch triggeredCallbacks[GPIO_PIN_COUNT];
    callbackFunctionButton buttonCallbacks[GPIO_PIN_COUNT];
    bool buttonRepeating[GPIO_PIN_COUNT];
    uint32_t buttonClosedMillis[GPIO_PIN_COUNT];
    Ticker pinTickers[GPIO_PIN_COUNT];
    
  public:
  
    ESP8266DebounceSwitch();

    static ESP8266DebounceSwitch *instance;

    void addSwitchPin(uint8_t pin, bool waitClosed, callbackFunctionSwitch callback);
    void removeSwitchPin(uint8_t pin);

    void addButtonPin(uint8_t pin, callbackFunctionButton  callback, bool repeat);
    void removeButtonPin(uint8_t pin);

    void update();

    void checkPinClosed(uint8_t pin);
    void checkPinOpen(uint8_t pin);

    void pinClosedISR(uint8_t pin);
    void pinOpenISR(uint8_t pin);

    static void _buttonSwitchClosed(uint8_t pin, bool closed);
    static void _buttonSwitchOpen(uint8_t pin, bool closed);
};

#endif