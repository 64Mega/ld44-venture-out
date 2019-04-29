#include "ball.h"
#include "../../src/modex.h"
#include "../s_game.h"
#include "../util.h"
#include "../data.h"
#include <stdlib.h>

#define BALL_MAX_SPEED 300

void ball_init(struct ball_t* b, PAKFILE* pak, int x, int y) {
    if(pak) {
        b->spr_ball[0] = pak_getchunk(pak, "ball/normal");
        b->spr_ball[1] = pak_getchunk(pak, "ball/arrow");
        b->spr_ball[2] = pak_getchunk(pak, "ball/bomb");
        b->spr_ball[3] = pak_getchunk(pak, "ball/key");
    }

    b->x = x;
    b->y = y;
    b->dx = 1;
    b->dy = -1;
    b->speed = 110;
    b->hcount = 0;
    b->vcount = 0;
    b->sw = 8;
    b->sh = 8;
    b->alive = 1;
    b->type = BALLTYPE_NORMAL;
}

void ball_update(struct ball_t* b, struct paddle_t* p, struct bush_t* bushes, struct rock_t* rocks) {
    unsigned int vspeed = 0, hspeed = 0;
    unsigned int i;
    int vdx, vdy;
    struct bush_t* bush;
    struct rock_t* rock;
    if(!b->alive) {
        return;
    }

    if(b->speed > BALL_MAX_SPEED) {
        b->speed = BALL_MAX_SPEED;
    }

    b->hcount += b->speed;
    b->vcount += b->speed;
    while(b->hcount > 100) { b->hcount -= 100; hspeed++; }
    while(b->vcount > 100) { b->vcount -= 100; vspeed++; }

    b->x += b->dx * hspeed;
    b->y += b->dy * vspeed;

    if(b->x < 16) {
        b->dx = 1;
        b->x = 16;
    }
    if(b->x > 304-b->sw) {
        b->dx = -1;
        b->x = 304-b->sw;
    }
    if(b->y < 20) {
        b->dy = 1;
        b->y = 20;
    }
    //if(b->y > 239-b->sh) {
    //    b->dy = -1;
    //    b->y = 239-b->sh;
    //}

    if(p) {
        if(AABB_Test(b->x, b->y, b->sw, b->sh, p->x, p->y, 32, 8)) {
            // Collision with paddle
            if(b->y > p->y+8) { b->dy = 1; }
            if(b->y < p->y+8) {
                //b->dy = -1;
                //b->y = p->y-b->sh;
                // Spawn dustup effect
                ball_collide(b, p->x, p->y, 32, 32);
                spawn_effect(b->x, b->y, EFFECT_DUSTUP);
                b->speed += 4;
            }                
        }
    }        

    if(b->y >= 239) {
        b->alive = 0;
        spawn_effect(b->x-4, 207, EFFECT_BIGEXPLOSION);
        DATA.snd_ball_die = 1;
        switch(b->type) {
            case BALLTYPE_NORMAL: DATA.num_balls--; break;
            case BALLTYPE_ARROW: DATA.num_arrows--; break;
            case BALLTYPE_BOMB: DATA.num_bombs--; break;
            case BALLTYPE_KEY: DATA.num_keys--; break;
        }        
    }
}

void ball_collide(struct ball_t* ball, int x, int y, int w, int h) {
    // Collide with another object and bounce
    unsigned int hw = w>>1;
    unsigned int hh = h>>1;
    int vdx = abs((x+hw) - (ball->x+4));
    int vdy = abs((y+hh) - (ball->y+4));
    if(vdx > vdy) {
        if(ball->x+4 < x+8) {
            ball->x = x - ball->sw;
            ball->dx = -1;
        } else if(ball->x+4 > x+hw) {
            ball->x = x + w;
            ball->dx = 1;
        }
    } else {
        if(ball->y+4 < y+hw) {
            ball->y = y - ball->sw;
            ball->dy = -1;
        } else if(ball->y+4 > y+hw) {
            ball->y = y + h;
            ball->dy = 1;
        }
    }

    DATA.snd_ball_bounce = 1;

    // Slowly increment speed
    ball->speed += 1;
}

void ball_draw(struct ball_t* b, unsigned char* buffer) {
    if(!b->alive) {
        return;
    }
    modex_blitsprite_buffer_trans(b->x, b->y, b->sw, b->sh, b->spr_ball[b->type].base, buffer);    
}