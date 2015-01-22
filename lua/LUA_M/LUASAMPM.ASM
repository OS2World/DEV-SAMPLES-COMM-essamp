;******************************************************************************
;*                                                                            *
;*   MODULE NAME      : LUASAMP.ASM                                           *
;*                                                                            *
;*   DESCRIPTIVE NAME : LUA MASM/2 SAMPLE PROGRAM FOR IBM EXTENDED SERVICES   *
;*                      FOR OS/2                                              *
;*                                                                            *
;*   COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1989, 1990, 1991              *
;*                      LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           *
;*                      ALL RIGHTS RESERVED                                   *
;*                                                                            *
;*   FUNCTION         : This program                                          *
;*                      - Issues an SLI_OPEN to establish an LU_LU session.   *
;*                      - Issues an SLI_SEND to transmit data to the host.    *
;*                      - Issues an SLI_RECEIVE to get data from the host.    *
;*                      - Issues an SLI_SEND to transmit a response.          *
;*                      - Issues an SLI_CLOSE to end the LU_LU session.       *
;*                                                                            *
;*   GENERAL SERVICE                                                          *
;*     VERBS USED     : CONVERT - Translates data between ASCII and EBCDIC.   *
;*                                                                            *
;*   MODULE TYPE      : IBM Personal Computer Macro Assembler/2 Version 1.00  *
;*                                                                            *
;******************************************************************************
INCLUDE DOSCALLS.INC                             ; Include Doscall Macros
INCLUDE SUBCALLS.INC                             ; Include OS/2 Call Macros
INCLUDE  LUA_M.INC                               ; The LUA MASM/2 includes
INCLUDE  ACSSVCA.INC                             ; Adv. Comm. Gen. Serv. include
TITLE   luasamp.asm
.286p                                            ; Enable protected mode assembly
.287                                             ; Enable 80287 floating point

EXTRN   ACSSVC:FAR                               ; APPC service verb entry point
EXTRN   SLI:FAR                                  ; LUA SLI verb enrty point

ClearVerbRecord  MACRO   Record,Size             ; Define Macro to clear a
                 mov     di,OFFSET Record        ; verb record.
                 mov     cx,Size
                 mov     ax,ds
                 mov     es,ax
                 sub     ax,ax
                 rep     stosb
                 ENDM
                                                 ; Define DGROUP
DGROUP  GROUP  DATA
                                                 ; Create 4k Stack
STACK  SEGMENT  WORD stack 'STACK'
       dw  4000  dup(0)
STACK  ENDS

DATA   SEGMENT  WORD PUBLIC 'DATA'
;******************************************************************************
; Declare storage for the verb records, data transactions, posting semaphore,
; and storage variables.
;******************************************************************************

LuaVerb             lua_common <>                ; Declare LuaVerb record
LuaVerbSize         EQU   $-LuaVerb              ; Equate for size of LuaVerb.
LuaVerb_Open        lua_open <>                  ; using the structures provided
                    ORG    LuaVerb_Open          ; in LUA_M.INC.
LuaVerb_Bid         lua_bid <>
                    ORG    LuaVerb_Open
LuaVerb_Send        lua_send <>
                    ORG    LuaVerb
LuaVerb_VR          DB 198 dup (0)               ; Initialize LuaVerb to zero.

ConvertVerb         CONVERT <>                   ; Declare ConvertVerb record
                                                 ; using the structures provided
                                                 ; in ACSSVCA.INC.

InitSelfRU          DB      01H                  ; Define 25 byte structure
                    DB      06H                  ; for the INITSELF command RU.
                    DB      081H
                    DB      00H
   IS_ModeName      DB      'LUA768RU'           ; This field needs to be
   ModenameSize     EQU     $-IS_ModeName        ; to EBCDIC.
                    DB      0f3H
                    DB      08H
   IS_PluName       DB      'VTAMPGM '           ; This field needs to be
   PlunameSize      EQU     $-IS_PluName         ; to EBCDIC.
                    DB      00H
                    DB      00H
                    DB      00H
                    ORG     $+1

TestData            DB  'TEST#SENDING#DATA#TO#HOST' ; SLI_SEND data.
TestDataSize        EQU $-TestData
DataBuffer          DB      00H                  ; Declare a 256 byte data
                    DB      255 DUP(0)           ; buffer for SLI_RECEIVE.
DataBufferSize      EQU $-DataBuffer

