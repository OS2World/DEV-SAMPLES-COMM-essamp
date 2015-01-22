.286P

;*************************************************************************
;*                                                                       *
;*   MODULE NAME: ACDIMXMT.ASM                                           *
;*                                                                       *
;*   DESCRIPTIVE NAME: ACDI MASM SAMPLE TRANSMIT PROGRAM                 *
;*                     OS/2 EXTENDED SERVICES                            *
;*                                                                       *
;*   COPYRIGHT:  (C) COPYRIGHT IBM CORP. 1988, 1990                      *
;*               LICENSED MATERIAL - PROGRAM PROPERTY OF IBM             *
;*               ALL RIGHTS RESERVED                                     *
;*                                                                       *
;*   STATUS:   LPP Release 1.0 Modification 0                            *
;*                                                                       *
;*   FUNCTION: The sample program will use the ACDI interface to echo    *
;*             line by line screen image from the first Personal         *
;*             Computer on the second Personal Computer connected        *
;*             through asynchronous line.                                *
;*             The TRANSMIT PROGRAM, is executed in the first Personal   *
;*             Computer.                                                 *
;*                                                                       *
;*             Uses the following ACDI Verbs:                            *
;*                                                                       *
;*                COMOPEN                                                *
;*                COMDEFOUTPUTBUFF                                       *
;*                COMDEFINPUT                                            *
;*                COMSETBITRATE                                          *
;*                COMSETLINECTRL                                         *
;*                COMCONNECT                                             *
;*                COMSETTIMEOUTS                                         *
;*                COMWRITECHARSTRING                                     *
;*                COMFLUSHOUTPUT                                         *
;*                COMSENDBREAK                                           *
;*                COMDISCONNECT                                          *
;*                COMCLOSE                                               *
;*                                                                       *
;*   NOTE:     The asynchronous device name to be used in this           *
;*             program is COM1. If this is to be changed it must         *
;*             be changed in the main header.                            *
;*                                                                       *
;*             This program is designed to run with connect type 4       *
;*             only.                                                     *
;*                                                                       *
;*   MODULE TYPE = IBM Personal Computer Macro Assembler/2 Version 1.00  *
;*                                                                       *
;*   PREREQS = Requires Message file "ACX.MSG" at runtime.               *
;*                                                                       *
;*************************************************************************

           IF1
            INCLUDE ACDI_A.INC              ;acdi include file
            INCLUDE DOSCALLS.INC            ;other dos files
            INCLUDE SUBCALLS.INC
           ENDIF

@MOVW      MACRO     DESTINATION,SOURCE
           MOV       AX,SOURCE
           MOV       DESTINATION,AX
           ENDM

@MOVB      MACRO     DESTINATION,SOURCE
           MOV       AL,SOURCE
           MOV       DESTINATION,AL
           ENDM

@ACDI      MACRO     VCB                    ;a macro to define and call acdi
           @DEFINE   ACDI
           PUSH      DS
           PUSH      OFFSET DGROUP:VCB
           CALL      FAR PTR ACDI
           ENDM

DGROUP          GROUP                DATA

;*****************************************************************************

STACK      SEGMENT WORD STACK 'STACK'       ;Stack segment for main process.
           DW      4096 DUP (?)
STACK      ENDS
;*****************************************************************************

DATA       SEGMENT WORD PUBLIC 'DATA'       ;Data segment.

SYS_ERR        DB       0                   ;system error flag
HANDLE         DW       0                   ;com device handle
DEV_NAME       DB       'COM1\0',0          ;name of com device to be used

OUT_BUFF_LEN   EQU      82                  ;length of output buffer
IN_BUFF_LEN    EQU      82                  ;length of input buffer

I_ANBPTR_SEL   DW       0                   ;selector returned by DOSALLOCSEG
                                            ;for input buffer
O_ANBPTR_SEL   DW       0                   ;selector returned by DOSALLOCSEG
                                            ;for output buffer

CONCTTIMEOUT1  EQU      0                   ;timeout values for the
CONCTTIMEOUT2  EQU      30                  ;comconnect verb

INF_RDBLKTMOUT EQU      0                   ;timeout parameters for the
INF_RDCHRTMOUT EQU      0                   ;verb -
INF_WRTTMOUT   EQU      0                   ;comsettimeouts

MINDURATION    EQU      20                  ;minimum length or duration for the
                                            ;break signal

BYTESWRITTEN   DW       0                   ;parameters for comwritecharstring
BYTESNEEDED    EQU      81                  ;verb

;*****************************************************************************

