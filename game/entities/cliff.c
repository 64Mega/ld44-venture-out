#include "cliff.h"
#include "../../src/modex.h"
#include "../s_game.h"
#include "ball.h"
#include "../util.h"

void cliff_init(struct cliff_t* this, PAKCHUNK* spr_cliffarray, int x, int y, int state) {    
    this->x = x;
    this->y = y;
    this->state = state;
    this->spr_cliff = &spr_cliffarray[0];
    this->spr_cliff_grass = &spr_cliffarray[1];
    this->spr_cliff_bomb = &spr_cliffarray[2];
    this->spr_cliff_door = &spr_cliffarray[3];
    this->alive = 1;
}

void cliff_update(struct cliff_t* this, struct ball_t* balls) {    
    int i;
    if(!this->alive) {
        return;
    }

    for(i = 0; i < MAX_BALLS; i++) {
        if(balls[i].alive == 0) { continue; }
        if(AABB_Test(this->x, this->y, 16, 16, balls[i].x, balls[i].y, 8, 8)) {            
            if(this->state < 3) { // Solid wall
                ball_collide(&balls[i], this->x, this->y, 16, 16);
                if(this->state == 2) { // Bombable wall
                    spawn_effect(this->x, this->y, EFFECT_GRASSBREAK);
                    this->state = 3;
                }
            } else
            if(this->state == 3) {
                // A door
            }
        }
    }
}

void cliff_draw(struct cliff_t* this, unsigned char* buffer) {
    if(!this->alive) {
        return;
    }
        
    switch(this->state) {
        case 0: modex_blitsprite_buffer(this->x, this->y, 16, 16, this->spr_cliff->base, buffer); break;
        case 1: modex_blitsprite_buffer(this->x, this->y, 16, 16, this->spr_cliff_grass->base, buffer); break;
        case 2: modex_blitsprite_buffer(this->x, this->y, 16, 16, this->spr_cliff_bomb->base, buffer); break;
        case 3: modex_blitsprite_buffer(this->x, this->y, 16, 16, this->spr_cliff_door->base, buffer); break;
    }
}