;*
;* Memory Manipulation Code
;*
;*   Started 16-Aug-97
;*
;* Initials: JF = Jeff Frohwein, CS = Carsten Sorensen
;*
;* V1.0 - 16-Aug-97 : Original Release - JF, most code from CS
;* V1.1 - 29-Nov-97 : Added monochrome font copy. - JF
;*                    Fixed bug in mem_SetVRAM. - JF
;*
;* Library Subroutines:
;*
;* lcd_WaitVRAM -
;*   Macro that pauses until VRAM available.
;*
;* mem_Set -
;*   Set a memory region.
;*    Entry: a = value, hl = start address, bc = length
;*
;* mem_Copy -
;*   Copy a memory region.
;*    Entry: hl = start address, de = end address, bc = length
;*
;* mem_SetVRAM -
;*   Set a memory region in VRAM.
;*    Entry: a = value, hl = start address, bc = length
;*
;* mem_CopyVRAM -
;*   Copy a memory region to or from VRAM.
;*    Entry: hl = start address, de = end address, bc = length
;*

;If all of these are already defined, don't do it again.

        IF      !DEF(MEMORY1_ASM)
MEMORY1_ASM  SET  1

rev_Check_memory1_asm: MACRO
;NOTE: REVISION NUMBER CHANGES MUST BE ADDED
;TO SECOND PARAMETER IN FOLLOWING LINE.
        IF      \1 > 1.1      ; <---- NOTE!!! PUT FILE REVISION NUMBER HERE
        WARN    "Version \1 or later of 'memory1.asm' is required."
        ENDC
        ENDM

        INCLUDE "hardware.inc"

; Make sure include files are useable revisions.

        rev_Check_hardware_inc   1.5

; Macro that pauses until VRAM available.

lcd_WaitVRAM: MACRO

        ldh     a,[rSTAT]  	    ; <---+
        and     STATF_LCD      ;     |
        jr      z,@-4         ; ----+
        ldh     a,[rSTAT]  	    ; <---+
        and     STATF_LCD      ;     |
        jr      nz,@-4         ; ----+

        ENDM

        SECTION "Memory1 Code",HOME

;***************************************************************************
;*
;* mem_Set - "Set" a memory region
;*
;* input:
;*    a - value
;*   hl - pMem
;*   bc - bytecount
;*
;***************************************************************************
mem_Set::

		inc		b
		inc		c
		jr		.skip
.loop	
		ld		[hl+],a
.skip	
		dec		c
		jr		nz,.loop
		dec		b
		jr		nz,.loop
		ret

;***************************************************************************
;*
;* mem_Copy - "Copy" a memory region
;*
;* input:
;*   hl - pSource
;*   de - pDest
;*   bc - bytecount
;*
;***************************************************************************
mem_Copy::

		inc		b
		inc		c
		jr		.skip
.loop	
		ld		a,[hl+]
		ld		[de],a
		inc		de
.skip	
		dec		c
		jr		nz,.loop
		dec		b
		jr 		nz,.loop
		ret

;***************************************************************************
;*
;* mem_Copy - "Copy" a monochrome font from ROM to RAM
;*
;* input:
;*   hl - pSource
;*   de - pDest
;*   bc - bytecount of Source
;*
;***************************************************************************

mem_CopyMono::

		inc		b
		inc		c
		jr		.skip
.loop		
		ld		a,[hl+]
		ld		[de],a
		inc		de
        ld      [de],a
        inc     de
.skip	
		dec		c
		jr		nz,.loop
		dec		b
		jr		nz,.loop
		ret


;***************************************************************************
;*
;* mem_SetVRAM - "Set" a memory region in VRAM
;*
;* input:
;*    a - value
;*   hl - pMem
;*   bc - bytecount
;*
;***************************************************************************
mem_SetVRAM::
		inc		b
		inc		c
		jr		.skip
.loop   
		push    af
        di
        lcd_WaitVRAM
        pop     af
        ld      [hl+],a
        ei
.skip	
		dec		c
		jr		nz,.loop
		dec		b
		jr		nz,.loop
		ret

;***************************************************************************
;*
;* mem_CopyVRAM - "Copy" a memory region to or from VRAM
;*
;* input:
;*   hl - pSource
;*   de - pDest
;*   bc - bytecount
;*
;***************************************************************************
mem_CopyVRAM::
		inc		b
		inc		c
		jr		.skip
.loop
		di
        lcd_WaitVRAM
        ld      a,[hl+]
		ld		[de],a
        ei
		inc		de
.skip	
		dec		c
		jr		nz,.loop
		dec		b
		jr		nz,.loop
		ret

        ENDC    ;MEMORY1_ASM

