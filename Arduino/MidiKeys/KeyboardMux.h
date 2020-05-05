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

#define NUM_KEYS 128

#ifndef KEYBOARDMUX_H
#define KEYBOARDMUX_H

#define PORTF_BASE 0x00

// When this matches a primary key switch has been trigged
#define PRIMARY_BANK_MASK 0x01
#define PRIMARY_BANK_BITS 0x01

class KeyboardMux {
  private:
    int current_bank = 0;
    unsigned char bankstate[NUM_BANKS];
    unsigned char pinb;
    unsigned long time;

    // Timestamps for half events
    unsigned long half_on[NUM_KEYS];
    unsigned long half_off[NUM_KEYS];

    void setupPortB();
    void setupPortF();

    void bankChanged(unsigned char data);
    
    void _keyOn(unsigned char key, bool half);
    void _keyOff(unsigned char key, bool half);

    unsigned char velocity(unsigned long micros);

    void (*keyOn)(unsigned char key, unsigned char velocity);
    void (*keyOff)(unsigned char key, unsigned char velocity);

  public:
    void init();
    void loop();
    //void setBankChanged(void (*bankChanged)(int bank, unsigned char data));
    void setKeyOn(void (*keyOn)(unsigned char key, unsigned char velocity));
    void setKeyOff(void (*keyOff)(unsigned char key, unsigned char velocity));
};

#endif
