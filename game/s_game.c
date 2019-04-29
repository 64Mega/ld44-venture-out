#include <stdio.h>
#include <stdlib.h>
#include "s_game.h"
#include "../src/modex.h"
#include "../src/ipack.h"
#include "data.h"
#include "../src/deblog.h"
#include "../src/kbd.h"
#include "entities/paddle.h"
#include "entities/ball.h"
#include "entities/effect.h"
#include "entities/bush.h"
#include "entities/rock.h"
#include "entities/cliff.h"
#include "entities/chest.h"
#include "../src/pal.h"
#include "text.h"

#include "areas.h"

// Declare these as static to avoid polluting global scope too much
static unsigned int game_initialized = 0;
static unsigned int init_ok = 1;
static struct paddle_t paddle;

static struct ball_t balls[MAX_BALLS];
static struct bush_t bushes[MAX_ENTS];
static struct rock_t rocks[MAX_ENTS];
static struct cliff_t cliffs[MAX_ENTS];
static struct chest_t chests[MAX_ENTS];

static PAKCHUNK bg_grass_top, bg_grass_sides[3];
static PAKCHUNK bg_hud;
static PAKCHUNK bg_hudside;
static PAKCHUNK bg_leftdoor, bg_rightdoor;
static PAKCHUNK spr_itembox;
static PAKCHUNK spr_reticule;
static PAKCHUNK spr_thing1, spr_thing2;

// Misc sprites
static PAKCHUNK spr_ball;
static PAKCHUNK spr_arrowball;
static PAKCHUNK spr_keyball;
static PAKCHUNK spr_bombball;
static PAKCHUNK spr_ballpickup[2];

// Area sprites
static PAKCHUNK spr_rock;
static PAKCHUNK spr_bush;
static PAKCHUNK spr_stump;
static PAKCHUNK spr_cliff[4];
static PAKCHUNK spr_chest[2];

// Shop sprites
static PAKCHUNK spr_shopkeep;
static PAKCHUNK spr_carpet;
static PAKCHUNK spr_shoparrow;
static PAKCHUNK spr_shop_bomb;
static PAKCHUNK spr_shop_key;
static PAKCHUNK spr_shop_arrow;

// Enemy sprites
static PAKCHUNK enm1_left[2];
static PAKCHUNK enm1_right[2];

// Effect lists
static PAKCHUNK effect_dustup[5];
static PAKCHUNK effect_grassbreak[4];
static PAKCHUNK effect_bigexplosion[6];

// Area stuff
static unsigned int area_current = 0;
static unsigned int area_inner = 0;
static unsigned int area_is_inner = 0;
static unsigned int area_clear_right = 0;
static unsigned int area_clear_left = 0;

// Transition states
static unsigned int transition = 0;
static unsigned int tr_counter1 = 0;
static unsigned int tr_counter2 = 0;
static unsigned int switch_to_inner = 0;
static unsigned int switch_to_area = 0;

// Flags
static unsigned char selected_item = 0;
static unsigned char shop_selected = 0;

// Static storage for printed strings
static char str_num_balls[4] = {0,0,0,0};
static char str_num_arrows[4] = {0,0,0,0};
static char str_num_bombs[4] = {0,0,0,0};
static char str_num_keys[4] = {0,0,0,0};

// Effect spawner
#define MAX_EFFECTS 20
static struct effect_t effects[MAX_EFFECTS];

void spawn_effect(int x, int y, int effect) {
    int i;
    for(i = 0; i < MAX_EFFECTS; i++) {
        if(effects[i].alive == 0) {
            switch(effect) {
                case EFFECT_DUSTUP:
                    effect_init(&effects[i], x, y, 8, 8, 50, effect_dustup, 5);
                break;
                case EFFECT_GRASSBREAK:
                    effect_init(&effects[i], x, y, 16, 16, 50, effect_grassbreak, 4);
                break;
                case EFFECT_BIGEXPLOSION:
                    effect_init(&effects[i], x, y, 16, 32, 35, effect_bigexplosion, 6);
                break;
                case EFFECT_BALLPICKUP:
                    effect_init(&effects[i], x, y, 12, 12, 10, spr_ballpickup, 2);
                    effects[i].lifetime = 20;
                    effects[i].dy = -1;
                break;
            }
            break;
        }
    }
}

