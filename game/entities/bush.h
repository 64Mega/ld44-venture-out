// Bush Entity
// =---------=

#ifndef BUSH_H
#define BUSH_H

#include "..\..\src\ipack.h"
#include "ball.h"
#include "../util.h"

struct bush_t {
    int x, y;
    int alive;
    int state; // 0 or 1: 1 is a stump and doesn't collide.    
    PAKCHUNK *spr_bush, *spr_stump;
};

void bush_init(struct bush_t* this, PAKCHUNK* spr_bush, PAKCHUNK* spr_stump, int x, int y, int state);
void bush_update(struct bush_t* this, struct ball_t* balls);
void bush_draw(struct bush_t* this, unsigned char* buffer);

#endif