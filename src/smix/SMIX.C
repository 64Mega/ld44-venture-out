/*      SMIXW is Copyright 1995 by Ethan Brodsky.  All rights reserved      */

/* � smix.c v1.30 ��������������������������������������������������������� */

#define TRUE  1
#define FALSE 0

#define ON  1
#define OFF 0

#define BLOCK_LENGTH    512   /* Length of digitized sound output block     */
#define VOICES          8     /* Number of available simultaneous voices    */
#define VOLUMES         64    /* Number of volume levels for sound output   */
#define SAMPLING_RATE   22050 /* Sampling rate for output                   */
#define SHIFT_16_BIT    5     /* Bits to shift left for 16-bit output       */

typedef struct
  {
    signed   char *soundptr;
    unsigned long soundsize;
  } SOUND;

int  init_sb(int baseio, int irq, int dma, int dma16);
void shutdown_sb(void);

void set_sampling_rate(unsigned short rate);

void init_mixing(void);
void shutdown_mixing(void);

int  open_sound_resource_file(char *filename);
void close_sound_resource_file(void);

int  load_sound(SOUND **sound, char *key);
void free_sound(SOUND **sound);

int  start_sound(SOUND *sound, int index, unsigned char volume, int loop);
void stop_sound(int index);
int  sound_playing(int index);

void set_sound_volume(unsigned char new_volume);

volatile long intcount;               /* Current count of sound interrupts  */
volatile int  voicecount;             /* Number of voices currently in use  */

short dspversion;
int   autoinit;
int   sixteenbit;
int   smix_sound;

/* ������������������������������������������������������������������������ */

#include <conio.h>
#include <dos.h>
#include <i86.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lowmem.h"

#define BUFFER_LENGTH BLOCK_LENGTH*2

#define BYTE unsigned char

#define lo(value) (unsigned char)((value) & 0x00FF)
#define hi(value) (unsigned char)((value) >> 8)

#define MAX(a, b) (((a) > (b)) ? (a) : (b))
#define MIN(a, b) (((a) > (b)) ? (b) : (a))

static int resetport;
static int readport;
static int writeport;
static int pollport;
static int ackport;

static int pic_rotateport;
static int pic_maskport;

static int dma_maskport;
static int dma_clrptrport;
static int dma_modeport;
static int dma_addrport;
static int dma_countport;
static int dma_pageport;

static char irq_startmask;
static char irq_stopmask;
static char irq_intvector;

static char dma_startmask;
static char dma_stopmask;
static char dma_mode;
static int  dma_length;

static void (interrupt far *oldintvector)(void);

static int smix_initialized  = FALSE;
static int handler_installed = FALSE;

static int sampling_rate = SAMPLING_RATE;

static void write_dsp(BYTE value)
  {
    while ((inp(writeport) & 0x80));
    outp(writeport, value);
  }

static BYTE read_dsp(void)
  {
    while (!(inp(pollport) & 0x80));
    return(inp(readport));
  }

static int reset_dsp(void)
  {
    int i;

    outp(resetport, 1);
    for (i=0; i < 100; i++)    /* The delay function doesn't work correctly */
      { };
    outp(resetport, 0);

    i = 100;
    while ((i-- > 0) && (read_dsp() != 0xAA));

    return(i > 0);
  }

void install_handler(void);
void uninstall_handler(void);
void smix_exitproc(void);

