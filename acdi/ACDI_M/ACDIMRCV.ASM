.286P

;*************************************************************************
;*                                                                       *
;*   MODULE NAME: ACDIMRCV.ASM                                           *
;*                                                                       *
;*   DESCRIPTIVE NAME: ACDI MASM SAMPLE RECEIVE PROGRAM                  *
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
;*             The RECEIVE PROGRAM, is executed in the second Personal   *
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
;*                COMREADEVENT                                           *
;*                COMREADCHARSTRING                                      *
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


DGROUP           GROUP      DATA

;*****************************************************************************
;The thread process stack segment

THRD_STK   SEGMENT WORD STACK 'STACK1'
           DW      4096 DUP (?)
THRD_STK_END EQU   $
THRD_STK   ENDS                             ;end of thread proc stack segment

;*****************************************************************************
;The main program stack segment

STACK      SEGMENT STACK 'STACK'
           DW      4096 DUP (?)
STACK      ENDS                             ;end of main program stack segment

;*****************************************************************************

DATA       SEGMENT WORD PUBLIC 'DATA'       ;Data segment.

SYS_ERR        DB       0                   ;system error flag
HANDLE         DW       0                   ;com device handle
DEV_NAME       DB       'COM1\0',0          ;name of com device to be used

OUT_BUFF_LEN   EQU      82                  ;length of output buffer
IN_BUFF_LEN    EQU      82                  ;length of input buffer

INTEG          DB       0

I_ANBPTR_SEL   DW       0                   ;selector returned by DOSALLOCSEG
                                            ;for input buffer
O_ANBPTR_SEL   DW       0                   ;selector returned by DOSALLOCSEG
                                            ;for output buffer

CONCTTIMEOUT1  EQU      0                   ;timeout values for the
CONCTTIMEOUT2  EQU      30                  ;comconnect verb

INF_RDBLKTMOUT EQU      0                   ;timeout parameters for the
RDCHRTMOUT     EQU      01H                 ;verb -
INF_WRTTMOUT   EQU      0                   ;comsettimeouts

INITWAIT       DW       0                   ;parameters for comreadcharstring
BYTESFREED     DW       0                   ;verb
BYTESNEEDED    DW       0

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

ZERO           DD       0
MSG_FILE       DB       'ACX.MSG',0         ;file containing messages
MSG_BUFF       DB       100 DUP  (?)        ;buffer for messages
MSG_NO         DW       0                   ;message to load and display
MSG_LEN        DW       0                   ;length of message

HEXTAB         DB       '0123456789ABCDEF'  ;table for hex conversion

;*****************************************************************************

ACTION         EQU      1
RESULT_CODE    DW       0

;*****************************************************************************

RECVD_CHAR     DB       0                   ;character received
EVENTFLAG      DB       0                   ;variable for communication
                                            ;between to asynchronously running
                                            ;processes-main and thread

ROW            DW       0                   ;row and column numbers used in
COL            DW       0                   ;screen displays
CH_ATR         DW       720H

CAR_RET        EQU      0DH                 ;carriage return

;*****************************************************************************

THRD_PGM_ADR   DD       EVENT               ;selector returned by DOSALLOCSEG
THRD_STK_ADR   DD       THRD_STK_END        ;address of stack
THRD_ID        DW       0                   ;thread identifier
SEMAPHOR       DD       0                   ;ram semaphore used in thread
SEMHANDLE      DD       SEMAPHOR            ;semaphore handle (address)
SEMTIMOUT_INF  DD      -1                   ;infinite wait time for DOSSEMWAIT

;*****************************************************************************
;   VERB CONTROL BLOCK (VCB)
;*****************************************************************************

OPN            COMOPEN_CB               <>  ;use same VCB with structs
               ORG              OPN         ;defined over same storage
DFOTBFF        COMDEFOUTPUTBUFF_CB      <>
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
RDCHRSTR       COMREADCHARSTRING_CB     <>
               ORG              OPN
DSCONCT        COMDISCONNECT_CB         <>
               ORG              OPN
