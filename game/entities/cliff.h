// Cliff Entity
// =---------=

#ifndef CLIFF_H
#define CLIFF_H

#include "..\..\src\ipack.h"
#include "ball.h"

struct cliff_t {
    int x, y;
    int alive; 
    int state; // Is it a solid cliff, connected to grass, a bomb-able cliff or a door?
    PAKCHUNK *spr_cliff;
    PAKCHUNK *spr_cliff_grass;
    PAKCHUNK *spr_cliff_bomb;
    PAKCHUNK *spr_cliff_door;
};

void cliff_init(struct cliff_t* this, PAKCHUNK* spr_cliffarray, int x, int y, int state);
void cliff_update(struct cliff_t* this, struct ball_t* balls);
void cliff_draw(struct cliff_t* this, unsigned char* buffer);

#endif