int init_sb(int baseio, int irq, int dma, int dma16)
  {
   /* Sound card IO ports */
    resetport  = baseio + 0x006;
    readport   = baseio + 0x00A;
    writeport  = baseio + 0x00C;
    pollport   = baseio + 0x00E;

   /* Reset DSP, get version, choose output mode */
    if (!reset_dsp())
      return(FALSE);
    write_dsp(0xE1);  /* Get DSP version number */
    dspversion = read_dsp() << 8;  dspversion += read_dsp();
    autoinit   = (dspversion >= 0x0200);
    sixteenbit = (dspversion >= 0x0400) && (dma16 != 0) && (dma16 > 3);

   /* Compute interrupt controller ports and parameters */
    if (irq < 8)
      { /* PIC1 */
        irq_intvector  = 0x08 + irq;
        pic_rotateport = 0x20;
        pic_maskport   = 0x21;
      }
    else
      { /* PIC2 */
        irq_intvector  = 0x70 + irq-8;
        pic_rotateport = 0xA0;
        pic_maskport   = 0xA1;
      }
    irq_stopmask  = 1 << (irq % 8);
    irq_startmask = ~irq_stopmask;

   /* Compute DMA controller ports and parameters */
    if (sixteenbit)
      { /* Sixteen bit */
        dma_maskport   = 0xD4;
        dma_clrptrport = 0xD8;
        dma_modeport   = 0xD6;
        dma_addrport   = 0xC0 + 4*(dma16-4);
        dma_countport  = 0xC2 + 4*(dma16-4);
        switch (dma16)
          {
            case 5:
              dma_pageport = 0x8B;
              break;
            case 6:
              dma_pageport = 0x89;
              break;
            case 7:
              dma_pageport = 0x8A;
              break;
          }
        dma_stopmask  = dma16-4 + 0x04;  /* 000001xx */
        dma_startmask = dma16-4 + 0x00;  /* 000000xx */
        if (autoinit)
          dma_mode = dma16-4 + 0x58;     /* 010110xx */
        else
          dma_mode = dma16-4 + 0x48;     /* 010010xx */
        ackport = baseio + 0x00F;
      }
    else
      { /* Eight bit */
        dma_maskport   = 0x0A;
        dma_clrptrport = 0x0C;
        dma_modeport   = 0x0B;
        dma_addrport   = 0x00 + 2*dma;
        dma_countport  = 0x01 + 2*dma;
        switch (dma)
          {
            case 0:
              dma_pageport = 0x87;
              break;
            case 1:
              dma_pageport = 0x83;
              break;
            case 2:
              dma_pageport = 0x81;
              break;
            case 3:
              dma_pageport = 0x82;
              break;
          }
        dma_stopmask  = dma + 0x04;      /* 000001xx */
        dma_startmask = dma + 0x00;      /* 000000xx */
        if (autoinit)
          dma_mode    = dma + 0x58;      /* 010110xx */
        else
          dma_mode    = dma + 0x48;      /* 010010xx */
        ackport = baseio + 0x00E;
      }

    if (autoinit)
      dma_length = BUFFER_LENGTH;
    else
      dma_length = BLOCK_LENGTH;

    install_handler();

    smix_initialized = FALSE;
    smix_sound = FALSE;

    atexit(smix_exitproc);

    return(TRUE);
  }

void shutdown_sb(void)
  {
    if (handler_installed) uninstall_handler();
    reset_dsp();
  }

/* Voice control */

typedef struct
  {
    SOUND *sound;
    int   index;
    int   volume;
    int   loop;
    long  curpos;
    int   done;
  } VOICE;

static int   inuse[VOICES];
static VOICE voice[VOICES];

static int curblock;

/* Volume lookup table */
static signed int (*volume_table)[VOLUMES][256];

/* Mixing buffer */
static signed int  mixingblock[BLOCK_LENGTH];  /* Signed 16 bit */

/* Output buffers */
static void          (*outmemarea)                = NULL;
static unsigned char (*out8buf)[2][BLOCK_LENGTH]  = NULL;
static signed  short (*out16buf)[2][BLOCK_LENGTH] = NULL;

static void *blockptr[2];

static short int outmemarea_sel;              /* Selector for output buffer */

/* Addressing for auto-initialized transfers (Whole buffer)   */
static unsigned long buffer_addr;
static unsigned char buffer_page;
static unsigned int  buffer_ofs;

/* Addressing for single-cycle transfers (One block at a time */
static unsigned long block_addr[2];
static unsigned char block_page[2];
static unsigned int  block_ofs[2];

static unsigned char sound_volume;

/* 8-bit clipping */

