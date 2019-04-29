// Rock Entity
// =---------=

#ifndef ROCK_H
#define ROCK_H

#include "..\..\src\ipack.h"
#include "ball.h"

struct rock_t {
    int x, y;
    int alive;    
    PAKCHUNK *spr_rock;
};

void rock_init(struct rock_t* this, PAKCHUNK* spr_rock, int x, int y);
void rock_update(struct rock_t* this, struct ball_t* balls);
void rock_draw(struct rock_t* this, unsigned char* buffer);

#endif