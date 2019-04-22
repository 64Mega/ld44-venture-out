// I have no idea if I can even get this to work properly.
// =-----------------------------------------------------=

#ifndef _KBD_H
#define _KBD_H

extern void kbd_install();
extern void kbd_uninstall();

extern volatile unsigned char kbd_scanbuffer[];
volatile unsigned char kbd_scanbuffer_c[];

#define MAX_KEYS 128

#define KBD_RETURN      0x1C
#define KBD_ENTER       0x1C // Just an alias for RETURN
#define KBD_ESCAPE      0x01
#define KBD_SPACE       0x39
#define KBD_BACKSPACE   0x0E
#define KBD_TAB         0x0F
#define KBD_ALT         0x38
#define KBD_CONTROL     0x1D
#define KBD_CAPSLOCK    0x3A
#define KBD_LSHIFT      0x2A
#define KBD_RSHIFT      0x36
#define KBD_UPARROW     0x48
#define KBD_DOWNARROW   0x50
#define KBD_LEFTARROW   0x4B
#define KBD_RIGHTARROW  0x4D

#define	KBD_A			0x1E
#define	KBD_B			0x30
#define	KBD_C			0x2E
#define	KBD_D			0x20
#define	KBD_E			0x12
#define	KBD_F			0x21
#define	KBD_G			0x22
#define	KBD_H			0x23
#define	KBD_I			0x17
#define	KBD_J			0x24
#define	KBD_K			0x25
#define	KBD_L			0x26
#define	KBD_M			0x32
#define	KBD_N			0x31
#define	KBD_O			0x18
#define	KBD_P			0x19
#define	KBD_Q			0x10
#define	KBD_R			0x13
#define	KBD_S			0x1F
#define	KBD_T			0x14
#define	KBD_U			0x16
#define	KBD_V			0x2F
#define	KBD_W			0x11
#define	KBD_X			0x2D
#define	KBD_Y			0x15
#define	KBD_Z			0x2C

#define	KBD_F1			0x3B
#define	KBD_F2			0x3C
#define	KBD_F3			0x3D
#define	KBD_F4			0x3E
#define	KBD_F5			0x3F
#define	KBD_F6			0x40
#define	KBD_F7			0x41
#define	KBD_F8			0x42
#define	KBD_F9			0x43
#define	KBD_F10			0x44
#define	KBD_F11			0x57
#define	KBD_F12			0x59

#define KBD_KeyDown(keycode) (kbd_scanbuffer[(keycode)])

#endif