static unsigned char (*clip_8_buf)[256*VOICES];
static unsigned char (*clip_8)[256*VOICES];

static char time_constant(int rate)
  {
    return (256 - (1000000 / rate));
  }

static void init_sampling_rate(unsigned short rate)
  {
    if (sixteenbit)
      {
        write_dsp(0x41);        /* Set digitized sound output sampling rate */
        write_dsp(hi(rate));
        write_dsp(lo(rate));
      }
    else
      {
        write_dsp(0x40);        /* Set digitized sound time constant        */
        write_dsp(time_constant(rate));
      }
  }

void set_sampling_rate(unsigned short rate)
  {
    sampling_rate = rate;

    if (smix_sound)
      {
        if (sixteenbit)
          {
            init_sampling_rate(sampling_rate);
            write_dsp(0xD6);    /* Continue 16-bit DMA mode digitized sound */
          }
        else
          {
            write_dsp(0xD0);    /* Pause 8-bit DMA mode digitized sound     */
            init_sampling_rate(sampling_rate);
            write_dsp(0xD4);    /* Continue 8-bit DMA mode digitized sound  */
          }
      }
  }

static void start_dac(void)
  {
    outp(dma_maskport,   dma_stopmask);
    outp(dma_clrptrport, 0x00);
    outp(dma_modeport,   dma_mode);
    outp(dma_addrport,   lo(buffer_ofs));
    outp(dma_addrport,   hi(buffer_ofs));
    outp(dma_countport,  lo(dma_length-1));
    outp(dma_countport,  hi(dma_length-1));
    outp(dma_pageport,   buffer_page);
    outp(dma_maskport,   dma_startmask);

    init_sampling_rate(sampling_rate);

    if (sixteenbit)
      { /* Sixteen bit auto-initialized: SB16 and up (DSP 4.xx)             */
        write_dsp(0xB6);                /* 16-bit cmd  - D/A - A/I - FIFO   */
        write_dsp(0x10);                /* 16-bit mode - signed mono        */
        write_dsp(lo(BLOCK_LENGTH-1));
        write_dsp(hi(BLOCK_LENGTH-1));
      }
    else
      { /* Eight bit */
        write_dsp(0xD1);                /* Turn on speaker                  */

        if (autoinit)
          { /* Eight bit auto-initialized:  SBPro and up (DSP 2.00+)        */
            write_dsp(0x48);            /* Set DSP block transfer size      */
            write_dsp(lo(BLOCK_LENGTH-1));
            write_dsp(hi(BLOCK_LENGTH-1));
            write_dsp(0x1C);            /* 8-bit auto-init DMA mono output  */
          }
        else
          { /* Eight bit single-cycle:  Sound Blaster (DSP 1.xx+)           */
            write_dsp(0x14);            /* 8-bit single-cycle DMA output    */
            write_dsp(lo(BLOCK_LENGTH-1));
            write_dsp(hi(BLOCK_LENGTH-1));
          }
      }

    smix_sound = TRUE;
  }

static void stop_dac(void)
  {
    smix_sound = FALSE;

    if (sixteenbit)
      {
        write_dsp(0xD5);                /* Pause 16-bit DMA sound I/O       */
      }
    else
      {
        write_dsp(0xD0);                /* Pause 8-bit DMA sound I/O        */
        write_dsp(0xD3);                /* Turn off speaker                 */
      }

    outp(dma_maskport, dma_stopmask);   /* Stop DMA                         */
  }

/* Volume control */

static void init_volume_table(void)
  {
    long volume;
    int  insample;

    volume_table = malloc(VOLUMES * 256 * sizeof(signed int));

    for (volume=0; volume < VOLUMES; volume++)
      for (insample = -128; insample <= 127; insample++)
        {
          (*volume_table)[volume][(unsigned char)insample] =
            (signed int)(((insample*volume) << SHIFT_16_BIT) / (VOLUMES-1));
        }

    sound_volume = 255;
  }

void set_sound_volume(unsigned char new_volume)
  {
    sound_volume = new_volume;
  }

