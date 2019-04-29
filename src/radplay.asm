;; A rewrite of RAD Tracker's player. The original was TASM code, 16-bit.
;; This one is "I tried" Protected-mode code. 

%ifndef RADPLAY_ASM
%define RADPLAY_ASM

%include "src\util.asm"

SEGMENT _DATA PUBLIC ALIGN=4 USE32 class=DATA
;; Some data definitions

;; Tracker command eqs
cmPortamentoUp      equ 1
cmPortamentoDown    equ 2
cmToneSlide         equ 3
cmToneVolSlide      equ 5
cmVolSlide          equ 10
cmSetVol            equ 12
cmJumpToLine        equ 13
cmSetSpeed          equ 15

FreqStart           equ 0x156
FreqEnd             equ 0x2AE
FreqRange           equ FreqEnd - FreqStart

ChannelOffs	        db	20h,21h,22h,28h,29h,2ah,30h,31h,32h

GLOBAL NoteFreq
NoteFreq	dw	0x016B,0x0181,0x0198,0x01B0,0x01CA,0x01E5	; 156h = C
		    dw	0x0202,0x0220,0x0241,0x0263,0x0287,0x02AE
            ;;dw	0x016B,0x0181,0x0198,0x01B0,0x01CA,0x01E5	; 156h = C  
            ;;dw	0x0202,0x0220,0x0241,0x0263,0x0287,0x02AE
;; Adlib base port.
AdlibPort           dw 0x388

;; Pointer to module in memory
mod_ptr             dd 0
mod_speed           db 0
mod_speed_counter   db 0

num_instruments     dw 0

mod_order_size      dw 0
mod_order_offset    dd 0
mod_order_idex      dw 0

mod_ptable_offset   dd 0
mod_pattern_idex    dw 0
mod_pattern_line    db 0

Line db 0
PatternList dw 0
PatternPos dw 0
OrderSize dw 0
OrderList dw 0
OrderPos dw 0
InstPtrs times 31 dw 0
ModSeg dw 0

;; Special user-callback
callback_user_effect dd 0

;; Callback that triggers when the song reaches its 'end' (Or loops)
callback_song_end dd 0

;; Record of certain adlib register values
old43               times 9 db 0
oldA0               times 9 db 0
oldB0               times 9 db 0

;; Effect storage
toneSlideSpeed      times 9 db 1
toneSlideFreqL      resb 9
toneSlideFreqR      resb 9

toneSlide           resb 9
portSlide           resb 9
volSlide            resb 9

RadVolume db 63

SEGMENT _TEXT PUBLIC ALIGN=4 USE32 class=CODE
GROUP DGROUP _DATA

;; Global symbols
GLOBAL _rad_set_usercallback
GLOBAL _rad_set_endcallback
GLOBAL _rad_init
GLOBAL _rad_playback
GLOBAL _rad_end
GLOBAL _rad_get_num_orders

; #############################################################################################
; void rad_set_usercallback(funcptr* callback)
; =--------------------------------------=
; Call this to set up a callback to be invoked on certain effects. 
; Must accept two arguments: char* command, char* value
; #############################################################################################
_rad_set_usercallback: FUNCTION
    %arg func_ptr:dword

    mov eax, dword [func_ptr]
    mov dword [callback_user_effect], eax
ENDFUNCTION

; #############################################################################################
; void rad_set_endcallback(funcptr* callback)
; =--------------------------------------=
; Sets up a callback that triggers when the song reaches its end or loops.
; #############################################################################################
_rad_set_endcallback: FUNCTION
    %arg func_ptr:dword

    mov eax, dword [func_ptr]
    mov dword [callback_song_end], eax
ENDFUNCTION

;##############################################################################################
; unsigned int rad_init(unsigned char* data_ptr)
; =--------------------------------------------=
; Initializes the player state with the data block provides. Data is just a RAD file in a
; buffer. 
; Returns non-zero on success
;##############################################################################################

