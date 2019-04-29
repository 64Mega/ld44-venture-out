#include "pal.h"

void pal_rotate_right(unsigned char* pal, unsigned int start, unsigned int end) {
    unsigned char last[3];
    int i;
    unsigned char num = (end-start)-1;

    unsigned char* ptr = (pal + (start*3));

    last[0] = ptr[(num*3)+0];
    last[1] = ptr[(num*3)+1];
    last[2] = ptr[(num*3)+2];

    for(i = num; i > 0; i--) {
        ptr[(i*3) + 0] = ptr[((i-1)*3) + 0];
        ptr[(i*3) + 1] = ptr[((i-1)*3) + 1];
        ptr[(i*3) + 2] = ptr[((i-1)*3) + 2];
    }

    ptr[0] = last[0];   
    ptr[1] = last[1];   
    ptr[2] = last[2];   
}