void spawn_ball(int x, int y) {
    int i = 0;
    int idx = rand()%100;
    int balls_active = 0;
    int aballs_active = 0, bballs_active = 0, kballs_active = 0;
    
    int type = selected_item;    

    for(i = 0; i < MAX_BALLS; i++) {
        if(balls[i].alive && balls[i].type == type) {             
            balls_active++;
        }
    }

    switch(type) {
        case BALLTYPE_NORMAL: if(balls_active > DATA.num_balls-1) { return; } break;
        case BALLTYPE_ARROW: if(balls_active > DATA.num_arrows-1) { return; } break;
        case BALLTYPE_BOMB: if(balls_active > DATA.num_bombs-1) { return; } break;
        case BALLTYPE_KEY: if(balls_active > DATA.num_keys-1) { return; } break;
    }

    for(i = 0; i < MAX_BALLS; i++) {
        // Check for space for another ball to spawn        
        if(balls[i].alive == 0) {
            deblog("Spawning a ball @ %d %d!\n", x, y);
            ball_init(&balls[i], DATA.pak_game, x, y);
            if(idx < 50) { balls[i].dx = -1; }
            else { balls[i].dx = 1; }            
            balls[i].type = type;
            break;
        }
    }
}

void spawn_bush(int x, int y, int state) {
    int i = 0;
    for(i = 0; i < MAX_ENTS; i++) {
        if(bushes[i].alive == 0) {
            deblog("Spawning a bush @ %d %d\n", x, y);
            bush_init(&bushes[i], &spr_bush, &spr_stump, x, y, state);
            break;
        }
    }
}

void spawn_cliff(int x, int y, int state) {
    int i = 0;
    for(i = 0; i < MAX_ENTS; i++) {
        if(cliffs[i].alive == 0) {
            deblog("Spawning a type %d cliff @ %d %d\n", state, x, y);
            cliff_init(&cliffs[i], spr_cliff, x, y, state);
            cliffs[i].alive = 1;
            break;
        }
    }
}

void spawn_rock(int x, int y) {
    int i = 0;
    for(i = 0; i < MAX_ENTS; i++) {
        if(rocks[i].alive == 0) {
            deblog("Spawning a rock @ %d %d\n", x, y);
            rock_init(&rocks[i], &spr_rock, x, y);
            break;
        }
    }
}

void spawn_chest(int x, int y, int mcguffin) {
    int i = 0;
    for(i = 0; i < MAX_ENTS; i++) {
        if(chests[i].alive == 0) {
            deblog("Spawning a chest @ %d %d\n", x, y);
            chest_init(&chests[i], spr_chest, x, y, 0);
            chests[i].mcguffin = mcguffin;
            break;
        }
    }
}

#define AREA_WIDTH 18
#define AREA_HEIGHT 10
void spawn_level(unsigned char *area) {
    int ix, iy;
    unsigned char c;
    if(!area) { return; }
    for(iy = 0; iy < AREA_HEIGHT; iy++) {
        for(ix = 0; ix < AREA_WIDTH; ix++) {
            // Spawn objects
            
            c = area[(AREA_WIDTH*iy)+ix];
            switch(c) {
                case '#': spawn_bush(16 + (ix*16), 20 + (iy*16), 0); break;
                case '.': spawn_bush(16 + (ix*16), 20 + (iy*16), 1); break;
                case '@': spawn_rock(16 + (ix*16), 20 + (iy*16)); break;
                case 'C': spawn_cliff(16 + (ix*16), 20 + (iy*16), 0); break;
                case 'c': spawn_cliff(16 + (ix*16), 20 + (iy*16), 1); break;
                case 'X': spawn_cliff(16 + (ix*16), 20 + (iy*16), 2); break;
                case 'D': spawn_cliff(16 + (ix*16), 20 + (iy*16), 3); break;
                case '?': spawn_chest(16 + (ix*16), 20 + (iy*16), 0); break;
                case '!': spawn_chest(16 + (ix*16), 20 + (iy*16), 1); break;
            }
        }
    }
}

#define STARTING_BALLS 10

