        PAGE    ,132  ; (CTRL-OH) sets printer to condensed mode
        TITLE   X25MRCV.ASM - Receiver Sample Program for MASM 5.1
;*************************************************************************
; Module      : X25MRCV.ASM
;
; Copyright   : (C) Copyright IBM Corp. 1989,1991
;               Licensed Material - Program Property of IBM
;               All Rights Reserved
;
; Compiler    : Microsoft Macro Assembler 5.1
;               Microsoft is a registered trademark of Microsoft Corporation.
;
; Description : This is the receiver part of the sample program to
;               demonstrate use of the X.25 API supplied in the OS/2
;               Extended Services Communications Manager.
;
;               After initialising the X.25 API, the program starts
;               listening for an incoming call corresponding to a name
;               in the routing table. This name is given as a parameter
;               to the program.
;
;               When the incoming call is received, it is accepted and
;               the program then starts to receive data transmitted from
;               from the remote application. Acknowledgments are sent
;               when required.
;
;               The program loops until a an end of data marker is
;               received. It then sends a clear packet to acknowledge
;               that all data has been received before tidying up and
;               exiting.
;
;               The program loops until a Clear Indication packet is
;               received. It then tidies up, and exits.
;
;               In the event of an error, a message is printed,
;               and the program tries to tidy up before exiting.
;
;               The program uses the following X.25 API verbs:
;
;                   X25APPINIT
;                   X25LISTEN
;                   X25CALLRECEIVE
;                   X25CALLACCEPT
;                   X25CALLCLEAR
;                   X25DATARECEIVE
;                   X25ACK
;                   X25DEAFEN
;                   X25APPTERM
;
; Note        : The name supplied as a parameter must already exist in
;               the routing table. It must allow reception of any
;               incoming calls. An entry created from model profile M7
;               is suitable for this purpose.
;
;               This program should be started before the transmitter
;               program.
;*************************************************************************
.286p                           ; Allows use of 286 protected mode instructions
.xlist                                ; turns off the listing of included files
            INCLUDE sysmac.inc              ; Include macros for OS/2 functions
            INCLUDE x25_aset.inc                                 ; X25 includes
            INCLUDE x25_a.inc
.list                                                ; restores the list option

@mova      macro    full_address, ofs, sel      ; move selector and offset
            mov   ax, sel                       ; move address of data selector
            mov   word ptr full_address+2,ax    ; and
            mov   ax, ofs                       ; offset into the data buffer
            mov   word ptr full_address,ax      ; double word
           endm

@X25       macro                                        ; define x25 call macro
            push shrdseg
            push ofs_xvrb
            call far ptr x25
           endm



stack       SEGMENT word stack 'STACK'
            DB     3500d DUP (?)                         ; define our stacksize
stack       ENDS

dgroup      GROUP  data                               ; DGROUP required by OS/2

data            SEGMENT word public 'DATA'


ofs_xvrb        DW  0                                          ; offset of xvrb
ofs_call_data   DW  0                              ; offset of call data buffer
ofs_data_data   DW  0                 ; offset of data buffer for received data
ofs_listen_data DW  0                            ; offset of listen data buffer
SemAddress      DD  0                                ; address of ram semaphore

buffer_size     EQU 4096d                             ;data receive buffer size
message_size    EQU 36d                            ;length of message in buffer
Rt_Pointer      DD  0                     ; pointer to routing table entry name
InputString     DB  9 DUP (?)                           ; buffer for char input
InputStringLen  EQU $-InputString                             ;length of buffer
ValidChars      DW  ?                            ; actual number of chars input
TableName       DB  8 DUP (' ')                   ; name in table to listen for
TableNameLen    EQU $-TableName                            ;length of TableName
data_type       DB  ?                               ; used by data receive verb
connid          DD  0                                   ; connection identifier
data_complete   DB  ?                               ;set when all data received
complete        EQU 1                                ;all data received setting
more_to_come    EQU 0                               ;more data to come setting
ErrNum          DW  ?                   ; return code from x25 and system calls
ErrorType       DW  ?                        ; to check if error was Dos or X25
Immediate       EQU 1                                      ;X25 immediate error
Completion      EQU 2                                     ;X25 completion error
DosError        EQU 3                                      ;Error from OS2 call