NULL           DB       20H                 ;null character
RET_CODE       DW       0                   ;area for function and return
FUNC_CODE      DB       0                   ;codes from acdi

R_C_HX         DB       '    ',0DH,0AH      ;return code in printable hex
R_C_HX_LEN     EQU      $-R_C_HX            ;length of printable return code

F_C_HX         DB       '  ',0DH,0AH        ;function code in printable hex
F_C_HX_LEN     EQU      $-F_C_HX            ;length of printable function code

GETMSERR       DB       'Unable to process message file - ACX.MSG',0DH,0AH
GETMSERR_LEN   EQU      $-GETMSERR          ;message to be printed in case of
                                            ;an error in DOSGETMESSAGE

DOS_RC         DW       0                   ;return code from dos calls
DOS_RC_HX      DB       '    ',0DH,0AH      ;dos return codes in printable hex
DOS_RC_HX_LEN  EQU      $-DOS_RC_HX         ;and its length

VIO_RC         DW       0                   ;return code from vio calls
VIO_RC_HX      DB       '    ',0DH,0AH      ;vio return codes in printable hex
VIO_RC_HX_LEN  EQU      $-VIO_RC_HX         ;and its length

KRD_RC         DW       0                   ;return code from keyboard calls
KRD_RC_HX      DB       '    ',0DH,0AH      ;keyboard return codes in printable
KRD_RC_HX_LEN  EQU      $-KRD_RC_HX         ;hex and its length

ZERO           DD       0
MSG_FILE       DB       'ACX.MSG',0         ;file containing messages
MSG_BUFF       DB       100 DUP  (?)        ;buffer for messages
MSG_NO         DW       0                   ;message to load and display
MSG_LEN        DW       0                   ;length of message

HEXTAB         DB       '0123456789ABCDEF'  ;table for hex conversion

;*****************************************************************************

ACTION         EQU      1                   ;parameters for DOSEXIT
RESULT_CODE    DW       0

SLEEPTIME      DD       3000                ;parameter for DOSSLEEP

;*****************************************************************************

ROW            DW       0                   ;row and column numbers used in
COL            DW       0                   ;screen displays
CH_ATR         DW       720H

CAR_RET        EQU      0DH                 ;carriage return
ESCAPE         EQU      1BH                 ;eacape
INDEX          EQU      80
CHARTOBESENT   DB       0                   ;variable to be used in keyboard
                                            ;function to get character which
                                            ;is to be sent
CHARDATA        DB       10 DUP (0)         ;data array to be used in
                                            ;keyboard function

;*****************************************************************************
;   VERB CONTROL BLOCK
;*****************************************************************************

OPN            COMOPEN_CB               <>  ;use one VCB with structs
               ORG              OPN         ;defined over the same
DFOTBFF        COMDEFOUTPUTBUFF_CB      <>  ;storage
               ORG              OPN
DFINBFF        COMDEFINPUT_CB           <>
               ORG              OPN
BITRT          COMSETBITRATE_CB         <>
               ORG              OPN
LINCTRL        COMSETLINECTRL_CB        <>
               ORG              OPN
CONCT          COMCONNECT_CB            <>
               ORG              OPN
TIMOUTS        COMSETTIMEOUTS_CB        <>
               ORG              OPN
WRTCHRSTRG     COMWRITECHARSTRING_CB    <>
               ORG              OPN
FLSHOUT        COMFLUSHOUTPUT_CB        <>
               ORG              OPN
SNDBRK         COMSENDBREAK_CB          <>
               ORG              OPN
DSCONCT        COMDISCONNECT_CB         <>
               ORG              OPN
CLOS           COMCLOSE_CB              <>
               ORG              OPN
VCB            DB               100 DUP (0)
VCB_LEN        EQU              $-VCB       ;size of VCB

DATA           ENDS                         ;end of data segment.

;*****************************************************************************

CSEG       SEGMENT PUBLIC 'CODE'
           ASSUME   CS:CSEG, DS:DGROUP

XMIT       PROC       FAR

;This is the main process. It calls subroutines to issue acdi verbs, calls
;SENDLINES process to send the message line by line.

           MOV          MSG_NO,1            ;msg-no 1 is "ACDI sample
           CALL         SHOW_MSG            ;transmit program

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP2               ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP2:
           CALL         OPEN                ;call subr. to issue comopen

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP3               ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP3:
           @DOSALLOCSEG OUT_BUFF_LEN,O_ANBPTR_SEL,0
                                            ;Doscall to get an output buffer.
           CMP          AX,0                ;check return code
           JE           STEP4               ;if successful jump to next step
           MOV          DOS_RC,AX           ;otherwise copy the return code,
           CALL         SHOW_DOS_ERR        ;show the error, set system error;
           MOV          SYS_ERR,1           ;end the program
           JMP          ENDD
