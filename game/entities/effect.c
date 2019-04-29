#include "effect.h"
#include "../../src/kbd.h"
#include "../../src/modex.h"
#include "../s_game.h"
#include "../../src/deblog.h"

void effect_init(struct effect_t* this, int x, int y, int w, int h, int speed, PAKCHUNK* framelist, int num_frames) {
    this->x = x;
    this->y = y;
    this->w = w;
    this->h = h;
    this->dx = 0;
    this->dy = 0;
    this->lifetime = 0;
    this->speed = speed;
    this->framelist = framelist;
    this->num_frames = num_frames;
    this->fcount = 0;
    this->frame = 0;
    this->alive = 1;
}

void effect_update(struct effect_t* this) {    
    int framedelta = 0;    

    this->x += this->dx;
    this->y += this->dy;

    this->fcount += this->speed;
    while(this->fcount > 100) {
        this->fcount -= 100;
        framedelta++;
    }

    this->frame += framedelta;
    if(this->frame >= this->num_frames) {
        if(this->lifetime <= 0) { this->alive = 0; }
        this->frame = 0;
    }

    if(this->lifetime > 0) {
        this->lifetime--;
    }
}

void effect_draw(struct effect_t* this, unsigned char* buffer) {
    if(this->framelist)  {
        modex_blitsprite_buffer_trans(this->x, this->y, this->w, this->h, this->framelist[this->frame].base, buffer);    
    }
}