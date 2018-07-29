#ifndef _ESP8266_DEBOUNCE_BUTTONS_H
#define _ESP8266_DEBOUNCE_BUTTONS_H

#include <Ticker.h>
#include <eagle_soc.h>


class ESP8266DebounceButtons{
  typedef void(*callbackFunction)(uint16_t pressedMS);
  private:
    uint8_t checkDelayMS;
    callbackFunction buttonCallbacks[GPIO_PIN_COUNT];
    uint32_t buttonPressMS;
    Ticker theTicker;
    uint16_t buttonsMask;
    uint16_t buttonsReleased;
    bool isEnabled;

    void _disable();
    void _enable();
    void _checkButtons();
  public:
    ESP8266DebounceButtons():ESP8266DebounceButtons(50){};
    ESP8266DebounceButtons(uint8_t checkDelayMS);

    static ESP8266DebounceButtons* instance;

    void addButtonPin(uint8_t pin,  callbackFunction callback);
    void removeButtonPin(uint8_t pin);

    void enable();
    void disable();


    void update();
};

#endif