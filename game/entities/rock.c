#include "rock.h"
#include "../../src/modex.h"
#include "../s_game.h"
#include "ball.h"
#include "../util.h"
#include "../data.h"

void rock_init(struct rock_t* this, PAKCHUNK* spr_rock, int x, int y) {    
    this->x = x;
    this->y = y;
    this->spr_rock = spr_rock;    
    this->alive = 1;
}

void rock_update(struct rock_t* this, struct ball_t* balls) {    
    int i;
    if(this->alive == 0) {
        return;
    }

    for(i = 0; i < MAX_BALLS; i++) {
        if(balls[i].alive) {
            if(AABB_Test(this->x, this->y, 16, 16, balls[i].x, balls[i].y, balls[i].sw, balls[i].sh)) {
                ball_collide(&balls[i], this->x, this->y, 16, 16);

                if(balls[i].type == BALLTYPE_BOMB) {
                    spawn_effect(this->x, this->y-16, EFFECT_BIGEXPLOSION);
                    balls[i].alive = 0;
                    this->alive = 0;
                    DATA.snd_bomb = 1;
                    DATA.num_bombs--;
                }
            }
        }
    }
}

void rock_draw(struct rock_t* this, unsigned char* buffer) {
    if(!this->alive) {
        return;
    }
        
    modex_blitsprite_buffer_trans(this->x, this->y, 16, 16, this->spr_rock->base, buffer);    
}