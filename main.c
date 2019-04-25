// Standard "Hello World" in C

#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <string.h>
#include "src\modex.h"
#include "src\sound.h"
#include "src\timer.h"
#include "src\kbd.h"
#include "src\ipack.h"

unsigned char* VGA_START = (unsigned char*) 0xA0000;

unsigned char testimg[] = {
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D,
    0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D, 0x0D
};

unsigned char music = 0;

volatile unsigned int ticks = 0;
volatile unsigned int dticks = 0;
volatile unsigned int mticks = 0;

void timer_tick(void) {
    ticks++;   
    dticks++;
    mticks++;
    if(music) {
        rad_playback();
    }
}

volatile unsigned int endevents = 0;

void song_end_event(void) {
    endevents++;
}

void pal_rotate_right(unsigned char* pal, unsigned int start, unsigned int end) {
    unsigned char last[3];
    int i;
    unsigned char num = (end-start)-1;

    unsigned char* ptr = (pal + (start*3));

    // abcdefg : 7 chars. 0x00->0x07. Start at index 0x06 (num-1)
    //

    last[0] = ptr[(num*3)+0];
    last[1] = ptr[(num*3)+1];
    last[2] = ptr[(num*3)+2];

    for(i = num; i > 0; i--) {
        ptr[(i*3) + 0] = ptr[((i-1)*3) + 0];
        ptr[(i*3) + 1] = ptr[((i-1)*3) + 1];
        ptr[(i*3) + 2] = ptr[((i-1)*3) + 2];
    }

    ptr[0] = last[0];   
    ptr[1] = last[1];   
    ptr[2] = last[2];   
}

unsigned char* active_palette = 0;

PAKFILE* breakpak;
PAKCHUNK spr_topframe;
PAKCHUNK pal_main;

