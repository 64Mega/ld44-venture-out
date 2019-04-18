#include "dosmem.h"

void* dmalloc(int size, short int* selector) {
    short int seg;

    _dos_malloc((size >> 4) + 1, &seg, selector);
    return((char *)(seg << 4));
}

void dfree(short int selector) {
    _dos_free(selector);
}