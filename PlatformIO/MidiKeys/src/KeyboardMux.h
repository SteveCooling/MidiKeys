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
#ifndef NUM_BANKS
#define NUM_BANKS 16
#endif

#ifndef KEYBOARDMUX_H
#define KEYBOARDMUX_H

#define PORTF_BASE 0x00

class KeyboardMux {
  private:
    bool portb_invert = true;
    int current_bank = 0;
    unsigned char bankstate[NUM_BANKS];
    unsigned char state;

    void setupPortB();
    void setupPortF();

    //typedef void (KeyboardMux::*bankchange)(int bank, unsigned char data);
    //bankchange bankChanged;

    // Bank has changes
    void bankChanged();
    // Called when a 
    void (*noteOn)(unsigned char note);
    void (*noteOff)(unsigned char note);

  public:
    void init(bool pullup);
    void loop();
    //void setBankChanged(void (*bankChanged)(int bank, unsigned char data));
    void setNoteOn(void (*noteOn)(unsigned char note));
    void setNoteOff(void (*noteOff)(unsigned char note));
    //bool detectPullup();
};

#endif