void game_init() {
    int i;
    deblog("Initializing Game State!\n");

    // Initialize play-state
    DATA.has_arrows = 0;
    DATA.has_bombs = 0;
    DATA.num_arrows = 0;
    DATA.num_keys = 0;
    DATA.num_balls = STARTING_BALLS;
    DATA.num_bombs = 0;
    DATA.has_mcguffin[0] = 0;
    DATA.has_mcguffin[1] = 0;

    // Load and initialize data

    if(DATA.pak_game) {
        deblog("PAK OK\n");        
        bg_grass_top = pak_getchunk(DATA.pak_game, "bg/grass1");
        bg_grass_sides[0] = pak_getchunk(DATA.pak_game, "bg/grass2");
        bg_grass_sides[1] = pak_getchunk(DATA.pak_game, "bg/grass3");
        bg_grass_sides[2] = pak_getchunk(DATA.pak_game, "bg/grass4");
        
        bg_hud = pak_getchunk(DATA.pak_game, "hud/bar");
        bg_hudside = pak_getchunk(DATA.pak_game, "hud/side");
        bg_leftdoor = pak_getchunk(DATA.pak_game, "hud/doorl");
        bg_rightdoor = pak_getchunk(DATA.pak_game, "hud/doorr");
        spr_itembox = pak_getchunk(DATA.pak_game, "hud/itembox");
        spr_reticule = pak_getchunk(DATA.pak_game, "hud/reticule");
        spr_thing1 = pak_getchunk(DATA.pak_game, "hud/thing1");
        spr_thing2 = pak_getchunk(DATA.pak_game, "hud/thing2");

        // Misc Sprites
        spr_ball = pak_getchunk(DATA.pak_game, "ball/normal");
        spr_arrowball = pak_getchunk(DATA.pak_game, "ball/arrow");
        spr_keyball = pak_getchunk(DATA.pak_game, "ball/key");
        spr_bombball = pak_getchunk(DATA.pak_game, "ball/bomb");
        spr_ballpickup[0] = pak_getchunk(DATA.pak_game, "ball/pickup1");
        spr_ballpickup[1] = pak_getchunk(DATA.pak_game, "ball/pickup2");

        // Shop sprites
        spr_shopkeep = pak_getchunk(DATA.pak_game, "spr/shopkeep");
        spr_carpet = pak_getchunk(DATA.pak_game, "spr/shop/carpet");
        spr_shoparrow = pak_getchunk(DATA.pak_game, "spr/shop/arrow");

        spr_shop_arrow = pak_getchunk(DATA.pak_game, "spr/shop/arrows");
        spr_shop_bomb = pak_getchunk(DATA.pak_game, "spr/shop/bomb");
        spr_shop_key = pak_getchunk(DATA.pak_game, "spr/shop/key");

        // Load effect sprites
        effect_dustup[0] = pak_getchunk(DATA.pak_game, "DUSTUP0");
        effect_dustup[1] = pak_getchunk(DATA.pak_game, "DUSTUP1");
        effect_dustup[2] = pak_getchunk(DATA.pak_game, "DUSTUP2");
        effect_dustup[3] = pak_getchunk(DATA.pak_game, "DUSTUP3");
        effect_dustup[4] = pak_getchunk(DATA.pak_game, "DUSTUP4");

        effect_grassbreak[0] = pak_getchunk(DATA.pak_game, "GRASSBRK0");
        effect_grassbreak[1] = pak_getchunk(DATA.pak_game, "GRASSBRK1");
        effect_grassbreak[2] = pak_getchunk(DATA.pak_game, "GRASSBRK2");
        effect_grassbreak[3] = pak_getchunk(DATA.pak_game, "GRASSBRK3");

        effect_bigexplosion[0] = pak_getchunk(DATA.pak_game, "BIGEXP00");
        effect_bigexplosion[1] = pak_getchunk(DATA.pak_game, "BIGEXP01");
        effect_bigexplosion[2] = pak_getchunk(DATA.pak_game, "BIGEXP02");
        effect_bigexplosion[3] = pak_getchunk(DATA.pak_game, "BIGEXP03");
        effect_bigexplosion[4] = pak_getchunk(DATA.pak_game, "BIGEXP04");
        effect_bigexplosion[5] = pak_getchunk(DATA.pak_game, "BIGEXP05");

        // Load Area sprites
        spr_rock = pak_getchunk(DATA.pak_game, "spr/rock");
        spr_bush = pak_getchunk(DATA.pak_game, "spr/bush");
        spr_stump = pak_getchunk(DATA.pak_game, "spr/stump");
        spr_cliff[0] = pak_getchunk(DATA.pak_game, "spr/cliff1");
        spr_cliff[1] = pak_getchunk(DATA.pak_game, "spr/cliff2");
        spr_cliff[2] = pak_getchunk(DATA.pak_game, "spr/cliffbomb");
        spr_cliff[3] = pak_getchunk(DATA.pak_game, "spr/cliffdoor");
        spr_chest[0] = pak_getchunk(DATA.pak_game, "spr/chest");
        spr_chest[1] = pak_getchunk(DATA.pak_game, "spr/chestopen");

        // Load Enemy sprites
        enm1_left[0] = pak_getchunk(DATA.pak_game, "spr/e1_left_1");
        enm1_left[1] = pak_getchunk(DATA.pak_game, "spr/e1_left_2");
        enm1_right[0] = pak_getchunk(DATA.pak_game, "spr/e1_right_1");
        enm1_right[1] = pak_getchunk(DATA.pak_game, "spr/e1_right_2");
    } else {
        deblog("[ERROR] PAK not loaded?\n");
        init_ok = 0;
    }

    for(i = 0; i < MAX_BALLS; i++) {
        ball_init(&balls[i], DATA.pak_game, 0, 0);
        balls[i].alive = 0;
    }
    for(i = 0; i < MAX_ENTS; i++) {
        bush_init(&bushes[i], &spr_bush, &spr_stump, 0, 0, 0);
        bushes[i].alive = 0;
        cliff_init(&cliffs[i], &spr_cliff, 0, 0, 0);
        cliffs[i].alive = 0;
    }

    paddle_init(&paddle, DATA.pak_game);    

    spawn_level(areas[area_current].ptr);

    deblog("Game State initialized!\n");
    game_initialized = 1;
}

