%ifndef MODEX_ASM
%define MODEX_ASM

%include "src\util.asm"

SEGMENT _DATA PUBLIC ALIGN=4 USE32 class=DATA
    old_video_mode db 0x03

    SC_INDEX        equ 0x03C4
    CRTC_INDEX      equ 0x03D4
    MISC_OUT        equ 0x03C2
    PALETTE_INDEX   equ 0x03C8

    VGA_START       equ 0xA0000

    PAGE_SIZE       equ 0x4B00
    PLANE_SIZE      equ (PAGE_SIZE >> 2)

    MAP_MASK        equ 0x02
    SCREEN_WIDTH    equ 80

    GLOBAL _modexvar_active_start
    GLOBAL _modexvar_visible_start

    _modexvar_active_start  dd 0
    _modexvar_visible_start dd 0

SEGMENT _TEXT PUBLIC ALIGN=4 USE32 class=CODE
GROUP DGROUP _DATA

;; Some internal procs

;; WriteSC: Writes a value to the Sequencer
;; AH == Port to Write to
;; AL == Data to Write
WriteSC:
    mov dx, SC_INDEX
    xchg ah, al
    out dx, al
    inc dx
    xchg ah, al
    out dx, al
    ret

;; WriteCRTC: Writes a value to the CRT Controller
;; AH == Port to Write to
;; AL == Data to Write
WriteCRTC:
    mov dx, CRTC_INDEX
    xchg ah, al
    out dx, al
    inc dx
    xchg ah, al
    out dx, al
    ret

;; SwitchPlane: Internal version of _modex_switch_plane
;; =------------------------=
;; Input: SI - Plane Number
SwitchPlane:
    pusha
    mov cx, si
    and cl, 3
    mov bx, 1
    shl bx, cl
    mov al, bl
    mov ah, 0x02
    call WriteSC
    popa
    ret

;; void modex_init(void);
;; =--------------------=
;; Call when you want to switch to Mode X
;; =--------------------=

GLOBAL _modex_init
_modex_init: FUNCTION
    pusha
    
    ;; Get old video mode and save it somewhere
    mov ax, 0x0F00
    int 0x10
    mov byte [old_video_mode], al

    ;; Set-up standard Mode 13h
    mov ax, 0x0013
    int 0x10

    ;; Clear the 64K memory window
    mov cx, 0xFFFF
    mov al, 0x00
    mov edi, VGA_START
    rep stosb

    ;; And now, to 'tweak' Mode 13h into Mode X:
    ;; A lot of disparate bits of assembly ported from multiple different old source repositories
    ;; for a bunch of old games and some ancient archived BBS posts.
    ;; It works, somehow.

    ;; Disable Chain-4 mode
    mov ax, 0x0406
    call WriteSC
    
    ;; Turn off word mode
    mov ax, 0x17E3
    call WriteCRTC

    ;; Turn off doubleword mode
    mov ax, 0x1400
    call WriteCRTC
    
    ;; At this point we're in "Mode Y", or 320x200-but-with-bitplanes mode.
    ;; We'll add another 40 rows of pixels because that extra space is nice to have.
    ;; This is mostly taken from Michael Abrash's Graphics Programming Black Book

    ;; Turn off register write protection
    mov ax, 0x112C
    call WriteCRTC

    ;; Change Vertical Total
    mov ax, 0x060D
    call WriteCRTC    

    ;; Set Overflow Register
    mov ax, 0x073E
    call WriteCRTC

    ;; Set Vertical Retrace Start
    mov ax, 0x10EA ;; EA, it's in the game! I'd exorcise it, but we'd be losing valuable screen space.
    call WriteCRTC

    ;; Set Vertical Retrace End & Write Protection Enable
    mov ax, 0x11AC
    call WriteCRTC

    ;; Set Vertical Display Enable
    mov ax, 0x12DF
    call WriteCRTC
    
    ;; Set VBlank Start
    mov ax, 0x15E7
    call WriteCRTC

    ;; Set VBlank End
    mov ax, 0x1606
    call WriteCRTC

    ;; Delay a bit to wait for the VGA controllers to finish doing their thing
    %rep 1000
        mov al, byte [old_video_mode]
    %endrep

    ;; Set up active start/visible values with their defaults
    mov dword [_modexvar_active_start], 0x00000000
    mov dword [_modexvar_visible_start], 0x00000000

    popa
ENDFUNCTION

;; void modex_restore(void)
;; =----------------------=
;; Restores video mode to whatever it was before Mode X was set
;; =----------------------=