CLOS           COMCLOSE_CB              <>
               ORG              OPN
VCB            DB               300 DUP (0)
VCB_LEN        EQU              $-VCB       ;size of VCB

;*****************************************************************************
;   VERB CONTROL BLOCK - FOR THE THREAD PROCESS
;*****************************************************************************

RDEVNT         COMREADEVENT_CB          <>   ;VCB for the thread process
               ORG              RDEVNT
VCB_THRD       DB               200 DUP (0)  ;size of VCB for thread
VCB_THRD_LEN   EQU              $-VCB_THRD

MASK           MASKS                    <>
EVENTSTRUCT    EVENT_STRUCT             <>

DATA           ENDS

;*****************************************************************************

CSEG       SEGMENT WORD PUBLIC 'CODE'
           ASSUME   CS:CSEG, DS:DGROUP, SS:STACK

EVENT      PROC         FAR

;  This is the thread process. It will set the EVENTFLAG to zero to enable
;  receiving characters by the main process, then set the semaphore; issue
;  comreadevent verb, and wait for the semaphore to be cleared by async
;  subsystem - which will happen when any one of three events specified -
;  break signal, disconnect or a stop is received. When this occurs it
;  will set the EVENTFLAG to signal end of session.
;  This process runs asynchronously with the main process so that receiving
;  message, and watching for the break signal can be done simultaneously

           MOV          EVENTFLAG,0         ;set flag to session active

           @DOSSEMSET   SEMHANDLE           ;doscall to set semaphore
           CMP          AX,0                ;check for a good return code
           JE           SUCC1               ;if good return code - go on
           MOV          DOS_RC,AX           ;otherwise show dos error and
           CALL         SHOW_DOS_ERR        ;jump to end
           MOV          SYS_ERR,1
           JMP          EVNT_ENDD
SUCC1:
           CALL         READEVENT           ;call subroutine to issue
                                            ;comreadevent verb
           CMP          SYS_ERR,0           ;check system error
           JNE          EVNT_ENDD
           @DOSSEMWAIT  SEMHANDLE,SEMTIMOUT_INF
                                            ;doscall to wait for semaphore
                                            ;to be cleared by async subsys
           CMP          AX,0                ;check return code
           JE           SUCC2               ;handle any error
           MOV          DOS_RC,AX
           CALL         SHOW_DOS_ERR
           JMP          EVNT_ENDD
           MOV          SYS_ERR,1
SUCC2:
           MOV          EVENTFLAG,0FFH      ;set flag to session ended
EVNT_ENDD:
           @DOSEXIT     ACTION,RESULT_CODE

EVENT      ENDP


READEVENT  PROC         NEAR

;  This subroutine will issue comreadevent verb so that the calling process
;  can wait on the semaphore to be cleared.

           PUSHA                            ;push work registers

           MOV          AX,DS
           MOV          ES,AX
           MOV          BYTE PTR ES:MASK.EVENT_MASK_1,0
           MOV          BYTE PTR ES:MASK.EVENT_MASK_1+2,AA_BREAK_RECEIVED
           MOV          BYTE PTR ES:MASK.EVENT_MASK_1+3,AA_STOP_ISSUED
           MOV          BYTE PTR ES:MASK.EVENT_MASK_1+4,0
                                            ;set up event masks for the events
                                            ;we want to be notified of
           MOV          RDEVNT.FUNCTION_CODE,COM_READ_EVENT
                                            ;insert function code
           @MOVW        RDEVNT.COM_DEV_HANDLE,HANDLE
                                            ;com. device handle
           MOV          WORD PTR RDEVNT.SEM_HANDLE+2,DS
           LEA          AX,SEMAPHOR
           MOV          WORD PTR RDEVNT.SEM_HANDLE,AX
                                            ;seg. & offset values of SEMAPHOR
           MOV          WORD PTR RDEVNT.BUFFER_ADDRESS+2,DS
           MOV          AX,OFFSET EVENTSTRUCT.EVENT_TYPE_1
           MOV          WORD PTR RDEVNT.BUFFER_ADDRESS,AX
                                            ;seg. & offset of EVENT_STRUCT
           MOV          WORD PTR RDEVNT.EVENT_MASKS+2,DS
           MOV          AX,OFFSET MASK.EVENT_MASK_1
           MOV          WORD PTR RDEVNT.EVENT_MASKS,AX
                                            ;seg. & offset of MASKS
           @ACDI        RDEVNT.FUNCTION_CODE
                                            ;call acdi to issue the verb
           MOV          AX,RDEVNT.RETURN_CODE
           CMP          AX,0
           JE           RETT_EVNT           ;jump to end if good ret_code
           MOV          RET_CODE,AX         ;otherwise copy ret and func codes
           @MOVB        FUNC_CODE,RDEVNT.FUNCTION_CODE
           CALL         SHOW_ERR            ;show error
           MOV          SYS_ERR,1           ;set system error
