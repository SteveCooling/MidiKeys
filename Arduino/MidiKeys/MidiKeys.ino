/**
 *
 * test.ino
 * Test use of the keyboard scanner library
 * 
 * Copyright (c) 2020 <morten@bzzt.no> 
 * 
 */

#define NUM_BANKS 16
#define NUM_NOTES 128

//#define SERIAL_DEBUG

// When this matches a primary key switch has been trigged
#define PRIMARY_BANK_MASK 0x01
#define PRIMARY_BANK_BITS 0x01

#include <math.h>
#include "KeyboardMux.h"
#include "MIDIUSB.h"
#include "velocity.h"

#define BLINKLED LED_BUILTIN
#define MIDI_CHAN 0

KeyboardMux keyboard;
bool primary_bank;

// Timestamps for half events
unsigned long half_on[NUM_NOTES];
unsigned long half_off[NUM_NOTES];
// Stores computed velocity for each note
unsigned char note_velocity[NUM_NOTES];

char transpose = 24;

float velocity_factor = 0.2;
float velocity_curve  = 60;

unsigned long time = micros();

unsigned char bankAndPosToNote(int bank, unsigned char pos) {
  // XXX: This requires primary/second bank bit to be LSB
  unsigned char note = (bank>>1) * 8;
  return note + (7-pos) + transpose;
}

unsigned char microsToVelocity(unsigned long micros) {
    if(micros >= 65535) micros = 65535;
    // Drop the 8 lowest bits of the microsecond duration and lookup the corresponding MIDI velocity
    unsigned char time = micros>>8;
    return velocity_log[time];
}

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteChanged(unsigned char note, bool primary_bank) {
  if(note_velocity[note] == 0) {
    // Note is turning on
    if(primary_bank) {
      //Serial.println(" is half on");
      half_on[note] = time;
    } else {
      note_velocity[note] = microsToVelocity(time-half_on[note]);
      //half_on[note] = 0;
      noteOn(MIDI_CHAN, note, note_velocity[note]);
      #ifdef SERIAL_DEBUG
      Serial.print(note, DEC);
      Serial.print(" is fully on  - ");
      Serial.println(time-half_on[note], DEC);
      Serial.print(" - ");
      Serial.println(note_velocity[note], DEC);
      #endif
    }
  } else {
    // Note is turning off
    if(primary_bank) {
      note_velocity[note] = microsToVelocity(time-half_off[note]);
      //half_off[note] = 0;
      noteOff(MIDI_CHAN, note, note_velocity[note]);
      #ifdef SERIAL_DEBUG
      Serial.print(note, DEC);
      Serial.print(" is fully off - ");
      Serial.println(time-half_on[note], DEC);
      Serial.print(" - ");
      Serial.println(note_velocity[note], DEC);
      #endif
      note_velocity[note] = 0;
    } else {
      //Serial.println(" is half off");
      half_off[note] = time;
    }
  }

}

void bankChanged(int bank, unsigned char data) {
  unsigned char pos;
  unsigned char note;

  primary_bank = (bank & PRIMARY_BANK_MASK) == PRIMARY_BANK_BITS;

  #ifdef SERIAL_DEBUG
  Serial.print(bank);
  Serial.print("\t");
  Serial.println(data, HEX);
  #endif

  for(pos = 0; pos < 8; pos++) {
    if(data & 0x01 == 0x01) {
      note = bankAndPosToNote(bank, pos);
      //Serial.print("\t");
      //Serial.println(note, DEC);
      // Now we know what note changed
      noteChanged(note, primary_bank);
    }
    data = data>>1;
  }
}

// the setup function runs once when you press reset or power the board
void setup() {
  // init arrays
  int c = 0;
  for(c=0; c < NUM_NOTES; c++) {
    half_on[c] = 0;
    half_off[c] = 0;
    note_velocity[c] = 0;
  }
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(BLINKLED, OUTPUT);
  #ifdef SERIAL_DEBUG
  Serial.begin(115200);
  #endif
  // initialize the keyboard mux reader
  keyboard.init();
  keyboard.setBankChanged(bankChanged);
}

// the loop function runs over and over again forever
void loop() {
  time = micros();
  keyboard.loop();
  MidiUSB.flush();

}