STEP4:
           CALL         DEFOUTBUFF          ;issue comdefoutputbuff

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP5               ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP5:
           @DOSALLOCSEG IN_BUFF_LEN,I_ANBPTR_SEL,0
                                            ;Doscall to get the input buffer.
           CMP          AX,0                ;check return code
           JE           STEP6               ;if successful jump to next step
           MOV          DOS_RC,AX           ;otherwise copy the return code,
           CALL         SHOW_DOS_ERR        ;show the error, set system error;
           MOV          SYS_ERR,1           ;end the program
           JMP          ENDD
STEP6:
           CALL         DEFINBUFF           ;issue comdefinput

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP7               ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP7:
           CALL         SETBITRTE           ;issue comsetbitrate

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP8               ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP8:
           CALL         SETLINCTRL          ;issue comsetlinectrl

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP9               ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP9:
           CALL         SETTIMOUTS          ;issue comsettimouts

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP10              ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP10:
           CALL         CONNECT             ;issue comconnect

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP11              ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP11:
           CALL         SENDLINES           ;call subr. to get message from
                                            ;keyboard, display it on screen
                                            ;and send it
           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP12              ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP12:
           CALL         SENDBREAK           ;issue comsendbreak

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP13              ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP13:

           @DOSSLEEP    SLEEPTIME           ;doscall to waste some time
           CALL         DISCONNECT          ;issue comdisconnect

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP14              ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP14:
           CALL         CLOSE               ;issue comclose
ENDD:
           @DOSEXIT     ACTION,RESULT_CODE  ;doscall to exit

XMIT       ENDP                             ;end of the main program.


SENDLINES  PROC         NEAR

;  This subroutine will clear the screen, wait for user to type in the
;  message, echo it back on the screen, issue comwritecharstring for
;  every line of message to send the message across line by line. Escape
;  key is end of message indicator

           PUSHA                            ;push work registers
           CALL         CLR_SCRN            ;clear screen

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           LOOP1               ;error) jump to next step; else
           JMP          RETT1               ;end the program

LOOP1:     CALL         SET_CRSR_POS        ;set cursor position

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           SENDSTEP1           ;error) jump to next step; else
           JMP          RETT1               ;end the program
SENDSTEP1:
           MOV          CX,INDEX
           MOV          DI,0                ;set DI to 0
           MOV          ES,O_ANBPTR_SEL     ;ES=sel. value from DOSALLOCSEG

LOOP2:     CALL         GET_CHAR_TOBE_SENT  ;subr. to get character from keyb.

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           SENDSTEP2           ;error) jump to next step; else
           JMP          RETT1               ;end the program
SENDSTEP2:
           @MOVB        CHARTOBESENT,CHARDATA
                                            ;move into CHARTOBESENT the
                                            ;character returned by subr.
           CMP          CHARTOBESENT,1BH    ;compare it with escape
           JNE          SENDSTEP3           ;if equal, jump to loop1exit
           JMP          LOOP1EXIT
SENDSTEP3:
           CMP          CHARTOBESENT,0DH    ;compare it with carriage ret.
           JNE          SENDSTEP4
           JMP          LOOP2EXIT           ;if equal, jump to loop2exit
SENDSTEP4:
           CALL         WRT_CHAR_ON_SCRN    ;otherwise call subr. to write
                                            ;character on screen
           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           SENDSTEP5           ;error) jump to next step; else
           JMP          RETT1               ;end the program
SENDSTEP5:
           CMP          COL,79
           JE           NO_COL_INC
           INC          COL                 ;increment column
NO_COL_INC:
           CALL         SET_CRSR_POS        ;set cursor to new position

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           SENDSTEP6           ;error) jump to next step; else
           JMP          RETT1               ;end the program
SENDSTEP6:
           @MOVB        ES:[DI],CHARTOBESENT
                                            ;write character in output buff
           INC          DI                  ;inc. DI to point to next byte
           DEC          CX                  ;repeat untill carriage return
           JNZ          LOOP2               ;or index (CX) = 0
           JMP          LOOP2EXIT

LOOP2EXIT:
           MOV          BYTE PTR ES:[DI],CAR_RET
                                            ;append carriage return in the
           INC          DI                  ;output buff
           MOV          BYTESWRITTEN,DI     ;set byteswritten=DI
           CALL         WRITCHARSTR         ;issue comwritecharstring

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           SENDSTEP7           ;error) jump to next step; else
           JMP          RETT1               ;end the program
