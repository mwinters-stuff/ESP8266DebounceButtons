#ifndef _ESP8266_DEBOUNCE_BUTTONS_H
#define _ESP8266_DEBOUNCE_BUTTONS_H

#include <Ticker.h>
#include <eagle_soc.h>

class ESP8266DebounceButtons
{
  typedef void (*callbackFunction)(uint16_t timeMS);

private:
  uint8_t checkDelayMS;
  callbackFunction buttonCallbacks[GPIO_PIN_COUNT];
  uint32_t buttonTimeMS;
  Ticker theTicker;
  uint16_t buttonsReleaseMask;
  uint16_t buttonsPressMask;
  uint16_t buttonsReleased;
  uint16_t buttonsPressed;
  bool isEnabled;

  uint8_t press_state;
  uint32_t press_prevTimeMillis;
  uint16_t press_mask;

  uint8_t release_state;
  uint32_t release_prevTimeMillis;
  uint16_t release_mask;

  void _disable();
  void _enable();
  void _checkButtons();

  uint16_t _debounceButtons(const uint16_t buttonsMask, const uint16_t buttonsNow,
                            uint8_t &state, uint32_t &prevTimeMillis, uint16_t &mask);

public:
  ESP8266DebounceButtons() : ESP8266DebounceButtons(10){};
  ESP8266DebounceButtons(uint8_t checkDelayMS);

  static ESP8266DebounceButtons *instance;

  void addButtonReleasePin(uint8_t pin, callbackFunction callback);
  void removeButtonReleasePin(uint8_t pin);

  void addButtonPressPin(uint8_t pin, callbackFunction callback);
  void removeButtonPressPin(uint8_t pin);

  void enable();
  void disable();

  void update();
};

#endif