shrdseg         DW  0                  ; segment selector for our shared memory
initalise       EQU 1               ; flag for dossubset to initalise a segment
shareable       EQU 0011b                                ; flag for DosAllocSeg
segsize         EQU 32768                ; size for DosAllocSeg and DosSubAlloc

NoTimeOut       DD  -1                             ; no time out for DosSemWait
hundreds        DB  ?                      ; used for hex to decimal conversion
tens            DB  ?
units           DB  ?
hundred         DW  100d
ten             DW  10d

                                                 ; Messages used by the program
BadEnvMsg       DB  'Bad Enviroment - cannot find routing table entry name'
BadEnvLen       EQU $-BadEnvMsg
InitGoodMsg     DB  'X.25 API Initialised'
InitGoodLen     EQU $-InitGoodMsg
InitBadMsg      DB  'Couldn''t initialise X.25 API'
InitBadLen      EQU $-InitBadMsg
ListenGoodMsg   DB  'Now ready to receive an incoming call'
ListenGoodLen   EQU $-ListenGoodMsg
ListenBadMsg    DB  'Listen failed'
ListenBadLen    EQU $-ListenBadMsg
CallRcvGoodMsg  DB  'Incoming call has arrived'
CallRcvGoodLen  EQU $-CallRcvGoodMsg
CallRcvBadMsg   DB  'Call receive failed'
CallRcvBadLen   EQU $-CallRcvBadMsg
CallAccGoodMsg  DB  'Call accepted OK'
CallAccGoodLen  EQU $-CallAccGoodMsg
CallAccBadMsg   DB  'Call accept failed'
CallAccBadLen   EQU $-CallAccBadMsg
DataRcvGoodMsg  DB  'Data Receive completed successfully'
DataRcvGoodLen  EQU $-DataRcvGoodMsg
DataRcvBadMsg   DB  'Data receive failed'
DataRcvBadLen   EQU $-DataRcvBadMsg
AckGoodMsg      DB  'Data acknowledged'
AckGoodLen      EQU $-AckGoodMsg
AckBadMsg       DB  'Ack failed'
AckBadLen       EQU $-AckBadMsg
EOMRcvMsg       DB  'End of data marker received'
EOMRcvLen       EQU $-EOMRcvMsg
ClrCallGoodMsg  DB  'Call cleared OK'
ClrCallGoodLen  EQU $-ClrCallGoodMsg
ClrCallBadMsg   DB  'Call Clear failed'
ClrCallBadLen   EQU $-ClrCallBadMsg
ClrErrMsg       DB  'Call was cleared - Cause :'
ClrErrLen       EQU $-ClrErrMsg
DeafenGoodMsg   DB  'Deafen successful'
DeafenGoodLen   EQU $-DeafenGoodMsg
DeafenBadMsg    DB  'Deafen failed'
DeafenBadLen    EQU $-DeafenBadMsg
ClearMsg        DB  'Clear indication packet has been received'
ClearLen        EQU $-ClearMsg
UknDataMsg      DB  'Unexpected type of data has been received'
UknDataLen      EQU $-UknDataMsg
TermGoodMsg     DB  'X.25 API Terminated'
TermGoodLen     EQU $-TermGoodMsg
TermBadMsg      DB  'Couldn''t Terminate X.25 API'
TermBadLen      EQU $-TermBadMsg
X25ImmErrMsg    DB  'X25 Error Occured Immediate Return Code of :'
X25ImmErrLen    EQU $-X25ImmErrMsg
X25DelErrMsg    DB  'X25 Error Occured Completion Return Code of :'
X25DelErrLen    EQU $-X25DelErrMsg
DosErrMsg       DB  'Dos Error Occured Return Code of :'
DosErrLen       EQU $-DosErrMsg
DosTermMsg      DB  ' Terminating Program'
DosTermLen      EQU $-DosTermMsg
crlf            DB  0Dh, 0Ah                     ;carriage return and line feed

data            ENDS                             ; end of the main data segment

shared_buffer   SEGMENT word public 'shared'

xvrb            X25VERB         <>
                org     xvrb                     ; define over the same storage
call_data       X25CALL_DATA    <>              ; we will override the selector
                org     xvrb                                ; and offset anyway
