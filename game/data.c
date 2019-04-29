// Game Data definitions
// =-------------------=
// Some Data is stored in a globally accessible struct instance
// to allow for easier access and so as not to duplicate anything
// unnecessarily. 

#include "data.h"

struct global_data_t DATA;

void init_global_data() {
    //DATA.draw_function = 0;
    //DATA.update_function = 0;
    DATA.event_head = 0;
    DATA.music_is_playing = 0;
    //DATA.pak_game = 0;

    //DATA.has_arrows = 0;
    //DATA.has_bombs = 0;
    //DATA.num_arrows = 0;
    //DATA.num_balls = 0;
    //DATA.num_bombs = 0;
    //DATA.num_keys = 0;

    DATA.gameover = 0;
}
