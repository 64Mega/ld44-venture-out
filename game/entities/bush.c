#include <stdlib.h>
#include "bush.h"
#include "../../src/modex.h"
#include "../data.h"
#include "../s_game.h"

void bush_init(struct bush_t* this, PAKCHUNK* spr_bush, PAKCHUNK* spr_stump, int x, int y, int state) {    
    this->x = x;
    this->y = y;
    this->state = state;
    this->spr_bush = spr_bush;
    this->spr_stump = spr_stump;
    this->alive = 1;
}

void bush_update(struct bush_t* this, struct ball_t* balls) {    
    unsigned int i, d;
    if(this->alive == 0) { return; }
    if(this->state == 1) { return; }

    d = rand() % 10000;

    for(i = 0; i < MAX_BALLS; i++) {
        if(balls[i].alive) {
            if(AABB_Test(this->x, this->y, 16, 16, balls[i].x, balls[i].y, balls[i].sw, balls[i].sh)) {
                ball_collide(&balls[i], this->x, this->y, 16, 16);
                this->state = 1;
                spawn_effect(this->x, this->y, EFFECT_GRASSBREAK);

                DATA.snd_ball_bounce = 1;

                // Chance of dropping a ball
                if(d < 5000 && DATA.num_balls < 255) {
                    spawn_effect(this->x+2, this->y+2, EFFECT_BALLPICKUP);                                        
                    DATA.snd_getball = 1;
                    DATA.num_balls++;
                    if(DATA.has_arrows && DATA.num_arrows < 255) { DATA.num_arrows++; }
                    if(DATA.has_bombs && DATA.num_bombs < 255) { DATA.num_bombs++; }
                }
            }
        }
    }
}

void bush_draw(struct bush_t* this, unsigned char* buffer) {
    if(!this->alive) {
        return;
    }
    
    if(this->state == 0) {
        modex_blitsprite_buffer_trans(this->x, this->y, 16, 16, this->spr_bush->base, buffer);
    } else {
        modex_blitsprite_buffer_trans(this->x, this->y, 16, 16, this->spr_stump->base, buffer);
    }
}