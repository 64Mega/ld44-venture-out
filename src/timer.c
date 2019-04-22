// Timer implementation
// =------------------=

#include "timer.h"
#include <dos.h>
#include <stdlib.h>
#include <conio.h>
#include <i86.h>

#define TIMER_INT   0x08
#define PIT_FREQ    0x1234DD

// The callback that the timer will call every tick
static fnptr_timer_callback registered_callback = 0;

// Storage for a pointer to the BIOS PIT handler
static void (interrupt *fnptr_bios_timer)() = 0;

// Internal tracking of ticks and a counter
static unsigned long ticks = 0;
static unsigned long divider = 0;

static void interrupt timer_handler();

void timer_register(fnptr_timer_callback callback, unsigned int freq) {    
    ticks = 0;    
    divider = PIT_FREQ / freq;
    registered_callback = callback;

    _disable();

    fnptr_bios_timer = _dos_getvect(TIMER_INT);
    _dos_setvect(TIMER_INT, timer_handler);

    _enable();

    // Set Timer 0 frequency
    outp(0x43, 0x34);
    outp(0x40, divider % 256);
    outp(0x40, divider / 256);
}

void timer_unregister() {
    _disable();
    registered_callback = 0;
    _dos_setvect(TIMER_INT, fnptr_bios_timer);
    _enable();

    // Restore frequency
    outp(0x43, 0x34);
    outp(0x40, 0x00);
    outp(0x40, 0x00);
}

float timer_tick_ms() {
    return 1.0 / (divider / 1000);
}

static void interrupt timer_handler() {
    if(registered_callback) {
        registered_callback();
    }

    ticks += divider;
    if(ticks >= 0x10000) {
        ticks -= 0x10000;
        _chain_intr(fnptr_bios_timer);
    } else {
        outp(0x20, 0x20); // PIT ACK
    }    
}

