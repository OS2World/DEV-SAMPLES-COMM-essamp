        PAGE    ,132  ; (CTRL-OH) sets printer to condensed mode
        TITLE   X25MXMIT - Transmitter Sample Program for MASM 5.1
;***************************************************************************
; Module      : X25MXMIT.ASM
;
; Copyright   : (C) Copyright IBM Corp. 1989.1991
;               Licensed Material - Program Property of IBM
;               All Rights Reserved
;
; Compiler    : Microsoft Macro Assembler 5.1
;               Microsoft is a registered trademark of Microsoft Corporation.
;
; Description : This is the transmitter part of the sample program to
;               demonstrate use of the X.25 API supplied in the OS/2
;               Extended Services Communications Manager.
;
;               After initialising the X.25 API, the program places
;               a call to the name given as the parameter to the program.
;               Then transmits several blocks of data before sending an end
;               of data marker. It then waits for the receiver to acknowledge
;               receipt of the data by sending a clear packet.
;
;               In the event of an error, a message is printed,
;               and the program tries to tidy up before exiting.
;
;               The program uses the following X.25 API verbs:
;
;                   X25APPINIT
;                   X25CALL
;                   X25DATASEND
;                   X25DATARECEIVE
;                   X25APPTERM
;
; Note        : This program should be started after the corresponding
;               receiver program, X25MRCV.
;
;               The name given must exist in the X.25 Directory. It should
;               refer to a non-SNA remote SVC.
;
;                D bit acknowledgement can be requested by setting use_dbit
;                EQU d_bit.
;***************************************************************************

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

data        SEGMENT word public 'DATA'

ofs_xvrb        DW  0                                          ; offset of xvrb
ofs_call_data   DW  0                              ; offset of call data buffer
ofs_data_data   DW  0                 ; offset of data buffer for received data
SemAddress      DD  0                                ; address of ram semaphore

buffer_size     EQU 128d                             ; size of Data Send buffer
rcv_buffer_size EQU 512d                            ; size of Call Clear buffer
Dir_Pointer     DD  0                         ; pointer to directory entry name
num_of_messages DB  5d                                      ; that we will send
DirName         DB  8  DUP (' ')                       ; directory name to call
DirNameLen      EQU $-DirName                           ; length of this string
DirNameChars    DW  ?                            ; actual length of input chars
data_type       DB  ?                               ; used by data receive verb
datatosend      DB  40 DUP(' ')
datatosendlen   DW  0
shrdseg         DW  0                  ; segment selector for our shared memory
initalise       EQU 1               ; flag for dossubset to initalise a segment
shareable       EQU 0011b                                ; flag for DosAllocSeg
segsize         EQU 32768                ; size for DosAllocSeg and DosSubAlloc
use_dbit        EQU 0                                ; do not want to use d bit
connid          DD  0                                   ; connection identifier
ErrNum          DW  ?                   ; return code from x25 and system calls
ErrorType       DW  ?                        ; to check if error was Dos or X25
Immediate       EQU 1                                      ;X25 immediate error
Completion      EQU 2                                     ;X25 completion error
DosError        EQU 3                                      ;Error from OS2 call

NoTimeOut       DD  -1                             ; no time out for DosSemWait
hundreds        DB  ?                      ; used for hex to decimal conversion
tens            DB  ?
units           DB  ?
hundred         DW  100d
ten             DW  10d

                                                 ; Messages used by the program
