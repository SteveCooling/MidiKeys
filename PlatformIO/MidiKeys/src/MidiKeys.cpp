/**
 *
 * test.ino
 * 
 * Test use of the keyboard scanner library
 * Uses the MidiUSB extension to send MIDI over USB to the host
 * Reference: https://www.arduino.cc/en/Reference/MIDIUSB
 * 
 * Copyright (c) 2020 <morten@bzzt.no> 
 * 
 */

#define NUM_BANKS 16
#define NUM_NOTES 128
#define SIZE_NOTESTACK 64
#define SERIAL_DEBUG

// When this matches a primary key switch has been trigged
#define PRIMARY_BANK_MASK 0x01
#define PRIMARY_BANK_BITS 0x01

#define SPEAKER 12
#define ADCPIN 6

//#include <math.h>
#include "KeyboardMux.h"
#include "Synth.h"

#include <MIDIUSB.h>
#include <EveryTimer.h>

#include "velocity.h"
#include "lut.h"

#define BLINKLED LED_BUILTIN
#define MIDI_CHAN 0

KeyboardMux keyboard;
Synth synth;
EveryTimer timer;

bool primary_bank;

// Timestamps for half events
unsigned long half_on[NUM_NOTES];
unsigned long half_off[NUM_NOTES];
// Stores computed velocity for each note
unsigned char note_velocity[NUM_NOTES];

char transpose = 24;

unsigned char playing = 0;
unsigned char notestack[SIZE_NOTESTACK];

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


  /*
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
  }*/

}

void noteOn(unsigned char note) {
  Serial.print("on\t");
  Serial.println(note, DEC);

  // Put the note in the stack
  playing += 1;
  notestack[playing] = note;

  synth.noteOn(note, time/1000);

}

void noteOff(unsigned char note) {
  Serial.print("off\t");
  Serial.println(note, DEC);

  for(unsigned char i = playing; i > 0; i --) {
    // Remove note from stack
    if(notestack[i] == note) {
      notestack[i] = 0;
      break;
    }
  }

  for(unsigned char i = playing; i > 0; i --) {
    if(notestack[i] != 0) {
      playing = i;
      synth.noteOn(notestack[i], time/1000);
      return;
    }
  }

  playing = 0;
  synth.noteOff();

}

void analogValue(int input, int value) {
  #ifdef SERIAL_DEBUG
  Serial.print("Input: ");
  Serial.print(input);
  Serial.print(" now value: ");
  Serial.println(value);
  #endif
}

void cb() {
  synth.update();
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

  notestack[0] = 0;

  // initialize digital pin LED_BUILTIN as an output.
  pinMode(BLINKLED, OUTPUT);
  pinMode(ADCPIN, INPUT);
  #ifdef SERIAL_DEBUG
  Serial.begin(115200);
  #endif
  // initialize the keyboard mux reader
  keyboard.init(true);

  keyboard.setNoteOn(noteOn);
  keyboard.setNoteOff(noteOff);
  keyboard.setAdcRead(analogValue);

  timer.Every(10, cb);

  // Ready beep
  synth.init(SPEAKER);
}

// the loop function runs over and over again forever
void loop() {
  time = micros();
  keyboard.loop();
  MidiUSB.flush();
  timer.Update();
}