data_data       X25DATA_DATA    <>
                org     xvrb
listen_data     X25LISTEN_DATA  <>

shared_buffer   ENDS

code        SEGMENT byte public 'CODE'
            ASSUME cs:code,ds:dgroup,ss:stack

;=============================================================================
; Main program starts here
;=============================================================================
x25mrcv     PROC   FAR                                  ; Entry point from OS/2
            mov    word ptr Rt_Pointer+2,AX          ;Save command line pointer
            mov    word ptr Rt_Pointer,BX
            mov    ax,dgroup                         ; Set up addressability to
            mov    ds,ax                           ; the automatic data segment
            call   allocmem               ; allocate the shared memory segments

            call   parse_rtname        ; get parameter passed from env string
            assume es:shared_buffer    ; saves us preceding everything with ES:
            mov    es, shrdseg         ; set ES to our allocated segment
            call   init_app                                     ; start the API
            call   listen                                ; listen to the sender
            call   callrcv                                ; and wait for a call
            call   callacc                                    ; accept the call
            mov    data_complete,more_to_come           ;indicate start of data
looprcv:
            call   rcv_data                             ; receive data
            cmp    data_complete,complete             ;see if all data received
            je     finish
            cmp    data_type, X25DATARCV_DATA     ; see if data packet received
            je     looprcv                                   ;receive more data
            cmp    data_type, X25DATARCV_CLEAR       ;see if data type is clear
            jne    Unexpected                     ;if it is not goto unexpected
            @VioWrtTTY ClearMsg, ClearLen,0           ; say we received a clear
            @VioWrtTTY crlf,2,0
            jmp    finish
Unexpected:
            @VioWrtTTY UknDataMsg, UknDataLen,0           ; not data or a clear
            @VioWrtTTY crlf,2,0                                 ; finish anyway
finish:
            call   clear_call                  ; finished so clear the call and
            call   deafen                                      ; stop listening
            call   term_app                               ; finish with the API

            @dosexit       1,0                     ; Exit with return code of 0

;=============================================================================
; Get Routing Table entry name
;=============================================================================
parse_rtname proc

; Gets the routing table entry name from the command line and places it into
; a variable for later use

         les     di,Rt_Pointer              ;points to command line in env str
         xor     ax,ax                      ;set AL = 00
         mov     cx,100                     ;max size to scan = 100 bytes
         cld                                ;set DF to increment
Pf_Skip_Command:
         scas    es:byte ptr [di]           ;look for 00 (end of command)
         je      Pf_Next                    ;we are now past the command
         dec     cx                         ;else try again
         jcxz    Pf_Bad_Env                 ;if we count out,something's wrong
         jmp     Pf_Skip_Command            ;keep looking for end of command
Pf_Next:
         cmp     byte ptr es:0 [di],20h     ;Usually we have a leading blank
         jne     Pf_Noadj                   ;If not, the name addr is ok
         inc     di                         ;Else, increment to skip leading
                                            ;blank
         jmp     Pf_Next                    ;Keep looking for first nonblank
Pf_Noadj:
         mov     word ptr Rt_Pointer,di     ;reset ptr to indicate ASCIIZ
                                            ;filename
         xor     cx,cx                      ;string size = 0
         mov     si,0                       ;offset into TableName
                                            ;to put data
Pf_Loop:
         mov     al,byte ptr es:[di]        ;get a byte of name
         cmp     al,00                      ;end of name flag ?
         je      Pf_Done                    ;if so we are done moving it
         cmp     al,20h                     ;is it a blank ?
         Je      Pf_Skipit                  ;if blank, skip it
         mov     byte ptr [TableName + si],al        ;move the data
         inc     si                         ;SI = index in TableName
         inc     cx                         ;increment count
Pf_Skipit:
         inc     di                         ;DI = source index in env string
         cmp     cx,TableNameLen            ;check for max size
         je      Pf_8_Bytes                 ;only take first 8 bytes
         Jmp     Pf_Loop                    ;go move another
Pf_Done:
         mov     byte ptr [TableName + si],al        ;move the data
Pf_8_Bytes:
         ret

