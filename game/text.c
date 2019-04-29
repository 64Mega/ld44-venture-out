#include "text.h"
#include "data.h"
#include "../src/modex.h"

void draw_text_buffer(unsigned int x, unsigned int y, unsigned char* str, unsigned char* buffer) {
    unsigned char *p = str;
    do {
        if(x > 319-8) { break; }
        if(y > 239-9) { break; }
        modex_blitsprite_buffer_trans(x, y, 8, 8, DATA.pk_font[(*p)-' '].base, buffer);
        x += 8;
    } while(*p++);
}