_rad_init: FUNCTION
    %arg data_ptr:dword

    push ebx
    push ecx 
    push edx
    push esi
    push edi

    call _rad_end

    ; Initialize certain Adlib registers that aren't changed
        mov ax, 0x0120
        call Adlib
        mov ax, 0x0800
        call Adlib
        mov ah, 0xBD
        call Adlib

    ; Save segment of module - Not really necessary in Protected Mode        
    ; This is how we're doing it:
        mov edi, [data_ptr]
        mov esi, edi
        mov dword [mod_ptr], edi

    ; Check to see the provided module is a RAD file
        cmp word [edi+0], 'RA'
        jnz .err
        cmp word [edi+2], 'D '
        jnz .err
        cmp byte [edi+16], 0x10
        jnz .err

    ; Read initial speed
        mov al, byte [edi+17]
        mov ah, al
        and al, 0x1F
        mov byte [mod_speed], al

    ; See if there's a description to skip
        mov edi, [mod_ptr]
        add edi, 18
        test ah, 0x80
        jz .lc

        xor al, al
        jmp .le

    .ld: inc edi
    .le: cmp byte [edi], al
        jnz .ld
        inc edi

    ; Create table of instrument pointers
    .lc: xor ebx, ebx
    .la: mov bl, byte [edi]
        inc edi
        add bx, bx
        jz .lb

        mov esi, edi
        sub esi, dword [mod_ptr]
        mov [InstPtrs-2+ebx], si
        add edi, 11
        jmp .la

    ; Record offset order list
    .lb: xor ax, ax
        mov al, byte [edi]
        mov word [OrderSize], ax
        inc edi

        mov esi, edi
        sub esi, dword [mod_ptr]
        mov word [OrderList], si
        xor ebx, ebx
        mov bl, [edi]
        add bx, bx
        add di, ax

    ; Record table of pattern offsets
        mov esi, edi
        sub esi, dword [mod_ptr]
        mov word [PatternList], si
        mov ax, [edi+ebx]
        mov [PatternPos], ax

    ; Initial pointers
        xor ax, ax
        mov word [OrderPos], ax
        mov byte [mod_speed_counter], al
        mov byte [Line], al

        mov eax, 1
        jmp .lx
    .err:
        mov eax, 0
    .lx:
        pop edi
        pop esi
        pop edx
        pop ecx
        pop ebx
ENDFUNCTION

;##############################################################################################
; unsigned int rad_get_num_orders()
; =------------------------------------=
; Returns number of orders in song
;##############################################################################################

_rad_get_num_orders: FUNCTION
    xor eax, eax;
    mov ax, word [mod_order_size]
ENDFUNCTION

;##############################################################################################
; void rad_end()
; =------------=
; End playback of currently playing module
;##############################################################################################

_rad_end: FUNCTION
    push eax
    mov ax, 0x20 << 8
    .la:
        call Adlib
        inc ah
        cmp ah, 0xF6
        jb .la
    pop eax
ENDFUNCTION

;##############################################################################################
; void rad_playback()
; =-----------------=
; Plays back the currently loaded module
; Must be called exactly 50 times per second or shenanigans happen.
;##############################################################################################
_rad_playback: FUNCTION
    pusha
    xor esi, esi
    xor ebx, ebx
    push edi

    mov edi, [mod_ptr]    

    cmp byte [mod_speed_counter], 0    
    jz .la
    dec byte [mod_speed_counter]
    jmp .lx

