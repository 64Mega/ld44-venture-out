// Sound library
// =-----------=

#ifndef SOUND_H
#define SOUND_H

/** AdLib/FM section - Compatible with SB Compatible cards **/

// Rhythm channels (Percussion)
#define FM_RHYTHM_HIHAT 0x01
#define FM_RHYTHM_CYMBAL 0x02
#define FM_RHYTHM_TOM 0x04
#define FM_RHYTHM_SNARE 0x08
#define FM_RHYTHM_BASS 0x10

// Port addresses for Center channel 
#define FM_C_PORT 0x388
#define FM_C_DATA 0x389

// Port addresses for Left/Right channels
#define FM_L_PORT 0x220
#define FM_L_DATA 0x221
#define FM_R_PORT 0x222
#define FM_R_DATA 0x223

typedef struct fm_instrument {
    unsigned char amp_mod[2];
    unsigned char vibrato[2];
    unsigned char eg_type[2];
    unsigned char key_scaling[2];
    unsigned char multiple[2];
    unsigned char key_scaling_level[2];
    unsigned char operator_output_level[2];
    unsigned char attack_rate[2];
    unsigned char decay_rate[2];
    unsigned char sustain_level[2];
    unsigned char release_rate[2];
    unsigned char key_on;
    unsigned char octave;
    unsigned int frequency;
};

void fm_write_reg(unsigned char reg, unsigned char data);
void fm_clear();
void fm_set_instrument(unsigned char idex, unsigned char inst[]);
void fm_play_note(unsigned char voice, unsigned char note);
void fm_stop_note(unsigned char voice);
void fm_enable_rhythm();
void fm_disable_rhythm();
void fm_rhythm_on(unsigned char type);
void fm_rhythm_off(unsigned char type);

// RAD function defs
extern unsigned int rad_init(unsigned char* data_ptr);
extern void rad_end();
extern void rad_playback();
extern void rad_set_usercallback(void (*funcptr)(unsigned char, unsigned char));
extern void rad_set_endcallback(void (*funcptr)(void));

unsigned char* rad_load(const char* filename);
void rad_free(unsigned char* modfile);

#endif