SENDSTEP7:
           CALL         FLUSHOUTPUT         ;issue flushoutput

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           SENDSTEP8           ;error) jump to next step; else
           JMP          RETT1               ;end the program
SENDSTEP8:

           INC          ROW                 ;inc. row
           MOV          COL,0               ;set col to 0
           CMP          ROW,24              ;compare row to 24
           JE           SENDSTEP9
           JMP          LOOP1               ;if less, repeat loop1
SENDSTEP9:
           CALL         SCROL_SCRN          ;otherwise scroll screen,
           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           SENDSTEP10          ;error) jump to next step; else
           JMP          RETT1               ;end the program
SENDSTEP10:
           MOV          ROW,23              ;set row to 23 and.....
           JMP          LOOP1               ;repeat loop1 for next line
LOOP1EXIT:
RETT1:
           POPA                             ;restore work registers
           RET                              ;return

SENDLINES  ENDP                             ;end of SENDLINES

;*****************************************************************************
; UTILITY PROCESSES
;*****************************************************************************

CLEAR_VCB    PROC       NEAR
;Writes 0's in the control block. This is done before the control block is
;built for each acdi verb because the same memory area is used for control
;blocks for every acdi verb.

           PUSHA
           XOR          DI,DI
LOOP_CB:   MOV          BYTE PTR VCB[DI],0
           INC          DI
           CMP          DI,VCB_LEN
           JL           LOOP_CB
           POPA
           RET
CLEAR_VCB    ENDP

SHOW_ERR     PROC       NEAR
;  This function shows errors relating to ACDI verbs.

           PUSHA
           MOV          MSG_NO,2            ;msg number 2 is "ACDI verb in
                                            ;error - function code =    "
           CALL         SHOW_MSG            ; call proc to show msg_no 2
           MOV          AH,0
           MOV          AL,FUNC_CODE        ;set parameters for CVHEX proc
           LEA          DI,F_C_HX
           CALL         CVHEX               ;proc to convert hex data into
                                            ;printable hex
           @VIOWRTTTY F_C_HX,F_C_HX_LEN,0 ;vio call to write on screen

           MOV          MSG_NO,3            ;msg number 3 is "return code =  "
           CALL         SHOW_MSG
           MOV          AX,RET_CODE
           LEA          DI,R_C_HX
           CALL         CVHEX
           @VIOWRTTTY R_C_HX,R_C_HX_LEN,0
           POPA
           RET
SHOW_ERR     ENDP

SHOW_DOS_ERR  PROC
;  This function shows errors relating to DOS function calls.

           PUSHA
           MOV          MSG_NO,4            ;msg number 4 is "DOSCALL
                                            ;in error - return code =    "
           CALL         SHOW_MSG            ;call proc to show msg no 3
           LEA          DI,DOS_RC_HX        ;set parameters for cvhex proc
           CALL         CVHEX               ;proc to convert hex data into
                                            ;printable hex
           @VIOWRTTTY DOS_RC_HX,DOS_RC_HX_LEN,0
                                            ;vio call to write on screen
           POPA
           RET
SHOW_DOS_ERR  ENDP

SHOW_VIO_ERR  PROC
;  This function shows errors relating to Vio function calls.

           PUSHA
           MOV          MSG_NO,5            ;msg number 5 is "VIOCALL
                                            ;in error - return code =    "
           CALL         SHOW_MSG            ;call proc to show msg no 5
           LEA          DI,VIO_RC_HX        ;set parameters for cvhex proc
           CALL         CVHEX               ;the convert proc
           @VIOWRTTTY VIO_RC_HX,VIO_RC_HX_LEN,0
                                            ;vio call to write on screen
           POPA
           RET
SHOW_VIO_ERR  ENDP

SHOW_KRD_ERR  PROC
;This process shows errors relating to Kbd function calls.

           PUSHA
           MOV          MSG_NO,6            ;msg number 6 is "KRDCALL
                                            ;in error - return code =    "
           CALL         SHOW_MSG            ;call proc to show msg no 6
           LEA          DI,KRD_RC_HX        ;set parameters for cvhex proc
           CALL         CVHEX               ;the convert proc
           @VIOWRTTTY KRD_RC_HX,KRD_RC_HX_LEN,0
                                            ;vio call to write on screen
           POPA
           RET
SHOW_KRD_ERR  ENDP