RETT_EVNT:
           POPA                             ;pop work registers
           RET

READEVENT  ENDP


RECV       PROC       FAR

;This is the main process, which calls subroutines one by one to issue
;acdi verbs, starts the thread process and then calls READLINES to read
;the message from the other PC

           MOV          MSG_NO,7            ;msg_no 7 is "ACDI sample
           CALL         SHOW_MSG            ;receiver program"

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP2               ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP2:
           CALL         OPEN                ;issue comopen

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP3               ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP3:
           @DOSALLOCSEG OUT_BUFF_LEN,O_ANBPTR_SEL,0
                                            ;Doscall to get an output buffer.
           CMP          AX,0
           JE           STEP4               ;if successful jump to next step
           MOV          DOS_RC,AX           ;otherwise copy the return code,
           CALL         SHOW_DOS_ERR        ;show the error, set system error
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
           CMP          AX,0
           JE           STEP6               ;if successful jump to next step
           MOV          DOS_RC,AX           ;otherwise copy the return code,
           CALL         SHOW_DOS_ERR        ;show the error, set system error
           MOV          SYS_ERR,1           ;and end the program
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
           @DOSCREATETHREAD  THRD_PGM_ADR,THRD_ID,THRD_STK_ADR
                                            ;Doscall to start thread proc.
           CMP          AX,0
           JE           STEP12              ;if successful jump to next step
           MOV          DOS_RC,AX           ;otherwise copy the return code,
           CALL         SHOW_DOS_ERR        ;show the error, set system error
           MOV          SYS_ERR,1           ;and end the program
           JMP          ENDD
STEP12:

           CALL         READLINES           ;call subr. to read message

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP13              ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP13:
           CALL         DISCONNECT          ;issue comdisconnect

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           STEP14              ;error) jump to next step; else
           JMP          ENDD                ;end the program
STEP14:
           CALL         CLOSE               ;issue comclose
ENDD:
           @DOSEXIT     ACTION,RESULT_CODE
RECV       ENDP


READLINES  PROC         NEAR

;  This subroutine will clear the screen in preparation for receiving the
;  message, issue com_read_char_string verb as long as EVENTFLAG is on,
;  read the message character by character, display it on the screen. It
;  will quit when the flag is set to session ended.

           PUSHA                            ;push work registers
           CALL         CLR_SCRN            ;call subr. to clear screen

LOOP1:
           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           READSTEP1           ;error) jump to next step; else
           JMP          RETT                ;end the program
READSTEP1:
           CALL         SET_CRSR_POS        ;set cursor on row, col
           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           LOOP2               ;error) jump to next step; else
           JMP          RETT                ;end the program
LOOP2:
           CMP          EVENTFLAG,0H        ;check the flag
           JE           READSTEP2
           JMP          RETT
READSTEP2:
           MOV          BYTESNEEDED,1
           CALL         READCHARSTR         ;issue comreadcharstring

           CMP          SYS_ERR,0           ;check system error; if zero (no
           JE           READSTEP3           ;error) jump to next step; else
           JMP          RETT                ;end the program
