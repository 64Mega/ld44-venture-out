// Palette manipulation functions
// =----------------------------=

#ifndef PAL_H
#define PAL_H

// Some functions are also defined in modex.h/modex.asm (E.G: Palette Set, Fade, Fade-in)
void pal_rotate_right(unsigned char* pal, unsigned int start, unsigned int end);

#endif