SHOW_MSG     PROC       NEAR
;  This function displays error messages using argument MSGNO ( message
;  number) from the message file "ACX.MSG"

           PUSHA
           @DOSGETMESSAGE ZERO,0,MSG_BUFF,100,MSG_NO,MSG_FILE,MSG_LEN
                                            ;doscall to get mes from msg file
           CMP          AX,0                ;check return code
           JNE          JUMP                ;show DOSGETMESSAGE in error
                                            ;message if not zero
           @VIOWRTTTY MSG_BUFF,MSG_LEN,0    ;otherwise display message from
                                            ;the message buffer
           JMP          RETT7
JUMP:
           @VIOWRTTTY GETMSERR,GETMSERR_LEN,0
RETT7:
           POPA
           RET
SHOW_MSG     ENDP

CVHEX        PROC       NEAR
;This proc. converts function codes and return codes into printable hex form

           PUSH         BX                  ;save work regs
           PUSH         CX
           MOV          BX,AX               ;get hex data
           AND          BX,000FH            ;cleanup to get just 1st nibble
           MOV          CL,HEXTAB[BX]       ;lookup the ASCII representation
           MOV          [DI]+3,CL           ;put it into the output area
           MOV          BX,AX               ;get the original data again
           AND          BX,00F0H            ;cleanup to get the 2nd nibble
           MOV          CL,04               ;shift it over for lookup index
           SHR          BX,CL
           MOV          CL,HEXTAB[BX]       ;lookup the ASCII representation
           MOV          [DI]+2,CL           ;put it into the result
           MOV          BX,AX               ;get the original again
           AND          BX,0F00H            ;3rd nibble
           MOV          CL,08               ;shift it
           SHR          BX,CL
           MOV          CL,HEXTAB[BX]       ;lookup the ASCII rep.
           MOV          [DI]+1,CL           ;into result
           MOV          BX,AX
           AND          BX,0F000H           ;4th nibble
           MOV          CL,12               ;shift
           SHR          BX,CL
           MOV          CL,HEXTAB[BX]       ;lookup
           MOV          [DI],CL             ;save result
           POP          CX                  ;restore regs & exit
           POP          BX
           RET
CVHEX        ENDP

;*****************************************************************************
;   ACDI RELATED PROC'S
;*****************************************************************************

;To issue an acdi verb the program has to build the control block structure
;with the parameters and pass the pointer to the control block to the
;acdi subsystem. Each of the following subroutines, when called, will build
;the control block structure and then call acdi. When the acdi subsystem
;returns the subroutine will check the return code, call SHOW_ERR process
;if the return code is bad, otherwise return to the calling process.

OPEN       PROC         NEAR
; This proceduere issue com_open verb to open the specified device for
; communication.

           PUSHA
           CALL         CLEAR_VCB           ;zero out the control block

           MOV          OPN.FUNCTION_CODE,COM_OPEN
                                            ;verb - com_open
           LEA          SI,DEV_NAME         ;copy device name in control blk.
           LEA          DI,OPN.COM_DEV_NAME
           MOV          AX,DS
           MOV          ES,AX
           CLD
           MOV          CX,4
LOOP_COM:
           MOVSB
           LOOP         LOOP_COM

           @ACDI        VCB                 ;issue the verb

           @MOVW        HANDLE,OPN.COM_DEV_HANDLE
                                            ;copy device handle returned by
                                            ;acdi for use in other verbs
           CMP          OPN.RETURN_CODE,0   ;check if return code is zero
           JE           RETT8               ;return if it is; if not,.....
           @MOVW        RET_CODE,OPN.RETURN_CODE
           @MOVB        FUNC_CODE,OPN.FUNCTION_CODE
           CALL         SHOW_ERR            ;copy ret and func codes, show
           MOV          SYS_ERR,1H          ;error, and set system err flag
RETT8:
           POPA
           RET
OPEN       ENDP

DEFOUTBUFF PROC         NEAR
; This subroutine will issue com_def_output_buff to define output buffer.

           PUSHA
           CALL         CLEAR_VCB           ;zero out the control block

           @MOVW        DFOTBFF.COM_DEV_HANDLE,HANDLE
           MOV          DFOTBFF.FUNCTION_CODE,COM_DEF_OUTPUT_BUFF
           MOV          AX,O_ANBPTR_SEL
           MOV          WORD PTR DFOTBFF.OUTPUT_BUFF+2,AX
           SUB          AX,AX
           MOV          WORD PTR DFOTBFF.OUTPUT_BUFF,AX
           @MOVW        DFOTBFF.OUT_BUFF_LENGTH,OUT_BUFF_LEN
                                            ;fill in the control block with
                                            ;function code and parameters
           @ACDI        VCB                 ;issue the verb

           MOV          AX,DFOTBFF.RETURN_CODE
           CMP          AX,0
           JZ           RETT9
           MOV          RET_CODE,AX
           @MOVB        FUNC_CODE,DFOTBFF.FUNCTION_CODE
           CALL         SHOW_ERR            ;check return code and handle
           MOV          SYS_ERR,1H          ;any errors