GLOBAL _modex_restore
_modex_restore: FUNCTION
    push edi
    ;; Let's clear the video buffer first to be nice to DOS
    mov cx, 0xFFFF
    mov edi, VGA_START
    mov al, 0x00
    rep stosb

    ;; And now jump back into the original mode (Likely Mode 0x03)
    mov ah, 0x00
    mov al, byte [old_video_mode]
    int 0x10

    pop edi
ENDFUNCTION

;; void modex_set_active_page(unsigned int page)
;; =-------------------------------------------=
;; Switches the currently ACTIVE page to the given page (0..2 in Mode X)
;; =-------------------------------------------=

GLOBAL _modex_set_active_page
_modex_set_active_page: FUNCTION
    %arg page:word
    xor eax, eax
    mov ax, word [page]
    and ax, 0x03 ;; Clamp to three pages. We can fit just over 3 Mode X screens in 64K.
    imul ax, 19200 ;; Magic Multiplication!! 19200 is the length of a page in bytes

    mov dword [_modexvar_active_start], eax
ENDFUNCTION

;; void modex_set_visible_page(unsigned int page)
;; =---------------------------------------------=
;; Much the same as setting the active page, but we
;; also have to set up the CRTC register to make the
;; switch this time.
;; =---------------------------------------------=

GLOBAL _modex_set_visible_page
_modex_set_visible_page: FUNCTION
    %arg page:word

    xor eax, eax
    mov ax, word [page]
    and ax, 0x03
    imul ax, 19200

    mov dword [_modexvar_visible_start], eax

    ;; Now write to the registers
    mov ah, 0x0C
    mov al, byte [_modexvar_visible_start+1]
    call WriteCRTC

    mov ah, 0x0D
    mov al, byte [_modexvar_visible_start+0]
    call WriteCRTC
ENDFUNCTION

;; void modex_switch_plane(unsigned int plane) 
;; =------------------------------------------=
;; Sets the bitplane mask on the Sequence Controller
;; =------------------------------------------=

GLOBAL _modex_switch_plane
_modex_switch_plane: FUNCTION
    %arg plane:word
    mov ah, 0x02
    mov bx, word [plane]
    mov cl, 1
    xchg bl, cl
    shl bx, cl
    mov al, bl
    call WriteSC
ENDFUNCTION

;; void modex_writepixel(unsigned int x, unsigned int y, unsigned int color)
;; =-----------------------------------------------------------------------=
;; Write a pixel directly to VRAM - slower than directly writing to the VGA
;; memory window @ 0xA0000
;; =-----------------------------------------------------------------------=

GLOBAL _modex_write_pixel
_modex_write_pixel: FUNCTION
    %arg x:word, y:word, color:word

    ;; Set mask based on X coordinate % 3
    mov ah, 0x02
    mov cx, word [x]    
    and cl, 3
    mov bx, 1
    shl bx, cl
    mov al, bl
    call WriteSC

    xor ebx, ebx
    mov bx, SCREEN_WIDTH
    imul bx, word [y]
    mov cx, word [x]
    shr cx, 2
    add bx, cx
    mov eax, VGA_START
    add eax, dword [_modexvar_active_start]
    
    mov cl, byte [color]
    mov byte [eax+ebx], cl
ENDFUNCTION

;; void modex_write_pixel_buffer(unsigned int x, unsigned int y, unsigned int color, unsigned char* bufferptr)
;; =--------------------------------------------------------------------------------------------------------=
;; Write a pixel to a buffer. Automatically writes to the correct plane offset so that the buffer can 
;; be copied in a linear way (Mask 0, Copy first 4th of the buffer, Mask 1, next 4th, etc).
;; =--------------------------------------------------------------------------------------------------------=
GLOBAL _modex_write_pixel_buffer
_modex_write_pixel_buffer: FUNCTION
    %arg x:word, y:word, color:word, bufferptr:dword

    ;; Get X mask value (0..3) and multiply by PAGE_SIZE to get the start offset
    xor ecx, ecx
    mov cx, word [x]
    and cx, 3
    imul cx, PAGE_SIZE ;; cx now holds the starting offset

    ;; Write into buffer
    mov eax, dword [bufferptr]
    mov ebx, 80
    imul bx, word [y]
    mov dx, word [x]
    shr dx, 1
    shr dx, 1
    add bx, dx
    mov dl, byte [color]
    add eax, ebx
    mov [eax+ecx], dl
ENDFUNCTION

