// Standard "Hello World" in C

#include <stdio.h>
#include <conio.h>

extern unsigned char foo();

void main(void) {
    printf("Hello World!\n");
    printf("Press a key to exit.\n");
    printf("foo() == %d\n", foo());
    getch();
}