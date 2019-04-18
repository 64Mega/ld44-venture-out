#include "sound.h"
#include <i86.h>
#include <conio.h>
#include <dos.h>

#define STATUS_TIMER_ANY_EXPIRED 0x80
#define STATUS_TIMER_1_EXPIRED 0x40
#define STATUS_TIMER_2_EXPIRED 0x20

// Some frequency data and other things
static unsigned char fm_key_freqs[12] = {0x57,0x6b,0x81,0x98,0xb0,0xca,0xe5,0x02,0x20,0x41,0x63,0x87};
static unsigned char fm_key_orval[12] = {1,1,1,1,1,1,1,2,2,2,2,2};
// Function defs

unsigned char fm_get_status() {
    return inp(FM_C_PORT);
}

static void delay1() {
    int i = 0;
    for(i = 0; i < 6; i++) {
        inp(FM_C_PORT);
    }
}

static void delay2() {
    int i = 0;
    for(i = 0; i < 35; i++) {
        inp(FM_C_PORT);
    }
}

void fm_write_reg(unsigned char reg, unsigned char data) {
    outp(FM_C_PORT, reg);
    delay1();
    outp(FM_C_DATA, data);
    delay2();
}

void fm_clear() {
    // Clear all bits and flags
    int i = 0;
    fm_write_reg(0x01, 0x00);
    fm_write_reg(0x02, 0x00);
    fm_write_reg(0x03, 0x00);
    fm_write_reg(0x04, 0x00);
    fm_write_reg(0x08, 0x00);
    fm_write_reg(0xBD, 0x00);
    
    for(i = 0x00; i < 0x15; i++) {
        fm_write_reg(0x20 + i, 0x00);
        fm_write_reg(0x40 + i, 0x00);
        fm_write_reg(0x60 + i, 0x00);
        fm_write_reg(0x80 + i, 0x00);
        fm_write_reg(0xE0 + i, 0x00);
    }
    for(i = 0x00; i < 0x08; i++) {
        fm_write_reg(0xA0 + i, 0x00);
        fm_write_reg(0xB0 + i, 0x00);
        fm_write_reg(0xC0 + i, 0x00);
    }
}

void fm_set_instrument(unsigned char idex, unsigned char inst[]) {    
    fm_write_reg(0x20 + idex, inst[0]); // OP1: Amp Mod/Vibrato/EG Type/Key Scaling/Multiple
    fm_write_reg(0x23 + idex, inst[1]); // OP2: ^^^

    fm_write_reg(0x40 + idex, inst[2]); // OP1: Key Scaling Level/Operator Input Level
    fm_write_reg(0x43 + idex, inst[3]); // OP2: ^^^

    fm_write_reg(0x60 + idex, inst[4]); // OP1: Attack Rate/Decay Rate
    fm_write_reg(0x63 + idex, inst[5]); // OP2: ^^^

    fm_write_reg(0x80 + idex, inst[6]); // OP1: Sustain Level/Release Rate
    fm_write_reg(0x83 + idex, inst[7]); // OP2: ^^^

    fm_write_reg(0xA0 + idex, inst[8]); // Frequency (Lower 8 bits)
    fm_write_reg(0xB0 + idex, inst[9]); // Key on, octave, frequency (Upper 2 bits)
    fm_write_reg(0xC0 + idex, inst[10]); // Feedback Strength / Connection Type
}

void fm_play_note(unsigned char voice, unsigned char note) {
    unsigned char octave;
    unsigned char note_index;
    note_index = note-24;
    octave = 1;
    while(note_index >= 12) {
        note_index -= 12;
        octave++;
    }

    octave = fm_key_orval[note_index]|(octave << 2);
    fm_write_reg(0xA0 + voice, fm_key_freqs[note_index]);
    fm_write_reg(0xB0 + voice, octave | 0x20);    
}

void fm_stop_note(unsigned char voice) {
    fm_write_reg(0xB0 + voice, 0x00);
}

void fm_enable_rhythm() {
    fm_write_reg(0xBD, 0xE0);
}

void fm_disable_rhythm() {
    fm_write_reg(0xBD, 0xC0);
}

void fm_rhythm_on(unsigned char type) {
    fm_write_reg(0xBD, 0xE0 | type);
}

void fm_rhythm_off(unsigned char type) {
    fm_write_reg(0xBD, 0xE0 & (~type));
}
