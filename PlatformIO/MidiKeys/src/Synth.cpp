#include "Synth.h"
#include "lut.h"
#include "Arduino.h"

void Synth::init(int tonepin) {
    this->tonepin = tonepin;
    tone(tonepin, 1000, 100);
}

void Synth::update() {
  unsigned long millis_from_start = millis() - this->note_start;
  signed long offset = this->vibrato_depth/4 * sin((float)millis_from_start * ((float)this->vibrato_speed/2048));
  if (this->note > 0) {
    signed int gliss_offset = (millis_from_start < (this->glissando*8)) ? 
      (1-((float)millis_from_start / (this->glissando*8))) * (float)this->gliss_offset
      : 0;

    tone(
      this->tonepin,
      midi_freq[this->note] + offset + gliss_offset
    );
  } else {
    noTone(this->tonepin);
  }
}

void Synth::noteOn(unsigned char note, unsigned long millis) {
  if(this->note > 0) {
    this->gliss_offset = midi_freq[this->note] - midi_freq[note];
  } else {
    this->sound_start = millis;
  }

  this->note_start = millis;
  this->note = note;
}

void Synth::noteOff() {
  this->note = 0;
  this->gliss_offset = 0;
}

void Synth::setVibratoDepth(unsigned char depth) {
  this->vibrato_depth = depth;
}

void Synth::setVibratoSpeed(unsigned char speed) {
  this->vibrato_speed = speed;
}

void Synth::setGlissandoTime(unsigned char gliss) {
  this->glissando = gliss;
}
