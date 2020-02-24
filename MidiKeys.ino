/**
 * 
 *  MidiKeys - a stab at a MIDI controller keyboard.
 * 
 *  - Uses parts from an old National organ
 *  - Multiplexed key reading
 *  - MIDI over USB
 *
 *  Copyright (c) 2020 - Morten Johansen <morten@bzzt.no> 
 *
 */
#include "MIDIUSB.h"
//#include <EveryTimer.h>

//EveryTimer timer;

#define DEBUG true

#define MIDI_CHAN 0

#define NUM_NOTES 128
bool note_on[NUM_NOTES];

#define NUM_KEYS 6
#define NUM_BANKS 7

byte note_offset = 42; // First note on first bank in MIDI land.

int keys[NUM_KEYS] = {2,3,4,5,6,7};
int banks[NUM_BANKS] = {8,9,10,14,15,16,A0};

void setupKeys() {
  int k = 0;
  int b = 0;
  int n = 0;

  for(n = 0; n < NUM_NOTES; n ++) {
    note_on[n] = true; // Inverse logic.
  }
    
  // Initialize ports
  // Key inputs
  for(k = 0; k < NUM_KEYS; k ++) {
    pinMode(keys[k], INPUT_PULLUP);
  }
  // Bank outputs. Set as inputs with pullup when they are not selected.
  for(b = 0; b < NUM_BANKS; b ++) {
    pinMode(banks[b], INPUT_PULLUP);
  }
  
}

void enableBank(int b) {
    pinMode(b, OUTPUT);
    digitalWrite(b, LOW);
    //delay(1);
}

void disableBank(int b) {
    digitalWrite(b, HIGH);
    pinMode(b, INPUT_PULLUP);
    //delay(1);
}

void scanKeys() {
  int k = 0;
  int b = 0;
  bool keyState = true;
  byte noteNumber = 0;

  //Serial.println("== KEYS ==");
  // Iterate banks
  for(b = 0; b < NUM_BANKS; b ++) {
    enableBank(banks[b]);

    // Iterate keys in the bank
    for(k = 0; k < NUM_KEYS; k ++) {
      keyState = digitalRead(keys[k]);
      noteNumber = note_offset + (b * NUM_KEYS) + k;
      if(keyState != note_on[noteNumber]) {
        // Note state has changed
        if(!keyState) {
          // note is On
          if(DEBUG) {
            Serial.print("Note on  ");
            Serial.println(noteNumber);
          }
          handleKeyboardNoteOn(noteNumber);
        } else {
          // note is Off
          if(DEBUG) {
            Serial.print("Note off ");
            Serial.println(noteNumber);
          }
          handleKeyboardNoteOff(noteNumber);
        }

        // Save the current state.
        note_on[noteNumber] = keyState;
      }
    }

    disableBank(banks[b]);
  }
}

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void handleKeyboardNoteOn(byte number) {
  noteOn(MIDI_CHAN, number, 64);
  MidiUSB.flush();
};

void handleKeyboardNoteOff(byte number) {
  noteOff(MIDI_CHAN, number, 64);
  MidiUSB.flush();
};


void setup() {
    Serial.begin(115200);
    if(DEBUG) while (!Serial); // Wait for serial port to start. Remove after debugging.
    setupKeys();
    //timer.Every(10, scanKeys);
    Serial.println("MidiKeys ready.");
}

void loop() {
    //timer.Update();
    scanKeys();
}