UserRamSem          DD      00H                  ; Allocate a RAM semaphore.
UserRamSemAddr      DD      UserRamSem           ; Set address of semaphore.
SemTimeout          DD      -1                   ; Sem timeout to wait forever.
LU_SessionID        DW      00H,00H              ; Save lua_sid value returned
                                                 ; by SLI_OPEN.
LU_Name             DB      'LUA1    '           ; LU to create session for.
LU_NameSize         EQU     $-LU_Name
SavedSeqNum         DB      00H,00H              ; Save Sequence # for responses.

DigitTable          DB      '0123456789ABCDEF'   ; table and buffer
HexBuffer           DB  8  dup(' ')              ; for PRTHEX procedure
HexBufferFilledSize DW  ?


;******************************************************************************
; Declare all the message strings and their sizes.
;******************************************************************************

OpenMessage         DB  0dH,0aH,'Opening communication with SLI interface...'
OpenMessageSize     EQU $-OpenMessage
InitSelfMessage     DB  0dH,0aH,'SLI session opened and INITSELF sent to host.'
InitSelfMessageSize EQU $-InitSelfMessage
DataMessage         DB  0dH,0aH,'Test data sent to host. Waiting for host data.'
DataMessageSize     EQU $-DataMessage
ResponseMessage     DB  0dH,0aH,'Host data received. Responding to host.'
ResponseMessageSize EQU $-ResponseMessage
CloseMessage        DB  0dH,0aH,'Response sent.  Preparing to close sesssion.'
CloseMessageSize    EQU $-CloseMessage
OK_Message          DB  0dH,0aH,'SLI interface closed with no errors.'
OK_MessageSize      EQU $-OK_Message
Error1_Message      DB  0dH,  0aH, 'Ab-ended LUA conversation due to error. '
Error1_MessageSize  EQU $-Error1_Message
Error2_Message      DB  0dH,0aH,'An error occurred during the conversion. '
Error2_MessageSize  EQU $-Error2_Message
Error3_Message      DB  0dH,0aH,'An error occurred with SLI API. Verb opcode: '
Error3_MessageSize  EQU $-Error3_Message
Error4_Message      DB  0dH,0aH,'The LU_LU session has failed due to an error.'
Error4_MessageSize  EQU $-Error4_Message
PrimRC_Message      DB  0dH,0aH,'The primary return code is: '
PrimRC_MessageSize  EQU $-PrimRC_Message
SecRC_Message       DB  0dH,0aH,'The secondary return code is: '
SecRC_MessageSize   EQU $-SecRC_Message

DATA      ENDS


CODE    SEGMENT  WORD PUBLIC 'CODE'
        ASSUME  CS: CODE, DS: DGROUP
        PUBLIC  main
main    PROC FAR

        @VIOWRTTTY  OpenMessage,OpenMessageSize,0

        call    NEAR PTR SLI_OPEN                ; Open communication with local
                                                 ; LU.
        cmp     ax,0                             ; Check procedure return code,
        je      MainSessOpen                     ; if zero then session opened.
        jmp     MainExitError                    ; if not zero then end program.

MainSessOpen:
        @VIOWRTTTY  InitSelfMessage,InitSelfMessageSize,0

        call    NEAR PTR SLI_SEND_DATA           ; Send trans-id and user data
                                                 ; to the host.
        cmp     ax,0                             ; Check procedure return code,
        jne     MainAbend                        ; if not zero then Ab-end.

        @VIOWRTTTY  DataMessage,DataMessageSize,0

        call    NEAR PTR SLI_RECEIVE             ; Receive Data and response
                                                 ; from the host.
        cmp     ax,0                             ; Check procedure return code,
        jne     MainAbend                        ; if not zero then Ab-end.

        @VIOWRTTTY  ResponseMessage,ResponseMessageSize,0

        call    NEAR PTR SLI_SEND_RESPONSE       ; Send response for data to
                                                 ; the host.
        cmp     ax,0                             ; Check procedure return code,
        jne     MainAbend                        ; if not zero then Ab-end.

        @VIOWRTTTY  CloseMessage,CloseMessageSize,0

        push    0                                ; Indicate normal SLI_CLOSE.
        call    NEAR PTR SLI_CLOSE               ; Close communcation with local
                                                 ; LU.
        cmp     ax,0                             ; Check procedure return code,
        je      MainExitNoError                  ; if zero, then session closed
                                                 ; successfully.

MainAbend:
        cmp     ax,LUA_SESSION_FAILURE           ; If session failed,
        je      MainSessFail                     ; display session failure message.
        push    LUA_FLAG1_CLOSE_ABEND            ; Indicate Abnormal SLI_CLOSE.
        call    NEAR PTR SLI_CLOSE               ; Close communication in ab-end