void reset_vars() {
    // Re-Initialize play-state
    DATA.has_arrows = 0;
    DATA.has_bombs = 0;
    DATA.num_arrows = 0;
    DATA.num_keys = 0;
    DATA.num_balls = STARTING_BALLS;
    DATA.num_bombs = 0;
    DATA.has_mcguffin[0] = 0;
    DATA.has_mcguffin[1] = 0;

    selected_item = 0;
    shop_selected = 0;
}

void reset_objects() {
    // Reset all game object arrays
    unsigned int i;
    paddle_init(&paddle, DATA.pak_game);

    for(i = 0; i < MAX_BALLS; i++) {
        ball_init(&balls[i], DATA.pak_game, 0, 0);
        balls[i].alive = 0;
    }
    for(i = 0; i < MAX_ENTS; i++) {
        rock_init(&rocks[i], &spr_rock, 0, 0);
        rocks[i].alive = 0;
        bush_init(&bushes[i], &spr_bush, &spr_stump, 0, 0, 0);
        bushes[i].alive = 0;
        cliff_init(&cliffs[i], spr_cliff, 0, 0, 3);
        cliffs[i].alive = 0;
        chest_init(&chests[i], spr_chest, 0, 0, 0);
        chests[i].alive = 0;
    }
}

void game_update(void) {
    int i;
    static int kp_up = 0;
    static int kp_down = 0;
    static int kp_left = 0;
    static int kp_right = 0;
    static int kp_space = 0;
    static int kp_tab = 0;
    static int ticker = 0;
    area_clear_left = 1;
    area_clear_right = 1;
    area_inner = 0;    

    if(!game_initialized) {
        deblog("[s_game//Update] Game not initialized yet, initializing.\n");
        game_init();
    }

    // Check for game-over condition!
    if(DATA.num_balls == 0) {
        reset_objects();
        reset_vars();
        DATA.gameover = 1;
        area_current = 0;
        for(i = 0; i < MAX_EFFECTS; i++) {
            effects[i].alive = 0;
        }
        spawn_level(areas[area_current].ptr);
        return;
    }

    if(DATA.has_mcguffin[0] && DATA.has_mcguffin[1]) {
        reset_objects();
        reset_vars();
        for(i = 0; i < MAX_EFFECTS; i++) {
            effects[i].alive = 0;
        }
        area_current = 0;
        spawn_level(areas[area_current].ptr);
        DATA.gamewin = 1;
        return;
    }

    // Convert all numbers that need to be printed to strings here,
    // since we only need to change that value when a change happens,
    // and changes can only occur in the update tick.
    sprintf(str_num_balls, "%03d", DATA.num_balls);
    sprintf(str_num_arrows, "%03d", DATA.num_arrows);
    sprintf(str_num_bombs, "%03d", DATA.num_bombs);
    sprintf(str_num_keys, "%03d", DATA.num_keys);

    if(ticker % 4 == 0) {
        if(transition == 0) {
            pal_rotate_right(DATA.active_palette, 233, 240);
            pal_rotate_right(DATA.active_palette, 249, 256);
            pal_rotate_right(DATA.active_palette, 224, 229);
            pal_rotate_right(DATA.active_palette, 229, 233);
            pal_rotate_right(DATA.active_palette, 240, 244);
            pal_rotate_right(DATA.active_palette, 244, 249);
            modex_set_palette(DATA.active_palette);
        }
    }

    ticker++;

    if(transition == 0) {
        if(area_is_inner && areas[area_current].innerShop) { 
            if(KBD_KeyDown(KBD_LEFTARROW)) {
                if(kp_left == 0) {
                    if(shop_selected == 0) { shop_selected = 2; }
                    else shop_selected--;
                }
                kp_left = 1;
            } else { kp_left = 0; }
            if(KBD_KeyDown(KBD_RIGHTARROW)) {
                if(kp_right == 0) {
                    if(shop_selected == 2) { shop_selected = 0; }
                    else shop_selected++;
                }
                kp_right = 1;
            } else { kp_right = 0; }   

            if(KBD_KeyDown(KBD_SPACE)) {
                if(kp_space == 0) {
                    if(shop_selected == 0 && DATA.num_balls > 15) {
                        DATA.num_balls -= 15;
                        DATA.num_bombs += 1;
                        DATA.has_bombs = 1;
                    } else
                    if(shop_selected == 1 && DATA.num_balls > 5) {
                        DATA.num_balls -= 5;
                        DATA.num_keys += 1;                        
                    } else
                    if(shop_selected == 2 && DATA.num_balls > 10) {
                        DATA.num_balls -= 10;
                        DATA.num_arrows += 1;
                        DATA.has_arrows = 1;
                    }
                }
                kp_space = 1;
            } else { kp_space = 0; }
        } else {
            paddle_update(&paddle);
        }
        for(i = 0; i < MAX_BALLS; i++) {
            if(balls[i].alive) { ball_update(&balls[i], &paddle, bushes, rocks); }
        }
        for(i = 0; i < MAX_EFFECTS; i++) {
            if(effects[i].alive) { effect_update(&effects[i]); }
        }
        for(i = 0; i < MAX_ENTS; i++) {
            if(bushes[i].alive) { 
                bush_update(&bushes[i], balls); 
                if(areas[area_current].killAllGrass && bushes[i].state == 0) {
                    area_clear_left = 0;
                    area_clear_right = 0;
                }
            }
            if(rocks[i].alive) { rock_update(&rocks[i], balls); }
            if(chests[i].alive) { chest_update(&chests[i], balls); }
            if(cliffs[i].alive) { 
                cliff_update(&cliffs[i], balls); 
                if(cliffs[i].state == 3) {
                    // Door type
                    area_inner = 1;
                }
            }
        }

        if(area_is_inner) {
            area_clear_left = area_clear_right = 0;
        }

        if(area_clear_left && area_clear_right) {
            if(paddle.x == 0) {
                // Left exit
                switch_to_area = 1;
                transition = 1;
            } else 
            if(paddle.x == 319-32) {
                // Right Exit
                switch_to_area = 2;
                transition = 1;
            }            
        } else {
            if(paddle.x < 16) { paddle.x = 16;}
            if(paddle.x > 319-48) { paddle.x = 319-48; }
        }

        if(KBD_KeyDown(KBD_UPARROW)) {
            if(kp_up == 0 && area_inner) {                
                if(areas[area_current].inner) {
                    switch_to_inner = 1;
                    transition = 1;
                }                
            }            
            kp_up = 1;
        } else { kp_up = 0; }
        if(KBD_KeyDown(KBD_DOWNARROW)) {
            if(kp_down == 0 && area_is_inner) {
                switch_to_inner = 2;
                transition = 1;
            }
            kp_down = 1;
        } else { kp_down = 0; }

        // Item switcher
        if(KBD_KeyDown(KBD_TAB)) {
            if(kp_tab == 0) {
                selected_item++;
                if(selected_item == 1 && (!DATA.has_arrows|| DATA.num_arrows == 0)) { selected_item++; }
                if(selected_item == 2 && (!DATA.has_bombs || DATA.num_bombs == 0)) { selected_item++; }
                if(selected_item == 3 && DATA.num_keys == 0) { selected_item++; }
                if(selected_item > 3) { selected_item = 0; }
            }
            kp_tab = 1;
        } else { kp_tab = 0; }

    } else {
        area_clear_left = 0;
        area_clear_right = 0;
        if(transition == 1) {
            if(modex_palette_fade(DATA.active_palette, 1)) {
                transition = 2;

                // Do any area switches here, or resets
                if(switch_to_inner == 1) {
                    area_is_inner = 1;
                    switch_to_inner = 0;
                    reset_objects();
                    spawn_level(areas[area_current].inner);
                } else
                if(switch_to_inner == 2) {
                    // Switch back to outer area
                    switch_to_inner = 0;
                    area_is_inner = 0;                   
                    reset_objects();
                    spawn_level(areas[area_current].ptr);
                } else
                if(switch_to_area == 1) { // Switch area to left
                    switch_to_area = 0;
                    if(area_current == 0) {
                        area_current = NUM_AREAS-1;
                    } else {
                        area_current--;
                    }                    
                    reset_objects();
                    deblog("Switching to area %d\n", area_current);
                    spawn_level(areas[area_current].ptr);
                } else
                if(switch_to_area == 2) { // Switch area to right
                    switch_to_area = 0;
                    area_current++;
                    if(area_current > NUM_AREAS-1) { area_current = 0; }                    
                    reset_objects();
                    deblog("Switching to area %d\n", area_current);
                    spawn_level(areas[area_current].ptr);
                }

            }
        } else
        if(transition == 2) {
            if(modex_palette_fadein(DATA.active_palette, DATA.pk_palette.base, 1)) {
                transition = 0;
            }
        }
    }
}

