// Paddle Declarations
// =-----------------=

#ifndef PADDLE_H
#define PADDLE_H

#include "..\..\src\ipack.h"

struct paddle_t {
    int x, y;
    int speed;
    int hcount;
    int maxspeed;
    PAKCHUNK spr_paddle;
};

void paddle_init(struct paddle_t* p, PAKFILE* pak);
void paddle_update(struct paddle_t* this);
void paddle_draw(struct paddle_t* this, unsigned char* buffer);

#endif
