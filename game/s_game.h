// Game state

#ifndef S_GAME_H
#define S_GAME_H

enum effect_e {
    EFFECT_NONE         = 0x0000,
    EFFECT_DUSTUP       = 0x0001,
    EFFECT_GRASSBREAK   = 0x0002,
    EFFECT_BIGEXPLOSION = 0x0003,
    EFFECT_BALLPICKUP   = 0x0004
};

void game_update(void);
void game_draw(unsigned char* buffer);
void game_latedraw(unsigned char* buffer);

void spawn_ball(int x, int y);
void spawn_effect(int x, int y, int effect);

#define MAX_BALLS 8
#define MAX_ENTS 64

#endif