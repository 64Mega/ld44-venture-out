// Standard "Hello World" in C

#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <string.h>
#include "smix\SMIX.H"
#include "src\modex.h"
#include "src\sound.h"
#include "src\timer.h"
#include "src\kbd.h"
#include "src\ipack.h"
#include "src\pal.h"
#include <stdarg.h>

#include "game\data.h"
#include "game\s_game.h"
#include "game\text.h"

#define DEBUG_ENABLED

unsigned char* VGA_START = (unsigned char*) 0xA0000;

// Simple debug logger
FILE* logfile = 0;

void deblog(const char* msg, ...) {
#ifdef DEBUG_ENABLED
    va_list args;
    if(!logfile) { return; }

    va_start(args, msg);
    vfprintf(logfile, msg, args);
    va_end(args);
    fflush(logfile);
#endif
}


volatile unsigned int ticks = 0;
volatile unsigned int dticks = 0;
volatile unsigned int mticks = 0;

void timer_tick(void) {
    ticks++;   
    dticks++;
    mticks++;
    if(DATA.music_is_playing) {
        rad_playback();
    }
}

void init_font(void) {
    // Time for shenanigans to save time. I'm not writing out the load-line for each font file by hand...
    unsigned int i;
    char namebuffer[16];
    for(i = 0; i < 95; i++) {
        memcpy(namebuffer, 0x00, 16);
        sprintf(namebuffer, "font/%d", i);
        DATA.pk_font[i] = pak_getchunk(DATA.pak_game, namebuffer);
    }
    deblog("Font initialized!\n");
}

enum GameStates {
    GAMESTATE_LOGOS = 0,
    GAMESTATE_TITLE = 1,
    GAMESTATE_GAME = 2,
    GAMESTATE_GAMEOVER = 3,
    GAMESTATE_GAMEWIN = 4,
    GAMESTATE_GAMEHELP = 5
};

