/**
 *  Synth
 *  - Bleep bloop!
 */

#ifndef SYNTH_H
#define SYNTH_H


class Synth {
  private:
    int tonepin;

    unsigned long note_start; // Time of current note start
    unsigned long sound_start; // Time of current sound start
    int note;

    unsigned char vibrato_depth = 8;
    unsigned char vibrato_speed = 100;
    unsigned char glissando = 10; 

    signed int gliss_offset = 0;

  public:
    void init(int tonepin);
    // call every 10ms
    void update();

    void noteOn(unsigned char note, unsigned long millis);
    void noteOff();
};

#endif