MainExitError:
        @VIOWRTTTY  Error1_Message,Error1_MessageSize,0
        jmp     MainDosExit

MainSessFail:
        @VIOWRTTTY  Error4_Message,Error4_MessageSize,0
        jmp     MainDosExit

MainExitNoError:
        @VIOWRTTTY  OK_Message,OK_MessageSize,0

MainDosExit:

        @DosExit 0,0                             ; Return to Operating System

main    ENDP

;****************************************************************************
;*  PROCEDURE NAME: SLI_OPEN                                                *
;*  FUNCTION      : Issue an SLI_OPEN of type secondary initself, using     *
;*                  the InitSelfRU as the SNA INITSELF command.             *
;*                  Preform the necessary translation of the Modename and   *
;*                  Pluname in the InitSelfRU.                              *
;*  RETURNS       : Any error is returned in the ax register.               *
;****************************************************************************
SLI_OPEN        PROC NEAR

        ClearVerbRecord  LuaVerb,LuaVerbSize+4            ; Clear Verb Record.
                                                          ; NOTE: No extension
                                                          ; list fields are used.

        mov     LuaVerb.lua_verb,LUA_VERB_SLI             ; set verb = '5200'x.
        mov     LuaVerb.lua_verb_length,68                ; set verb length.
        mov     LuaVerb.lua_opcode,LUA_OPCODE_SLI_OPEN    ; set opcode.

        mov     di,OFFSET LuaVerb.lua_luname              ; set luname.
        mov     si,OFFSET LU_Name
        mov     cx,LU_NameSize
        rep     movsb

        mov     LuaVerb.lua_data_length,25                ; set data length.
        mov     LuaVerb.lua_data_ptr,OFFSET InitSelfRU    ; set data pointer.
        mov     LuaVerb.lua_data_ptr+2,SEG  InitSelfRU

        mov     LuaVerb.lua_post_handle,OFFSET UserRamSem ; set posting
        mov     LuaVerb.lua_post_handle+2,SEG  UserRamSem ; semaphore.

        mov     LuaVerb_Open.lua_init_type,LUA_INIT_TYPE_SEC_IS ; Set init type
                                                                ; to InitSelf.

        push    SEG    IS_PluName                         ; translate Pluname
        push    OFFSET IS_PluName                         ; from ASCII to EBCDIC.
        push    PluNameSize
        call    NEAR PTR ASCII_EBCDIC
        cmp     ax,0                                      ; Exit SLI_OPEN if
        jne     DoOpenExit                                ; conversion error.

        push    SEG    IS_ModeName                        ; translate Modename
        push    OFFSET IS_ModeName                        ; from ASCII to EBCDIC.
        push    ModeNameSize
        call    NEAR PTR ASCII_EBCDIC
        cmp     ax,0                                      ; Exit SLI_OPEN if
DoOpenExit:
        jne     OpenExit                                  ; conversion error.

        push    SEG    LuaVerb                            ; Push address of
        push    OFFSET LuaVerb                            ; LuaVerb.
        call    SLI                                       ; Call SLI API.

        cmp     LuaVerb.lua_prim_rc,LUA_IN_PROGRESS       ; Check primary rc,
        jne     OpenComplete                              ; if complete don't
                                                          ; wait on semaphore;

        @DOSSEMWAIT   UserRamSemAddr,SemTimeout           ; otherwise wait for
                                                          ; completion.

OpenComplete:
        cmp     LuaVerb.lua_prim_rc,LUA_OK                ; If successful,
        je      OpenSetSessionID                          ; save session ID;

        push    LuaVerb.lua_prim_rc                       ; otherwise, display
        push    LuaVerb.lua_sec_rc+2                      ; error to screen.
        push    LuaVerb.lua_sec_rc
        push    LuaVerb.lua_opcode
        call    NEAR PTR ERROR2
        jmp     OpenSetRC                                 ; and set ax to
                                                          ; error primary rc.

OpenSetSessionID:
        mov     ax,LuaVerb.lua_sid                        ; Nove lua_sid into
        mov     dx,LuaVerb.lua_sid+2                      ; local storage for
        mov     LU_SessionID,ax                           ; use by other verbs
        mov     LU_SessionID+2,dx                         ; on this session.

OpenSetRC:
        mov     ax,LuaVerb.lua_prim_rc                    ; Set ax to reflect
                                                          ; SLI_OPEN completion.

