#ifndef IPACK_H
#define IPACK_H

// Functions for reading an IPACK PAK file

#define PACKVERSION 0x0001

#define TYPE_PALETTE 0xFF
#define TYPE_SPRITE 0xFE

#define HIGHBYTE(v16) ((v16 >> 8) & 0xFF)
#define LOWBYTE(v16) ((v16 & 0xFF))

#define HIGHWORD(v32) ((v32 >> 16) & 0xFFFF)
#define LOWWORD(v32) ((v32 & 0xFFFF))

#include <stdio.h>

typedef struct pak_t {    
    unsigned int numfiles;
    unsigned char** names;
    unsigned int* offsets;  
    unsigned int* lengths;

    unsigned int header_end;      
    unsigned char* datablock;    
};

typedef struct pakchunk_t {
    unsigned char* base;
    unsigned int length;
};

typedef struct pak_t PAKFILE;
typedef struct pakchunk_t PAKCHUNK;

PAKFILE* pak_load(const char* fname);
void pak_unload(PAKFILE* fp);

PAKCHUNK pak_getchunk(PAKFILE* pak, const char* name);

#endif