;; void modex_blitbuffer(unsigned char* buffer, unsigned long yoffset)
;; =------------------------------------------=
;; Copies an in-memory buffer to VRAM
;; =------------------------------------------=
GLOBAL _modex_blitbuffer
_modex_blitbuffer: FUNCTION
    %arg buffer:dword, yoffset:dword, length:dword

    push esi
    pop edi

    mov esi, dword [buffer]

    mov edi, VGA_START
    add edi, dword [_modexvar_active_start]
    add edi, dword [yoffset]

    push esi
        mov si, 0    
        call SwitchPlane
    pop esi

    mov ecx, PAGE_SIZE
    rep movsb    

    sub edi, PAGE_SIZE
    push esi
        mov si, 1    
        call SwitchPlane
    pop esi
    
    mov ecx, PAGE_SIZE
    rep movsb

    sub edi, PAGE_SIZE
    push esi
        mov si, 2
        call SwitchPlane
    pop esi
    
    mov ecx, PAGE_SIZE
    rep movsb

    sub edi, PAGE_SIZE
    push esi
        mov si, 3
        call SwitchPlane
    pop esi
    
    mov ecx, PAGE_SIZE
    rep movsb

    pop edi
    pop esi
ENDFUNCTION

;; void modex_blitsprite(uint x, uint y, uint width, uint height, unsigned char* impptr)
;; =-----------------------------------------------------------------------------------=
;; Blits a sprite with no transparency checks directly to VRAM
;; The sprite must be in linear planar format (First quarter of the sprite is all pixels
;;   for plane 0, next quarter is all pixels for plane 1, and so on)
;; =-----------------------------------------------------------------------------------=
GLOBAL _modex_blitsprite
_modex_blitsprite: FUNCTION
    %arg x:word, y:word, width:word, height:word, imgptr:dword
    push esi
    push edi

    mov esi, dword [imgptr]
    
    mov bx, word [width] ;; Calculate number of bytes per plane to copy
    shr bx, 2

    mov ax, 0 ;; Current plane

    .planeLoop:
        push esi ;; Preserve esi and switch plane to the current plane
        mov si, ax
        call SwitchPlane
        pop esi
        
        xor edx, edx
        push eax
        push ebx

        ;; Set starting offset
        xor eax, eax
        mov ax, word [y]
        imul ax, SCREEN_WIDTH
        mov bx, word [x]
        shr bx, 2
        add ax, bx
        mov edi, VGA_START
        add edi, eax
        add edi, dword [_modexvar_active_start]

        pop ebx
        pop eax

        ;; edi now contains the offset into the VRAM window
        mov dx, word [height]
        .rowLoop:
            mov cx, bx ;; Set count to total plane width
            rep movsb
            add edi, 80 ;; Add a screen width to jump down to next row
            sub di, bx ;; Subtract a row width to position us back at the starting X value
            dec dx
            jnz .rowLoop

        inc ax ;; Increment plane number
        cmp ax, 4 ;; Last plane?
        jb .planeLoop ;; If not, do the next plane

    pop edi
    pop esi
ENDFUNCTION

;; void modex_blitsprite_buffer(uint x, uint y, uint width, uint height, unsigned char* imgptr, unsigned char* buffer)
;; =-----------------------------------------------------------------------------------------------------------------=
;; Same as the previous function, but blits the sprite into a buffer instead. Buffer needs to be in linear plane
;; format, same as the sprites, and is assumed to be ready to hold a 320x240 image.
;; =-----------------------------------------------------------------------------------------------------------------=
GLOBAL _modex_blitsprite_buffer
_modex_blitsprite_buffer: FUNCTION
    %arg x:word, y:word, width:word, height:word, imgptr:dword, bufptr:dword
    push esi
    push edi

    mov esi, dword [imgptr]    

    mov ax, 0 ;; Current plane
    mov cx, word [x]
    and cx, 0x03
    mov ah, cl    

    mov bx, word [width] ;; Calculate number of bytes per plane to copy
    shr bx, 2    
    
    .planeLoop:
        xor edx, edx         

        push eax
        push ebx
        push ecx

        xor ebx, ebx        
        mov bl, ah
        and bx, 0x03
        imul bx, PAGE_SIZE

        cmp ah, 0x04
        jb .noNudge
        .nudge:
            inc bx            
        .noNudge:

        xor eax, eax
        mov edi, ebx
        mov ax, word [y]
        imul ax, SCREEN_WIDTH
        mov bx, word [x]
        shr bx, 1
        shr bx, 1
        add ax, bx
        add edi, dword [bufptr]
        add edi, eax

        pop ecx
        pop ebx 
        pop eax

        mov dx, word [height]
        .rowLoop:
            xor ebx, ebx
            mov bx, word [width]
            shr bx, 1
            shr bx, 1
            mov cx, bx ;; Set count to total plane width
            rep movsb
            add edi, 80 ;; Add a screen width to jump down to next row
            sub edi, ebx ;; Subtract a row width to position us back at the starting X value
            dec dx
            jnz .rowLoop

        inc ah        
        inc al ;; Increment plane number
        cmp al, 4 ;; Last plane?        
        jb .planeLoop ;; If not, do the next plane

    pop edi
    pop esi
