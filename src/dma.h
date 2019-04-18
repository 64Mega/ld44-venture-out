// DMA Routines
// =----------=
// DMA provides a way to copy data to and from IO ports without incurring
// any CPU overhead. It does this with a separate control chip (The DMA 
// controller) and a page chipset (For memory access)
// It can only write at most 64KB of memory at a time, and cannot 
// go outside 64K boundaries.

#ifndef DMA_H
#define DMA_H


void dma_copy(unsigned int destport, unsigned char* data, unsigned int length);

// Hook this to provide extra behavior on transfer completion (E.G: Setting up the next block)
extern (void dma_callback*)(unsigned int length_remaining);
void dma_init(int dmabase, int irqbase);

#endif