Pf_Bad_env:                                 ;error in env string
        @VioWrtTTY BadEnvMsg, BadEnvLen,0   ;issue message
        @VioWrtTTY crlf,2,0
        @DosExit 0,0                        ;and quit
parse_rtname endp
;=============================================================================
; Make and Call X25APPINIT
;=============================================================================
init_app    PROC                                 ;make verb and call X25APPINIT
            mov    di, ofs_xvrb                      ; address our given offset
            mov    [di+xvrb.verb_code], X25APPINIT
            mov    [di+xvrb.version_id], X25_API_VERSION
            mov    [di+xvrb.data_buffer_size], 0  ; The data buffer is not used

            @X25                                            ; Call the X.25 API

            cmp    ax, X25_OK                              ; check immediate RC
            je     NoImmErr1
            mov    ErrorType, Immediate              ; Indicate immediate error
            jmp    InitErr
NoImmErr1:
            call   SemWait                          ; wait for verb to complete
            mov    di, ofs_xvrb
            mov    ax, [di+xvrb.return_code]
            cmp    ax, X25_OK                             ; check Completion RC
            je     InitGood
            mov    ErrorType, Completion            ; Indicate completion error
            jmp    InitErr
InitErr:
            mov    ErrNum, ax
            @VioWrtTTY InitBadMsg,InitBadlen,0       ; report failure then call
            @VioWrtTTY crlf,2,0
            call   Error                                    ; the error routine
Initgood:
            @VioWrtTTY InitGoodMsg,InitGoodlen,0
            @VioWrtTTY crlf,2,0
            ret

init_app    ENDP

;=============================================================================
; Make and Call X25LISTEN
;=============================================================================
listen      PROC                                  ;make verb and call X25LISTEN
            mov    di, ofs_xvrb
            mov    [di+xvrb.verb_code], X25LISTEN
            mov    [di+xvrb.version_id], X25_API_VERSION
            mov    [di+xvrb.queue_number], 0         ; queues are not used here
            @mova  [di+xvrb.data_buffer_ptr], ofs_listen_data, shrdseg
            mov    [di+xvrb.data_buffer_size], X25LISTENDATA_SIZE

            mov    di, ofs_listen_data                          ; buffer offset
            mov    [di+listen_data.num_of_rtes], 1          ; set for one entry

            mov    bx, 0                                     ; initialise index
copyloop:
            mov    al , byte ptr[TableName + bx]
            mov    byte ptr [di+listen_data.rte_entry_list + bx], al
            inc    bx                     ; copy the contents of tablename into
            cmp    bx, TableNameLen          ; first position of the entry_list
            jne    copyloop                ; copy all 8 chars, unused ones will
                                                                    ; be spaces
            @X25                                            ; Call the X.25 API
            cmp    ax, X25_OK                              ; check immediate RC
            je     NoImmErr2
            mov    ErrorType, Immediate              ; Indicate immediate error
            jmp    ListenErr
NoImmErr2:
            call   SemWait                          ; wait for verb to complete
            mov    di, ofs_xvrb
            mov    ax, [di+xvrb.return_code]
            cmp    ax, X25_OK                             ; check Completion RC
            je     ListenGood
            mov    ErrorType, Completion            ; Indicate completion error
            jmp    ListenErr                         ; To be safe if code added
ListenErr:
            mov    ErrNum, ax                               ; store error value
            @VioWrtTTY ListenBadMsg,ListenBadlen,0
            @VioWrtTTY crlf,2,0
            call   Error                                 ; call error procedure
Listengood:
            @VioWrtTTY ListenGoodMsg,ListenGoodlen,0
            @VioWrtTTY crlf,2,0
            ret

listen      ENDP

;=============================================================================
; Make and Call X25CALLRECEIVE
;=============================================================================
callrcv     PROC                             ;make verb and call X25CALLRECEIVE
            mov    di, ofs_xvrb
            mov    [di+xvrb.verb_code], X25CALLRECEIVE  ; function to be called
            mov    [di+xvrb.version_id], X25_API_VERSION
            mov    [di+xvrb.verb_option], X25CALLRCV_ADDRESS
            mov    [di+xvrb.queue_number], 0
            @mova  [di+xvrb.data_buffer_ptr], ofs_call_data, shrdseg
            mov    [di+xvrb.data_buffer_size], X25CALLDATA_SIZE

            @X25                                            ; Call the X.25 API
            cmp    ax, X25_OK                              ; check immediate RC
            je     NoImmErr3
            mov    ErrorType, Immediate              ; Indicate immediate error
            jmp    CallRcvErr

