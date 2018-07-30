#include <Arduino.h>
#include "ESP8266DebounceButtons.h"

ESP8266DebounceButtons *ESP8266DebounceButtons::instance;

ESP8266DebounceButtons::ESP8266DebounceButtons(uint8_t checkDelayMS) : checkDelayMS(checkDelayMS),
                                                                       buttonsPressed(0),
                                                                       buttonsPressMask(0),
                                                                       buttonsReleased(0),
                                                                       buttonsReleaseMask(0),
                                                                       press_state(0),
                                                                       press_prevTimeMillis(0),
                                                                       press_mask(0),
                                                                       release_state(0),
                                                                       release_prevTimeMillis(0),
                                                                       release_mask(0),
                                                                       isEnabled(false)
{
  ESP8266DebounceButtons::instance = this;
  for (int pin = 0; pin < GPIO_PIN_COUNT; pin++)
  {
    buttonCallbacks[pin] = [](uint16_t) {};
  }
}

void ESP8266DebounceButtons::addButtonReleasePin(uint8_t pin, callbackFunction callback)
{
  if (isEnabled)
  {
    _disable();
  }
  pinMode(pin, INPUT_PULLUP);
  buttonCallbacks[pin] = callback;
  bitSet(buttonsReleaseMask, pin);
  bitClear(buttonsPressMask, pin);
  if (isEnabled)
  {
    _enable();
  }
}

void ESP8266DebounceButtons::removeButtonReleasePin(uint8_t pin)
{
  if (isEnabled)
  {
    _disable();
  }
  buttonCallbacks[pin] = [](uint16_t) {};
  bitClear(buttonsReleaseMask, pin);
  if (isEnabled)
  {
    _enable();
  }
}

void ESP8266DebounceButtons::addButtonPressPin(uint8_t pin, callbackFunction callback)
{
  if (isEnabled)
  {
    _disable();
  }
  pinMode(pin, INPUT_PULLUP);
  buttonCallbacks[pin] = callback;
  bitSet(buttonsPressMask, pin);
  bitClear(buttonsReleaseMask, pin);
  if (isEnabled)
  {
    _enable();
  }
}

void ESP8266DebounceButtons::removeButtonPressPin(uint8_t pin)
{
  if (isEnabled)
  {
    _disable();
  }
  buttonCallbacks[pin] = [](uint16_t) {};
  bitClear(buttonsPressMask, pin);
  if (isEnabled)
  {
    _enable();
  }
}

void ESP8266DebounceButtons::_enable()
{
  theTicker.attach_ms(checkDelayMS, [] {
    ESP8266DebounceButtons::instance->_checkButtons();
  });
}

void ESP8266DebounceButtons::_disable()
{
  theTicker.detach();
}

void ESP8266DebounceButtons::enable()
{
  if (!isEnabled)
  {
    isEnabled = true;
    _enable();
  }
}

void ESP8266DebounceButtons::disable()
{
  if (isEnabled)
  {
    isEnabled = false;
    _disable();
  }
}

void ESP8266DebounceButtons::update()
{
  if (isEnabled && (buttonsReleased > 0 || buttonsPressed > 0))
  {
    for (int pin = 0; pin < GPIO_PIN_COUNT; pin++)
    {
      if (bitRead(buttonsReleaseMask, pin) && bitRead(buttonsReleased, pin))
      {
        bitClear(buttonsReleased, pin);
        buttonCallbacks[pin](buttonTimeMS);
      }
      if (bitRead(buttonsPressMask, pin) && bitRead(buttonsPressed, pin))
      {
        bitClear(buttonsPressed, pin);
        buttonCallbacks[pin](buttonTimeMS);
      }
    }
    _enable();
  }
}

void ESP8266DebounceButtons::_checkButtons()
{
  uint16_t buttonsUp = GPIO_REG_READ(GPIO_IN_ADDRESS);
  buttonsReleased = _debounceButtons(buttonsReleaseMask, buttonsUp, release_state, release_prevTimeMillis, release_mask);
  buttonsPressed = _debounceButtons(buttonsPressMask, ~buttonsUp, press_state, press_prevTimeMillis, press_mask);
}

// Uses a finite state machine to detect a single button press and return
// the pressed button.  It requires the button to be up/down for at least 15 ms
// and then down for at least 15 ms before reporting the press.
uint16_t ESP8266DebounceButtons::_debounceButtons(const uint16_t buttonsMask, const uint16_t buttonsNow,
                                                  uint8_t &state, uint32_t &prevTimeMillis, uint16_t &mask)
{

  uint32_t timeMillis = millis();

  switch (state)
  {
  case 0:
    if (~buttonsNow & buttonsMask) // if one of the specified buttons is down
    {
      mask = ~buttonsNow & buttonsMask; // mask becomes all of masked down buttons
      prevTimeMillis = timeMillis;
      state = 1;
    }
    break;

  case 1:
    if (timeMillis - prevTimeMillis >= 15) // if 15 ms or longer has elapsed
    {
      if (~buttonsNow & mask) // and if a masked button is still down
      {
        buttonTimeMS = prevTimeMillis;
        state = 2;                 // proceed to next state
        mask = ~buttonsNow & mask; // new mask becomes all of masked down buttons
      }
      else
      {
        state = 0; // go back to previous (initial) state
      }
    }
    break;

  case 2:
    if (buttonsNow & mask) // if a masked button is now up
    {
      state = 3;                // proceed to next state
      mask = buttonsNow & mask; // new mask becomes all of masked up buttons
      prevTimeMillis = timeMillis;
    }
    else if (mask != (~buttonsNow & buttonsMask)) // if our mask becomes inaccurate
    {
      state = 0; // go back to the initial state
    }
    break;

  case 3:
    if (timeMillis - prevTimeMillis >= 15) // if 15 ms or longer has elapsed
    {
      if (buttonsNow & mask) // and if a masked button is still up
      {
        state = 0; // next state becomes initial state
        buttonTimeMS = timeMillis - buttonTimeMS;
        _disable();
        return buttonsNow & mask; // return masked up buttons
      }
      else
      {
        state = 2; // go back to previous state
      }
    }
    break;
  }
  return 0;
}