ENDFUNCTION

;; void modex_blitsprite_trans(uint x, uint y, uint width, uint height, unsigned char* imgptr)
;; =-----------------------------------------------------------------------------------------=
;; Blits a sprite, treating color index 0 as transparency, otherwise the same as the previous
;; function.
;; =-----------------------------------------------------------------------------------------=
GLOBAL _modex_blitsprite_trans
_modex_blitsprite_trans: FUNCTION
    %arg x:word, y:word, width:word, height:word, imgptr:dword

    push edi
    push esi

    mov esi, dword [imgptr]
    mov edi, VGA_START

    mov dx, word [height]
    mov bx, word [width]
    shr bx, 2
    mov al, 0

    .planeLoop:
        push esi
        push eax
        mov ah, 0x00
        mov si, ax
        call SwitchPlane
        pop eax
        pop esi

        xor edx, edx
        push eax
        push ebx

        ;; Set up starting offset
        xor eax, eax
        
        mov ax, word [y]
        imul ax, SCREEN_WIDTH
        mov bx, word [x]
        shr bx, 2
        add ax, bx
        
        mov edi, VGA_START
        add edi, eax
        add edi, dword [_modexvar_active_start]

        pop ebx
        pop eax

        mov dx, word [height]
        .rowLoop:
            mov cx, bx
            .pixelLoop:
                mov ah, byte [esi] ;; Get source pixel
                or ah, ah ;; To this to set flags
                jz .noPixel
                mov [edi], ah
            .noPixel:
                inc edi
                inc esi
            dec cx
            jnz .pixelLoop

            ;; Increase destination pointer and reposition X coordinate
            add edi, SCREEN_WIDTH
            sub di, bx
            dec dx
            jnz .rowLoop

        ;; Change to next plane in sequence
        and al, 0xFF
        inc al
        cmp al, 4
        jb .planeLoop
    pop esi
    pop edi
ENDFUNCTION

;; void modex_blitsprite_buffer_trans(uint x, uint y, uint width, uint height, unsigned char* imgptr, unsigned char* buffer)
;; =-----------------------------------------------------------------------------------------------------------------------=
;; Same as the other function, but blits to an offscreen buffer
;; =-----------------------------------------------------------------------------------------------------------------------=
GLOBAL _modex_blitsprite_buffer_trans
_modex_blitsprite_buffer_trans: FUNCTION
    %arg x:word, y:word, width:word, height:word, imgptr:dword, buffer:dword

    push esi
    push edi

    mov esi, dword [imgptr]
    
    mov bx, word [width] ;; Calculate number of bytes per plane to copy
    shr bx, 2

    mov ax, 0 ;; Current plane
    mov cx, word [x]
    and cx, 0x03
    mov ah, cl    
    
    .planeLoop:
        xor edx, edx      

        push eax
        push ebx
        push ecx

        xor ebx, ebx        
        mov bl, ah
        and bx, 0x03
        imul ebx, PAGE_SIZE
        
        cmp ah, 0x04
        jb .noNudge
        .nudge:
            inc ebx
        .noNudge:

        xor eax, eax        
        mov edi, ebx
        mov ax, word [y]
        imul eax, SCREEN_WIDTH
        mov bx, word [x]
        shr bx, 1
        shr bx, 1
        add eax, ebx
        add edi, dword [bufptr]
        add edi, eax

        pop ecx
        pop ebx 
        pop eax 
        ;; edi now contains the offset into the VRAM window
        mov dx, word [height]
        push eax
        .rowLoop:
            mov cx, bx ;; Set count to total plane width
            .pixelLoop:
                mov ah, byte [esi] ;; Get source pixel
                or ah, ah ;; To this to set flags
                jz .noPixel
                mov [edi], ah
            .noPixel:
                inc esi
                inc edi
                dec cx
                jnz .pixelLoop
            
            add edi, SCREEN_WIDTH ;; Add a screen width to jump down to next row
            sub edi, ebx ;; Subtract a row width to position us back at the starting X value
            dec dx
            jnz .rowLoop
        pop eax
        ;; Increment EDI by PAGE_SIZE - (80*height) to position the sprite at the next
        ;; plane boundary.        
        push eax
        push ebx
        push ecx
            xor ecx, ecx
            xor ebx, ebx
            mov cx, ax
            inc cx
            imul ecx, PAGE_SIZE

            xor eax, eax
            mov ax, word [y]
            imul eax, 80
            mov bx, word [x]
            shr bx, 1
            shr bx, 1
            add eax, ebx        
            mov edi, dword [buffer]
            add edi, ebx
            add edi, ecx            
        pop ecx
        pop ebx
        pop eax

        inc ah                
        inc al ;; Increment plane number
        cmp al, 4 ;; Last plane?        
        jb .planeLoop ;; If not, do the next plane

    pop edi
    pop esi
