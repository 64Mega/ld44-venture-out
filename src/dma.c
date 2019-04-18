#include "dma.h"

// Configurable ports it is. Probably wisest in the long run.

static int dma_maskport;
static int dma_clrptrport;
static int dma_modeport;
static int dma_addrport;
static int dma_countport;
static int dma_pageport;

static char irq_startmask;
static char irq_stopmask;
static char irq_intvector;

static int pic_rotateport;
static int pic_maskport;

static char dma_startmask;
static char dma_stopmask;
static char dma_mode;
static int  dma_length;

static void (interrupt far *old_dma_interrupt)(void);

// I'll have to figure out how much I need to copy versus how much I can
// get away with leaving out.
// Also I need to generalize this code. SMIX is a bit of a mess.

void install_dma_handler(void);
void uninstall_dma_handler(void);

void dma_init(int dmabase, int irqbase) {
    // Let me be cheeky.
    irq_intvector = irqbase < 8 ? 0x08 + irqbase : 0x70 + irqbase - 8;    
    pic_rotateport = irqbase < 8 ? 0x20 : 0xA0;
    pic_maskport = irqbase < 8 ? 0x21 : 0xA1;

    irq_stopmask = 1 << (irqbase % 8);
    irq_startmask = ~irq_stopmask;

    // We'll be working in 8-bit samples for ease-of-use
    // so we don't need to worry about 16-bit DMA transfers

    dma_maskport = 0x0A;
    dma_clrptrport = 0x0C;
    dma_modeport = 0x0B;
    dma_addrport = 0x00 + 2*dmabase;
    dma_countport = 0x01 + 2*dmabase; // Now I understand the GPE docs, it's a word offset

    switch(dmabase) {
        case 0: dma_pageport = 0x87; break;
        case 1: dma_pageport = 0x83; break;
        case 2: dma_pageport = 0x81; break;
        case 3: dma_pageport = 0x82; break;
    }

    dma_stopmask = dmabase + 0x04;
    dma_startmask = dmabase + 0x00;

    // Trying to assume auto-init capability
    dma_mode = dmabase + 0x58;

    install_dma_handler();
}

void dma_close() {    
    uninstall_dma_handler();
}
