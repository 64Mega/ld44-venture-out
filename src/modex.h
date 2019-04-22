// Mode X functions
// =--------------=
// The implementations are all in modex.asm, decently commented.

#ifndef MODEX_H
#define MODEX_H

// This is the size of an off-screen buffer.
// Use it if you're allocating any.
#define MODEX_BUFFER_SIZE (320*240)

// Call this to enter Mode X
// Automagically stores the old video mode value
// for modex_restore();
extern void modex_init(void);

// Call this to go back to whatever less exciting mode
// you were using before Mode X
extern void modex_restore(void);

// This function switches the /active/ page
// This is the page that the draw-to-VRAM functions
// will draw to, and is not necessarily the /visible/ page.
// We have 3 pages to work with in Mode X
extern void modex_set_active_page(unsigned int page);

// This function sets the /visible/ page.
// Basically, use this for page flipping.
extern void modex_set_visible_page(unsigned int page);

// This function sets the /active plane/.
// If you don't know what this means, confusion awaits!
extern void modex_switch_plane(unsigned int plane);

// This function writes a pixel. It's slow, but useful for
// lazy debugging.
// The fastest way to write pixels is to write them per-plane
// by directly injecting them into an offscreen buffer OR
// writing them directly to VRAM.
extern void modex_write_pixel(unsigned short x, unsigned short y, unsigned short color);

// This variant writes to an offscreen buffer.
// Use the buffer size define to determine how many bytes
// you need for a single buffer page.
extern void modex_write_pixel_buffer(unsigned int x, unsigned int y, unsigned int color, unsigned char* buffer);


// NOTE: The non-buffer blit-sprite functions only draw in 4-pixel horizontal increments. This is
// because I'm not planning on drawing anything directly to VRAM every frame aside from a buffer that
// is going to have smooth animation. Use the buffer functions instead. This can work for 'quick'
// background draws in some instances.

// This function blits an entire offscreen buffer to VRAM
// and should be pretty fast.
extern void modex_blitbuffer(unsigned char* buffer);

// This function blits a sprite with /no transparency/ to
// VRAM directly.
extern void modex_blitsprite(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char* imgptr);

// This function blits a sprite with /no transparency/ to
// an offscreen buffer.
extern void modex_blitsprite_buffer(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char* imgptr, unsigned char* buffer);

// This function blits a sprite /with transparency/ to
// VRAM directly. Color index 0 is the transparent color.
extern void modex_blitsprite_trans(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char* imgptr);

// This function blits a sprite /with transparency/ to
// an offscreen buffer.
extern void modex_blitsprite_buffer_trans(unsigned int x, unsigned int y, unsigned int width, unsigned int height, unsigned char* imgptr, unsigned char* buffer);

// This function sets the entire palette.
// Expects an array of 768 values (256 RGB values in R, G, B... repeating sequence)
// RGB values can range from 0..63, due to the mode's limitations on color.
extern void modex_set_palette(unsigned char* palette);

// This function fades all palette entries to zero.
// Note that this modifies the original array!
// This means that if you want to, say, fade back in to a 'New' palette,
// you'll need to load it into a separate buffer and use the next function
// to do that.
//  Returns 1 when complete, 0 if not.
extern int modex_palette_fade(unsigned char* palette, unsigned int step);

// This function fades from one palette to another. Generally the
// 'active' palette will be all zero values after completing a fade.
// The 'dest' palette contains the palette you're fading to.
extern int modex_palette_fadein(unsigned char* activepalette, unsigned char* destpalette, unsigned int step);

// Sets a palette range.
// Expects an 'abridged' palette array. E.G: Pass in palette_ptr+offset_to_start, start, end
// start and end are in palette indices instead of array indices (E.G: 0..255 instead of 0..767)
extern void modex_palette_setrange(unsigned char* palette, unsigned int start, unsigned int end);

// Waits for vertical retrace. It's a good idea to call this before blitting any buffers/updating the
// screen, to both avoid tearing and to lock to screen refresh rate.
extern void modex_wait_retrace();

#endif