BadEnvMsg       DB  'Bad Enviroment - cannot find directory entry name'
BadEnvLen       EQU $-BadEnvMsg
InitGoodMsg     DB  'X.25 API Initialised'
InitGoodLen     EQU $-InitGoodMsg
InitBadMsg      DB  'Couldn''t initialise X.25 API'
InitBadLen      EQU $-InitBadMsg
MCallMsg        DB  'Making a Call'
MCallLen        EQU $-MCallMsg
MCallGoodMsg    DB  'Connection established to remote DTE'
MCallGoodLen    EQU $-MCallGoodMsg
MCallBadMsg     DB  'Couldn''t make a call'
MCallBadLen     EQU $-MCallBadMsg
ClrErrMsg       DB  'Call was cleared - Cause :'
ClrErrLen       EQU $-ClrErrMsg
SDataGoodMsg    DB  'Sent Data ...'
SDataGoodLen    EQU $-SDataGoodMsg
SDataBadMsg     DB  'Data send failed'
SDataBadLen     EQU $- SDataBadMsg
DataRcvGoodMsg  DB  'Data Receive completed successfully'
DataRcvGoodLen  EQU $-DataRcvGoodMsg
DataRcvBadMsg   DB  'Data receive failed'
DataRcvBadLen   EQU $-DataRcvBadMsg
UknDataMsg      DB  'Unexpected type of data has been received'
UknDataLen      EQU $-UknDataMsg
DiagCodeMsg     DB  ' Diagnostic :'
DiagCodeLen     EQU $-DiagCodeMsg
TermGoodMsg     DB  'X.25 API Terminated'
TermGoodLen     EQU $-TermGoodMsg
TermBadMsg      DB  'Couldn''t Terminate X.25 API'
TermBadLen      EQU $-TermBadMsg
ClearMsg        DB  'Clear indication packet has been received'
ClearLen        EQU $-ClearMsg
EOMSentMsg      DB  'End of data marker sent'
EOMSentLen      EQU $-EOMSentMsg
MessageStr      DB  'ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890'
MessageLen      EQU $-MessageStr
EomStr          DB  '***'
EomLen          EQU $-EomStr
X25ImmErrMsg    DB  'X25 Error occured Immediate return code of :'
X25ImmErrLen    EQU $-X25ImmErrMsg
X25DelErrMsg    DB  'X25 Error occured Completion return code of :'
X25DelErrLen    EQU $-X25DelErrMsg
DosErrMsg       DB  'Dos Error occured return code of :'
DosErrLen       EQU $-DosErrMsg
DosTermMsg      DB  ' Terminating Program'
DosTermLen      EQU $-DosTermMsg
crlf            DB  0Dh, 0Ah                     ;carriage return and line feed

                                                ; variables for character input

data        ENDS

shared_buffer SEGMENT word public 'shared'

xvrb            X25VERB         <>               ; define over the same storage
                org     xvrb                    ; we will override the selector
call_data       X25CALL_DATA    <>                          ; and offset anyway
                org     xvrb
data_data       X25DATA_DATA    <>

shared_buffer ENDS

code        SEGMENT byte public 'CODE'
            ASSUME cs:code,ds:dgroup,ss:stack

;=============================================================================
; Main program starts here
;=============================================================================
x25mxmit    PROC   FAR                                  ; Entry point from OS/2
            mov    word ptr Dir_Pointer+2,AX      ; Save pointer to command line
            mov    word ptr Dir_Pointer,BX
            mov    ax,dgroup                         ; Set up addressability to
            mov    ds,ax                           ; the automatic data segment
            call   allocmem               ; allocate the shared memory segments

            call   parse_dirname            ; get the directory entry to call
            assume es:shared_buffer         ; so we don't need ES: all the time
            mov    es, shrdseg                ; set es to our allocated segment
            call   init_app                                ; initialise the API
            call   make_call                                  ; set up the call

;=============================================================================
; Now send data to Receiver
;=============================================================================
            mov   bx, 0
Filldata:                                   ; copy data into send buffer
            mov   al, [MessageStr + BX]
            mov   [Datatosend + BX ], al
            inc   bx
            cmp   bx, messagelen
            jne   FillData
            mov   datatosendlen,messagelen  ; length of data to send

            mov    cl,0                     ; use cl to count the messages sent
LoopSend:

            call   send_data
            inc    cl
            cmp    cl, num_of_messages      ; test to see if all data sent
            jne    LoopSend

;=============================================================================
; Now send end of data marker
;=============================================================================
            mov   bx, 0
Eomdata:                                    ; copy data into send buffer
            mov   al, [EomStr + BX]
            mov   [Datatosend + BX ], al
            inc   bx
            cmp   bx, Eomlen
            jne   EomData
            mov   datatosendlen,eomlen      ; length of data to send

            call   send_data
            @VioWrtTTY EOMSentMsg, EOMSentLen,0
            @VioWrtTTY crlf,2,0

;=============================================================================
; Now wait for Receiver to send clear packet
;=============================================================================
looprcv:
            call   rcv_data                             ; receive data
            cmp    data_type, X25DATARCV_CLEAR          ; see if clear packet
            je     finish
            @VioWrtTTY UknDataMsg, UknDataLen,0
            @VioWrtTTY crlf,2,0
            jmp    looprcv                       ; repeat until clear received
finish:
            @VioWrtTTY ClearMsg, ClearLen,0
            @VioWrtTTY crlf,2,0
            call   term_app                                 ; terminate the API

            @dosexit       1,0                     ; Exit with return code of 0