RETT9:
           POPA
           RET
DEFOUTBUFF ENDP

DEFINBUFF  PROC         NEAR
; This subroutine will issue com_def_input verb to define input buffer.

           PUSHA
           CALL         CLEAR_VCB           ;zero out the control block

           @MOVW        DFINBFF.COM_DEV_HANDLE,HANDLE
           MOV          DFINBFF.FUNCTION_CODE,COM_DEF_INPUT
           MOV          AX,I_ANBPTR_SEL
           MOV          WORD PTR DFINBFF.INPUT_BUFF+2,AX
           SUB          AX,AX
           MOV          WORD PTR DFINBFF.INPUT_BUFF,AX
           @MOVW        DFINBFF.IN_BUFF_LENGTH,IN_BUFF_LEN
           MOV          DFINBFF.INPUT_MODE,AA_CHAR_MODE
                                            ;fill in the control block with
                                            ;function code and parameters
           @ACDI        VCB                 ;issue the verb

           MOV          AX,DFINBFF.RETURN_CODE
           CMP          AX,0
           JZ           RETTA
           MOV          RET_CODE,AX
           @MOVB        FUNC_CODE,DFINBFF.FUNCTION_CODE
           CALL         SHOW_ERR            ;check return code and handle
           MOV          SYS_ERR,1H          ;any errors
RETTA:
           POPA
           RET
DEFINBUFF  ENDP

SETBITRTE  PROC         NEAR
; This subroutine will issue com_set_bit_rate verb to set up the line
; data rates (bps).

           PUSHA
           CALL         CLEAR_VCB           ;zero out the control block

           @MOVW        BITRT.COM_DEV_HANDLE,HANDLE
           MOV          BITRT.FUNCTION_CODE,COM_SET_BIT_RATE
           @MOVW        BITRT.BIT_RATE_RCV,AA_300_BPS
           @MOVW        BITRT.BIT_RATE_SEND,AA_300_BPS
                                            ;fill in the control block with
                                            ;function code and parameters
           @ACDI        VCB                 ;issue the verb

           MOV          AX,BITRT.RETURN_CODE
           CMP          AX,0
           JZ           RETTB
           MOV          RET_CODE,AX
           @MOVB        FUNC_CODE,BITRT.FUNCTION_CODE
           CALL         SHOW_ERR            ;check return code and handle
           MOV          SYS_ERR,1H          ;any errors
RETTB:
           POPA
           RET
SETBITRTE  ENDP

SETLINCTRL PROC         NEAR
; This subroutine will issue com_set_line_ctrl to set up line control
; values.

           PUSHA
           CALL         CLEAR_VCB           ;zero out the control block

           @MOVW        LINCTRL.COM_DEV_HANDLE,HANDLE
           MOV          LINCTRL.FUNCTION_CODE,COM_SET_LINE_CTRL
           MOV          AX,0
           MOV          LINCTRL.STOP_BITS,AA_1_STOP_BIT
           MOV          LINCTRL.PARITY,AA_EVEN_PARITY
           MOV          LINCTRL.DATA_BITS,AA_7_DATA_BITS
                                            ;fill in the control block with
                                            ;function code and parameters
           @ACDI        VCB                 ;issue the verb

           MOV          AX,LINCTRL.RETURN_CODE
           CMP          AX,0
           JZ           RETTC
           @MOVB        FUNC_CODE,LINCTRL.FUNCTION_CODE
           CALL         SHOW_ERR            ;check return code and handle
           MOV          SYS_ERR,1H          ;any errors
RETTC:
           POPA
           RET
SETLINCTRL ENDP

CONNECT    PROC         NEAR
; This subroutine will issue com_connect to establish connection.

           PUSHA
           CALL         CLEAR_VCB           ;zero out the control block

           @MOVW       CONCT.COM_DEV_HANDLE,HANDLE
           MOV         CONCT.FUNCTION_CODE,COM_CONNECT
           MOV         CONCT.CONNECT_TYPE,AA_CONNECT_TYPE_4
           MOV         CONCT.CONNECT_TIMEOUT_1,CONCTTIMEOUT1
           MOV         CONCT.CONNECT_TIMEOUT_2,CONCTTIMEOUT2
                                            ;fill in the control block with
                                            ;function code and parameters
           @ACDI        VCB                 ;issue the verb

           MOV         AX,CONCT.RETURN_CODE
           CMP         AX,0
           JE          RETTD
           MOV         RET_CODE,AX
           @MOVB       FUNC_CODE,CONCT.FUNCTION_CODE
           CALL         SHOW_ERR            ;check return code and handle
           MOV          SYS_ERR,1H          ;any errors
