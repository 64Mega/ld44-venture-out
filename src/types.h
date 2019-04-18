// Some handy typedefs and macros

#ifndef TYPES_H
#define TYPES_H

typedef unsigned char Byte;
typedef unsigned int Word;
typedef unsigned long DWord;

#define BYTE_L(b) (unsigned char)((b) & 0x00FF)
#define BYTE_H(b) (unsigned char)((b) >> 8)
#define WORD_L(w) (unsigned int)((w) & 0x0000FFFF)
#define WORD_H(w) (unsigned int)((w) >> 16)

#endif