void main(void) {
    unsigned char* buffer = (unsigned char*) malloc(320*240);
    unsigned char* bufferhud = (unsigned char*) malloc(0xFFFF);
    unsigned int myticks = 0, last_mus_tick = 0, pal_ticks = 0;
    unsigned int game_state = 0;
    unsigned int i;
    unsigned char* testmod = 0;
    unsigned int endflags = 0;
    unsigned int substate = 0;
    unsigned int subcount = 0;
    int sx = 160, sy = 200, dx = 1, dy = 0;
    unsigned int active = 0;
    unsigned char* music;
    PAKCHUNK spr_hudbar;

    SOUND* snd_ball_bounce;    
    SOUND* snd_ball_die;    
    SOUND* snd_bomb;
    SOUND* snd_purchase;
    SOUND* snd_denied;
    SOUND* snd_getball;
    
    // Open logfile
    #ifdef DEBUG_ENABLED
    logfile = fopen("debug.txt", "w");
    #endif

    deblog("Beginning engine setup\n");
        
    memset(buffer, 0x0e, 0x4B00<<2);
        
    memset(DATA.active_palette, 0x00, PALETTE_LENGTH);

    // Load a RAD file
    music = rad_load("LEV1.RAD");
    if(!music) {
        printf("Error loading LEV1.RAD!\n");
        getch();
    }

    rad_init(music);

    // Load a PAK
    deblog("Loading game pak\n");
    DATA.pak_game = pak_load("break.pak");
    DATA.pk_palette = pak_getchunk(DATA.pak_game, "PALETTE");

    if(!DATA.pk_palette.base) {
        deblog("No palette found or read error!\n");        
    } else {
        //deblog("Copying PAK palette to active palette...\n");
        //memcpy(DATA.active_palette, DATA.pk_palette.base, PALETTE_LENGTH);
    }

    deblog("Entering Mode X.\n");
    modex_init();                    

    modex_set_palette(DATA.active_palette);

    ticks = myticks = 0;

    // Set up keyboard hooks
    deblog("Installing keyboard hooks\n");
    kbd_install();

    // Register our timer, run it at 100 ticks per second
    deblog("Installing timer hooks\n");
    timer_register(timer_tick, 50);
        
    // Set our initial visible/active VRAM pages
    modex_set_active_page(0);
    modex_set_visible_page(0);

    //rad_set_endcallback(song_end_event);

    // Set initial callbacks
    deblog("Setting game callbacks\n");
    DATA.draw_function = game_draw;
    DATA.update_function = game_update;    

    spr_hudbar = pak_getchunk(DATA.pak_game, "hud/bar");

    // Load Font Chunks
    init_font();

    // Assign buffers
    DATA.hud_buffer = bufferhud;
    DATA.screen_buffer = buffer;

    //game_state = GAMESTATE_GAME;
    //substate = 3;

    rad_set_volume(50);    

    // Try initializing SMIX's sound system
    init_sb(0x220, 7, 1, 5);
    init_mixing();

    open_sound_resource_file("sound.bnk");

    set_sound_volume(255);
    
    load_sound(&snd_ball_bounce, "BOUNCE");
    load_sound(&snd_ball_die, "BALLDIE");
    load_sound(&snd_bomb, "BOMB");
    load_sound(&snd_purchase, "BUY");
    load_sound(&snd_denied, "NO");
    load_sound(&snd_getball, "GETBALL");

    deblog("Entering Main Loop!\n");
    while(1) {
        if(KBD_KeyDown(KBD_ESCAPE)) {
            break;
        }                         

        if(myticks < dticks) {
            if(dticks - myticks > 1) {
                deblog("We're lagging by %d ticks!\n", dticks - myticks);
            }
        }

        if(game_state == GAMESTATE_GAME) {
            DATA.music_is_playing = 1;
        } else {
            DATA.music_is_playing = 0;
        }

        if(DATA.gameover) {
            DATA.gameover = 0;
            substate = 0;
            init_global_data();
            game_state = GAMESTATE_GAMEOVER;
        }

        if(DATA.gamewin) {
            DATA.gamewin = 0;
            substate = 0;
            init_global_data();
            game_state = GAMESTATE_GAMEWIN;
        }
       
        while(myticks != dticks) {
            myticks++;                      
            // Update Tick            
            //if(myticks % 4 ==  0) {
            //    
            //}        
            if(game_state == GAMESTATE_GAME) {
                if(substate == 3) {
                    if(modex_palette_fadein(DATA.active_palette, DATA.pk_palette.base, 1)) {
                        substate = 0;
                    }
                } else {
                    if(DATA.update_function) {                        
                        DATA.update_function();                                                
                    }

                    // Do basic sounds
                    if(DATA.snd_ball_bounce) {
                        DATA.snd_ball_bounce = 0;
                        stop_sound(0);
                        start_sound(snd_ball_bounce, 0, 255, 0);
                    }
                    if(DATA.snd_ball_die) {
                        DATA.snd_ball_die = 0;
                        stop_sound(1);
                        start_sound(snd_ball_die, 1, 255, 0);
                    }
                    if(DATA.snd_bomb) {
                        DATA.snd_bomb = 0;
                        stop_sound(2);
                        start_sound(snd_bomb, 2, 255, 0);
                    }       
                    if(DATA.snd_denied) {
                        DATA.snd_denied = 0;
                        stop_sound(3);
                        start_sound(snd_denied, 3, 255, 0);
                    }
                    if(DATA.snd_getball) {
                        DATA.snd_getball = 0;
                        stop_sound(4);
                        start_sound(snd_getball, 4, 255, 0);
                    }
                    if(DATA.snd_purchase) {
                        DATA.snd_purchase = 0;
                        stop_sound(5);
                        start_sound(snd_purchase, 5, 255, 0);
                    }
                }
            } else
            if(game_state == GAMESTATE_LOGOS) {
                if(substate == 0) {
                    if(modex_palette_fadein(DATA.active_palette, DATA.pk_palette.base, 1)) {
                        substate = 1;
                    }
                } else
                if(substate == 1) {
                    subcount++;
                    if(subcount > 100) {
                        subcount = 0;
                        substate = 2;
                    }
                } else
                if(substate == 2) {
                    if(modex_palette_fade(DATA.active_palette, 1)) {                        
                        substate = 3;
                        game_state = GAMESTATE_GAME;
                    }
                } 
            } else
            if(game_state == GAMESTATE_GAMEOVER) {
                if(substate == 0) {
                    //if(modex_palette_fade(DATA.active_palette, 1)) {
                        substate = 2;
                    //}
                 } else
                if(substate == 1) {
                    if(modex_palette_fadein(DATA.active_palette, DATA.pk_palette.base, 1)) {
                        substate = 2;
                    }
                } else
                if(substate == 2) {
                    subcount++;
                    if(subcount > 100) {
                        subcount = 0;
                        substate = 3;
                    }
                } else
                if(substate == 3) {
                    if(modex_palette_fade(DATA.active_palette, 1)) {                        
                        substate = 0;
                        game_state = GAMESTATE_LOGOS;
                    }
                }
            } else
            if(game_state == GAMESTATE_GAMEWIN) {
                if(substate == 0) {                    
                    substate = 2;                    
                 } else
                if(substate == 1) {
                    if(modex_palette_fadein(DATA.active_palette, DATA.pk_palette.base, 1)) {
                        substate = 2;
                    }
                } else
                if(substate == 2) {
                    subcount++;
                    if(subcount > 100) {
                        subcount = 0;
                        // substate = 3;
                    }
                } else
                if(substate == 3) {
                    if(modex_palette_fade(DATA.active_palette, 1)) {                        
                        substate = 0;
                        game_state = GAMESTATE_LOGOS;
                    }
                }
            }
        }

        // Do draws here
        if(game_state == GAMESTATE_GAME) {
            // Clear offscreen buffer
            memset(buffer, 0, 320*240);
            if(DATA.draw_function) {
                DATA.draw_function(buffer);
            }
        } else 
        if(game_state == GAMESTATE_LOGOS) {
            modex_blitsprite_buffer(0, 0, 320, 240, pak_getchunk(DATA.pak_game, "screen/logos").base, buffer);
        } else
        if(game_state == GAMESTATE_GAMEOVER) {
            modex_blitsprite_buffer(0, 0, 320, 240, pak_getchunk(DATA.pak_game, "screen/gameover").base, buffer);
        } else
        if(game_state == GAMESTATE_GAMEWIN) {
            modex_blitsprite_buffer(0, 0, 320, 240, pak_getchunk(DATA.pak_game, "screen/win").base, buffer);
        }

        // Wait for retrace to flip pages
        modex_wait_retrace();
        modex_blitbuffer(buffer, 0, 0x4B00);

        // Flip the pages
        if(active == 0) {
            // Temporarily disabled page flipping: I'm not making optimal use of it,
            // since I'm lazily redrawing the entire screen every frame.
            //modex_set_active_page(1);
            //modex_set_visible_page(0);
            active = 1;
        } else {
            //modex_set_active_page(0);
            //modex_set_visible_page(1);
            active = 0;
        }  
    }        

    deblog("Beginning engine shutdown and cleanup\n");

    free_sound(&snd_ball_bounce);
    free_sound(&snd_ball_die);
    free_sound(&snd_bomb);
    free_sound(&snd_purchase);
    free_sound(&snd_denied);
    free_sound(&snd_getball);

    close_sound_resource_file();
    shutdown_mixing();
    shutdown_sb();

    rad_end();
    rad_free(testmod);

    deblog("RADPLAY stopped\n");

    timer_unregister();
    deblog("Timer restored\n");
    kbd_uninstall();
    deblog("Keyboard Handler restored\n");
    
    modex_restore();
    deblog("Video mode restored\n");

    pak_unload(DATA.pak_game);
    deblog("Game pak unloaded\n");

    // Flush and close debug log
    #ifdef DEBUG_ENABLED
    fflush(logfile);
    fclose(logfile);
    #endif
    
    free(buffer);
    free(bufferhud);
}