NoImmErr3:  call   SemWait                          ; wait for verb to complete
            mov    di, ofs_xvrb
            mov    ax, [di+xvrb.return_code]
            cmp    ax, X25_OK                             ; check Completion RC
            je     CallRcvGood
            mov    ErrorType, Completion            ; Indicate completion error
            jmp    CallRcvErr
CallRcvErr:
            mov    ErrNum, ax
            @VioWrtTTY CallRcvBadMsg,CallRcvBadlen,0
            @VioWrtTTY crlf,2,0
            call   Error

CallRcvGood:
            mov   ax, word ptr [di+xvrb.connection_id]
            mov   word ptr connid, ax                    ; fill in the returned
            mov   ax, word ptr [di+xvrb.connection_id+2]        ; connection id
            mov   word ptr connid+2, ax
            @VioWrtTTY CallRcvGoodMsg,CallRcvGoodlen,0
            @VioWrtTTY crlf,2,0
            ret

callrcv     ENDP

;=============================================================================
; Make and Call X25CALLACCEPT
;=============================================================================
callacc     PROC                              ;make verb and call X25CALLACCEPT
            mov    di, ofs_xvrb
            mov    [di+xvrb.verb_code], X25CALLACCEPT   ; function to be called
            mov    [di+xvrb.version_id], X25_API_VERSION
            mov    [di+xvrb.queue_number], 0
            mov    [di+xvrb.data_buffer_size], 0
            mov    ax, word ptr connid+2
            mov    word ptr [di+xvrb.connection_id+2], ax         ; fill in the
            mov    ax, word ptr connid                          ; connection id
            mov    word ptr [di+xvrb.connection_id], ax
            mov    [di+xvrb.control],d_bit_management
                                                       ; acks to be done by app

            @X25                                            ; Call the X.25 API
            cmp    ax, X25_OK                              ; check immediate RC
            je     NoImmErr4
            mov    ErrorType, Immediate              ; Indicate immediate error
            jmp    CallAccErr
NoImmErr4:
            call   SemWait                          ; wait for verb to complete
            mov    di, ofs_xvrb
            mov    ax, [di+xvrb.return_code]
            cmp    ax, X25_OK                             ; check Completion RC
            je     CallAccGood
            mov    ErrorType, Completion            ; Indicate completion error
            jmp    CallAccErr
CallAccErr:
            mov    ErrNum, ax
            @VioWrtTTY CallAccBadMsg,CallAccBadlen,0
            @VioWrtTTY crlf,2,0
            call   Error
CallAccGood:
            @VioWrtTTY CallAccGoodMsg,CallAccGoodlen,0
            @VioWrtTTY crlf,2,0
            ret

CallAcc     ENDP

;=============================================================================
; Make and Call X25DATARECEIVE
;=============================================================================
rcv_data    PROC                             ;make verb and call X25DATARECEIVE
            mov    di, ofs_xvrb
            mov    [di+xvrb.verb_code], X25DATARECEIVE  ; function to be called
            mov    [di+xvrb.version_id], X25_API_VERSION
            mov    [di+xvrb.queue_number], 0
            @mova  [di+xvrb.data_buffer_ptr], ofs_data_data, shrdseg
            mov    ax, buffer_size                    ; size of the data packet
            mov    [di+xvrb.data_buffer_size], ax
            mov    ax, word ptr connid+2
            mov    word ptr [di+xvrb.connection_id+2], ax         ; fill in the
            mov    ax, word ptr connid                          ; connection id
            mov    word ptr [di+xvrb.connection_id], ax

            @X25                                            ; Call the X.25 API
            cmp    ax, X25_OK                              ; check immediate RC
            je     NoImmErr5
            mov    ErrorType, Immediate              ; Indicate immediate error
            jmp    DataRcvErr
NoImmErr5:
            call   SemWait                          ; wait for verb to complete
            mov    di, ofs_xvrb
            mov    ax, [di+xvrb.return_code]
            cmp    ax, X25_OK                             ; check Completion RC
            je     DataRcvGood
            mov    ErrorType, Completion            ; Indicate completion error
            jmp    DataRcvErr