void main(void) {
    unsigned char* buffer = (unsigned char*) malloc(MODEX_BUFFER_SIZE);    
    unsigned int myticks = 0, last_mus_tick = 0;
    unsigned char* pala;
    unsigned char* palb;
    unsigned int fadephase = 0;
    unsigned int i;
    unsigned char kp_p = 0;
    unsigned int rotcount = 0;
    unsigned char* testmod = 0;
    unsigned int endflags = 0;
    int sx = 160, sy = 200, dx = 1, dy = 0;
    unsigned int active = 0;
    memset(buffer, 0x00, 0xFFFF);
    memset(buffer+0xFFFF, 0x00, MODEX_BUFFER_SIZE - 0xFFFF);
    
    active_palette = (unsigned char*) malloc(768);
    pala = (unsigned char*) malloc(768);
    palb = (unsigned char*) malloc(768);
    memset(pala, 0x00, 768);
    memset(palb, 0x00, 768);
    memset(active_palette, 0x00, 768);

    // region of palette hard-codes
        pala[(0x0A * 3)+0] = 63;
        pala[(0x0A * 3)+1] = 0;
        pala[(0x0A * 3)+2] = 0;

        pala[(0x0B * 3)+0] = 0;
        pala[(0x0B * 3)+1] = 63;
        pala[(0x0B * 3)+2] = 0;

        pala[(0x0C * 3)+0] = 0;
        pala[(0x0C * 3)+1] = 0;
        pala[(0x0C * 3)+2] = 63;

        pala[(0x0D * 3)+0] = 63;
        pala[(0x0D * 3)+1] = 32;
        pala[(0x0D * 3)+2] = 63;

        palb[(0x0E * 3)+0] = 63;
        palb[(0x0E * 3)+1] = 63;
        palb[(0x0E * 3)+2] = 0;

        palb[(0x0C * 3)+0] = 0;
        palb[(0x0C * 3)+1] = 63;
        palb[(0x0C * 3)+2] = 63;

        palb[(0x0B * 3)+0] = 63;
        palb[(0x0B * 3)+1] = 63;
        palb[(0x0B * 3)+2] = 63;

        palb[(0x0A * 3)+0] = 23;
        palb[(0x0A * 3)+1] = 63;
        palb[(0x0A * 3)+2] = 63;

    memcpy(active_palette, pala, 768);

    // Load a RAD file
    testmod = rad_load("mus/WILLY1.RAD");
    if(!testmod) {
        printf("Error loading CRYSTAL2.RAD!\n");
        getch();
    }

    rad_init(testmod);

    // Load a PAK
    breakpak = pak_load("break.pak");
    spr_topframe = pak_getchunk(breakpak, "FULLFRAME");
    pal_main = pak_getchunk(breakpak, "PALETTE");

    if(!spr_topframe.base) {
        printf("Couldn't load FRAME1_TOP!\n");
        getch();
    }

    if(!pal_main.base) {
        printf("Couldn't load PALETTE!\n");
        getch();
    } else {
        memcpy(active_palette, pal_main.base, 768);
    }

    modex_init();                    

    modex_set_palette(active_palette);

    ticks = myticks = 0;
    kbd_install();
    timer_register(timer_tick, 50);
        
    modex_set_active_page(0);
    modex_set_visible_page(1);

    rad_set_endcallback(song_end_event);

    while(1) {
        if(KBD_KeyDown(KBD_ESCAPE)) {
            break;
        }        
        
        if(KBD_KeyDown(KBD_P)) {    
            if(kp_p == 0) {
                pal_rotate_right(active_palette, 0x0A, 0x0E);
                modex_set_palette(active_palette);
                if(music == 0) { music = 1; }
                else music = 0;
            } 
            kp_p = 1;
        } else { kp_p = 0; }                
       
        while(myticks != dticks) {
            myticks++;                      
            sx += dx;
            if(sx >= 319-16) {
                sx = 319-16; dx = -1;
            }
            if(sx <= 0) {
                sx = 0; dx = 1;
            }                        

            if(myticks % 4 == 0 && fadephase == 0) {
                // Rotate palette ranges
                pal_rotate_right(active_palette, 233, 240);
                pal_rotate_right(active_palette, 249, 255);
                pal_rotate_right(active_palette, 229, 232);
                //modex_palette_setrange(active_palette+(233*3), 233, 239);
                modex_set_palette(active_palette);
            }

            if(endflags != endevents) {
                endflags = endevents;
                if(fadephase == 0) {
                    fadephase = 1;
                }
            }

            if(fadephase == 0) {
                if(KBD_KeyDown(KBD_F)) {
                    fadephase = 1;
                }
                rotcount++;      
                if(rotcount > 5) {          
                    //pal_rotate_right(active_palette, 0x05, 0x0E+5);
                    //modex_set_palette(active_palette);
                    rotcount = 0;
                }
            } else
            if(fadephase == 1) {
                if(modex_palette_fade(active_palette, 1) == 1) {
                    fadephase = 2;
                }
            } else
            if(fadephase == 2) {
                if(modex_palette_fadein(active_palette, pal_main.base, 1) == 1) {
                    fadephase = 0;
                }
            }            
        }

        // Clear offscreen buffer
        memset(buffer, 0x00, 0xFFFF);
        memset(buffer+0xFFFF, 0x00, MODEX_BUFFER_SIZE - 0xFFFF);

        // Draw to offscreen buffer       1 
        
        //modex_blitsprite_buffer(sx, sy, 16, 16, testimg, buffer);           
        modex_blitsprite_buffer_trans(0, 0, 320, 240, spr_topframe.base, buffer);
        //modex_blitsprite_buffer_trans(sx, sy-32, 16, 16, testimg, buffer);
        modex_blitbuffer(buffer, 0);
        //modex_blitbuffer(buffer+0xFFFF,0xFFFF);

        modex_wait_retrace();
        if(active == 0) {
            modex_set_active_page(1);
            modex_set_visible_page(0);
            active = 1;
        } else {
            modex_set_active_page(0);
            modex_set_visible_page(1);
            active = 0;
        }        
    }        

    // timer_unregister();
    kbd_uninstall();
    
    modex_restore();

    pak_unload(breakpak);

    rad_end();
    rad_free(testmod);
    free(active_palette);
    free(pala);
    free(palb);
    free(buffer);
}