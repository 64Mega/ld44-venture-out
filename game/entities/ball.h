// Ball Entity
// =---------=

#ifndef BALL_H
#define BALL_H

#include "..\..\src\ipack.h"
#include "paddle.h"
#include "bush.h"
#include "rock.h"

enum BallTypes {
    BALLTYPE_NORMAL = 0,
    BALLTYPE_ARROW,
    BALLTYPE_BOMB,
    BALLTYPE_KEY
};

struct ball_t {
    int x, y;
    int dx, dy;
    int sw, sh;
    int speed;
    int hcount, vcount;
    int alive;
    int type; // Normal? Key? Bomb? Arrow?
    PAKCHUNK spr_ball[4];
};

void ball_init(struct ball_t* b, PAKFILE* pak, int x, int y);
void ball_update(struct ball_t* b, struct paddle_t* p, struct bush_t* bushes, struct rock_t* rocks);
void ball_draw(struct ball_t* b, unsigned char* buffer);
void ball_collide(struct ball_t* ball, int x, int y, int w, int h);

#endif