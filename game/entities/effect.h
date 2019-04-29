// A One-shot effect - plays until the end of its animation and sets its "dead" flag.
// =--------------------------------------------------------------------------------=

#ifndef EFFECT_H
#define EFFECT_H

#include "../../src/ipack.h"

struct effect_t {
    int x, y;
    int w, h;
    int num_frames;
    int alive;
    int speed;
    int fcount;
    int dy;
    int dx;
    int lifetime;
    int frame;
    PAKCHUNK* framelist;
};

void effect_init(struct effect_t* this, int x, int y, int w, int h, int speed, PAKCHUNK* framelist, int num_frames);
void effect_update(struct effect_t* this);
void effect_draw(struct effect_t* this, unsigned char* buffer);

#endif