; Switch off any effects that are in operation
    .la:
        mov si, 8
        xor al, al
    .laa:
        mov byte [portSlide+esi], al
        mov byte [volSlide+esi], al
        mov byte [toneSlide+esi], al
        dec si
        jns .laa

    ; Playing a new line, PatternPos should have been set-up already
        mov si, word [PatternPos]        
        or si, si
        jz .lb ;; Rest of pattern is blank

        mov al, byte [edi+esi]
        and al, 0x7F
        cmp al, byte [Line]
        jnz .lb

        test byte [edi+esi], 0x80 ;; Check if last line
        jz .lc ; No, need to check more
        mov word [PatternPos], 0 ;; Mark rest of pattern as blank
    
    .lc: inc si ;; Move to first channel
    
    ; Play channels
    .lf: mov cl, byte [edi+esi]
        push cx
        and cl, 0x7F
        mov ax, word [edi+esi+1]
        add si, 3

        test ah, 15
        jz .le ;; No effect parameter
        mov ch, byte [edi+esi]
        inc si
    .le:
        call PlayNote

        pop cx
        jc .lg

        test cl, 0x80
        jz .lf

        mov word [PatternPos], si

    ; Update pointers
    .lb: mov al, byte [mod_speed]
        dec al
        mov byte [mod_speed_counter], al

        inc byte [Line]
        cmp byte [Line], 64
        jb .lx ;; Is end of pattern?

        mov byte [Line], 0 ;; New pattern, start at beginning
        call NextPattern    

    .lx:       
        call UpdateNotes         
    
        pop edi
        popa
        ENDFUNCTION    
    
    .lg: ;; Jump to line AX
        mov bl, byte [mod_speed]
        mov byte [mod_speed_counter], bl

        mov byte [Line], al

        call NextPattern
        jz .lx

    .ll: mov cl, byte [edi+esi]
        and cl, 0x7F
        cmp cl, al
        jae .lh

        test byte [edi+esi], 0x80
        jz .li
        xor si, si
        jmp .lh

        ; Skip to next line definition
    .li: inc si
    .lj: mov cl, byte [edi+esi]
        add si, 3
        test byte [edi+esi-1], 15
        jz .lk
        inc si
    .lk: add cl, cl
        jnc .lj
        jmp .ll
    .lh: mov word [PatternPos], si
        jmp .lx

;##############################################################################################
; INTERNAL: NextPattern
; =-------------------=
; Advances pointers to next pattern in order list
;##############################################################################################
NextPattern:
    push ebx
    xor ebx, ebx
    mov bx, word [OrderPos]
    inc bx
    cmp bx, word [OrderSize]
    jb .ld
    xor ebx, ebx

    ;; Song loops here, call the song_end callback if it's set
    cmp dword [callback_song_end], 0
    mov eax, dword [callback_song_end]
    jz .ld
    call eax

    .ld: mov word [OrderPos], bx        
        mov si, word [OrderList]
        add esi, edi
        mov bl, [esi+ebx]
        sub esi, edi

        test bl, 0x80
        jz .lda
        and bl, 0x7F
        jmp .ld

    .lda: mov si, word [PatternList]
        add bx, bx
        add esi, edi
        mov bx, word [esi+ebx]
        sub esi, edi
        mov word [PatternPos], bx
        or si, si
        
        pop ebx
        ret


