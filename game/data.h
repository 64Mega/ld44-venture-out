// Global Data Struct Definition -> See data.c

#ifndef DATA_H
#define DATA_H

#define MAX_EVENTS 40

#define PALETTE_LENGTH 768

#include "../src/ipack.h"

struct global_data_t {
    // Global asset file
    PAKFILE* pak_game;
    PAKCHUNK pk_palette;

    // Current Palette
    unsigned char active_palette[PALETTE_LENGTH];

    // Buffers
    unsigned char* screen_buffer;
    unsigned char* hud_buffer;

    // Font array
    PAKCHUNK pk_font[95];

    // Update and Draw callbacks
    void (*update_function)(void);
    void (*draw_function)(unsigned char*);
    void (*latedraw_function)(unsigned char*);

    // Simple 'event' flags: These can trigger certain system events (E.G: Play a sound)
    // They work as simple stack. The max number of events is defined above, and the
    // current value of 'head' is the 'top' of the stack. The main loop must process
    // and pop these from top-to-bottom.
    unsigned char event_head;
    unsigned char event_commands[MAX_EVENTS]; // Contains the Command
    unsigned char event_data[MAX_EVENTS]; // Contains parameters for the Command    

    // Certain engine flags
    unsigned char music_is_playing;      

    // Game flags
    unsigned char has_arrows;
    unsigned char has_bombs;
    unsigned char num_keys;
    unsigned char num_bombs;
    unsigned char num_arrows;
    unsigned char num_balls;  

    unsigned char has_mcguffin[2];

    unsigned char gameover;
    unsigned char gamewin;
    unsigned char gameexit;

    // Sounds
    unsigned char snd_ball_bounce;    
    unsigned char snd_ball_die;    
    unsigned char snd_bomb;
    unsigned char snd_purchase;
    unsigned char snd_denied;
    unsigned char snd_getball;
};

void init_global_data();

extern struct global_data_t DATA;

#endif

