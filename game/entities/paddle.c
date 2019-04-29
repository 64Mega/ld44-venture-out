#include "paddle.h"
#include "../../src/kbd.h"
#include "../../src/modex.h"
#include "../s_game.h"
#include "../../src/deblog.h"

void paddle_init(struct paddle_t* p, PAKFILE* pak) {
    if(pak) {
        p->spr_paddle = pak_getchunk(pak, "paddle/normal");
    }

    p->x = 144;
    p->y = 220;
    p->hcount = 0;
    p->speed = 0;
    p->maxspeed = 300;
}

void paddle_update(struct paddle_t* this) {
    static int lastdirection = 0;
    static int kp_space = 0;
    //p->x -= KBD_KeyDown(KBD_LEFTARROW) * p->speed;
    //p->x += KBD_KeyDown(KBD_RIGHTARROW) * p->speed;
    int direction = KBD_KeyDown(KBD_RIGHTARROW) - KBD_KeyDown(KBD_LEFTARROW);
    int hspeed = 0;

    if(this->speed < this->maxspeed && direction != 0) {
        this->speed += 10;
        if(this->speed > this->maxspeed) {
            this->speed = this->maxspeed;

        }
    } else
    if(direction == 0) {
        if(this->speed > 0) {
            this->speed = this->speed - (this->speed >> 4);
            if(this->speed <= 20) { this->speed = 0; }
        }
    }

    this->hcount += this->speed;
    while(this->hcount > 100) {
        this->hcount -= 100;
        hspeed++;
    }

    if(direction) { this->x += hspeed * direction; }
    else if(lastdirection) { this->x += hspeed * lastdirection; }

    if(this->x < 0) { this->x = 0; }
    if(this->x > 287) { this->x = 287; }

    if(direction != 0) {
        lastdirection = direction;
    }

    // Check if space is pressed
    if(KBD_KeyDown(KBD_SPACE)) {
        if(kp_space == 0) {
            spawn_ball(this->x+12, this->y-8);
            deblog("Speed at this moment: %d\n", this->speed);
        }
        kp_space = 1;
    } else { kp_space = 0; }
}

void paddle_draw(struct paddle_t* this, unsigned char* buffer) {
    modex_blitsprite_buffer_trans(this->x, this->y, 32, 8, this->spr_paddle.base, buffer);    
}