;##############################################################################################
; INTERNAL: PlayNote
; =----------------=
; Plays a note on a channel
; IN:
;   AL - Octave (High nibble), Note (Low nibble)
;   AH - Instrument (High nibble), Command (Low Nibble)
;   CL - Channel to play note on (0..8)
;   CH - Parameter byte for command
; OUT:
;   CARRY - set if a line is to be jumped to
;   AX - line to jump to if carry is set
;##############################################################################################
PlayNote:    
    push edi
    xor edi, edi    
    mov di, cx
    xor ecx, ecx
    mov cx, di
    and di, 15
    mov dh, ah
    and dh, 15
    
    or al, al
    jz .lb

    ;; Check to see if we're performing a tone slide
        xor ebx, ebx
        cmp dh, cmToneSlide
        jnz .lt ;; Nope, play note

    ;; Note/octave are used as parameters        
        mov bx, ax
        and bx, 15
        shr al, 4
        and ax, 7
        dec bx
        cmp bx, 12
        jae .lx

        imul ax, FreqRange
        add bx, bx
        add ax, word [NoteFreq+ebx]
        sub ax, FreqStart
        mov byte [toneSlideFreqL+edi], al
        mov byte [toneSlideFreqR+edi], ah

        mov byte [toneSlide+edi], 1
        or ch, ch
        jz .lx
        mov byte [toneSlideSpeed+edi], ch
        jmp .lx

    ; KEY-OFF the previous note
    .lt: push ax
        mov al, byte [oldB0+edi]
        and al, !0x20
        mov byte [oldB0+edi], al
        mov ah, cl
        add ah, 0xB0
        call Adlib
        pop ax

    ; Load instrument (If any)
        mov dl, ah
        add al, al
        rcr dl, 1
        shr dl, 3
        jz .la
        call LoadInst
        
    ; Load note into channel
    .la: mov bl, al
        and bx, 15*2
        cmp bx, 15*2
        jz .lb
      
        mov bx, word [NoteFreq-2, ebx]
        shr al, 3
        and al, 28
        or al, 0x20
        or al, bh
        mov ah, 0xB0
        add ah, cl
        mov byte [oldB0+edi], al
        push ax

        sub ah, 0x10
        mov al, bl
        mov byte [oldA0+edi], al
        call Adlib

        pop ax
        call Adlib


    ; Process command (If any)
    .lb: xor ebx, ebx
        mov bl, dh

        ;jmp [Effects+ebx]
        ; Manually cmp/jmp-ing because NASM's labels are being weird
        cmp bl, 0x00
        je .lx

        cmp bl, 0x01
        je .EffectJumpToLine

        cmp bl, 0x02
        je .EffectPortDown

        cmp bl, 0x03
        je .EffectPortUp

        cmp bl, 0x04
        je .lx

        cmp bl, 0x05
        je .EffectToneVolSlide

        cmp bl, 0x06
        je .lx

        cmp bl, 0x07
        je .lx

        cmp bl, 0x08
        je .lx

        cmp bl, 0x09
        je .lx

        cmp bl, 0x0A
        je .EffectVolSlide

        cmp bl, 0x0B
        je .lx

        cmp bl, 0x0C
        je .EffectSetVolume

        cmp bl, 0x0D
        je .EffectJumpToLine

        cmp bl, 0x0E
        je .lx

        cmp bl, 0x0F
        je .EffectSetSpeed

    .lx: clc
        pop edi
        ret
; =-------------------------------------------------------------------------------------------=
; Effect Procedures (Local to PlayNote)
; =---------------=
; See effect jump-table further below
; =-------------------------------------------------------------------------------------------=

; Portamento Up
; =-----------=
.EffectPortUp:
    mov byte [portSlide+edi], ch
    jmp .lx

; Portamento Down
; =-------------=
.EffectPortDown:
    neg ch
    mov byte [portSlide+edi], ch
    jmp .lx

; Tone Slide
; =--------=
.EffectToneSlide:
    or ch, ch
    jz .lja
    mov byte [toneSlideSpeed+edi], ch

    .lja:
        mov byte [toneSlide+edi], 1
        jmp .lx

; Volume slide Up/Down + Tone Slide
.EffectToneVolSlide:
.EffectVolSlide:
    cmp ch, 50 ;; < 50 == slide down, > 50 slide up
    jb .lga
    sub ch, 50
    neg ch

    .lga:
        mov byte [volSlide+edi], ch
        cmp dh, cmToneVolSlide
        jnz .lx
        mov byte [toneSlide+edi], 1
        jmp .lx

; Set Volume
; =--------=
.EffectSetVolume:
    call SetVolume
    jmp .lx

; Jump to line in next pattern
.EffectJumpToLine:
    cmp ch, 64
    jae .lx ;; Invalid value
    xor ax, ax
    mov al, ch
    stc ;; Set carry flag
    pop edi
    ret ;; Skip other channels

; Set speed
; =-------=
.EffectSetSpeed:
    mov byte [mod_speed], ch
    jmp .lx

; User Callback
; =-----------=
.EffectUser:       
    cmp dword [callback_user_effect], 0
    jz .lx
    push eax
    mov eax, dword [callback_user_effect]
    push ecx
    push ebx
    call eax
    pop ebx
    pop ecx
    pop eax
    jmp .lx

