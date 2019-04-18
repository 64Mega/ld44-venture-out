// DOS Mem-alloc functions
// For doing DMA transfers from memory to the sound-card, we need the
// memory to live within a 20-bit address-space (First 1MB of memory)
// =----------------------------------------------------------------=

#ifndef DOSMEM_H
#define DOSMEM_H

void* dmalloc(int size, short int* selector);
void  dfree(short int selector);

// Implementation in dosmem.asm
void _dos_malloc(short int para, short int far* seg, short int far *selector);
void _dos_free(short int selector);

#endif