RETTD:
           POPA
           RET
CONNECT    ENDP

SETTIMOUTS PROC         NEAR
; This subroutine will issue com_set_timeouts to set the timeout values
; for read and write.

           PUSHA
           CALL         CLEAR_VCB           ;zero out the control block

           @MOVW        TIMOUTS.COM_DEV_HANDLE,HANDLE
           MOV          TIMOUTS.FUNCTION_CODE,COM_SET_TIMEOUTS
           MOV          TIMOUTS.READ_TIMEOUT_BLOCK,INF_RDBLKTMOUT
           MOV          TIMOUTS.READ_TIMEOUT_CHAR,INF_RDCHRTMOUT
           MOV          TIMOUTS.WRITE_TIMEOUT,INF_WRTTMOUT
                                            ;fill in the control block with
                                            ;function code and parameters
           @ACDI        VCB                 ;issue the verb

           MOV          AX,TIMOUTS.RETURN_CODE
           CMP          AX,0
           JE           RETTE
           MOV          RET_CODE,AX
           @MOVB        FUNC_CODE,TIMOUTS.FUNCTION_CODE
           CALL         SHOW_ERR            ;check return code and handle
           MOV          SYS_ERR,1H          ;any errors
RETTE:
           POPA
           RET
SETTIMOUTS ENDP

WRITCHARSTR   PROC      NEAR
; This subroutine will issue com_write_char_string verb to write data
; in output buffer to the communication device.

           PUSHA
           CALL         CLEAR_VCB           ;zero out the control block

           @MOVW        WRTCHRSTRG.COM_DEV_HANDLE,HANDLE
           MOV          WRTCHRSTRG.FUNCTION_CODE,COM_WRITE_CHAR_STRING
           @MOVW        WRTCHRSTRG.BYTES_WRITTEN,BYTESWRITTEN
           @MOVW        WRTCHRSTRG.WRITE_BYTES_NEEDED,BYTESNEEDED
                                            ;fill in the control block with
                                            ;function code and parameters
           @ACDI        VCB                 ;issue the verb

           MOV          AX,WRTCHRSTRG.RETURN_CODE
           CMP          AX,0
           JE           RETTF
           MOV          RET_CODE,AX
           @MOVB        FUNC_CODE,WRTCHRSTRG.FUNCTION_CODE
           CALL         SHOW_ERR            ;check return code and handle
           MOV          SYS_ERR,1H          ;any errors
RETTF:
           POPA
           RET
WRITCHARSTR   ENDP

FLUSHOUTPUT   PROC      NEAR
; This subroutine will issue com_flush_output verb to set next available
; write pointer to output buffer pointer and next available write length
; to size of output buffer.

           PUSHA
           CALL         CLEAR_VCB           ;zero out the control block

           @MOVW        FLSHOUT.COM_DEV_HANDLE,HANDLE
           MOV          FLSHOUT.FUNCTION_CODE,COM_FLUSH_OUTPUT
                                            ;fill in the control block with
                                            ;function code and parameters
           @ACDI        VCB                 ;issue the verb

           MOV          AX,FLSHOUT.RETURN_CODE
           CMP          AX,0
           JE           RETTG
           MOV          RET_CODE,AX
           @MOVB        FUNC_CODE,FLSHOUT.FUNCTION_CODE
           CALL         SHOW_ERR            ;check return code and handle
           MOV          SYS_ERR,1H          ;any errors
RETTG:
           POPA
           RET
FLUSHOUTPUT   ENDP

SENDBREAK  PROC         NEAR
; This subroutine will issue com_send_break verb to signal to receiver the
; end of session.

           PUSHA
           CALL         CLEAR_VCB           ;zero out the control block

           @MOVW        SNDBRK.COM_DEV_HANDLE,HANDLE
           MOV          SNDBRK.FUNCTION_CODE,COM_SEND_BREAK
           MOV          SNDBRK.MIN_DURATION,MINDURATION
                                            ;fill in the control block with
                                            ;function code and parameters
           @ACDI        VCB                 ;issue the verb

           MOV          AX,SNDBRK.RETURN_CODE
           CMP          AX,0
           JE           RETTH
           MOV          RET_CODE,AX
           @MOVB        FUNC_CODE,SNDBRK.FUNCTION_CODE
           CALL         SHOW_ERR            ;check return code and handle
           MOV          SYS_ERR,1H          ;any errors