;;##############################################################################################
;; EFFECT JUMP TABLE
;; =---------------=
;; Effects can be directly jumped to using this handy jump table. Effect 0x03 will jump to the code
;; pointed at by entry 3.
;;##############################################################################################
Effects     dd PlayNote.lx                      ; 0xx
            dd PlayNote.EffectPortUp            ; 1xx
            dd PlayNote.EffectPortDown          ; 2xx
            dd PlayNote.EffectToneSlide         ; 3xx
            dd PlayNote.lx ;; An lx is a no-op. ; 4xx
            dd PlayNote.EffectToneVolSlide      ; 5xx
            dd PlayNote.lx                      ; 6xx
            dd PlayNote.lx                      ; 7xx
            dd PlayNote.lx                      ; 8xx
            dd PlayNote.lx                      ; 9xx
            dd PlayNote.EffectVolSlide          ; Axx
            dd PlayNote.lx                      ; Bxx
            dd PlayNote.EffectSetVolume         ; Cxx
            dd PlayNote.EffectJumpToLine        ; Dxx
            dd PlayNote.lx                      ; Exx
            dd PlayNote.EffectSetSpeed          ; Fxx

;##############################################################################################
; INTERNAL: UpdateNotes
; =-------------------=
; Checks for ongoing effects to update
;##############################################################################################
UpdateNotes:
    xor bh, bh
    xor esi, esi

    ; Process portamentos
    .la:
        mov bl, byte [portSlide+esi]
        or bl, bl
        jz .lb
        call GetFreq
        mov ch, bl
        sar cx, 8
        add ax, cx
        call SetFreq
    
    ; Process volume slides
    .lb: mov ch, byte [volSlide+esi]
        cmp ch, 64
        jg .lc

        mov cl, byte [old43+esi]        
        and cl, 0x3F        
        xor cl, 0x3F
        or ch, ch
        jz .lc
        jns .lba

        ; Slide volume up
        sub cl, ch
        cmp cl, 64
        jb .lbb
        mov cl, 63        
        jmp .lbb

        ; Slide volume down
    .lba: sub cl, ch
        jns .lbb
        xor cl, cl
    .lbb: mov ch, cl
        mov cl, bh
        call SetVolume

    ; Process tone-slides
    .lc: cmp byte [toneSlide+esi], 0
        jz .lx
        mov bl, byte [toneSlideSpeed+esi]

        call GetFreq

        mov dh, bl
        sar dx, 8

        mov cl, byte [toneSlideFreqL+esi]
        mov ch, byte [toneSlideFreqR+esi]
        cmp ax, cx

        jz .le
        ja .ld

        ; Doing a tone slide up
        add ax, dx
        cmp ax, cx
        jb .lg
        jmp .le

        ; Doing a tone slide down
    .ld: sub ax, dx
        cmp ax, cx
        ja .lg

        ; Reached destination, stop slide
    .le: mov ax, cx
        mov byte [toneSlide+esi], 0

        ; Write new frequency back to channel
    .lg: call SetFreq
    .lx: inc bh
        inc si
        cmp si, 9
        jb .la

        ret

;##############################################################################################
; INTERNAL: GetFreq
; =---------------=
; Returns the current absolute frequency of a channel
; IN:
;   SI - channel idex
; OUT:
;   AX - frequency value
;############################################################################################## 
GetFreq:     
    mov cl, byte [oldA0+esi]
    mov ch, byte [oldB0+esi]
    and ch, 3
    sub cx, FreqStart
    mov al, byte [oldB0+esi]
    shr al, 2
    and ax, 7
    mov dx, FreqRange
    mul dx
    add ax, cx

    ret

