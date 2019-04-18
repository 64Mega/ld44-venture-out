// Timer functions
// =-------------=

#ifndef TIMER_H
#define TIMER_H

// Callback type. Define your callback as void callback(void) { // code here }
typedef void (*fnptr_timer_callback)(void);

// Takes a callback and a frequency.
// The frequency is the number of desired ticks per second. E.G: 60 == 60 ticks per second.
void timer_register(fnptr_timer_callback callback, unsigned int freq);

// Make sure to call this on cleanup.
void timer_unregister();

#endif