/* Mixing initialization */

static void init_clip8(void)
  {
    int i;
    int value;

    clip_8_buf = malloc(256*VOICES);
    clip_8     = (char *)clip_8_buf + 128*VOICES;

    for (i = -128*VOICES; i < 128*VOICES; i++)
      {
        value = i;
        value = MAX(value, -128);
        value = MIN(value, 127);

        (*clip_8)[i] = value + 128;
      }
  }

static unsigned long linear_addr(void *ptr)
  {
    return((unsigned long)(ptr));
  }

void deallocate_voice(int voicenum);

void init_mixing(void)
  {
    int i;

    for (i=0; i < VOICES; i++)
      deallocate_voice(i);
    voicecount = 0;

    if (sixteenbit)
      {
       /* Find a block of memory that does not cross a page boundary */
        out16buf = outmemarea =
          low_malloc(4*BUFFER_LENGTH, &outmemarea_sel);

        if ((((linear_addr(out16buf) >> 1) % 65536) + BUFFER_LENGTH) > 65536)
          out16buf += 1;  /* Choose second half of memory area */

        for (i=0; i<2; i++)
          blockptr[i] = &((*out16buf)[i]);

       /* DMA parameters */
        buffer_addr = linear_addr(out16buf);
        buffer_page = buffer_addr        / 65536;
        buffer_ofs  = (buffer_addr >> 1) % 65536;

        memset(out16buf, 0x00, BUFFER_LENGTH * sizeof(signed short));
      }
    else
      {
       /* Find a block of memory that does not cross a page boundary */
        out8buf = outmemarea =
          low_malloc(2*BUFFER_LENGTH, &outmemarea_sel);

        if (((linear_addr(out8buf) % 65536) + BUFFER_LENGTH) > 65536)
          out8buf += 1;  /* Choose second half of memory area */

        for (i=0; i<2; i++)
          blockptr[i] = &((*out8buf)[i]);

       /* DMA parameters */
        buffer_addr = linear_addr(out8buf);
        buffer_page = buffer_addr / 65536;
        buffer_ofs  = buffer_addr % 65536;
        for (i=0; i<2; i++)
          {
            block_addr[i] = linear_addr(blockptr[i]);
            block_page[i] = block_addr[i] / 65536;
            block_ofs[i]  = block_addr[i] % 65536;
          }
        memset(out8buf, 0x80, BUFFER_LENGTH * sizeof(unsigned char));

        init_clip8();

      }

    curblock = 0;
    intcount = 0;

    init_volume_table();
    start_dac();
  }

void shutdown_mixing(void)
  {
    stop_dac();

    free(volume_table);

    if (!sixteenbit) free(clip_8_buf);

    low_free(outmemarea_sel);
  }

/* Setup for sound resource files */

static int  resource_file = FALSE;
static char resource_filename[64] = "";

int fexist(char *filename)
  {
    FILE *f;

    f = fopen(filename, "r");

    fclose(f);

    return (f != NULL);
  }

int  open_sound_resource_file(char *filename)
  {
    resource_file = TRUE;
    strcpy(resource_filename, filename);

    return fexist(filename);
  }

void close_sound_resource_file(void)
  {
    resource_file = FALSE;
    strcpy(resource_filename, "");
  }


/* Loading and freeing sounds */

static FILE *sound_file;
static long sound_size;

typedef struct
  {
    char key[8];
    long start;
    long size;
  } RESOURCE_HEADER;

void get_sound_file(char *key)
  {
    static short numsounds;
    int   found;
    int   i;
    static RESOURCE_HEADER res_header;

    found = FALSE;
    sound_size = 0;

    if (resource_file)
      {
        sound_file = fopen(resource_filename, "rb");
        fread(&numsounds, sizeof(numsounds), 1, sound_file);

        for (i = 0; i < numsounds; i++)
          {
            fread(&res_header, sizeof(res_header), 1, sound_file);
            if (!strnicmp(key, res_header.key, 8))
              {
                found = TRUE;
                break;
              }
          }

        if (found)
          {
            fseek(sound_file, res_header.start, SEEK_SET);
            sound_size = res_header.size;
          }
      }
    else
      {
        sound_file = fopen(key, "rb");
        fseek(sound_file, 0, SEEK_END);
        sound_size = ftell(sound_file);
        fseek(sound_file, 0, SEEK_SET);
      }
  }