OpenExit:
        ret                                               ; return.

SLI_OPEN ENDP

;****************************************************************************
;*  PROCEDURE NAME: SLI_SEND_DATA                                           *
;*  FUNCTION      : Send data to the host on LU Normal flow.  Translate the *
;*                  data to EBCDIC.                                         *
;*  RETURNS       : Any error is returned in the ax register.               *
;****************************************************************************
SLI_SEND_DATA   PROC NEAR

        ClearVerbRecord  LuaVerb,LuaVerbSize+2            ; Clear Verb Record.

        mov     LuaVerb.lua_verb,LUA_VERB_SLI             ; set verb = '5200'x.
        mov     LuaVerb.lua_verb_length,66                ; set verb length.
        mov     LuaVerb.lua_opcode,LUA_OPCODE_SLI_SEND    ; set opcode.

        mov     ax,LU_SessionID                           ; set lua_sid with
        mov     dx,LU_SessionID+2                         ; the session ID
        mov     LuaVerb.lua_sid,ax                        ; returned by
        mov     LuaVerb.lua_sid+2,dx                      ; SLI_OPEN.

        mov     LuaVerb.lua_data_length,TestDataSize      ; set data length.
        mov     LuaVerb.lua_data_ptr,OFFSET  TestData     ; set data pointer.
        mov     LuaVerb.lua_data_ptr+2,SEG   TestData

        mov     LuaVerb.lua_post_handle,OFFSET UserRamSem ; set posting
        mov     LuaVerb.lua_post_handle+2,SEG  UserRamSem ; semaphore.

        or      LuaVerb.lua_rh_2,LUA_RH_DR1I              ; set definite rsp.
        or      LuaVerb.lua_rh_2,LUA_RH_RI                ; set exception rsp.
        or      LuaVerb.lua_rh_3,LUA_RH_BBI               ; set begin bracket.
        or      LuaVerb.lua_rh_3,LUA_RH_CDI               ; set change direction.
                                                          ; SLI sets the other
                                                          ; required RH bits.

        mov     LuaVerb.lua_message_type,LUA_MESSAGE_TYPE_LU_DATA ; set message
                                                                  ; type.

        push    LuaVerb.lua_data_ptr+2                    ; translate data
        push    LuaVerb.lua_data_ptr                      ; from ASCII to EBCDIC.
        push    LuaVerb.lua_data_length
        call    NEAR PTR ASCII_EBCDIC
        cmp     ax,0                                      ; Exit SLI_SEND_DATA
        jne     SendDataExit                              ; if conversion error.

        push    SEG    LuaVerb                            ; Push address of
        push    OFFSET LuaVerb                            ; LuaVerb.
        call    SLI                                       ; Call SLI API.

        cmp     LuaVerb.lua_prim_rc,LUA_IN_PROGRESS       ; Check primary rc,
        jne     SendDataComplete                          ; if complete don't
                                                          ; wait on semaphore;

        @DOSSEMWAIT   UserRamSemAddr,SemTimeout           ; otherwise wait for
                                                          ; completion.
SendDataComplete:
        cmp     LuaVerb.lua_prim_rc,LUA_OK                ; If successful
        je      SendDataSetRC                             ; set procedure rc;

        push    LuaVerb.lua_prim_rc                       ; otherwise, display
        push    LuaVerb.lua_sec_rc+2                      ; error to screen
        push    LuaVerb.lua_sec_rc
        push    LuaVerb.lua_opcode
        call    NEAR PTR ERROR2

SendDataSetRC:
        mov     ax,LuaVerb.lua_prim_rc                 ; set ax to the SLI_SEND
                                                       ; primary return code.

SendDataExit:
        ret

SLI_SEND_DATA   ENDP

