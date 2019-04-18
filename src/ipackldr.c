// IPack Loader implementation

#include "ipack.h"

#include <malloc.h>
#include <string.h>

PAKFILE* pak_load(const char* fname) {
    PAKFILE* fp = (PAKFILE*) malloc(sizeof(PAKFILE));
    FILE *ffp = fopen(fname, "rb");

    unsigned char ident[5];
    unsigned int vernum;
    unsigned int numfiles;
    unsigned int headerLength;
    unsigned int dataBlockSize = 0;

    unsigned int i;

    unsigned int headerBytes = 0;
    unsigned char chunktype;
    unsigned char chunklabel_len;
    unsigned char* chunklabel;
    unsigned int chunk_offset;
    unsigned int chunk_length;

    fread(ident, 1, 4, ffp);
    ident[4] = 0;
    if(strcmp(ident, "PACK") != 0) {
        printf("%s is an invalid PAK file! (Incorrect identifier, got %s instead of PACK)\n", ident);
        free(fp);
        return 0;
    }

    fread(&vernum, 1, sizeof(vernum), ffp);
    fread(&numfiles, 1, sizeof(numfiles), ffp);
    
    headerLength = (numfiles * 64) + 12;

    printf("PAK FILE \"%s\" INFO:\n", fname);
    printf("\tVerNumber: %d\n", vernum);
    printf("\tNumber of File Entries: %d\n", numfiles);

    fp->names = (unsigned char**) malloc(sizeof(unsigned char*) * numfiles);
    fp->lengths = (unsigned int*) malloc(sizeof(unsigned int) * numfiles);
    fp->offsets = (unsigned int*) malloc(sizeof(unsigned int) * numfiles);
    fp->datablock = 0;

    fp->header_end = headerLength;
    fp->numfiles = numfiles;

    for(i = 0; i < numfiles; i++) {
        headerBytes = 0;
        headerBytes += fread(&chunktype, 1, 1, ffp);
        headerBytes += fread(&chunklabel_len, 1, sizeof(chunklabel_len), ffp);
        fp->names[i] = (unsigned char*) malloc(chunklabel_len+1);
        fp->names[i][chunklabel_len] = 0;
        headerBytes += fread(fp->names[i], 1, chunklabel_len, ffp);
        headerBytes += fread(&fp->offsets[i], 1, sizeof(fp->offsets[i]), ffp);        
        headerBytes += fread(&fp->lengths[i], 1, sizeof(fp->lengths[i]), ffp);
                
        dataBlockSize += fp->lengths[i];

        while(headerBytes < 64) {
            headerBytes++;
            fgetc(ffp);    
        }
    }

    // Allocate data block
    fp->datablock = (unsigned char*) malloc(dataBlockSize);

    fread(fp->datablock, 1, dataBlockSize, ffp);

    return fp;
}

void pak_unload(PAKFILE* fp) {
    int i;
    if(fp) {
        for(i = 0; i < fp->numfiles; i++) {
            free(fp->names[i]);
        }

        free(fp->names);
        free(fp->lengths);
        free(fp->offsets);
        free(fp->datablock);

        free(fp);
    }
}

PAKCHUNK pak_getchunk(PAKFILE* pak, const char* name) {
    PAKCHUNK chunk = {0, 0};
    int i;
    for(i = 0; i < pak->numfiles; i++) {
        if(strcmp(pak->names[i], name) == 0) {
            chunk.base = pak->datablock + pak->offsets[i];
            chunk.length = pak->lengths[i];
            break;
        }
    }

    return chunk;
}