/* Loading and freeing sounds */

int load_sound(SOUND **sound, char *key)
  {
   /* Open file and compute size */
    get_sound_file(key);

    if (sound_size == 0)
      return FALSE;

   /* Allocate sound control structure and sound data block */
    (*sound) = (SOUND *) malloc(sizeof(SOUND));
    (*sound)->soundptr  = (signed char *)(malloc(sound_size));
    (*sound)->soundsize = sound_size;

   /* Read sound data and close file (Isn't flat mode nice?) */
    fread((*sound)->soundptr, sizeof(signed char), sound_size, sound_file);
    fclose(sound_file);

    return TRUE;
  }

void free_sound(SOUND **sound)
  {
    free((*sound)->soundptr);
    free(*sound);
    *sound = NULL;
  }

/* Voice maintainance */

void deallocate_voice(int voicenum)
  {
    inuse[voicenum] = FALSE;
    voice[voicenum].sound  = NULL;
    voice[voicenum].index  = -1;
    voice[voicenum].volume = 0;
    voice[voicenum].curpos = -1;
    voice[voicenum].loop   = FALSE;
    voice[voicenum].done   = FALSE;
  }

int start_sound(SOUND *sound, int index, unsigned char volume, int loop)
  {
    int i, voicenum;

    voicenum = -1;
    i = 0;

    do
      {
        if (!inuse[i])
          voicenum = i;
        i++;
      }
    while ((voicenum == -1) && (i < VOICES));

    if (voicenum != -1)
      {
        voice[voicenum].sound  = sound;
        voice[voicenum].index  = index;
        voice[voicenum].volume = volume;
        voice[voicenum].curpos = 0;
        voice[voicenum].loop   = loop;
        voice[voicenum].done   = FALSE;

        inuse[voicenum] = TRUE;
        voicecount++;
      }

    return (voicenum != -1);
  }

void stop_sound(int index)
  {
    int i;

    for (i=0; i < VOICES; i++)
      if (voice[i].index == index)
        {
          voicecount--;
          deallocate_voice(i);
        }
  }

int  sound_playing(int index)
  {
    int i;

   /* Search for a sound with the specified index */
    for (i=0; i < VOICES; i++)
      if (voice[i].index == index)
        return(TRUE);

   /* Sound not found */
    return(FALSE);
  }

static void update_voices(void)
  {
    int voicenum;

    for (voicenum=0; voicenum < VOICES; voicenum++)
      {
        if (inuse[voicenum])
          {
            if (voice[voicenum].done)
              {
                voicecount--;
                deallocate_voice(voicenum);
              }
          }
      }
  }

/* Mixing */

static void mix_voice(int voicenum)
  {
    SOUND *sound;
    int   mixlength;
    signed char *sourceptr;
    signed int *volume_lookup;
    int chunklength;
    int destindex;

   /* Initialization */
    sound = voice[voicenum].sound;

    sourceptr = sound->soundptr + voice[voicenum].curpos;
    destindex = 0;

   /* Compute mix length */
    if (voice[voicenum].loop)
      mixlength = BLOCK_LENGTH;
    else
      mixlength =
       MIN(BLOCK_LENGTH, sound->soundsize - voice[voicenum].curpos);

    volume_lookup =
      (signed int *)(&(*volume_table)[(unsigned char)((sound_volume*voice[voicenum].volume*VOLUMES) >> 16)]);

    do
      {
       /* Compute the max consecutive samples that can be mixed */
        chunklength =
         MIN(mixlength, sound->soundsize - voice[voicenum].curpos);

       /* Update the current position */
        voice[voicenum].curpos += chunklength;

       /* Update the remaining samples count */
        mixlength -= chunklength;

       /* Mix samples until end of mixing or end of sound data is reached */
        while (chunklength--)
          mixingblock[destindex++] += volume_lookup[(unsigned char)(*(sourceptr++))];

       /* If we've reached the end of the block, wrap to start of sound */
        if (sourceptr == (sound->soundptr + sound->soundsize))
          {
            if (voice[voicenum].loop)
              {
                voice[voicenum].curpos = 0;
                sourceptr = sound->soundptr;
              }
            else
              {
                voice[voicenum].done = TRUE;
              }
          }
      }
    while (mixlength); /* Wrap around to finish mixing if necessary */
  }

