// Chest Entity
// =---------=

#ifndef CHEST_H
#define CHEST_H

#include "..\..\src\ipack.h"
#include "ball.h"
#include "../util.h"

struct chest_t {
    int x, y;
    int alive;
    int state; 
    int mcguffin; // Which mcguffin is this?
    PAKCHUNK *spr_chest, *spr_chest_open;
};

void chest_init(struct chest_t* this, PAKCHUNK* chest_sprites, int x, int y, int state);
void chest_update(struct chest_t* this, struct ball_t* balls);
void chest_draw(struct chest_t* this, unsigned char* buffer);

#endif