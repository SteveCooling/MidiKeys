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
#include <Arduino.h>

void KeyboardMux::init(bool pullup) {
  this->portb_invert = pullup;
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

void KeyboardMux::setNoteOn(void (*noteOn)(unsigned char note)) {
  this->noteOn = noteOn;
}

void KeyboardMux::setNoteOff(void (*noteOff)(unsigned char note)) {
  this->noteOff = noteOff;
}

void KeyboardMux::setAdcRead(void (*adcRead)(int input, int value)) {
  this->adcRead = adcRead;
}

void KeyboardMux::bankChanged() {
  unsigned char pos;
  unsigned char note;

  unsigned char datanow  = this->state;
  unsigned char datalast = this->bankstate[this->current_bank];

  for(pos = 0; pos < 8; pos++) {
    // Compare last bit of datanow to datalast
    if((datanow & 0x01) ^ (datalast & 0x01)) {
      // This bit has flipped
      // Calculate note number. TODO: make this configurable
      note = this->current_bank - (pos * 6) - 173;

      // Callbacks
      if((datanow & 0x01) == 0x01) {
        this->portb_invert ? noteOff(note) : noteOn(note);
      } else {
        this->portb_invert ? noteOn(note) : noteOff(note);
      }
    }
    // Move on to the next bit in bank
    datanow = datanow>>1;
    datalast = datalast>>1;
  }
}

void KeyboardMux::analogBankChanged() {
  if (adcRead != nullptr) {
    adcRead(this->current_bank, this->astate);
  }
}

void KeyboardMux::loop() {

  // Read current bank state
  this->state = PINB;

  if(this->bankstate[this->current_bank] != this->state) {
    // State has changed
    this->bankChanged();
    // Save current bank state
    this->bankstate[this->current_bank] = this->state;
  }

  // Increment bank counter
  this->current_bank ++;
  this->current_bank = 0x0f & this->current_bank;
  
  // ADC multiplexing
  this->adc_count ++;
  if(this->adc_count >= ADC_LATCH_COUNT) {
    this->astate = analogRead(6); // Here be dragons. Check 32u4 datasheet for speedups.
    if(this->adcstate[this->current_bank] != this->astate) {
      this->analogBankChanged();
    }
    this->adcstate[this->current_bank] = this->astate;

    // Prepare for the next analog reading
    this->adc_count = 0;
    // Select bank and set the ADC_LATCH bit
    PORTF = (this->current_bank<<4) | PORTF_BASE | ADC_LATCH;
  }

  // Do bank selection (also clear ADC_LATCH bit)
  PORTF = (this->current_bank<<4) | PORTF_BASE;

}