static char tbuffer[64];

void game_draw(unsigned char* buffer) {
    int i, j;
    unsigned char c;
    char cs[64];
    cs[63] = 0;

    if(!game_initialized) {
        deblog("[s_game//Draw] Game not initialized yet, initializing.\n");
        game_init();
    }

    if(!init_ok) { return; }    

    if(area_is_inner == 0) {
        for(i = 0; i < 5; i++) {
            for(j = 0; j < 10; j++) {
                modex_blitsprite_buffer(j*32, 20 + (i*32), 32, 32, bg_grass_top.base, buffer);
            }
        }    
        for(j = 0; j < 10; j++) {
            modex_blitsprite_buffer(j*32, 180, 32, 32, bg_grass_sides[0].base, buffer);
        }
    } else {        
        for(j = 0; j < 10; j++) {
            modex_blitsprite_buffer(j*32, 239-32, 32, 32, bg_grass_sides[1].base, buffer);
        }
    }
    
    //modex_blitsprite_buffer_trans(192, 84, 32, 32, bg_grass_top.base, buffer);
    //modex_blitsprite_buffer_trans(192, 85, 32, 32, bg_grass_side.base, buffer);

    paddle_draw(&paddle, buffer);        

    for(i = 0; i < MAX_ENTS; i++) {
        if(bushes[i].alive) {
            bush_draw(&bushes[i], buffer);
        }
        if(rocks[i].alive) {
            rock_draw(&rocks[i], buffer);
        }
        if(cliffs[i].alive) {
            cliff_draw(&cliffs[i], buffer);
        }
        if(chests[i].alive) {
            chest_draw(&chests[i], buffer);
        }
    }

    for(i = 0; i < MAX_BALLS; i++) {
        if(balls[i].alive) { ball_draw(&balls[i], buffer); }
    }    

    for(i = 0; i < MAX_EFFECTS; i++) {
        if(effects[i].alive) { effect_draw(&effects[i], buffer); }
    }    

    // These two 16x16 pixel columns are going to 'border' the sides of the screen. This
    // allows me to cheat a bit when it comes to letting objects scroll off the side or
    // if I need to pan the screen to the left or right, since any overdraw happens
    // "behind" the two columns.
    for(i = 1; i < 15; i++) {
        modex_blitsprite_buffer(0, i*16, 16, 16, bg_hudside.base, buffer);
        modex_blitsprite_buffer(304, i*16, 16, 16, bg_hudside.base, buffer);
    }

    // Draw shop if we're in the shop
    if(area_is_inner && areas[area_current].innerShop) {
        modex_blitsprite_buffer(152, 100, 16, 16, spr_shopkeep.base, buffer);
        modex_blitsprite_buffer(124, 132, 72, 16, spr_carpet.base, buffer);
        modex_blitsprite_buffer(130+(24*shop_selected), 120, 12, 12, spr_shoparrow.base, buffer);
        draw_text_buffer(60, 60, "Buy something, won't you?", buffer);

        modex_blitsprite_buffer_trans(124 + 12 - 6, 132 + 8 - 6, 12, 12, spr_shop_bomb.base, buffer);
        modex_blitsprite_buffer_trans(124 + 24 + 12 - 6, 132 + 8 - 6, 12, 12, spr_shop_key.base, buffer);
        modex_blitsprite_buffer_trans(124 + 48 + 12 - 6, 132 + 8 - 6, 8, 12, spr_shop_arrow.base, buffer);

        draw_text_buffer(124, 152, " 15", buffer);
        draw_text_buffer(124+26, 152, " 5", buffer);
        draw_text_buffer(124+48, 152, " 10", buffer);
        modex_blitsprite_buffer(124, 152, 8, 8, spr_ball.base, buffer);
        modex_blitsprite_buffer(124+26, 152, 8, 8, spr_ball.base, buffer);
        modex_blitsprite_buffer(124+48, 152, 8, 8, spr_ball.base, buffer);
    }

    // Draw HUD bar last, handles any overdraw we get from objects leaving the screen vertically.
    modex_blitsprite_buffer(0, 0, 320, 20, bg_hud.base, buffer);
    draw_text_buffer(4, 6, "VENTURE-OUT!", buffer);

    // Draw number of balls remaining
    modex_blitsprite_buffer(104, 0, 28, 20, spr_itembox.base, buffer);
    modex_blitsprite_buffer(114, 2, 8, 8, spr_ball.base, buffer);    
    draw_text_buffer(106, 10, str_num_balls, buffer);    
    
    // Draw number of arrow balls remaining
    modex_blitsprite_buffer(104+28, 0, 28, 20, spr_itembox.base, buffer);    
    if(DATA.has_arrows) {
        modex_blitsprite_buffer(114+28, 2, 8, 8, spr_arrowball.base, buffer);        
        draw_text_buffer(106+28, 10, str_num_arrows, buffer);    
    }
    
    // Draw number of bomb balls remaining
    modex_blitsprite_buffer(104+56, 0, 28, 20, spr_itembox.base, buffer);
    if(DATA.has_bombs) {
        modex_blitsprite_buffer(114+56, 2, 8, 8, spr_bombball.base, buffer);
        //sprintf(cs, "%03d", DATA.num_bombs);
        draw_text_buffer(106+56, 10, str_num_bombs, buffer);
    }
    
    // Draw number of key balls remaining
    modex_blitsprite_buffer(104+84, 0, 28, 20, spr_itembox.base, buffer);
    modex_blitsprite_buffer(114+84, 2, 8, 8, spr_keyball.base, buffer);
    //sprintf(cs, "%03d", DATA.num_keys);
    draw_text_buffer(106+84, 10, str_num_keys, buffer);    

    // Draw selection reticule
    modex_blitsprite_buffer_trans(104+(28*selected_item), 0, 28, 20, spr_reticule.base, buffer);

    if(area_clear_left) { // Can progress right
        modex_blitsprite_buffer(304, 216, 16, 16, bg_rightdoor.base, buffer);
    }

    if(area_clear_right) { // Can progress right
        modex_blitsprite_buffer(0, 216, 16, 16, bg_leftdoor.base, buffer);
    }

    if(DATA.has_mcguffin[0]) {
        modex_blitsprite_buffer(288, 2, 16, 16, spr_thing1.base, buffer);
    }
    if(DATA.has_mcguffin[1]) {
        modex_blitsprite_buffer(272, 2, 16, 16, spr_thing2.base, buffer);
    }
}

// Used to draw the HUD
//void game_latedraw(unsigned char* buffer) {
//    if(!game_initialized) {
//        deblog("[s_game//LateDraw] Game not initialized yet, initializing.\n");
//        game_init();
//    }
//
//    if(!init_ok) { return; }
//
//    modex_blitsprite_buffer(0, 0, 320, 40, bg_hud.base, buffer);
//    draw_text_buffer(0, 2, "Venture-out!", buffer);   
//}