;=============================================================================
; Get Directory name
;=============================================================================
parse_dirname proc

; Gets the directory name from the command line and places it into a variable
; for later use

         les     di,Dir_Pointer              ;points to command line in env str
         xor     ax,ax                                             ;set AL = 00
         mov     cx,100                           ;max size to scan = 100 bytes
         cld                                               ;set DF to increment
Pf_Skip_Command:
         scas    es:byte ptr [di]                 ;look for 00 (end of command)
         je      Pf_Next                           ;we are now past the command
         dec     cx                                             ;else try again
         jcxz    Pf_Bad_Env                  ;if we count out,something's wrong
         jmp     Pf_Skip_Command               ;keep looking for end of command
Pf_Next:
         cmp     byte ptr es:0 [di],20h        ;Usually we have a leading blank
         jne     Pf_Noadj                          ;If not, the name addr is ok
         inc     di                            ;Else, increment to skip leading
                                               ;blank
         jmp     Pf_Next                       ;Keep looking for first nonblank
Pf_Noadj:
         mov     word ptr Dir_Pointer,di          ;reset ptr to indicate ASCIIZ
                                                  ;filename
         xor     cx,cx                                         ;string size = 0
         mov     si,0                                      ;offset into DirName
                                                           ;to put data
Pf_Loop:
         mov     al,byte ptr es:[di]                        ;get a byte of name
         cmp     al,00                                      ;end of name flag ?
         je      Pf_Done                           ;if so we are done moving it
         cmp     al,20h                                        ;is it a blank ?
         Je      Pf_Skipit                                   ;if blank, skip it
         mov     byte ptr [DirName + si],al                      ;move the data
         inc     si                                     ;SI = index in DirNname
         inc     cx                                            ;increment count
Pf_Skipit:
         inc     di                            ;DI = source index in env string
         cmp     cx,DirNameLen                              ;check for max size
         je      Pf_8_Bytes                            ;only take first 8 bytes
         Jmp     Pf_Loop                                       ;go move another
Pf_Done:
         mov     byte ptr [DirName + si],al                    ;move the data
         inc     cx                                            ;increment count
Pf_8_Bytes:
         mov     DirNameChars,cx
         ret

Pf_Bad_env:                                                ;error in env string
         @VioWrtTTY BadEnvMsg, BadEnvLen,0  ;issue message
         @VioWrtTTY crlf,2,0
         @DosExit 0,0                       ;and quit
parse_dirname endp
;=============================================================================
; Make and Call X25APPINIT
;=============================================================================
init_app    PROC                                 ;make verb and call X25APPINIT
            mov   di, ofs_xvrb
            mov   [di+xvrb.verb_code], X25APPINIT
            mov   [di+xvrb.version_id], X25_API_VERSION
            mov   [di+xvrb.data_buffer_size], 0   ; The data buffer is not used

            @X25                                                 ; call the API
            cmp   ax, X25_OK                               ; check immediate RC
            je    NoImmErr1
            mov   ErrorType, Immediate               ; Indicate immediate error
            jmp   InitErr
NoImmErr1:
            call  SemWait                           ; wait for verb to complete
            mov   di, ofs_xvrb
            mov   ax, [di+xvrb.return_code]
            cmp   ax, X25_OK                              ; check Completion RC
            je    InitGood
            mov   ErrorType, Completion             ; Indicate completion error
            jmp   InitErr
InitErr:
            mov   ErrNum, ax
            @VioWrtTTY InitBadMsg,InitBadlen,0
            @VioWrtTTY crlf,2,0
            call  Error
InitGood:
            @VioWrtTTY InitGoodMsg,InitGoodlen,0
            @VioWrtTTY crlf,2,0
            ret

init_app    ENDP

;=============================================================================
; Make and Call X25CALL
;=============================================================================
make_call   PROC                                    ;make verb and call X25CALL
            mov   di, ofs_xvrb
            mov   [di+xvrb.verb_code], X25CALL
            mov   [di+xvrb.version_id], X25_API_VERSION
            mov   [di+xvrb.queue_number], 0          ; queues are not used here
            @mova [di+xvrb.data_buffer_ptr], ofs_call_data, shrdseg
            mov   [di+xvrb.data_buffer_size], X25CALLDATA_SIZE

            ; indicate that we don't wish to use delivery confirmation for data
            mov   [di+xvrb.control], use_dbit


            mov   di, ofs_call_data
            mov   ax, DirNameChars
            mov   [di+call_data.called_address_length], ax
            mov   [di+call_data.calling_address_length], 0
            mov   [di+call_data.facilities_length], 0
            mov   [di+call_data.cud_length], 0

            mov   bx, 0                                      ; Initialise index
