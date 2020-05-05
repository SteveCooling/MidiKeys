/**
 *
 * test.ino
 * Test use of the keyboard scanner library
 * 
 * Copyright (c) 2020 <morten@bzzt.no> 
 * 
 */

//#define SERIAL_DEBUG

#include <math.h>
#include "KeyboardMux.h"
#include "MIDIUSB.h"

#define BLINKLED LED_BUILTIN
#define MIDI_CHAN 0

KeyboardMux keyboard;

char transpose = 24;

#define NUM_NOTES 128
unsigned char note_velocity[NUM_NOTES];

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void handleKeyOn(unsigned char key, unsigned char velocity) {
  unsigned char note = key+transpose;
  if(note_velocity[note] == 0) {
    note_velocity[note] = velocity;
    noteOn(MIDI_CHAN, note, velocity);
    #ifdef SERIAL_DEBUG
    Serial.print("keyOn ");
    Serial.print(key);
    Serial.print(" ");
    Serial.println(velocity);
    #endif
  }
  #ifdef SERIAL_DEBUG
  else {
    Serial.print("keyOn IGNORED ");
    Serial.print(key);
    Serial.print(" ");
    Serial.println(velocity);
  }
  #endif
}

void handleKeyOff(unsigned char key, unsigned char velocity) {
  unsigned char note = key+transpose;
  if(note_velocity[note] != 0) {
    note_velocity[note] = 0;
    noteOff(MIDI_CHAN, note, velocity);
    #ifdef SERIAL_DEBUG
    Serial.print("keyOff ");
    Serial.print(key);
    Serial.print(" ");
    Serial.println(velocity);
    #endif
  }
  #ifdef SERIAL_DEBUG
  else {
    Serial.print("keyOff IGNORED ");
    Serial.print(key);
    Serial.print(" ");
    Serial.println(velocity);
  }
  #endif
}

void setup() {
  // initialize digital pin LED_BUILTIN as an output.
  pinMode(BLINKLED, OUTPUT);

  int c = 0;
  for(c=0; c < NUM_NOTES; c++) {
    note_velocity[c] = 0;
  }

  #ifdef SERIAL_DEBUG
  Serial.begin(115200);
  while (!Serial);
  Serial.println("MidiKeys Ready.");
  #endif
  // initialize the keyboard mux reader
  keyboard.init();

  keyboard.setKeyOn(handleKeyOn);
  keyboard.setKeyOff(handleKeyOff);
}

void loop() {
  keyboard.loop();
  MidiUSB.flush();
}