RETTH:
           POPA
           RET
SENDBREAK  ENDP

DISCONNECT    PROC      NEAR
; This subroutine will issue com_disconnect to break the connection.

           PUSHA
           CALL         CLEAR_VCB           ;zero out the control block

           @MOVW        DSCONCT.COM_DEV_HANDLE,HANDLE
           MOV          DSCONCT.FUNCTION_CODE,COM_DISCONNECT
                                            ;fill in the control block with
                                            ;function code and parameters
           @ACDI        VCB                 ;issue the verb

           MOV          AX,DSCONCT.RETURN_CODE
           CMP          AX,0
           JE           RETTI
           MOV          RET_CODE,AX
           @MOVB        FUNC_CODE,DSCONCT.FUNCTION_CODE
           CALL         SHOW_ERR            ;check return code and handle
           MOV          SYS_ERR,1H          ;any errors
RETTI:
           POPA
           RET
DISCONNECT    ENDP

CLOSE      PROC         NEAR
; This subroutine will issue com_close to close the communication device.

           PUSHA
           CALL         CLEAR_VCB           ;zero out the control block

           @MOVW        CLOS.COM_DEV_HANDLE,HANDLE
           MOV          CLOS.FUNCTION_CODE,COM_CLOSE
                                            ;fill in the control block with
                                            ;function code and parameters
           @ACDI        VCB                 ;issue the verb

           MOV          AX,CLOS.RETURN_CODE
           CMP          AX,0
           JE           RETTJ
           MOV          RET_CODE,AX
           @MOVB        FUNC_CODE,CLOS.FUNCTION_CODE
           CALL         SHOW_ERR            ;check return code and handle
           MOV          SYS_ERR,1H          ;any errors
RETTJ:
           POPA
           RET
CLOSE      ENDP

;*****************************************************************************
;   OS/2 RELATED FUNCTIONS
;*****************************************************************************

CLR_SCRN   PROC         NEAR

;  This function clears the screen to prepare to take the message to be sent.
;  Writes null on the whole screen

           PUSHA

           @VIOWRTNCHAR NULL,2000,0,0,0
                                            ;doscall to write a character
                                            ;(null) on the screen.
           CMP          AX,0                ;check for good return
           JE           RETT2               ;if good, return; otherwise
           CALL         SHOW_VIO_ERR        ;handle the error
           MOV          SYS_ERR,1H
RETT2:
           POPA
           RET
CLR_SCRN   ENDP

SET_CRSR_POS  PROC

;  This function sets the cursor at specified row and column.

           PUSHA

           @VIOSETCURPOS ROW,COL,0
                                            ;doscall to set crsr pos on screen
           CMP          AX,0                ;check for good return
           JE           RETT3               ;if good, return; otherwise
           CALL         SHOW_VIO_ERR        ;handle the error
           MOV          SYS_ERR,1H
RETT3:
           POPA
           RET
SET_CRSR_POS  ENDP

GET_CHAR_TOBE_SENT  PROC

;This function gets the character to sent from the keyboard

           PUSHA

           @KBDCHARIN   CHARDATA,0,0
                                            ;doscall to get char from keyboard
           CMP          AX,0                ;check for good return
           JE           RETT4               ;if good, return; otherwise
           CALL         SHOW_VIO_ERR        ;handle the error
           MOV          SYS_ERR,1H
RETT4:
           POPA
           RET
GET_CHAR_TOBE_SENT  ENDP

WRT_CHAR_ON_SCRN  PROC

;This position displays character at the current position of the cursor

           PUSHA

           @VIOWRTNCHAR CHARTOBESENT,1,ROW,COL,0
                                            ;doscall to write a char on screen
           CMP          AX,0                ;check for good return
           JE           RETT5               ;if good, return; otherwise
           CALL         SHOW_VIO_ERR        ;handle the error
           MOV          SYS_ERR,1H
RETT5:
           POPA
           RET
WRT_CHAR_ON_SCRN  ENDP

SCROL_SCRN  PROC

;This position scrolls the screen by one line

           PUSHA

           @VIOSCROLLUP 0,0,24,79,1,CH_ATR,0
                                            ;doscall to scroll the screen.
           CMP          AX,0                ;check for good return
           JE           RETT6               ;if good, return; otherwise
           CALL         SHOW_VIO_ERR        ;handle the error
           MOV          SYS_ERR,1H
RETT6:
           POPA
           RET
SCROL_SCRN  ENDP

;*****************************************************************************
CSEG       ENDS
;*****************************************************************************
;end of program
           END          XMIT
