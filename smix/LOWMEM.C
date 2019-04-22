/*      SMIXW is Copyright 1995 by Ethan Brodsky.  All rights reserved.     */
/* �� lowmem.c ������������������������������������������������������������ */

/* ������������������������������������������������������������������������ */

void dos_memalloc(short int para, short int far *seg, short int far *sel);
#pragma  aux dos_memalloc = \
  "push  ecx"               \
  "push  edx"               \
  "mov   ax, 0100h"         \
  "int   31h"               \
  "pop   ebx"               \
  "mov   [ebx], dx"         \
  "pop   ebx"               \
  "mov   [ebx], ax"         \
  parm   [bx] [ecx] [edx]   \
  modify [ax ebx ecx edx];

/* ������������������������������������������������������������������������ */

void dos_memfree(short int sel);
#pragma  aux dos_memfree =  \
  "mov   ax, 0101h"         \
  "int   31h"               \
  parm   [dx]               \
  modify [ax dx];

/* ������������������������������������������������������������������������ */

void *low_malloc(int size, short int far *sel)
  {
    short int seg;

    dos_memalloc((size >> 4) + 1, &seg, sel);
    return((char *)(seg << 4));
  }

/* ������������������������������������������������������������������������ */

void low_free(short int sel)
  {
    dos_memfree(sel);
  }

/* ������������������������������������������������������������������������ */