;****************************************************************************
;*  PROCEDURE NAME: SLI_RECEIVE                                             *
;*  FUNCTION      : Receive a message from the host on LU Normal flow into  *
;*                  DataBuffer. Translate the data from EBCDIC to ASCII.    *
;*  RETURNS       : Any error is returned in the ax register.               *
;****************************************************************************
SLI_RECEIVE     PROC NEAR

        ClearVerbRecord  LuaVerb,LuaVerbSize              ; Clear Verb Record.

        mov     LuaVerb.lua_verb,LUA_VERB_SLI             ; set verb = '5200'x.
        mov     LuaVerb.lua_verb_length,64                ; set verb length.
        mov     LuaVerb.lua_opcode,LUA_OPCODE_SLI_RECEIVE ; set opcode.

        mov     ax,LU_SessionID                           ; set lua_sid with
        mov     dx,LU_SessionID+2                         ; the session ID
        mov     LuaVerb.lua_sid,ax                        ; returned by
        mov     LuaVerb.lua_sid+2,dx                      ; SLI_OPEN.

        mov     LuaVerb.lua_max_length,DataBufferSize     ; set max receive
                                                          ; length.
        mov     LuaVerb.lua_data_ptr,OFFSET DataBuffer    ; set data pointer.
        mov     luaVerb.lua_data_ptr+2,SEG  DataBuffer

        mov     LuaVerb.lua_post_handle,OFFSET UserRamSem ; set posting
        mov     LuaVerb.lua_post_handle+2,SEG  UserRamSem ; semaphore

        or      LuaVerb.lua_flag1,LUA_FLAG1_LU_NORM       ; set flow to receive on.

        push    SEG    LuaVerb                            ; Push address of
        push    OFFSET LuaVerb                            ; LuaVerb.
        call    SLI                                       ; Call SLI API.

        cmp     LuaVerb.lua_prim_rc,LUA_IN_PROGRESS       ; Check primary rc,
        jne     ReceiveComplete                           ; if complete don't
                                                          ; wait on semaphore;

        @DOSSEMWAIT   UserRamSemAddr,SemTimeout           ; otherwise wait for
                                                          ; completion.

ReceiveComplete:
        cmp     LuaVerb.lua_prim_rc,LUA_OK                ; If successful
        je      ReceiveData                               ; receive data;

        push    LuaVerb.lua_prim_rc                       ; otherwise, display
        push    LuaVerb.lua_sec_rc+2                      ; error to screen
        push    LuaVerb.lua_sec_rc
        push    LuaVerb.lua_opcode
        call    NEAR PTR ERROR2
        jmp     ReceiveSetRC                              ; set ax to the SLI_SEND
                                                          ; primary return code.

ReceiveData:
        mov     al,LuaVerb.lua_th_snf                     ; save the sequence
        mov     SavedSeqNum,al                            ; number from the
        mov     al,LuaVerb.lua_th_snf+1                   ; request TH for
        mov     SavedSeqNum+1,al                          ; use in the response.

        push    LuaVerb.lua_data_ptr+2                    ; convert the data to
        push    LuaVerb.lua_data_ptr                      ; ASCII.
        push    LuaVerb.lua_data_length
        call    NEAR PTR EBCDIC_ASCII
        cmp     ax,0                                      ; exit SLI_RECEIVE
        jne     ReceiveExit                               ; if conversion error.

ReceiveSetRC:
        mov     ax,LuaVerb.lua_prim_rc                 ; set ax to the SLI_RECEIVE
                                                       ; primary return code.

ReceiveExit:
        ret

SLI_RECEIVE     ENDP

;****************************************************************************
;*  PROCEDURE NAME: SLI_SEND_RESPONSE                                       *
;*  FUNCTION      : Send positive data response to the host for the LU      *
;*                  Normal data received.                                   *
;*  RETURNS       : Any error is returned in the ax register.               *
;****************************************************************************
SLI_SEND_RESPONSE       PROC NEAR

        ClearVerbRecord  LuaVerb,LuaVerbSize+2            ; Clear Verb Record.

        mov     LuaVerb.lua_verb,LUA_VERB_SLI             ; set verb = '5200'x.
        mov     LuaVerb.lua_verb_length,66                ; set verb length.
        mov     LuaVerb.lua_opcode,LUA_OPCODE_SLI_SEND    ; set opcode.

        mov     ax,LU_SessionID                           ; set lua_sid with
        mov     dx,LU_SessionID+2                         ; the session ID
        mov     LuaVerb.lua_sid,ax                        ; returned by
        mov     LuaVerb.lua_sid+2,dx                      ; SLI_OPEN.

        mov     LuaVerb.lua_post_handle,OFFSET UserRamSem ; set posting
        mov     LuaVerb.lua_post_handle+2,SEG  UserRamSem ; semaphore

        mov     al,SavedSeqNum                            ; set sequence number
        mov     LuaVerb.lua_th_snf,al                     ; to indicate which
        mov     al, SavedSeqNum+1                         ; message is being
        mov     LuaVerb.lua_th_snf+1,al                   ; responded to.

        or      LuaVerb.lua_rh_2,LUA_RH_DR1I              ; set definite response

        or      LuaVerb.lua_flag1,LUA_FLAG1_LU_NORM       ; set flow to send

        mov     LuaVerb.lua_message_type,LUA_MESSAGE_TYPE_RSP ; set message type
                                                              ; to indicate rsp

        push    SEG    LuaVerb                            ; Push address of
        push    OFFSET LuaVerb                            ; LuaVerb.
        call    SLI                                       ; Call SLI API.

        cmp     LuaVerb.lua_prim_rc,LUA_IN_PROGRESS       ; Check primary rc,
        jne     SendResponseComplete                      ; if complete don't
                                                          ; wait on semaphore;

        @DOSSEMWAIT   UserRamSemAddr,SemTimeout           ; otherwise wait for
                                                          ; completion.