CopyLoop:                                          ;now copy the directory name
            mov   al , byte ptr [DirName+bx]         ;into the call data buffer
            mov   byte ptr [di+call_data.called_address+bx], al
            inc   bx
            cmp   bx, DirNameLen                            ; see if all copied
            jne   CopyLoop

            @VioWrtTTY MCallMsg,MCalllen,0
            @VioWrtTTY crlf,2,0

            @X25                                            ; Call the X.25 API
            cmp   ax, X25_OK                               ; check immediate RC
            je    NoImmErr2
            mov   ErrorType, Immediate               ; Indicate immediate error
            jmp   MakeCallErr
NoImmErr2:
            call  SemWait                           ; wait for verb to complete
            mov   di, ofs_xvrb
            mov   ax, [di+xvrb.return_code]
            cmp   ax, X25_OK                              ; check Completion RC
            je    MakeCallGood
            mov   ErrorType, Completion             ; Indicate completion error
            jmp   MakeCallErr
MakeCallErr:
            mov  ErrNum, ax
            cmp  ax, X25_CALL_CLEARED                     ; ax still has the rc
            je   ClearError
            @VioWrtTTY MCallBadMsg,MCallBadlen,0
            @VioWrtTTY crlf,2,0
            call Error
MakeCallGood:
            @VioWrtTTY MCallGoodMsg,MCAllGoodlen,0
            @VioWrtTTY crlf,2,0
            mov   di, ofs_xvrb
            mov   ax, word ptr [di+xvrb.connection_id]            ; fill in the
            mov   word ptr connid, ax                           ; connection id
            mov   ax, word ptr [di+xvrb.connection_id+2]
            mov   word ptr connid+2, ax
            ret
ClearError:
            @VioWrtTTY ClrErrMsg,ClrErrLen,0
            xor   ax,ax                                              ; clear ax
            mov   al, byte ptr [di+xvrb.cause_code]
            call  WrtDec
            @VioWrtTTY DiagCodeMsg,DiagCodeLen,0
            xor   ax,ax                                              ; clear ax
            mov   al, byte ptr [di+xvrb.diagnostic_code]
            call  WrtDec
            @VioWrtTTY crlf,2,0
            call  Error

make_call   ENDP

;=============================================================================
; Make and Call X25DATASEND
;=============================================================================
send_data   PROC                                ;make verb and call X25DATASEND
            mov   di, ofs_xvrb
            mov   [di+xvrb.verb_code], X25DATASEND      ; function to be called
            mov   [di+xvrb.version_id], X25_API_VERSION
            mov   [di+xvrb.queue_number], 0
            @mova [di+xvrb.data_buffer_ptr], ofs_data_data, shrdseg
            mov   [di+xvrb.data_buffer_size], buffer_size
            mov   [di+xvrb.data_length], messagelen
            mov   ax, word ptr connid+2
            mov   word ptr [di+xvrb.connection_id+2], ax          ; fill in the
            mov   ax, word ptr connid                           ; connection id
            mov   word ptr [di+xvrb.connection_id], ax

            ; now we set the d, m and q bits to off
            ; all 3 bits are in the control byte
            mov   [di+xvrb.control], use_dbit

            mov   di, ofs_data_data
            mov   bx, 0
FillBuffer:
            mov   al, [Datatosend + BX]
            mov   [di+ data_data + BX ], al
            inc   bx
            cmp   bx, datatosendlen
            jne   FillBuffer

            @X25                                            ; Call the X.25 API

            cmp   ax, X25_OK                               ; check immediate RC
            je    NoImmErr3
            mov   ErrorType, Immediate               ; Indicate immediate error
            jmp   SDataErr
NoImmErr3:
            call  SemWait                           ; wait for verb to complete
            mov   di, ofs_xvrb
            mov   ax, [di+xvrb.return_code]
            cmp   ax, X25_OK                              ; check Completion RC
            je    SDataGood
            mov   ErrorType, Completion             ; Indicate completion error
            jmp   SDataErr
SDataErr:
            mov   ErrNum, ax
            @VioWrtTTY SDataBadMsg,SDataBadlen,0
            @VioWrtTTY crlf,2,0
            call  error
