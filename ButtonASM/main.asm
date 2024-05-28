
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

            call    #io_config              ; Configure I/O

MainLoop    bit.b   #BIT1, &P2IN            ; Check if P2.1 is high
            jnz     MainLoop                ; If high, loop (button not pressed)
            call    #debounce               ; Debounce delay
            inc     R4                      ; Increment count
            call    #count_leds             ; Update LEDs
WaitRelease bit.b   #BIT1, &P2IN            ; Check if P2.1 is low
            jz      WaitRelease             ; If low, wait (button pressed)
            call    #debounce               ; Debounce delay
            jmp     MainLoop                ; Repeat

;-------------------------------------------------------------------------------
; count_leds function
;-------------------------------------------------------------------------------
count_leds  push.w  R5                      ; Save R5
            mov.w   R4, R5                  ; Move count to R5
            and.w   #3, R5                  ; Mask count to 0-3
            cmp.w   #0, R5
            jeq     ClrBoth                 ; If 0, clear both LEDs
            cmp.w   #1, R5
            jeq     SetP47                  ; If 1, set P4.7
            cmp.w   #2, R5
            jeq     SetP10                  ; If 2, set P1.0
            bis.b   #BIT0, &P1OUT           ; If 3, set both LEDs
            bis.b   #BIT7, &P4OUT
            jmp     EndCount
ClrBoth     bic.b   #BIT0, &P1OUT           ; Clear both LEDs
            bic.b   #BIT7, &P4OUT
            jmp     EndCount
SetP47      bic.b   #BIT0, &P1OUT           ; Clear P1.0, set P4.7
            bis.b   #BIT7, &P4OUT
            jmp     EndCount
SetP10      bis.b   #BIT0, &P1OUT           ; Set P1.0, clear P4.7
            bic.b   #BIT7, &P4OUT
EndCount    pop.w   R5                      ; Restore R5
            ret

;-------------------------------------------------------------------------------
; debounce function
;-------------------------------------------------------------------------------
debounce    mov.w   #1000, R5               ; Load debounce count
DebounceLoop dec.w  R5                      ; Decrement debounce counter
            jnz     DebounceLoop            ; Loop until counter is zero
            ret

;-------------------------------------------------------------------------------
; io_config function
;-------------------------------------------------------------------------------
io_config   bis.b   #BIT0, &P1DIR           ; Set P1.0 as output
            bic.b   #BIT0, &P1OUT           ; Clear P1.0

            bis.b   #BIT7, &P4DIR           ; Set P4.7 as output
            bic.b   #BIT7, &P4OUT           ; Clear P4.7

            bic.b   #BIT1, &P2DIR           ; Set P2.1 as input
            bis.b   #BIT1, &P2REN           ; Enable pull-up/pull-down resistor on P2.1
            bis.b   #BIT1, &P2OUT           ; Set P2.1 pull-up resistor

            ret

;-------------------------------------------------------------------------------
; Stack Pointer definition
;-------------------------------------------------------------------------------
            .global __STACK_END
            .sect   .stack

;-------------------------------------------------------------------------------
; Interrupt Vectors
;-------------------------------------------------------------------------------
            .sect   ".reset"                ; MSP430 RESET Vector
            .short  RESET