SendResponseComplete:
        cmp     LuaVerb.lua_prim_rc,LUA_OK                 ; If successful
        je      SendResponseSetRC                          ; set procedure rc;

        push    LuaVerb.lua_prim_rc                        ; otherwise, display
        push    LuaVerb.lua_sec_rc+2                       ; error to screen
        push    LuaVerb.lua_sec_rc
        push    LuaVerb.lua_opcode
        call    NEAR PTR ERROR2

SendResponseSetRC:
        mov     ax,LuaVerb.lua_prim_rc                  ; set ax to the SLI_SEND
                                                        ; primary return code.
        ret

SLI_SEND_RESPONSE       ENDP

;****************************************************************************
;*  PROCEDURE NAME: SLI_CLOSE                                               *
;*  FUNCTION      : Close the session for the LU.  Issue a normal or abend  *
;*                  close depending upon the passed parameter.              *
;*  RETURNS       : Any error is returned in the ax register.               *
;*  PARAMETERS    : The passed indicator will be at [bp+4]                  *
;****************************************************************************
SLI_CLOSE       PROC NEAR
        enter    0,0                                       ; set bp for stack
                                                           ; referencing.

        ClearVerbRecord  LuaVerb,LuaVerbSize               ; Clear Verb Record.

        mov     LuaVerb.lua_verb,LUA_VERB_SLI              ; set verb = '5200'x.
        mov     LuaVerb.lua_verb_length,64                 ; set verb length.
        mov     LuaVerb.lua_opcode,LUA_OPCODE_SLI_CLOSE    ; set opcode.

        mov     ax,LU_SessionID                            ; set lua_sid with
        mov     dx,LU_SessionID+2                          ; the session ID
        mov     LuaVerb.lua_sid,ax                         ; returned by
        mov     LuaVerb.lua_sid+2,dx                       ; SLI_OPEN.

        mov     LuaVerb.lua_post_handle,OFFSET UserRamSem  ; set posting
        mov     LuaVerb.lua_post_handle+2,SEG  UserRamSem  ; semaphore.

        mov     al,BYTE PTR [bp+4]                         ; set lua_flag1
        or      LuaVerb.lua_flag1,al                       ; with the passed
                                                           ; indicator.

        push    SEG    LuaVerb                             ; Push address of
        push    OFFSET LuaVerb                             ; LuaVerb.
        call    SLI                                        ; Call SLI API.

        cmp     LuaVerb.lua_prim_rc,LUA_IN_PROGRESS        ; Check primary rc,
        jne     CloseComplete                              ; if complete don't
                                                           ; wait on semaphore;

        @DOSSEMWAIT   UserRamSemAddr,SemTimeout            ; otherwise wait for
                                                           ; completion.
CloseComplete:
        cmp     LuaVerb.lua_prim_rc,LUA_OK                 ; If successful
        je      CloseSetRC                                 ; set procedure rc;

        push    LuaVerb.lua_prim_rc                        ; otherwise, display
        push    LuaVerb.lua_sec_rc+2                       ; error to screen
        push    LuaVerb.lua_sec_rc
        push    LuaVerb.lua_opcode
        call    NEAR PTR ERROR2

CloseSetRC:
        mov     ax,WORD PTR LuaVerb.lua_prim_rc          ; set ax to the SLI_CLOSE
                                                         ; primary return code.
        leave
        ret     2

SLI_CLOSE       ENDP