SDataGood:
            @VioWrtTTY SDataGoodMsg,SDataGoodlen,0
            @VioWrtTTY crlf,2,0
            ret

send_data   ENDP

;=============================================================================
; Make and Call X25DATARECEIVE
;=============================================================================
rcv_data    PROC                             ;make verb and call X25DATARECEIVE
            mov    di, ofs_xvrb
            mov    [di+xvrb.verb_code], X25DATARECEIVE  ; function to be called
            mov    [di+xvrb.version_id], X25_API_VERSION
            mov    [di+xvrb.queue_number], 0
            @mova  [di+xvrb.data_buffer_ptr], ofs_data_data, shrdseg
            mov    ax, rcv_buffer_size          ; size of the call clear packet
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
            ret

rcv_data    ENDP

;=============================================================================
; Make and Call X25APPTERM
;=============================================================================
term_app    PROC                                 ;make verb and call X25APPTERM
            mov   di, ofs_xvrb
            mov   [di+xvrb.verb_code], X25APPTERM
            mov   [di+xvrb.version_id], X25_API_VERSION

            @X25                                            ; Call the X.25 API
            cmp   ax, X25_OK                               ; check immediate RC
            je    NoImmErr6
            mov   ErrorType, Immediate               ; Indicate immediate error
            jmp   TermErr
NoImmErr6:
            call  SemWait                           ; wait for verb to complete
            mov   di, ofs_xvrb
            mov   ax, [di+xvrb.return_code]
            cmp   ax, X25_OK                              ; check Completion RC
            je    TermGood
            mov   ErrorType, Completion             ; Indicate completion error
            jmp   TermErr
TermErr:
            mov   ErrNum, ax
            @VioWrtTTY TermBadMsg,TermBadlen,0
            @VioWrtTTY crlf,2,0
            call  Error
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
                                                     ; and allocated to shrdseg
            cmp    ax, 0                                     ; check rc is zero
            je     SubSet                           ; then jump to next command
            mov    ErrNum, ax                            ; else store the error
            mov    ErrorType, DosError                    ; note the error type
            Call   Error                   ; and show that an error has occured
SubSet:
            @DosSubSet shrdseg, initalise, segsize             ; initialise for
                                                                ; suballocation
            @DosSubAlloc shrdseg,ofs_xvrb, X25VERB_SIZE
            @DosSubAlloc shrdseg,ofs_call_data, X25CALLDATA_SIZE
            @DosSubAlloc shrdseg,ofs_data_data, rcv_buffer_size
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
            mov    ax, ErrorType                       ; place error type in AX
            cmp    ax, Immediate                 ; compare with immediate error
            je     ImmErr                      ; If immediate error goto ImmErr
            cmp    ax, DosError                       ; commpare with Dos error
            je     DosErr                            ; If Dos error goto DosErr
CompErr:
            @VioWrtTTY x25DelErrMsg,x25DelErrLen,0 ;indicate error has occurred
            mov    ax, ErrNum                       ; Move error number into AX
            call   WrtDec                            ; Display out error number
            jmp    terminate                                ; terminate program
ImmErr:
            @VioWrtTTY x25ImmErrMsg,x25ImmErrLen,0 ;indicate error has occurred
            mov    ax, ErrNum                       ; Move error number into AX
            call   WrtDec                            ; Display out error number
            jmp    terminate                                ; terminate program
DosErr:
            @VioWrtTTY DosErrMsg,DosErrLen,0      ; indicate error has occurred
            mov    ax, ErrNum                       ; Move error number into AX
            call   WrtDec                            ; Display out error number
                                                            ; terminate program
terminate:  @VioWrtTTY crlf,2,0                 ; carriage return and line feed
            @VioWrtTTY DosTermMsg,DosTermLen,0    ; Display terminating message
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
            add    hundreds, 30h     ; add 30h to make it a printable character
            mov    ax, dx                              ; move remainder into ax
            cwd                               ; extends ax into the dx register
            div    ten                       ; divide the word by word value 10
            mov    tens, al                   ; quotient in ax, remainder in dx
            add    tens, 30h         ; add 30h to make it a printable character
            mov    units, dl                               ; remainder is units
            add    units, 30h        ; add 30h to make it a printable character
            @VioWrtTTY hundreds,1,0                        ; print out hundreds
            @VioWrtTTY tens,1,0                                ; print out tens
            @VioWrtTTY units,1,0                              ; print out units
            ret

WrtDec      ENDP

x25mxmit    ENDP
code        ENDS
            END    x25mxmit