;##############################################################################################
; INTERNAL: SetFreq
; =---------------=
; Sets a channel's frequency
;  AX - absolute frequency
;  SI - channel
; Clobbers:
;  CX, DX
;##############################################################################################
SetFreq:     
    mov cx, FreqRange
    xor dx, dx
    div cx
    add dx, FreqStart

    mov ah, byte [oldB0+esi]
    and ah, 11100000b
    shl al, 2
    or al, ah
    or al, dh
    mov ah, bh
    add ah, 0xB0
    mov byte [oldB0+esi], al
    call Adlib

    sub ah, 0x10
    mov al, dl
    mov byte [oldA0+esi], al    

    jmp Adlib

;##############################################################################################
; INTERNAL: LoadInst
; =----------------=
; Loads Adlib carrier/operator registers from instrument table
;  CL - Channel to load instrument to
;  DL - Instrument idex to pull from table
;##############################################################################################
LoadInst:
    push ax
    push ebx
    push esi
    push edi

    xor esi, esi
    xor edi, edi
    mov di, dx
    mov si, cx
    and si, 0xFF
    mov ah, byte [ChannelOffs+esi]

    xor ebx, ebx
    mov bl, dl
    dec bx
    add bx, bx
    mov bx, word [InstPtrs+ebx]
    or bx, bx
    jz .lx ;; No instrument data

    ;; Calculate absolute offset
    add ebx, dword [mod_ptr]

    mov al, byte [ebx+2]           
    mov byte [old43+esi], al

    mov dl, 4
    
    .la: mov al, byte [ebx+1]
        call Adlib
        add ah, 3
        mov al, byte [ebx]
        call Adlib
        add ebx, 2

        add ah, 0x20-3
        dec dl
        jnz .la

        add ah, 0x40
        mov al, [ebx+2]
        call Adlib
        add ah, 3
        mov al, [ebx+1]
        call Adlib

        mov ah, 0xC0
        add ah, cl
        mov al, [ebx]
        call Adlib   

    .lx: 

    ; End of routine
    pop edi
    pop esi
    pop ebx
    pop ax
    ret

;##############################################################################################
; INTERNAL: SetVolume
; =-----------------=
; Sets the volume of a channel
;  CL - Channel to set volume on
;  CH - Volume value
;############################################################################################## 
SetVolume:
    push eax
    push ebx
    push edx

    xor ebx, ebx
    xor edx, edx
    
    mov bl, cl
    
    ; Ensure volume is within range    
    cmp ch, 64
    jb .la
    mov ch, 63
   
    ; Get old 43xx value
    .la: mov al, byte [old43+ebx]
        and al, 0xC0        
        xor ch, 0x3F        
        or al, ch
        mov byte [old43+ebx], 63

        mov ah, byte [ChannelOffs+ebx]
        add ah, 0x23
        call Adlib

    pop edx
    pop ebx
    pop eax
    ret

;##############################################################################################
; EXTERNAL: void rad_set_volume(unsigned char vol)
; =----------------------------------------------=
; Set volume in a range of 0..63
;##############################################################################################
GLOBAL _rad_set_volume
_rad_set_volume: FUNCTION
    %arg volume:byte
    mov al, byte [volume]
    mov byte [RadVolume], al
ENDFUNCTION

;##############################################################################################
; INTERNAL: adlib_reg
; =-----------------=
; Writes a byte to an adlib register. 
;  AH = Register Number
;  AL = Data
;##############################################################################################

Adlib: 
    push ax
    push cx
    push bx
    push dx

    cmp ah, 0x40
    jae .volume_control
    jmp .ll 
    .volume_control:
        cmp ah, 0x55
        ja .ll

        mov cl, al
        and al, 0xC0
        and cl, 0x3F

        mov ch, byte [RadVolume]
        xor ch, 0x3F
        add ch, cl
        cmp ch, 64
        jmp .vx
        mov ch, 63
    .vx:
        or al, ch        
    .ll:
    mov dx, word [AdlibPort]
    xchg ah, al
    out dx, al
    %rep 6
    in al, dx
    %endrep

    inc dx
    mov al, ah
    out dx, al
    dec dx
    mov ah, 22

    .la:
        in al, dx
        dec ah
        jnz .la
    
    pop dx
    pop bx
    pop cx
    pop ax
    ret
%endif
