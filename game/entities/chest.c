#include <stdlib.h>
#include "chest.h"
#include "../../src/modex.h"
#include "../data.h"
#include "../s_game.h"

void chest_init(struct chest_t* this, PAKCHUNK* chest_sprites, int x, int y, int state) {    
    this->x = x;
    this->y = y;
    this->state = state;
    this->spr_chest = &chest_sprites[0];
    this->spr_chest_open = &chest_sprites[1];
    this->mcguffin = 0;
    this->alive = 1;
}

void chest_update(struct chest_t* this, struct ball_t* balls) {    
    unsigned int i, d;
    if(this->alive == 0) { return; }
    if(this->state == 1) { return; }

    if(DATA.has_mcguffin[this->mcguffin]) {
        this->state = 1;
        return;
    }    

    for(i = 0; i < MAX_BALLS; i++) {
        if(balls[i].alive) {
            if(AABB_Test(this->x, this->y, 16, 16, balls[i].x, balls[i].y, balls[i].sw, balls[i].sh)) {
                ball_collide(&balls[i], this->x, this->y, 16, 16);

                if(balls[i].type == BALLTYPE_KEY) {
                    this->state = 1;                
                    spawn_effect(this->x, this->y-28, EFFECT_BIGEXPLOSION);
                    balls[i].alive = 0;
                    DATA.has_mcguffin[this->mcguffin] = 1;
                    DATA.num_keys--;
                }
            }
        }
    }
}

void chest_draw(struct chest_t* this, unsigned char* buffer) {
    if(!this->alive) {
        return;
    }
    
    if(this->state == 0) {
        modex_blitsprite_buffer_trans(this->x, this->y, 16, 16, this->spr_chest->base, buffer);
    } else {
        modex_blitsprite_buffer_trans(this->x, this->y, 16, 16, this->spr_chest_open->base, buffer);
    }
}