ENDFUNCTION

;; Palette manipulation functions!
;; --

;; void modex_set_palette(unsigned char* palette)
;; =--------------------------------------------=
;; Sets the entire palette in one loop - input is 768 palette entries
;; =--------------------------------------------=
GLOBAL _modex_set_palette
_modex_set_palette: FUNCTION
    %arg palette:dword

    push esi

    mov esi, dword [palette]
    mov dx, PALETTE_INDEX
    mov al, 0x00
    out dx, al

    ;; Loop
    inc dx
    mov cx, 768
    rep outsb

    pop esi
ENDFUNCTION

;; int modex_palette_fade(unsigned char* palette, unsigned int step)
;; =---------------------------------------------=
;; Fades the entire palette to black by whatever the step
;; value is.
;; =---------------------------------------------=
GLOBAL _modex_palette_fade
_modex_palette_fade: FUNCTION
    %arg palette:dword, step:word
    push edi
    push esi
    
    mov esi, dword [palette]
    mov cx, 768
    
    mov dx, PALETTE_INDEX
    mov al, 0x00
    out dx, al
    inc dx

    mov bx, 1

    .la:
        mov al, [esi]
        or al, al
        jz .skip
    .decrement:
        sub al, byte [step]
        mov bx, 0
        mov [esi], al
    .skip:          
        out dx, al  
        inc esi    
        dec cx
        jnz .la

    pop esi
    pop edi
    xor eax, eax
    mov ax, bx
ENDFUNCTION

;; int modex_palette_fadein(unsigned char* activepalette, unsigned char* destpalette, unsigned int step)
;; =---------------------------------------------------------------------------------=
;; Fades activepalette, which represents the currently loaded palette, towards destpalette
;; using simple linear increments determined by the step value.
;; =---------------------------------------------------------------------------------=
GLOBAL _modex_palette_fadein
_modex_palette_fadein: FUNCTION
    %arg activepalette:dword, destpalette:dword, step:word
    push edi
    push esi
    
    mov esi, dword [activepalette]
    mov edi, dword [destpalette]
    mov cx, 768
    
    mov dx, PALETTE_INDEX
    mov al, 0x00
    out dx, al
    inc dx

    mov bx, 1

    .la:
        mov al, [esi]
        cmp al, byte [edi]
        je .skip
    .decrement:
        add al, byte [step]
        mov bx, 0
        mov [esi], al
    .skip:          
        out dx, al      
        inc esi
        inc edi
        dec cx
        jnz .la

    pop esi
    pop edi
    xor eax, eax
    mov ax, bx
ENDFUNCTION
%endif

;; void modex_pal_setrange(unsigned char* palette, unsigned int start, unsigned int end)
;; =-----------------------------------------------------------------------------------=
;; Sets a palette range. Expects an abridged palette array to be passed in.
;; =-----------------------------------------------------------------------------------=
GLOBAL _modex_palette_setrange
_modex_palette_setrange: FUNCTION
    %arg palette:dword, start:word, end:word
    push esi    

    mov esi, dword [palette]
    mov dx, PALETTE_INDEX
    mov al, byte [start]
    out dx, al

    ;; Loop
    inc dx

    mov bx, word [end]
    sub bx, word [start]
    imul bx, 3
    
    mov cx, bx
    rep outsb
    
    pop esi
ENDFUNCTION

;; void modex_wait_retrace()
;; =-----------------------=
;; Halt execution until next retrace begins
;; =-----------------------=

%define INPUT_STATUS 0x3DA
%define INPUT_FLAG_ENABLED 0x01
%define INPUT_FLAG_RETRACE 0x08

GLOBAL _modex_wait_retrace
_modex_wait_retrace:FUNCTION
    .retrace_check:
        mov dx, INPUT_STATUS
        in al, dx
        and al, INPUT_FLAG_RETRACE
        jz .retrace_check
    .retrace_end:
    .enable_check:
        in al, dx
        and al, INPUT_FLAG_ENABLED
        jnz .enable_check
.enable_end:
ENDFUNCTION

