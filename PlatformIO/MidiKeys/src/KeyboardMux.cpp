/**
 *  KeyboardMux
 *  - Keyboard scanner for Arduino Leonardo (ATMEGA32U4)
 *
 *  Scan 16 banks (High nybble of PORTF) and read PORTB for each bank.
 * 
 *  For velocity sensitive keyboards, PORTF bit 4 selects the second switch for each key.
 *  Put another way: Bank 1 will be the secondary switches of bank 0.
 * 
 *  Output of LSB on PORTF is sent to the MUX and is subsequently read on PORTB.
 *  If you are running pullups on PORTB, set it low - if you need to read high bits, set it high.
 */

#include "KeyboardMux.h"
#include <avr/io.h>
#include <pins_arduino.h>

void KeyboardMux::init() {
  this->setupPortB();
  this->setupPortF();
}

void KeyboardMux::setupPortB() {
  // Disable RXLED, it is part of PORTB and is used to read the keyboard state
  RXLED0;
  // See ATMEGA 32u4 datasheet section 10.2.1
  // Pin directions (input)
  DDRB = 0x00;
  // Pull-ups enabled (board has physical pullups)
  PORTB = 0xFF; // 0xFF for enable pullups
}

void KeyboardMux::setupPortF() {
  // See ATMEGA 32u4 datasheet section 10.2.1
  // Pin directions (output)
  DDRF = 0xFF;
  // Default data
  PORTF = 0x00;
}

void KeyboardMux::setBankChanged(void (*bankChanged)(int bank, unsigned char data)) {
  this->bankChanged = bankChanged;
}

bool KeyboardMux::detectPullup() {
  return PINB >= 0x80;
}

void KeyboardMux::loop() {

  // Read current bank state
  unsigned char state = PINB;

  if(this->bankstate[this->current_bank] != state) {
    // State has changed
    // Save current bank state
    this->bankstate[this->current_bank] = state;
    // Run callback
    (this->bankChanged)(this->current_bank, state);
  }

  // Increment bank counter
  this->current_bank ++;
  this->current_bank = 0x0f & this->current_bank;

  // Do bank selection
  PORTF = (this->current_bank<<4) | PORTF_BASE;

}