;****************************************************************************
;*  PROCEDURE NAME: ASCII_EBCDIC                                            *
;*  FUNCTION      : Use the Convert service verb to convert selected data   *
;*                  from ASCII to EBCDIC.                                   *                                        *
;*  RETURNS       : Any error is returned in the ax register.               *
;*  PARAMETERS    : The data length will be at [bp+4].                      *
;*                : The data pointer will be at [bp+6].                     *
;****************************************************************************
ASCII_EBCDIC    PROC NEAR
        enter      0,0                                       ; set bp for stack
                                                             ; referencing.

        mov     ConvertVerb.opcode_cvt,SV_CONVERT            ; set opcode.
        mov     ConvertVerb.direction_cvt,SV_ASCII_TO_EBCDIC ; set direction.
        mov     ConvertVerb.char_set_cvt,SV_AE               ; set char sets.

        mov     ax,[bp+4]      ;len
        mov     ConvertVerb.len_cvt,ax                       ; set data length.

        mov     ax,[bp+6]      ;ptr                          ; set source
        mov     dx,[bp+8]                                    ; pointer.
        mov     WORD PTR ConvertVerb.src_ptr_cvt,ax
        mov     WORD PTR ConvertVerb.src_ptr_cvt+2,dx

        mov     WORD PTR ConvertVerb.targ_ptr_cvt,ax         ; set destination
        mov     WORD PTR ConvertVerb.targ_ptr_cvt+2,dx       ; pointer.

        push    SEG    ConvertVerb                           ; issue the
        push    OFFSET ConvertVerb                           ; convert verb.
        call    FAR PTR ACSSVC

        cmp     ConvertVerb.primary_rc_cvt,0                 ; check primary rc
        je      AsciiEbcdicExit                              ; exit if ok

        push    ConvertVerb.primary_rc_cvt                   ; otherwise
        push    WORD PTR ConvertVerb.secondary_rc_cvt+2      ; call error
        push    WORD PTR ConvertVerb.secondary_rc_cvt        ; routine
        call    NEAR PTR ERROR1

AsciiEbcdicExit:
        mov     ax,WORD PTR ConvertVerb.primary_rc_cvt       ; set ax
        leave
        ret     6

ASCII_EBCDIC    ENDP

;****************************************************************************
;*  PROCEDURE NAME: EBCDIC_ASCII                                            *
;*  FUNCTION      : Use the Convert service verb to convert selected data   *
;*                  from EBCDIC to ASCII.                                   *                                        *
;*  RETURNS       : Any error is returned in the ax register.               *
;*  PARAMETERS    : The data length will be at [bp+4].                      *
;*                : The data pointer will be at [bp+6].                     *
;****************************************************************************
EBCDIC_ASCII    PROC NEAR
        enter     0,0                                        ; set bp for stack
                                                             ; referencing.

        mov     ConvertVerb.opcode_cvt,SV_CONVERT            ; set opcode.
        mov     ConvertVerb.direction_cvt,SV_EBCDIC_TO_ASCII ; set direction.
        mov     ConvertVerb.char_set_cvt,SV_AE               ; set char sets.

        mov     ax,[bp+4]      ;len
        mov     ConvertVerb.len_cvt,ax                       ; set data length.

        mov     ax,[bp+6]      ;ptr                          ; set source
        mov     dx,[bp+8]                                    ; pointer.
        mov     WORD PTR ConvertVerb.src_ptr_cvt,ax
        mov     WORD PTR ConvertVerb.src_ptr_cvt+2,dx

        mov     WORD PTR ConvertVerb.targ_ptr_cvt,ax         ; set destination
        mov     WORD PTR ConvertVerb.targ_ptr_cvt+2,dx       ; pointer.

        push    SEG    ConvertVerb                           ; issue the
        push    OFFSET ConvertVerb                           ; convert verb.
        call    FAR PTR ACSSVC

        cmp     ConvertVerb.primary_rc_cvt,0                 ; check primary rc
        je      EbcdicAsciiExit                              ; exit if ok

        push    ConvertVerb.primary_rc_cvt                   ; otherwise
        push    WORD PTR ConvertVerb.secondary_rc_cvt+2      ; call error
        push    WORD PTR ConvertVerb.secondary_rc_cvt        ; routine
        call    NEAR PTR ERROR1

EbcdicAsciiExit:
        mov     ax,WORD PTR ConvertVerb.primary_rc_cvt       ; set ax
        leave
        ret     6

EBCDIC_ASCII    ENDP

