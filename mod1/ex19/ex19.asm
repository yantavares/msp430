;-------------------------------------------------------------------------------
; MSP430 Assembler Code Template for use with TI Code Composer Studio
;
;
;-------------------------------------------------------------------------------
            .cdecls C,LIST,"msp430.h"       ; Include device header file

;-------------------------------------------------------------------------------
            .def    RESET                   ; Export program entry-point to
                                            ; make it known to linker.
;-------------------------------------------------------------------------------
            .text                           ; Assemble into program memory.
            .retain                         ; Override ELF conditional linking
                                            ; and retain current section.
            .retainrefs                     ; And retain any sections that have
                                            ; references to current section.

;-------------------------------------------------------------------------------
RESET       mov.w   #__STACK_END,SP         ; Initialize stackpointer
StopWDT     mov.w   #WDTPW|WDTHOLD,&WDTCTL  ; Stop watchdog timer

;-------------------------------------------------------------------------------
START:
            mov     #0x89AB,	R12
            mov     #STR,		R13
            call    #W16_ASC
            jmp     $
W16_ASC:
		    mov     R12,		R5
		    mov     #4,			R6
		    mov     R13,		R7

		    add		#0x04,		R7
		    mov.b   #0,			0(R7)

W16_ASC_LOOP:
		    mov     R5, 	 	R4
		    and     #0x000F, 	R4

			clrc
		    rra     R5
		    rra     R5
		    rra     R5
		    rra     R5

		    call    #NIB_ASC

		    dec    	R7
		    mov.b   R4,			0(R7)


		    dec     R6
		    jnz     W16_ASC_LOOP
		    ret
NIB_ASC:
            cmp     #0x0A,  	R4
            jl      NIB_ASC_NUM
            add     #0x37,   	R4
            ret
NIB_ASC_NUM:
            add     #0x30,   	R4
            ret


;-------------------------------------------------------------------------------
            .data
STR         .space  4

;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack

;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"
            .short  RESET