DataRcvErr:
            mov    ErrNum, ax
            @VioWrtTTY DataRcvBadMsg,DataRcvBadlen,0
            @VioWrtTTY crlf,2,0
            call   Error
DataRcvGood:
            @VioWrtTTY DataRcvGoodMsg,DataRcvGoodlen,0
            @VioWrtTTY crlf,2,0

            mov    di, ofs_xvrb
            mov    al, [di+xvrb.data_event_type]
            mov    data_type, al                           ; save received type
            cmp    al, X25DATARCV_DATA                            ; see if data
            jne    Rcv_end                           ; if not data goto rcv_end
            push   di
            mov    di, ofs_data_data                 ; load buffer offset
            cmp    byte ptr es:[di], '*'             ; test buffer first byte
            pop    di
            jne    Displaydata
            mov    data_complete,complete
            @VioWrtTTY EOMRcvMsg,EOMRcvlen,0
            @VioWrtTTY crlf,2,0
            jmp    Nodisplay
Displaydata:
            push   shrdseg                                      ; push selector
            push   ofs_data_data                                  ; push offset
            push   MESSAGE_SIZE                             ; push message size
            push   0
            call    far ptr VIOWRTTTY                             ;display data
            @VioWrtTTY crlf,2,0
Nodisplay:
                                                   ; check if the D bit is set,
                                                         ; if so acknowledge it
            test   [di+xvrb.control], d_bit
            jz     rcv_end                                   ; d bit is not set

            mov    al,[di+xvrb.data_event_type]
            cmp    al, X25DATARCV_DATA               ; did we receive data ?
            jne    rcv_end                           ; no, so don't acknowledge
            call   ack_data                     ; else we must send an ack back
rcv_end :   ret

rcv_data    ENDP

;=============================================================================
; Make and Call X25ACK
;=============================================================================
ack_data    PROC                                     ;make verb and call X25ACK

            mov    di, ofs_xvrb
            mov    [di+xvrb.verb_code], X25ACK
            mov    [di+xvrb.version_id], X25_API_VERSION
            mov    [di+xvrb.queue_number], 0         ; queues are not used here
            mov    ax, word ptr connid+2
            mov    word ptr [di+xvrb.connection_id+2], ax         ; fill in the
            mov    ax, word ptr connid                          ; connection id
            mov    word ptr [di+xvrb.connection_id], ax

            @X25                                            ; Call the X.25 API
            cmp    ax, X25_OK                              ; check immediate RC
            je     NoImmErr6
            mov    ErrorType, Immediate              ; Indicate immediate error
            jmp    AckErr
NoImmErr6:
            call   SemWait                          ; wait for verb to complete
            mov    di, ofs_xvrb
            mov    ax, [di+xvrb.return_code]
            cmp    ax, X25_OK                             ; check Completion RC
            je     AckGood
            mov    ErrorType, Completion            ; Indicate completion error
            jmp    AckErr
AckErr:
            mov    ErrNum, ax
            @VioWrtTTY AckBadMsg,AckBadlen,0
            @VioWrtTTY crlf,2,0
            call   Error
AckGood:
            @VioWrtTTY AckGoodMsg,AckGoodlen,0
            @VioWrtTTY crlf,2,0
            ret

ack_data    ENDP

;=============================================================================
; Make and Call X25CALLCLEAR
;=============================================================================
clear_call  PROC                               ;make verb and call X25CALLCLEAR
            mov   di, ofs_xvrb
            mov   [di+xvrb.verb_code], X25CALLCLEAR
            mov   [di+xvrb.version_id], X25_API_VERSION
            mov   [di+xvrb.queue_number], 0          ; queues are not used here
            mov   ax, word ptr connid+2
            mov   word ptr [di+xvrb.connection_id+2], ax          ; fill in the
            mov   ax, word ptr connid                           ; connection id
            mov   word ptr [di+xvrb.connection_id], ax
            mov   [di+xvrb.data_buffer_size], 0
            mov   [di+xvrb.cause_code], 0
            mov   [di+xvrb.diagnostic_code], 0

            ; We want the verb to complete before the clear confirm packet
            ; has been received so we set the option to no wait
            mov [di+xvrb.verb_option], X25CALLCLEAR_NOWAIT

            @X25                                            ; Call the X.25 API
            cmp   ax, X25_OK                               ; check immediate RC
            je    NoImmErr7
            mov   ErrorType, Immediate               ; Indicate immediate error
            jmp   ClrCallErr