static void silenceblock(void)
  {
    memset(&mixingblock, 0x00, BLOCK_LENGTH*sizeof(signed int));
  }

static void mix_voices(void)
  {
    int i;

    silenceblock();

    for (i=0; i < VOICES; i++)
      if (inuse[i])
        mix_voice(i);
  }

static void copy_sound16(void)
  {
    int i;
    signed short *destptr;

    destptr   = blockptr[curblock];

    for (i=0; i < BLOCK_LENGTH; i++)
      destptr[i] = mixingblock[i];
  }

static void copy_sound8(void)
  {
    int i;
    unsigned char *destptr;

    destptr   = blockptr[curblock];

    for (i=0; i < BLOCK_LENGTH; i++)
      destptr[i] = (*clip_8)[mixingblock[i] >> 5];
  }

static void copy_sound(void)
  {
    if (sixteenbit)
      copy_sound16();
    else
      copy_sound8();
  }

static void startblock_sc(void)     /* Starts a single-cycle DMA transfer   */
  {
    outp(dma_maskport,   dma_stopmask);
    outp(dma_clrptrport, 0x00);
    outp(dma_modeport,   dma_mode);
    outp(dma_addrport,   lo(block_ofs[curblock]));
    outp(dma_addrport,   hi(block_ofs[curblock]));
    outp(dma_countport,  lo(BLOCK_LENGTH-1));
    outp(dma_countport,  hi(BLOCK_LENGTH-1));
    outp(dma_pageport,   block_page[curblock]);
    outp(dma_maskport,   dma_startmask);
    write_dsp(0x14);                /* 8-bit single-cycle DMA sound output  */
    write_dsp(lo(BLOCK_LENGTH-1));
    write_dsp(hi(BLOCK_LENGTH-1));
  }

static void interrupt inthandler(void)
  {
    intcount++;

    if (!autoinit)   /* Start next block quickly if not using auto-init DMA */
      {
        startblock_sc();
        copy_sound();
        curblock = !curblock;  /* Toggle block */
      }

    update_voices();
    mix_voices();

    if (autoinit)
      {
        copy_sound();
        curblock = !curblock;  /* Toggle block */
      }

    inp(ackport);       /* Acknowledge interrupt with sound card */
    outp(0xA0, 0x20);   /* Acknowledge interrupt with PIC2 */
    outp(0x20, 0x20);   /* Acknowledge interrupt with PIC1 */
  }

void install_handler(void)
  {
    _disable();  /* CLI */
    outp(pic_maskport, (inp(pic_maskport) | irq_stopmask));

    oldintvector = _dos_getvect(irq_intvector);
    _dos_setvect(irq_intvector, inthandler);

    outp(pic_maskport, (inp(pic_maskport) & irq_startmask));
    _enable();   /* STI */

    handler_installed = TRUE;
  }

void uninstall_handler(void)
  {
    _disable();  /* CLI */
    outp(pic_maskport, (inp(pic_maskport) | irq_stopmask));

    _dos_setvect(irq_intvector, oldintvector);

    _enable();   /* STI */

    handler_installed = FALSE;
  }

void smix_exitproc(void)
  {
    if (smix_initialized)
      {
        stop_dac();
        shutdown_sb();
      }
  }

/* ������������������������������������������������������������������������ */