READSTEP3:
           CMP          RET_CODE,0          ;check return code-if zero
           JE           CHAR_RECVD          ;a character is received
           MOV          BYTESFREED,0        ;set bytesfreed to 0
           JMP          LOOP2               ;issue comreadcharstring again
CHAR_RECVD:
           MOV          ES,WORD PTR RDCHRSTR.NEXT_AVAIL_READ_PTR+2
                                            ;ES = seg of next_avail_read_ptr
           MOV          SI,WORD PTR RDCHRSTR.NEXT_AVAIL_READ_PTR
                                            ;SI = off of next_avail_read_ptr
           MOV          AL,ES:[SI]          ;mov to AL the received char.
           CMP          AL,CAR_RET          ;if received character is carriage
           JE           NXT_LIN             ;return jump to NXT_LIN
           MOV          RECVD_CHAR,AL       ;else copy received character in
                                            ;RECVD_CHAR and call vio
                                            ;function to write it on screen
           @VIOWRTNCHAR  RECVD_CHAR,1,ROW,COL,0
           MOV          BYTESFREED,1        ;set bytesfreed to 1
           INC          COL                 ;increment column
           CMP          COL,80              ;79 is the last column on screen
           JE           SET_COL_ZERO        ;so if column 79 is reached -
           JMP          LOOP1
SET_COL_ZERO:
           MOV          COL,0               ;set it back to zero
           JMP          LOOP1               ;repeat again to get the next
                                            ;character
NXT_LIN:
           MOV          BYTESFREED,1        ;set bytesfreed to 1
           INC          ROW                 ;increment row
           CMP          ROW,24              ;if row is 24
           JE           SCROLL              ;jump to scroll
           MOV          COL,0               ;else set col to 0
           JMP          LOOP1               ;repeat again, get next character
SCROLL:
           CALL         SCROL_SCRN          ;subr. to scroll screen by 1 row
           MOV          ROW,23              ;set row to 23
           MOV          COL,0               ;set column to 0
           JMP          LOOP1               ;repeat again,get next character
RETT:
           POPA
           RET                              ;pop work registers
READLINES  ENDP


;*****************************************************************************
;   UTILITY PROC'S
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
           @VIOWRTTTY F_C_HX,F_C_HX_LEN,0   ;vio call to write on screen

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
           MOV          BX,AX               ;get the original data again
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
           MOV          TIMOUTS.READ_TIMEOUT_CHAR,RDCHRTMOUT
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

READCHARSTR   PROC   NEAR

           PUSHA
           CALL         CLEAR_VCB           ;zero out the control block

           @MOVW        RDCHRSTR.COM_DEV_HANDLE,HANDLE
           MOV          RDCHRSTR.FUNCTION_CODE,COM_READ_CHAR_STRING
           @MOVW        RDCHRSTR.BYTES_FREED,BYTESFREED
           @MOVW        RDCHRSTR.READ_BYTES_NEEDED,BYTESNEEDED
           @MOVW        RDCHRSTR.INITIAL_WAIT,INITWAIT
                                            ;fill in the control block with
                                            ;function code and parameters
           @ACDI        VCB                 ;issue the verb

           MOV          AX,RDCHRSTR.RETURN_CODE
           MOV          RET_CODE,AX         ;copy return code(AX) in RET_CODE
           CMP          AX,0                ;if return code is zero, return
           JE           RETTF
           CMP          AX,0054H            ;otherwise, if ret. code is 54H -
                                            ;means timeout occured - which is
           JE           RETTF               ;ok, so just return
                                            ;if we reach here - error occured
           @MOVB        FUNC_CODE,RDCHRSTR.FUNCTION_CODE
           CALL         SHOW_ERR            ;so show error
           MOV          SYS_ERR,1H          ;set system error to true & ret.
RETTF:
           POPA
           RET
READCHARSTR   ENDP

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

;  This function clears the screen to prepare to display message received.
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

;  This function sets the cursor position at the specified row and column.

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
           END          RECV