NoImmErr7:
            call  SemWait                           ; wait for verb to complete
            mov   di, ofs_xvrb
            mov   ax, [di+xvrb.return_code]
            cmp   ax, X25_OK                              ; check Completion RC
            je    ClrCallGood
            mov   ErrorType, Completion             ; Indicate completion error
            jmp   ClrCallErr
ClrCallErr:
            mov   ErrNum, ax
            @VioWrtTTY ClrCallBadMsg,ClrCallBadlen,0
            @VioWrtTTY crlf,2,0
            call  Error
ClrCallGood:
            @VioWrtTTY ClrCallGoodMsg,ClrCallGoodlen,0
            @VioWrtTTY crlf,2,0
            ret

clear_call  ENDP


;=============================================================================
; Make and Call X25DEAFEN
;=============================================================================
deafen      PROC                                 ; make verb and call X25DEAFEN
            mov    di, ofs_xvrb
            mov    [di+xvrb.verb_code], X25DEAFEN
            mov    [di+xvrb.version_id], X25_API_VERSION
            mov    [di+xvrb.queue_number], 0         ; queues are not used here
                                                     ; uses same data as listen
            @mova  [di+xvrb.data_buffer_ptr], ofs_listen_data, shrdseg
            mov    [di+xvrb.data_buffer_size], X25LISTENDATA_SIZE

            mov    di, ofs_listen_data
            mov    [di+listen_data.num_of_rtes], 1          ; set for one entry

            mov    bx, 0                                     ; initialise index
copyloop2:
            mov    al , byte ptr [TableName + bx]
            mov    byte ptr [di+listen_data.rte_entry_list + bx], al
            inc    bx                     ; copy the contents of tablename into
            cmp    bx, TableNameLen          ; first position of the entry_list
            jne    copyloop2               ; copy all 8 chars, unused ones will
                                                                    ; be spaces

            @X25                                            ; Call the X.25 API
            cmp    ax, X25_OK                              ; check immediate RC
            je     NoImmErr8
            mov    ErrorType, Immediate              ; Indicate immediate error
            jmp    DeafenErr
NoImmErr8:
            call   SemWait                          ; wait for verb to complete
            mov    di, ofs_xvrb
            mov    ax, [di+xvrb.return_code]
            cmp    ax, X25_OK                             ; check Completion RC
            je     DeafenGood
            mov    ErrorType, Completion            ; Indicate completion error
            jmp    DeafenErr
DeafenErr:
            mov    ErrNum, ax
            @VioWrtTTY DeafenBadMsg,DeafenBadlen,0
            @VioWrtTTY crlf,2,0
            call   Error
DeafenGood:
            @VioWrtTTY DeafenGoodMsg,DeafenGoodlen,0
            @VioWrtTTY crlf,2,0
            ret

deafen      ENDP

;=============================================================================
; Make and Call X25APPTERM
;=============================================================================
term_app    PROC
            mov    di, ofs_xvrb
            mov    [di+xvrb.verb_code], X25APPTERM
            mov    [di+xvrb.version_id], X25_API_VERSION

            @X25                                            ; Call the X.25 API
            cmp    ax, X25_OK                              ; check immediate RC
            je     NoImmErr9
            mov    ErrorType, Immediate              ; Indicate immediate error
            jmp    TermErr
NoImmErr9:
            call   SemWait                          ; wait for verb to complete
            mov    di, ofs_xvrb
            mov    ax, [di+xvrb.return_code]
            cmp    ax, X25_OK                             ; check Completion RC
            je     TermGood
            mov    ErrorType, Completion            ; Indicate completion error
            jmp    TermErr
TermErr:
            mov    ErrNum, ax
            @VioWrtTTY TermBadMsg,TermBadlen,0
            @VioWrtTTY crlf,2,0
            call   Error
