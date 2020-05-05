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
#include "velocity.h"
#include <avr/io.h>
#include <pins_arduino.h>
#include <wiring.c>

void KeyboardMux::init() {
  this->setupPortB();
  this->setupPortF();

  // init arrays
  int c = 0;
  for(c=0; c < NUM_KEYS; c++) {
    this->half_on[c] = 0;
    this->half_off[c] = 0;
  }
  for(c=0; c < NUM_BANKS; c++) {
    this->bankstate[c] = 0xff;
  }
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

void KeyboardMux::setKeyOn(void (*keyOn)(unsigned char key, unsigned char velocity)) {
  this->keyOn = keyOn;
}

void KeyboardMux::setKeyOff(void (*keyOff)(unsigned char key, unsigned char velocity)) {
  this->keyOff = keyOff;
}

unsigned char KeyboardMux::velocity(unsigned long micros) {
    if(micros >= 65535) micros = 65535;
    // Drop the 8 lowest bits of the microsecond duration and lookup the corresponding MIDI velocity
    unsigned char time = micros>>8;
    return velocity_log[time];
}

void KeyboardMux::_keyOn(unsigned char key, bool half) {
  if(half) {
    this->half_on[key] = this->time;
  } else {
    (this->keyOn)(key, this->velocity(this->time-this->half_on[key]));
  }
}

void KeyboardMux::_keyOff(unsigned char key, bool half) {
  if(!half) { // Measuring goes backwards
    this->half_off[key] = this->time;
  } else {
    (this->keyOff)(key, this->velocity(this->time-this->half_off[key]));
  }
}

void KeyboardMux::bankChanged(unsigned char data) {
  unsigned char pos;
  unsigned char key;
  unsigned char changed = data ^ this->bankstate[this->current_bank];
  // Determine if the bit is in a primary bank for velocity timing.
  unsigned char primary_bank = (this->current_bank & PRIMARY_BANK_MASK) == PRIMARY_BANK_BITS;

  for(pos = 0; pos < 8; pos++) {
    if(changed & 0x01 == 0x01) {
      // This position has changed.
      // Strip a bit of current bank and determine key number
      key = ((this->current_bank>>1)*8)+(7-pos);

      if(data & 0x01 == 0x01) {
        this->_keyOff(key, (bool)primary_bank);
      } else {
        this->_keyOn(key, (bool)primary_bank);
      }
    }
    changed = changed>>1;
    data = data>>1;
  }
}

void KeyboardMux::loop() {
  this->time = micros();

  // Detect bank state changes
  this->pinb = PINB; // Save the state and use the copy to avoid race conditions
  if(this->bankstate[this->current_bank] != this->pinb) {
    this->bankChanged(PINB);
    // Save current bank state
    this->bankstate[this->current_bank] = this->pinb;
  }

  // Increment bank counter
  this->current_bank ++;
  this->current_bank = 0x0f & this->current_bank;

  // Do bank selection
  PORTF = (this->current_bank<<4) | PORTF_BASE;

}
