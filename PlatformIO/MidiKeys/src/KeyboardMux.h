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

#ifndef KEYBOARDMUX_H
#define KEYBOARDMUX_H

#define NUM_BANKS 16
#define PORTF_BASE 0x00
#define ADC_LATCH 0x01
// The ADC MUX is latched every ADC_LATCH_COUNT. It must be set such that all
// desired inputs are eventually read. A multiple of NUM_BANKS + 1 will do.
#define ADC_LATCH_COUNT 481  

class KeyboardMux {
  private:
    bool portb_invert = true;
    int current_bank = 0;
    unsigned char bankstate[NUM_BANKS];
    //int adcstate[NUM_BANKS];
    int adc_count = 0;

    unsigned char state;
    int astate;

    void setupPortB();
    void setupPortF();

    //typedef void (KeyboardMux::*bankchange)(int bank, unsigned char data);
    //bankchange bankChanged;

    // Bank has changes
    void bankChanged();
    void analogBankChanged();

    // Callbacks
    void (*noteOn)(unsigned char note);
    void (*noteOff)(unsigned char note);
    void (*adcRead)(int input, int value) = nullptr;

  public:
    void init(bool pullup);
    void loop();
    //void setBankChanged(void (*bankChanged)(int bank, unsigned char data));
    void setNoteOn(void (*noteOn)(unsigned char note));
    void setNoteOff(void (*noteOff)(unsigned char note));
    void setAdcRead(void (*adcRead)(int input, int value));
    //bool detectPullup();
    int adcstate[NUM_BANKS];
};

#endif