TermGood:
            @VioWrtTTY TermGoodMsg,TermGoodlen,0
            @VioWrtTTY crlf,2,0
            ret

term_app    ENDP

;=============================================================================
; Allocate Memory for shared data segment
;=============================================================================
allocmem    PROC
            @DosAllocSeg segsize, shrdseg, shareable    ; size of 32k, sharable
                                                      ; and allocated to shrseg
            cmp    ax,0                                      ; check rc is zero
            je     SubSet                           ; then jump to next command
            mov    ErrNum, ax                            ; else store the error
            mov    ErrorType, DosError                    ; note the error type
            Call   Error                   ; and show that an error has occured
SubSet:
            @DosSubSet shrdseg, initalise, segsize             ; initialise for
                                                                ; suballocation
            @DosSubAlloc shrdseg,ofs_xvrb, X25VERB_SIZE
            @DosSubAlloc shrdseg,ofs_call_data, X25CALLDATA_SIZE
            @DosSubAlloc shrdseg,ofs_data_data, buffer_size
            @DosSubAlloc shrdseg,ofs_listen_data, X25LISTENDATA_SIZE
            ret

allocmem    ENDP

;=============================================================================
; wait for the verb to complete
;=============================================================================
SemWait    PROC                       ; for a RAM semaphore we give the address
                                         ; where the semaphore handle is stored
           mov   di, ofs_xvrb
           lea   ax, [di+xvrb.ram_semaphore]                           ; offset
           mov   word ptr SemAddress,ax
           mov   ax, shrdseg                                         ; selector
           mov   word ptr SemAddress+2,ax
           @DosSemWait SemAddress , NoTimeOut
           ret

SemWait    ENDP

;=============================================================================
; Register an Error
;=============================================================================
Error       PROC
            mov    ax, ErrorType                        ;place error type in AX
            cmp    ax, Immediate                  ;compare with immediate error
            je     ImmErr                       ;If immediate error goto ImmErr
            cmp    ax, DosError                        ;commpare with Dos error
            je     DosErr                             ;If Dos error goto DosErr
CompErr:
            @VioWrtTTY x25DelErrMsg,x25DelErrLen,0 ;indicate error has occurred
            mov    ax, ErrNum                        ;Move error number into AX
            call   WrtDec                             ;Display out error number
            jmp    terminate                                 ;terminate program
ImmErr:
            @VioWrtTTY x25ImmErrMsg,x25ImmErrLen,0 ;indicate error has occurred
            mov    ax, ErrNum                        ;Move error number into AX
            call   WrtDec                             ;Display out error number
            jmp    terminate                                 ;terminate program
DosErr:
            @VioWrtTTY DosErrMsg,DosErrLen,0       ;indicate error has occurred
            mov    ax, ErrNum                        ;Move error number into AX
            call   WrtDec                             ;Display out error number
                                                             ;terminate program
terminate:  @VioWrtTTY crlf,2,0                 ; carriage return and line feed
            @VioWrtTTY DosTermMsg,DosTermLen,0     ;Display terminating message
            @dosexit       1,1                     ; Exit with return code of 1
            ret

Error       ENDP

;=============================================================================
; convert a hex word to a decimal number
;=============================================================================
WrtDec      PROC                        ; write the decimal value of a hex word
                                                 ; less than 1000d stored in ax
                                               ; at the current cursor position
            cwd                               ; extends ax into the dx register
            div    hundred                  ; divide the word by word value 100
            mov    hundreds, al               ; quotient in ax, remainder in dx
            add    hundreds, 30h    ;  add 30h to make it a printable character
            mov    ax, dx                              ; move remainder into ax
            cwd                               ; extends ax into the dx register
            div    ten                       ; divide the word by word value 10
            mov    tens, al                   ; quotient in ax, remainder in dx
            add    tens, 30h        ;  add 30h to make it a printable character
            mov    units, dl                               ; remainder is units
            add    units, 30h       ;  add 30h to make it a printable character
            @VioWrtTTY hundreds,1,0                        ; print out hundreds
            @VioWrtTTY tens,1,0                                ; print out tens
            @VioWrtTTY units,1,0                              ; print out units
            ret

WrtDec      ENDP

x25mrcv     ENDP
code        ENDS
            END    x25mrcv