;****************************************************************************
;*  PROCEDURE NAME: ERROR1                                                  *
;*  FUNCTION      : Display return codes for conversion errors.             *
;*  PARAMETERS    : The secondary return code will be at [bp+4].            *
;*                : The primary return code will be at [bp+8].              *
;****************************************************************************
ERROR1  PROC NEAR
        enter     0,0                                     ; set bp for stack
                                                          ; referencing.
        @VIOWRTTTY  Error2_Message,Error2_MessageSize,0

        @VIOWRTTTY  PrimRC_Message,PrimRC_MessageSize,0

        lea     ax,WORD PTR [bp+8]                        ; Call PRTHEX to
        push    ss                                        ; display the primary
        push    ax                                        ; return code.
        push    2
        call    NEAR PTR PRTHEX

        @VIOWRTTTY  SecRC_Message,SecRC_MessageSize,0

        lea     ax,WORD PTR [bp+4]                        ; Call PRTHEX to
        push    ss                                        ; display the secondary
        push    ax                                        ; return code.
        push    4
        call    NEAR PTR PRTHEX

        leave
        ret     6

ERROR1  ENDP

;****************************************************************************
;*  PROCEDURE NAME: ERROR2                                                  *
;*  FUNCTION      : Display return codes for unsuccessful SLI API calls.    *
;*  PARAMETERS    : The verb opcode will be at [bp+4].                      *
;*                : The secondary return code will be at [bp+6].            *
;*                : The primary return code will be at [bp+10].             *
;****************************************************************************
ERROR2  PROC NEAR
        enter     0,0                                     ; set bp for stack
                                                          ; referencing.
        @VIOWRTTTY  Error3_Message,Error3_MessageSize,0

        lea     ax,WORD PTR [bp+4]                        ; Call PRTHEX to
        push    ss                                        ; display the opcode.
        push    ax
        push    2
        call    NEAR PTR PRTHEX

        @VIOWRTTTY  PrimRC_Message,PrimRC_MessageSize,0

        lea     ax,WORD PTR [bp+10]                       ; Call PRTHEX to
        push    ss                                        ; display the primary
        push    ax                                        ; return code.
        push    2
        call    NEAR PTR PRTHEX

        @VIOWRTTTY  SecRC_Message,SecRC_MessageSize,0

        lea     ax,WORD PTR [bp+6]                        ; Call PRTHEX to
        push    ss                                        ; display the secondary
        push    ax                                        ; return code.
        push    4
        call    NEAR PTR PRTHEX

        leave
        ret     8

ERROR2  ENDP

;****************************************************************************
;*  PROCEDURE NAME: PRTHEX                                                  *
;*  FUNCTION      : Convert Hex to string and output it to the screen.      *
;*  PARAMETERS    : The hex length will be at [bp+4].                       *
;*                : The hex data pointer will be at [bp+6].                 *
;****************************************************************************
PRTHEX  PROC NEAR
        enter     0,0                                     ; set bp for stack
                                                          ; referencing.
        push    ds                                        ; Save segment
        push    es                                        ; registers.
        cld                                               ; Clear direction flag.

        mov     si,WORD PTR [bp+6]                        ; Set ds:si to source
        mov     ax,WORD PTR [bp+8]                        ; hex data.
        mov     ds,ax

        mov     di,OFFSET HexBuffer                       ; Set es:di to string
        mov     ax,SEG    HexBuffer                       ; output buffer.
        mov     es,ax

        mov     cx,[bp+4]                                 ; Set loop counter.
        mov     HexBufferFilledSize,cx                    ; Set string length
        shl     HexBufferFilledSize,1                     ; to double hex data
                                                          ; size.

        mov     dx,SEG     DigitTable                     ; Get pointer to
        mov     bx,OFFSET  DigitTable                     ; translation table

TopOfLoop:
        lodsb                                             ; Load next byte.
        push    cx
        push    ax
        and     al,0F0h                                   ; Get high order digit.
        mov     cl,4
        shr     al,cl
        push    bx
        add     bx,ax                                     ; Set table offset
        push    ds
        mov     ds,dx
        mov     al,[bx]                                   ; Move character to ax
        pop     ds
        pop     bx
        stosb                                             ; Store character in
                                                          ; string
                                                          ; Load next byte.
        pop     ax                                        ; Get low order digit.
        and     al,0Fh
        push    bx
        add     bx,ax                                     ; Set table offset.
        push    ds
        mov     ds,dx
        mov     al,[bx]                                   ; Move character to ax.
        pop     ds
        pop     bx
        stosb                                             ; Store character in
                                                          ; string.
        pop     cx
        loop    TopOfLoop                                 ; Loop

        @VIOWRTTTY  HexBuffer,HexBufferFilledSize,0       ; Display HexBuffer

        pop     es                                        ; Restore registers
        pop     ds
        leave
        ret     6

PRTHEX  ENDP
CODE            ENDS
                END  main
