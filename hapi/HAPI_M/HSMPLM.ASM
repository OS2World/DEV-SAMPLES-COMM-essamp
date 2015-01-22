;*********************************************************************
;*                                                                   *
;*                                                                   *
;* FILE NAME: HSMPLM.ASM                                             *
;*                                                                   *
;* MODULE NAME= HSMPLM.ASM                                           *
;*                                                                   *
;* DESCRIPTIVE NAME= MASM SAMPLE PROGRAM FOR EHLLAPI                 *
;*                                                                   *
;*                                                                   *
;* Displays EHLLAPI and session information.                         *
;* Writes string to host.                                            *
;* Searches for written string on host.                              *
;* Displays host session screen.                                     *
;* Manipulates the Presentation Manager properties of                *
;* the emulator session to: change window title name, switch         *
;* list name, make window invisible, query window status,            *
;* window coordinates, change window size, and restore the           *
;* emulator session window to its original conditions.               *
;* Next, the structured field functions are used.  The               *
;* communications buffer is queried, the read and write buffers      *
;* allocated, a connection is initiated to the communications        *
;* buffer, and an asynchronus read structured field is issued        *
;* disabling the inbound host.  Then, the sendkey function is        *
;* used to send the command 'IND$FILE PUT SF_TEST EXEC A'            *
;* to the host which puts a non-existent file from the               *
;* PC to the host using a structured field.  Next, a get             *
;* completion request is issued to determine if the                  *
;* previous asynchronus read structured field is completed,          *
;* Upon completion, a synchronus write structured field is           *
;* issued, the communications buffers are de-allocated, and          *
;* then a disconnect from structured field is issued.                *
;*                                                                   *
;*                                                                   *
;*                                                                   *
;*   COPYRIGHT:  XXXXXXXXX  (C) COPYRIGHT IBM CORP. 1987,1988,1989   *
;*               LICENSED MATERIAL - PROGRAM PROPERTY OF IBM         *
;*               ALL RIGHTS RESERVED                                 *
;*                                                                   *
;* NOTES=                                                            *
;*                                                                   *
;**********************-END OF SPECIFICATIONS-************************
.286P
;*********************************************************************
;********************** BEGIN INCLUDE FILES **************************
;*********************************************************************



INCLUDE HAPI_M.INC                            ; Get EHLLAPI
                                              ; include file for MASM



;********************************************************************
;******************** AUTOMATIC DATA SEG STUFF **********************
;********************************************************************

DSEG                  SEGMENT  'DATA'
DSEG                  ENDS

SSEG                  SEGMENT STACK 'STACK'
SSEG                  ENDS

CSEG                  SEGMENT 'CODE'
CSEG                  ENDS

DGROUP  GROUP   DSEG



;********************************************************************
;************************ FUNCTION EXTERNS **************************
;********************************************************************




EXTRN VIOWRTCELLSTR:FAR                        ; DOS vidio call.
EXTRN VIOWRTTTY    :FAR                        ; DOS Write string.
EXTRN DOSEXIT      :FAR                        ; Exit to DOS call.
EXTRN KBDCHARIN    :FAR                        ; DOS keyin routine.
EXTRN DOSSLEEP     :FAR                        ; DOS Sleep.
EXTRN HLLMASM      :FAR                        ; EHLLAPI LIM entry point.


;********************************************************************
;***************************** EQUATES ******************************
;********************************************************************

MAX_DATA_SIZE      EQU      3840               ; The maximum data size
                                               ; for this application.
EABS               EQU      80H                ; Extended attribute
                                               ; bit.
PSS                EQU      40H                ; Programmed Symbol
                                               ; Set bit.
CR                 EQU      0DH                ; Return

LF                 EQU      0AH                ; Line feed

NULL               EQU      0                  ; Set Null

HEX8               EQU        8H               ; Set Hexadecimal
HEX10              EQU        10H              ; Constants
HEX80              EQU        80H
HEX100             EQU        100H
HEX400             EQU        400H
HEX800             EQU        800H
HEX808             EQU        808H
HEX1000            EQU        1000H

;********************************************************************
;*************************** BEGIN MACROS ***************************
;********************************************************************



DISPLAY            MACRO    MSG
                   LOCAL    TEXT

DSEG               SEGMENT  'DATA'

TEXT               DB       MSG
                   DB       NULL

DSEG               ENDS

                   PUSH     BX
                   LEA      BX, TEXT
                   CALL     DSP_MSG
                   POP      BX
                   ENDM





;********************************************************************
;******************* BEGIN STATIC VARIABLES *************************
;********************************************************************


DSEG                SEGMENT  'DATA'

BLANK_SCR           DB       CR,LF,CR,LF,CR,LF,CR,LF,CR,LF
                    DB       CR,LF,CR,LF,CR,LF,CR,LF,CR,LF
                    DB       CR,LF,CR,LF,CR,LF,CR,LF,CR,LF
                    DB       CR,LF,CR,LF,CR,LF,CR,LF,CR,LF
                    DB       CR,LF,CR,LF,CR,LF,CR,LF,CR,LF
                    DB       CR,LF
                    DB       NULL

PRESS_ENT_MSG       DB       CR,LF,CR,LF,'Press ENTER to continue...'
                    DB       NULL


DFT_SESS            DB       NULL               ; Session to write string
                    DB       NULL               ; string to and search from

HOST_TEXT           DB       'EHLLAPI-1'        ; String to send to host
                    DB       NULL               ; and to search for

HOST_TEXT_LEN       EQU      $ - HOST_TEXT -1   ; Length of host text

INHOST_TEXT         DB       'EHLLAPI-2'        ; String to send to host
                    DB       NULL               ; and to search for when
                                                ; screen is invisible

COMMAND_STR         DB       'IND$FILE PUT SF_TEST EXEC A@E'
                    DB       NULL
COMMAND_LEN         EQU       $ - COMMAND_STR


TEST_NAME           DB       'Sample_Test_Name' ; new title name
                    DB        NULL
TEST_NAME_LEN       EQU      $ - TEST_NAME      ; Length of sample text

HOME_KEY            DB       '@L@0'             ; The Home Key.
HOME_KEY_LEN        EQU      $ - HOME_KEY       ; Length

SETPARM_TEXT        DB       'NOATTRB EAB XLATE'; String to to use
SETPARM_TEXT_LEN    EQU      $ - SETPARM_TEXT   ; for set parms.

RC                  DW       NULL               ; Return code.

DIGIT_BUFF          DB       6 DUP (NULL)       ; Buffer to contain
                                                ; ascii decimal digits

COORD_BUFF          DB       8 DUP (NULL)       ; Buffer to contain
                                                ; ascii decimal digits

DUMMY_BUFF          DB       80 DUP (NULL)      ; Misc dummy buffer.

DOUBLE_BUFF         DB       6 DUP (NULL)       ; Misc buffer.

SESS_STUC           DB       SIZE QSST_STRUCT DUP (?) ; session structure

OPTIMAL_INB         DW       NULL               ; Optimal inbound
                                                ; buffer size
OPTIMAL_OUTB        DW       NULL               ; Optimal outbound
                                                ; buffer size

RD_ADDRESS_OFF      DW       NULL               ; Read buffer address
RD_ADDRESS_SEG      DW       NULL

WR_ADDRESS_OFF      DW       NULL               ; Write buffer address
WR_ADDRESS_SEG      DW       NULL

ASEM_SEG            DW       NULL               ; Semaphore segment
ASEM_OFF            DW       NULL               ; Semaphore offset

DOID                DW       NULL               ; Destination/Origin ID

QUERY_REPLY         DB      0,12,129,149,0,0,1,0,1,0,1,1  ;ddm type

;**********************************************************************
; EHLLAPI variables.
;**********************************************************************

HPCB                DB SIZE HLPCB DUP (NULL)    ; Allocate the EHLLAPI
                                                ; control block.

HDATA_STR           DB MAX_DATA_SIZE DUP(?)     ; EHLLAPI data
                                                ; string.
HEX_TABLE           DB       '0123456789ABCDEF'

CRLF_VAR            DB       CR, LF             ; line FEED VAR

ARR_ELE_START       DW ?                        ; Running start of session
                                                ; element structures.
NUM_SESS            DW ?                        ; Number of sessions

DSEG                ENDS




SSEG                SEGMENT STACK 'STACK'

                    DB       2000 DUP ('S')     ; Allocate the stack.
STACK_BEG           DW       ?                  ; First stack entry.

SSEG                ENDS





;********************************************************************
;*************************** BEGIN CODE *****************************
;********************************************************************

CSEG                SEGMENT 'CODE'
                    ASSUME CS:CSEG, DS:DSEG, SS:SSEG, ES:DSEG



;**************************************************************************
;MAIN - Main code calls routines to do real work.                         *
;                                                                         *
;INPUT                                                                    *
;                                                                         *
;OUTPUT                                                                   *
;                                                                         *
;                                                                         *
;**************************************************************************

MAIN       PROC     FAR
           MOV      AX, SEG SSEG          ; Get stack selector
           MOV      SS, AX                ; Put selector in SS
           MOV      SP, OFFSET STACK_BEG  ; Set SP


           MOV      AX, SEG DSEG          ; Get DATA selector
           MOV      DS, AX                ; Put selector is DS
           MOV      ES, AX                ; Put selector is ES


           ;INIT parmater control block

           MOV      AX, SEG HDATA_STR     ; Set up selector to
           MOV      HPCB.HLPCB_DSEG, AX   ; data string.

           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                          ; Set up offset to
                                          ; data string
           ;DO main program

           LEA      BX, BLANK_SCR         ; Blank the screen.
           CALL     DSP_MSG
           CALL     DISP_EHLLAPI_INFO     ; Call routine to
                                          ; display EHLLAPI info.
           CMP      RC,0                  ;
           JE       NEXT_0                ; If error goto done
           JMP      DONE

NEXT_0:    LEA      BX, PRESS_ENT_MSG     ; Print press enter msg.
           CALL     DSP_MSG
           CALL     FGETCHAR              ; Make user press enter.
           CALL     NEW_LINE
           CALL     DISP_SESSION_INFO     ; Call routine to
                                          ; display Host session info
           CMP      RC,0
           JE       NEXT_1
           JMP      DONE                  ; If error goto done

NEXT_1:
           CMP      DFT_SESS, NULL        ;
           JE       NO_DFT_SESS           ; If no dft session jmp

           LEA      BX, BLANK_SCR         ; Blank the screen.
           CALL     DSP_MSG
           DISPLAY  <'Press ENTER to send string "'>
           LEA      BX, HOST_TEXT         ;
           CALL     DSP_MSG
           DISPLAY  <'" to session short name '>
           LEA      BX, DFT_SESS
           CALL     DSP_MSG
           DISPLAY  <'...'>
           CALL     FGETCHAR              ; Make user press enter.
           CALL     NEW_LINE
           CALL     WRITE_STR_2_HOST      ; Call routine to write
                                          ; string to host session.
           CMP      RC,0                  ;
           JE       NEXT_2                ; If error goto done
           JMP      DONE

NO_DFT_SESS:

           DISPLAY <'NO DFT SESSION SESSION STARTED.'>
           CALL     NEW_LINE
           JMP      DONE                  ; Get out of program.

NEXT_2:

           DISPLAY  <'Press ENTER to search for string "'>
           LEA      BX, HOST_TEXT         ;
           CALL     DSP_MSG
           DISPLAY  <'" on Host Presentation Space...'>
           CALL     FGETCHAR              ; Make user press enter.
           CALL     NEW_LINE
           CALL     SEARCH_STR_ON_HOST    ; Call routine to
                                          ; search for string on
                                          ; host session.
           CMP      RC,0
           JE       NEXT_3                ; If error goto done
           JMP      DONE

NEXT_3:
           DISPLAY <'Press ENTER to display first 1920 '>
           DISPLAY <'bytes of Host presentation space...'>
           CALL     FGETCHAR              ; Make user press enter.
           CALL     NEW_LINE
           CALL     DISP_HOST_SCR         ; Call routine to
                                          ; display Host session
                                          ; screen.
           CMP      RC,0
           JE       NEXT_4                ; If error goto done
           JMP      DONE

NEXT_4:
           DISPLAY <'Press ENTER to change the '>
           DISPLAY <'PM window title'>
           DISPLAY <' for session short name '>
           LEA      BX, DFT_SESS
           CALL     DSP_MSG
           DISPLAY  <'...'>
           CALL     FGETCHAR               ; Make user press enter.
           CALL     NEW_LINE
           CALL     CHANGE_WINDOW_NAME     ; Call routine to
           CMP      RC,0                   ; change window name
           JE       NEXT_5                 ; If error goto done
           JMP      DONE

NEXT_5:
           CALL     NEW_LINE
           DISPLAY  <'Press ENTER to change the '>
           DISPLAY  <'switch list title '>
           DISPLAY  <'for session short name '>
           LEA      BX, DFT_SESS
           CALL     DSP_MSG
           DISPLAY  <'...'>
           CALL     FGETCHAR              ; Make user press enter.
           CALL     NEW_LINE
           CALL     CHANGE_SWITCH_LIST_TITLE  ; Call routine to
                                              ; change switch list title.
           CMP      RC,0
           JE       NEXT_6
           JMP      DONE                  ; If error goto done
NEXT_6:
           CALL     NEW_LINE
           DISPLAY  <'Press ENTER to query the '>
           DISPLAY  <'PM window status'>
           DISPLAY  <' for session short name '>
           LEA      BX, DFT_SESS
           CALL     DSP_MSG
           CALL     FGETCHAR               ; Make user press enter.
           CALL     NEW_LINE
           CALL     QUERY_PM_WINDOW_STATUS ; Call routine to
                                           ; query window status
           CMP      RC,0
           JE       NEXT_7
           JMP      DONE                   ; If error goto done

NEXT_7:
           CALL     NEW_LINE
           CALL     QUERY_PM_WINDOW_COORDS ; Call routine to
                                           ; query coordinates.
           CMP      RC,0
           JE       NEXT_7B
           JMP      DONE                   ; If error goto done
NEXT_7B:
           CALL     NEW_LINE
           DISPLAY  <'Press ENTER to make the '>
           DISPLAY  <'PM window'>
           DISPLAY  <' of session short name '>
           LEA      BX, DFT_SESS
           CALL     DSP_MSG
           DISPLAY  <' invisible...'>
           CALL     FGETCHAR               ; Make user press enter.
           CALL     NEW_LINE
           CALL     MAKE_WINDOW_INVISIBLE  ; Call routine to
                                           ; make window invisible.
           CMP      RC,0
           JE       NEXT_8
           JMP      DONE                   ; If error goto done
NEXT_8:
           MOV      SI, OFFSET INHOST_TEXT ; replace host text
           MOV      DI, OFFSET HOST_TEXT   ; with another string of
           MOV      CX, HOST_TEXT_LEN      ; the same length
REP        MOVS     HOST_TEXT,INHOST_TEXT

           DISPLAY  <'Press ENTER to send string "'>

           LEA      BX, HOST_TEXT         ;
           CALL     DSP_MSG

           DISPLAY  <'" to invisible session short name '>
           LEA      BX, DFT_SESS
           CALL     DSP_MSG

           DISPLAY  <'...'>
           CALL     FGETCHAR              ; Make user press enter.
           CALL     NEW_LINE
           CALL     WRITE_STR_2_HOST      ; Call routine to
                                          ; write string to host
                                          ; session.
           CMP      RC,0                  ;
           JE       NEXT_9                ; If error goto done
           JMP      DONE
NEXT_9:
           DISPLAY  <'Press ENTER to search for string "'>
           LEA      BX, HOST_TEXT         ;
           CALL     DSP_MSG
           DISPLAY  <'" on Host Presentation Space...'>
           CALL     FGETCHAR              ; Make user press enter.
           CALL     NEW_LINE

           CALL     SEARCH_STR_ON_HOST    ; Call routine to
                                          ; search for string on
                                          ; host session.
           CMP      RC,0
           JE       NEXT_10               ; If error goto done
           JMP      DONE

NEXT_10:   DISPLAY  <'Press ENTER to display first 1920 '>
           DISPLAY  <'bytes of the invisible presentation space...'>
           CALL     FGETCHAR               ; Make user press enter.
           CALL     NEW_LINE
           CALL     DISP_HOST_SCR          ; Call routine to
                                           ; display Host session
                                           ; screen.
           CMP      RC,0
           JE       NEXT_11                ; If error goto done
           JMP      DONE

NEXT_11:
           DISPLAY  <'Press ENTER to maximize and '>
           DISPLAY  <'make visible the PM window'>
           DISPLAY  <' for session '>
           LEA      BX, DFT_SESS
           CALL     DSP_MSG
           DISPLAY  <'...'>
           CALL     FGETCHAR                ; Make user press enter.
           CALL     NEW_LINE
           CALL     MAKE_WINDOW_VISIBLE     ; Call routine to
                                            ; make PM window visible                              CMP      RC,0                      ;and maximized.
           JE       NEXT_12
           JMP      DONE                    ; If error goto done

NEXT_12:
           CALL     NEW_LINE
           DISPLAY  <'Press ENTER to disconnect '>
           DISPLAY  <'from the PM window'>
           DISPLAY  <' of session short name '>
           LEA      BX, DFT_SESS
           CALL     DSP_MSG
           DISPLAY  <'...'>
           CALL     FGETCHAR                ; Make user press enter.
           CALL     NEW_LINE
           CALL     DISCONNECT_PM_WINDOW_SERVICE ; Call routine to
                                                 ; disconnect PM window
           CMP      RC,0
           JE       NEXT_13
           JMP      DONE                    ; If error goto done

NEXT_13:
           CALL     NEW_LINE
           DISPLAY  <'Press ENTER to restore the '>
           DISPLAY  <'window size and titles '>
           DISPLAY  <'of session short name '>
           LEA      BX, DFT_SESS
           CALL     DSP_MSG
           DISPLAY  <'...'>
           CALL     FGETCHAR                ; Make user press enter.
           CALL     NEW_LINE
           CALL     RESET_SWITCH_AND_WINDOW_NAME ; Call routine to
                                                 ; restore PM settings.
           CMP      RC,0
           JE       NEXT_SF
           JMP      DONE                    ; If error goto done

NEXT_SF:   DISPLAY  <'The Sample Program Continues With Structured Field Calls.'>
           CALL     NEW_LINE
           DISPLAY  <'The host session must be active and have access to '>
           CALL     NEW_LINE
           DISPLAY  <'the IND$FILE file transfer application.'>
           CALL     NEW_LINE
           DISPLAY  <'Press "y" To Continue, Any Other Key To Exit.'>
           CALL     NEW_LINE
           CALL     FGETCHAR
           MOV      AL,DUMMY_BUFF
           CMP      AL,89                       ; 'Y'
           JE       NEXT_14A
           CMP      AL,121                      ; 'y'
           JE       NEXT_14A
           JMP      DONE

NEXT_14A:  MOV      AX,0

           CALL     RESET_SYSTEM
           CMP      RC,0
           JE       NEXT_14B
           JMP      DONE
NEXT_14B:
           CALL     QUERY_COM_BUFFER_SIZE
           CMP      RC,0
           JE       NEXT_15
           JMP      DONE                    ; If error goto done

NEXT_15:
           CALL     ALLOCATE_READ_COM_BUFFER
           CMP      RC,0
           JE       NEXT_16
           JMP      DONE                    ; If error goto done

NEXT_16:
           CALL     ALLOCATE_WRITE_COM_BUFFER
           CMP      RC,0
           JE       NEXT_17
           JMP      DONE                    ; If error goto done
NEXT_17:
           CALL     CONNECT_TO_SF
           CMP      RC,0
           JE       NEXT_18
           JMP      DONE                    ; If error goto done

NEXT_18:   CALL     NEW_LINE
           CALL     NEW_LINE
           DISPLAY  <'Press Enter To Continue'>
           CALL     FGETCHAR
           LEA      BX, BLANK_SCR           ; Blank the screen.
           CALL     DSP_MSG
           CALL     READ_SF_ASYNC
           CMP      RC,0
           JE       NEXT_19
           JMP      DONE                    ; If error goto done
NEXT_19:
           CALL     CREATE_SF
           CMP      RC,0
           JE       NEXT_20
           JMP      DONE                    ; If error goto done
NEXT_20:
           CALL     GET_ASYNC_COMPLETE
           CMP      RC,0
           JE       NEXT_21
           JMP      DONE                    ; If error goto done
NEXT_21:
           CALL     WRITE_SF_SYNC
           CMP      RC,0
           JE       NEXT_22
           JMP      DONE                    ; If error goto done
NEXT_22:
           CALL     FREE_READ_COM_BUFFER
           CMP      RC,0
           JE       NEXT_23
           JMP      DONE                    ; If error goto done

NEXT_23:
           CALL     FREE_WRITE_COM_BUFFER
           CMP      RC,0
           JE       NEXT_24
           JMP      DONE                    ; If error goto done

NEXT_24:
           CALL     DISCONNECT_SF
           CMP      RC,0
           JE       NEXT_25
           JMP      DONE                    ; If error goto done

NEXT_25:
           CALL     NEW_LINE
           DISPLAY  <'SAMPLE PROGRAM DONE.  To Exit Program '>
           DISPLAY  <'Press ENTER...'>
           CALL     FGETCHAR                ; Make user press enter.
           CALL     NEW_LINE
DONE:
           PUSH     1
           PUSH     0
           CALL     DOSEXIT                 ; Exit to program.

MAIN       ENDP






;**************************************************************************
;DISP_EHLLAPI_INFO - CALLs EHLLAPI QUERY_SYSTEM and then displays the     *
;                    requested info.                                      *
;                                                                         *
;INPUT                                                                    *
;                                                                         *
;OUTPUT                                                                   *
;                                                                         *
;**************************************************************************

DISP_EHLLAPI_INFO     PROC     NEAR                  ; Routine to display
                                                     ;EHLLAPI info.

              PUSH     AX
              PUSH     BX
              PUSH     CX
              PUSH     DX
              PUSH     SI
              PUSH     DI
              PUSH     BP
              PUSH     ES

              MOV      HPCB.HLPCB_FUNC, HA_QUERY_SYSTEM
                                                ; Issue query system

              MOV      SI, OFFSET HPCB          ; DS:SI-> PCB
                                                ; DS already set up.

              CALL     HLLMASM                  ; Call EHLLAPI.

              CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
              JE       OK_DEI                   ; If GOOD rc, jmp.
              JMP      ERROR_DEI                ; goto error routine.
OK_DEI:
              DISPLAY  <'                     EHLLAPI INFORMATION'>
              CALL     NEW_LINE
              CALL     NEW_LINE

              DISPLAY  <'  EHLLAPI version              : '>
              MOV      BX, OFFSET HDATA_STR.QSYS_HLLAPI_VER
              MOV      AX,1
              CALL     DSP_MSG_LEN
              CALL     NEW_LINE

              DISPLAY  <'  EHLLAPI level                : '>
              MOV      BX, OFFSET HDATA_STR.QSYS_HLLAPI_LVL
              MOV      AX, 2
              CALL     DSP_MSG_LEN
              CALL     NEW_LINE

              DISPLAY  <'  EHLLAPI release date         : '>
              MOV      BX, OFFSET HDATA_STR.QSYS_HLLAPI_DATE
              MOV      AX, 6
              CALL     DSP_MSG_LEN
              CALL     NEW_LINE

              DISPLAY  <'  EHLLAPI LIM version          : '>
              MOV      BX, OFFSET HDATA_STR.QSYS_LIM_VER
              MOV      AX, 1
              CALL     DSP_MSG_LEN
              CALL     NEW_LINE

              DISPLAY  <'  EHLLAPI LIM level            : '>
              MOV      BX, OFFSET HDATA_STR.QSYS_LIM_LVL
              MOV      AX, 2
              CALL     DSP_MSG_LEN
              CALL     NEW_LINE

              DISPLAY  <'  EHLLAPI hardware base        : '>
              MOV      BX, OFFSET HDATA_STR.QSYS_HARDWARE_BASE
              MOV      AX, 1
              CALL     DSP_MSG_LEN
              DISPLAY  <' = '>
              CMP      HDATA_STR.QSYS_HARDWARE_BASE, 'Z'
              JNE      NEXT_DEI3
              DISPLAY  <'(See System model/submodel below)'>
NEXT_DEI3:
              CALL     NEW_LINE
              DISPLAY  <'  EHLLAPI CTRL program type    : '>
              MOV      BX, OFFSET HDATA_STR.QSYS_CTRL_PROG_TYPE
              MOV      AX, 1
              CALL     DSP_MSG_LEN
              DISPLAY  <' = '>
              CMP      HDATA_STR.QSYS_CTRL_PROG_TYPE, 'X'
              JNE      NEXT_DEI6
              DISPLAY  <'OS/2'>
NEXT_DEI6:
              CALL     NEW_LINE
              DISPLAY  <'  EHLLAPI sequence number      : '>
              MOV      BX, OFFSET HDATA_STR.QSYS_SEQ_NUM
              MOV      AX, 2
              CALL     DSP_MSG_LEN
              CALL     NEW_LINE

              DISPLAY  <'  EHLLAPI CTRL program version : '>
              MOV      BX, OFFSET HDATA_STR.QSYS_CTRL_PROG_VER
              MOV      AX, 2
              CALL     DSP_MSG_LEN
              CALL     NEW_LINE

              DISPLAY  <'  EHLLAPI PC session name      : '>
              MOV      BX, OFFSET HDATA_STR.QSYS_PC_SNAME
              MOV      AX, 1
              CALL     DSP_MSG_LEN
              CALL     NEW_LINE

              DISPLAY  <'  EHLLAPI extended error 1     : '>
              MOV      BX, OFFSET HDATA_STR.QSYS_ERR1
              MOV      AX, 4
              CALL     DSP_MSG_LEN
              CALL     NEW_LINE

              DISPLAY  <'  EHLLAPI extended error 2     : '>
              MOV      BX, OFFSET HDATA_STR.QSYS_ERR2
              MOV      AX, 4
              CALL     DSP_MSG_LEN
              CALL     NEW_LINE

              DISPLAY  <'  EHLLAPI system model/submodel: '>
              MOV      AH,HDATA_STR.QSYS_SYS_MODEL
              MOV      BX, OFFSET DIGIT_BUFF
              CALL     ITOH                  ;Integer to ASCII hex.
              MOV      AX, 2
              CALL     DSP_MSG_LEN
              MOV      AH,HDATA_STR.QSYS_SYS_SUBMODEL
              MOV      BX, OFFSET DIGIT_BUFF
              CALL     ITOH                  ;Integer to ASCII HEX.
              MOV      BX, OFFSET DIGIT_BUFF
              MOV      AX, 2
              CALL     DSP_MSG_LEN
              DISPLAY  <' HEX  '>
              CMP      WORD PTR HDATA_STR.QSYS_SYS_MODEL, 00FCH
              JNE      NEXT_DEI7
              DISPLAY <'= Model PC AT'>
NEXT_DEI7:    CMP      WORD PTR HDATA_STR.QSYS_SYS_MODEL, 01FCH
              JNE      NEXT_DEI8
              DISPLAY <'= Model PC AT ENHANCED'>
NEXT_DEI8:    CMP      WORD PTR HDATA_STR.QSYS_SYS_MODEL, 02FCH
              JNE      NEXT_DEI11
              DISPLAY <'= Model PC XT Model 286'>
NEXT_DEI11:   CMP      WORD PTR HDATA_STR.QSYS_SYS_MODEL, 04FCH
              JNE      NEXT_DEI12
              DISPLAY <'= Model 50'>
NEXT_DEI12:   CMP      WORD PTR HDATA_STR.QSYS_SYS_MODEL, 05FCH
              JNE      NEXT_DEI13
              DISPLAY <'= Model 60'>
NEXT_DEI13:   CMP      WORD PTR HDATA_STR.QSYS_SYS_MODEL, 00F8H
              JNE      NEXT_DEI14
              DISPLAY <'= Model 80'>
NEXT_DEI14:   CMP      WORD PTR HDATA_STR.QSYS_SYS_MODEL, 09F8H
              JNE      NEXT_DEI141
              DISPLAY <'= Model 70'>
NEXT_DEI141:  CALL     NEW_LINE

              DISPLAY <'  EHLLAPI National Language    : '>
              MOV      AX,HDATA_STR.QSYS_PC_NLS
              MOV      BX, OFFSET DIGIT_BUFF
              CALL     ITOA                  ;Integer to ASCII routine.
              CALL     DSP_MSG
              CALL     NEW_LINE

              DISPLAY  <'  EHLLAPI monitor type         : '>
              MOV      BX, OFFSET HDATA_STR.QSYS_MONITOR_TYPE
              MOV      AX, 1
              CALL     DSP_MSG_LEN
              DISPLAY  <' = '>
              CMP      HDATA_STR.QSYS_MONITOR_TYPE, 'M'
              JNE      NEXT_DEI15
              DISPLAY  <'PC MONOCHROME'>
NEXT_DEI15:   CMP      HDATA_STR.QSYS_MONITOR_TYPE, 'C'
              JNE      NEXT_DEI16
              DISPLAY  <'PC CGA'>
NEXT_DEI16:   CMP      HDATA_STR.QSYS_MONITOR_TYPE, 'E'
              JNE      NEXT_DEI17
              DISPLAY  <'PC EGA'>
NEXT_DEI17:   CMP      HDATA_STR.QSYS_MONITOR_TYPE, 'A'
              JNE      NEXT_DEI18
              DISPLAY  <'PS MONOCHROME'>
NEXT_DEI18:   CMP      HDATA_STR.QSYS_MONITOR_TYPE, 'V'
              JNE      NEXT_DEI19
              DISPLAY  <'PS 8512'>
NEXT_DEI19:   CMP      HDATA_STR.QSYS_MONITOR_TYPE, 'H'
              JNE      NEXT_DEI20
              DISPLAY  <'PS 8514'>
NEXT_DEI20:   CMP      HDATA_STR.QSYS_MONITOR_TYPE, 'U'
              JNE      NEXT_DEI21
              DISPLAY  <'UNKNOWN monitor type'>
NEXT_DEI21:   CALL     NEW_LINE
              JMP      RET_DEI


ERROR_DEI:
              MOV      AX,HPCB.HLPCB_RETCODE ;Put bad rc in RC.
              MOV      RC, AX
              XOR      BX,BX                 ;Set function number.
              MOV      BL,HPCB.HLPCB_FUNC
              CALL     ERROR_HAND

RET_DEI:

           POP      ES
           POP      BP
           POP      DI
           POP      SI
           POP      DX
           POP      CX
           POP      BX
           POP      AX
           RET

DISP_EHLLAPI_INFO     ENDP




;**************************************************************************
;DISP_SESSION_INFO - CALLs EHLLAPI QUERY funtions and then displays the   *
;                    requested session info.                              *
;                                                                         *
;INPUT                                                                    *
;                                                                         *
;OUTPUT                                                                   *
;                                                                         *
;**************************************************************************

DISP_SESSION_INFO     PROC     NEAR                  ; Routine to display
                                                     ; Host session info.

            PUSH     AX
            PUSH     BX
            PUSH     CX
            PUSH     DX
            PUSH     SI
            PUSH     DI
            PUSH     BP
            PUSH     ES


            CALL     NEW_LINE
            CALL     NEW_LINE
            CALL     NEW_LINE
            CALL     NEW_LINE
            CALL     NEW_LINE
            CALL     NEW_LINE
            CALL     NEW_LINE
            CALL     NEW_LINE
            CALL     NEW_LINE
            CALL     NEW_LINE
            CALL     NEW_LINE
            CALL     NEW_LINE

            DISPLAY  <'                           SESSION INFO'>
            CALL     NEW_LINE
            CALL     NEW_LINE

            MOV      HPCB.HLPCB_FUNC, HA_QUERY_SESSIONS
                                             ; Issue query
                                             ; sessions.

            MOV      HPCB.HLPCB_LENGTH, MAX_DATA_SIZE / 12 * 12
                                          ; Make sure len is
                                          ; multiple of 12.

            MOV      SI, OFFSET HPCB      ; DS:SI-> PCB
                                          ; DS already set up.

            CALL     HLLMASM     ; Call EHLLAPI.

            CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
            JE       OK_DSI                ; If GOOD rc, jmp.
            JMP      ERROR_DSI             ; goto error routine.

OK_DSI:
            CALL     NEW_LINE
            CALL     NEW_LINE


            MOV      AX, HPCB.HLPCB_LENGTH ; Set Number of sessions
            MOV      NUM_SESS,AX           ; started.

            DISPLAY  <'Number of started sessions = '>
            MOV      BX, OFFSET DIGIT_BUFF
            CALL     ITOA                  ; Integer to ASCII routine.
            CALL     DSP_MSG
            CALL     NEW_LINE
            CALL     NEW_LINE
            CALL     NEW_LINE

            MOV      AX, OFFSET HDATA_STR  ; Initialize the start
            MOV      ARR_ELE_START, AX     ; of the array element.

            MOV      DI,0                  ; LOOP thru queried
                                           ; sessions. DI will be
                                           ; index into sessions
TOP_DSI:
            CMP      DI,NUM_SESS           ; Reached max session?
            JB       NEXT_DSI8
            JMP      DONE_DSI
NEXT_DSI8:
            CALL     NEW_LINE
            DISPLAY  <'Session number     : '>
            MOV      AX, DI
            INC      AX
            MOV      BX, OFFSET DIGIT_BUFF
            CALL     ITOA                  ; Integer to ASCII routine.
            CALL     DSP_MSG
            CALL     NEW_LINE

            MOV      SI, ARR_ELE_START

            DISPLAY  <'Session Long name  : '>
            LEA      BX, [SI].QSES_LONGNAME
            MOV      AX, 8
            CALL     DSP_MSG_LEN
            CALL     NEW_LINE

            DISPLAY  <'Session Short name : '>
            LEA      BX, [SI].QSES_SHORTNAME
            MOV      AX, 1
            CALL     DSP_MSG_LEN
            CALL     NEW_LINE

            DISPLAY <'Session Type       : '>
            LEA      BX, [SI].QSES_SESTYPE
            MOV      AX, 1
            CALL     DSP_MSG_LEN
            DISPLAY <' = '>
            CMP      [SI].QSES_SESTYPE, 'H'
            JNE      NEXT_DSI
            DISPLAY <'Host'>
            CMP      DFT_SESS, NULL        ; If first HOST not
                                           ; set already
            JNE      NEXT_DSI
            MOV      AL, [SI].QSES_SHORTNAME
            MOV      DFT_SESS, AL          ; Set the session to
                                           ; write string to.
NEXT_DSI:   CMP      [SI].QSES_SESTYPE, 'P'
            JNE      NEXT_DSI1
            DISPLAY <'PC'>
NEXT_DSI1:
            CALL     NEW_LINE

            DISPLAY <'Session PS size    : '>
            MOV      AX, [SI].QSES_PSSIZE
            MOV      BX, OFFSET DIGIT_BUFF
            CALL     ITOA                   ; Integer to ASCII routine.
            CALL     DSP_MSG
            CALL     NEW_LINE

            MOV      HPCB.HLPCB_FUNC, HA_QUERY_SESSION_STATUS
                                            ; Issue query
                                            ; sessions.
            MOV      HPCB.HLPCB_LENGTH, 18 ;Set length
            MOV      AL, [SI].QSES_SHORTNAME
            MOV      SESS_STUC.QSST_SHORTNAME, AL
                                            ; Set the short
                                            ; name.
            MOV      HPCB.HLPCB_DADDR, OFFSET SESS_STUC
                                            ; Set new EHLLAPI data
                                            ; area.


            MOV      SI, OFFSET HPCB        ; DS:SI-> PCB
                                            ; DS already set up.
            PUSH     DI
            CALL     HLLMASM                ; Call EHLLAPI.
            POP      DI

            CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
            JE       OK_DSI1                ; If GOOD rc, jmp.
            JMP      ERROR_DSI              ; goto error routine.

OK_DSI1:
            DISPLAY  <'Session PS rows    : '>
            MOV      AX,SESS_STUC.QSST_PS_ROWS
            MOV      BX, OFFSET DIGIT_BUFF
            CALL     ITOA                   ; Integer to ASCII routine.
            CALL     DSP_MSG
            CALL     NEW_LINE

            DISPLAY  <'Session PS columns : '>
            MOV      AX,SESS_STUC.QSST_PS_COLS
            MOV      BX, OFFSET DIGIT_BUFF
            CALL     ITOA                   ; Integer to ASCII routine.
            CALL     DSP_MSG
            CALL     NEW_LINE

            DISPLAY <'Session type 2     : '>
            MOV      BX, OFFSET SESS_STUC.QSST_SESTYPE
            MOV      AX, 1
            CALL     DSP_MSG_LEN

            DISPLAY <' = '>
            CMP      SESS_STUC.QSST_SESTYPE, 'F'
            JNE      NEXT_DSI11
            DISPLAY <'5250'>

NEXT_DSI11: CMP      SESS_STUC.QSST_SESTYPE, 'G'
            JNE      NEXT_DSI12
            DISPLAY <'5250 Printer Session'>

NEXT_DSI12: CMP      SESS_STUC.QSST_SESTYPE, 'D'
            JNE      NEXT_DSI2
            DISPLAY <'DFT Host'>

NEXT_DSI2:  CMP      SESS_STUC.QSST_SESTYPE, 'P'
            JNE      NEXT_DSI3
            DISPLAY <'PC'>
NEXT_DSI3:
            CALL     NEW_LINE
            DISPLAY <'Session supports Extended attributes (EABs)? : '>
            TEST     SESS_STUC.QSST_CHAR, EABS
            JZ       NEXT_DSI4
            DISPLAY <'YES'>
            JMP      NEXT_DSI5
NEXT_DSI4:
            DISPLAY <'NO'>
NEXT_DSI5:
            CALL     NEW_LINE
            DISPLAY <'Session supports Program Symbols (PSS)?      : '>
            TEST     SESS_STUC.QSST_CHAR, PSS
            JZ       NEXT_DSI6
            DISPLAY <'YES'>
            JMP      NEXT_DSI7
NEXT_DSI6:
            DISPLAY <'NO'>
NEXT_DSI7:
            CALL     NEW_LINE
            LEA      BX, PRESS_ENT_MSG     ; Print press enter msg.
            CALL     DSP_MSG
            CALL     FGETCHAR              ; Make user press enter.
            CALL     NEW_LINE
            MOV      AX, SIZE QSES_STRUCT  ; Get next start pos.
            ADD      ARR_ELE_START, AX
            INC      DI                    ; GET next session.
            JMP      TOP_DSI

ERROR_DSI:
            MOV      AX,HPCB.HLPCB_RETCODE ; Put bad rc in RC.
            MOV      RC, AX
            XOR      BX,BX                 ; Set function number.
            MOV      BL,HPCB.HLPCB_FUNC
            CALL     ERROR_HAND
DONE_DSI:

            MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                           ; Reset the orig EHLLAPI
                                           ; data_str.

            POP      ES
            POP      BP
            POP      DI
            POP      SI
            POP      DX
            POP      CX
            POP      BX
            POP      AX

            RET

DISP_SESSION_INFO     ENDP




;**************************************************************************
;WRITE_STR_2_HOST  - Connects to first session and writes home_key and    *
;                    string to host.                                      *
;                                                                         *
;INPUT                                                                    *
;                                                                         *
;OUTPUT                                                                   *
;                                                                         *
;**************************************************************************

WRITE_STR_2_HOST      PROC     NEAR                  ; Call routine to
                                                     ; write string to host.

            PUSH     AX
            PUSH     BX
            PUSH     CX
            PUSH     DX
            PUSH     SI
            PUSH     DI
            PUSH     BP
            PUSH     ES

            MOV      AL, DFT_SESS          ; Set session id to
            MOV      HDATA_STR, AL         ; connect.

            MOV      HPCB.HLPCB_FUNC, HA_CONNECT_PS
                                           ; Issue Connect
                                           ; Presentation Space.

            MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                           ; DS already set up.

            CALL     HLLMASM               ; Call EHLLAPI.

            CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
            JE       OK_WSH                ; If GOOD rc, jmp.
            JMP      ERROR_WSH             ; goto error routine.

OK_WSH:
            MOV      HPCB.HLPCB_FUNC, HA_SENDKEY
                                           ; Issue sendkey
            MOV      HPCB.HLPCB_LENGTH, HOME_KEY_LEN
                                           ; Set length
            MOV      HPCB.HLPCB_DADDR, OFFSET HOME_KEY
                                           ; Set new data_str.
            MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                           ; DS already set up.

            CALL     HLLMASM               ;Call EHLLAPI.

            CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
            JE       OK_WSH1               ; If GOOD rc, jmp.
            JMP      ERROR_WSH             ; goto error routine.

OK_WSH1:
            MOV      HPCB.HLPCB_LENGTH, HOST_TEXT_LEN
                                           ; Set length
            MOV      HPCB.HLPCB_DADDR, OFFSET HOST_TEXT
                                           ; Set new data_str.
            MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                           ; DS already set up.
            CALL     HLLMASM               ; Call EHLLAPI.

            CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
            JE       OK_WSH2               ; If GOOD rc, jmp.
            JMP      ERROR_WSH             ; goto error routine.

OK_WSH2:
            DISPLAY  <'Sent String to Host.'>
            CALL     NEW_LINE
            CALL     NEW_LINE
            CALL     NEW_LINE
            JMP      DONE_WSH
ERROR_WSH:
            MOV      AX,HPCB.HLPCB_RETCODE ; Put bad rc in RC.
            MOV      RC, AX
            XOR      BX,BX                 ; Set function number.
            MOV      BL,HPCB.HLPCB_FUNC
            CALL     ERROR_HAND
DONE_WSH:
            MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                           ; Reset the orig EHLLAPI
                                           ; data_str.
            POP      ES
            POP      BP
            POP      DI
            POP      SI
            POP      DX
            POP      CX
            POP      BX
            POP      AX

            RET

WRITE_STR_2_HOST      ENDP


;**************************************************************************
;SEARCH_STR_ON_HOST- Searches for string on host.                         *
;                                                                         *
;INPUT                                                                    *
;                                                                         *
;OUTPUT                                                                   *
;                                                                         *
;**************************************************************************

SEARCH_STR_ON_HOST    PROC     NEAR                  ; Routine to search
                                                     ; for string on host
                                                     ; session.
           PUSH     AX
           PUSH     BX
           PUSH     CX
           PUSH     DX
           PUSH     SI
           PUSH     DI
           PUSH     BP
           PUSH     ES

           MOV      HPCB.HLPCB_FUNC, HA_SEARCH_PS
                                          ; Issue SEARCH PS
           MOV      HPCB.HLPCB_LENGTH, HOST_TEXT_LEN
                                          ; Set length

           MOV      HPCB.HLPCB_DADDR, OFFSET HOST_TEXT
                                          ;Set new data_str.

           MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.

           CALL     HLLMASM               ; Call EHLLAPI.

           CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
           JE       OK_SSOH               ; If GOOD rc, jmp.
           JMP      ERROR_SSOH            ; goto error routine.

OK_SSOH:
           DISPLAY  <'Found string "'>
           MOV      BX, OFFSET HOST_TEXT
           CALL     DSP_MSG
           DISPLAY  <'" at PS position '>
           LEA      BX, DIGIT_BUFF
           MOV      AX, HPCB.HLPCB_LENGTH ;
           CALL     ITOA
           CALL     DSP_MSG
           DISPLAY  <'.'>
           CALL     NEW_LINE
           CALL     NEW_LINE
           CALL     NEW_LINE
           JMP      DONE_SSOH
ERROR_SSOH:
           MOV      AX,HPCB.HLPCB_RETCODE ; Put bad rc in RC.
           MOV      RC, AX
           XOR      BX,BX                 ; Set function number.
           MOV      BL,HPCB.HLPCB_FUNC
           CALL     ERROR_HAND
DONE_SSOH:
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                          ; Reset the orig EHLLAPI
                                          ; data_str.
           POP      ES
           POP      BP
           POP      DI
           POP      SI
           POP      DX
           POP      CX
           POP      BX
           POP      AX
           RET

SEARCH_STR_ON_HOST    ENDP


;*************************************************************************
;DISP_HOST_SCR - Displays first 1920 bytes of host screen.               *
;                                                                        *
;                                                                        *
;*************************************************************************

DISP_HOST_SCR         PROC     NEAR                  ; Routine to
                                                     ; display host screen.




           PUSH     AX
           PUSH     BX
           PUSH     CX
           PUSH     DX
           PUSH     SI
           PUSH     DI
           PUSH     BP
           PUSH     ES

           MOV      HPCB.HLPCB_FUNC, HA_SET_SESSION_PARMS
                                          ; Issue Set session Parms
           MOV      HPCB.HLPCB_LENGTH, SETPARM_TEXT_LEN
                                          ; Set length
           MOV      HPCB.HLPCB_DADDR, OFFSET SETPARM_TEXT
                                          ; Set new data_str.
           MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.
           CALL     HLLMASM               ; Call EHLLAPI.

           CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
           JE       OK_DHS                ; If GOOD rc, jmp.
           JMP      ERROR_DHS             ; goto error routine.

OK_DHS:
           MOV      HPCB.HLPCB_FUNC, HA_COPY_PS_TO_STR
                                          ; Issue Copy PS
           MOV      HPCB.HLPCB_LENGTH, MAX_DATA_SIZE
                                          ; Set length
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                          ; Reset the orig EHLLAPI
                                          ; data_str.
           MOV      HPCB.HLPCB_RETCODE, 1 ; Set PS position to
                                          ; top,left corner.
           MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.
           CALL     HLLMASM               ; Call EHLLAPI.

           CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
           JE       OK_DHS1              ; If GOOD rc, jmp.
           JMP      ERROR_DHS            ; goto error routine.
OK_DHS1:
           PUSH     DS
           PUSH     OFFSET HDATA_STR
           PUSH     MAX_DATA_SIZE
           PUSH     0
           PUSH     0
           PUSH     0
           CALL     VIOWRTCELLSTR
           JMP      DONE_DHS

ERROR_DHS:
           MOV      AX,HPCB.HLPCB_RETCODE ; Put bad rc in RC.
           MOV      RC, AX
           XOR      BX,BX                 ; Set function number.
           MOV      BL,HPCB.HLPCB_FUNC
           CALL     ERROR_HAND
DONE_DHS:
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                          ; Reset the orig EHLLAPI
                                          ; data_str.
           POP      ES
           POP      BP
           POP      DI
           POP      SI
           POP      DX
           POP      CX
           POP      BX
           POP      AX
           RET

DISP_HOST_SCR       ENDP




;**************************************************************************
;CHANGE WINDOW NAME - Change window title on PM host session              *
;                                                                         *
;                                                                         *
;                                                                         *
;**************************************************************************

CHANGE_WINDOW_NAME      PROC     NEAR


             PUSH     AX
             PUSH     BX
             PUSH     CX
             PUSH     DX
             PUSH     SI
             PUSH     DI
             PUSH     BP
             PUSH     ES

             MOV      AL, DFT_SESS          ; Set session id to
             MOV      HDATA_STR, AL         ; connect.

             MOV      HPCB.HLPCB_FUNC, HA_CONNECT_PM_SRVCS
                                            ; Issue Connect
                                            ; Presentation Space.
             MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                            ; DS already set up.
             CALL     HLLMASM               ; Call EHLLAPI.
             CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
             JE       OK_WCH1               ; If GOOD rc, jmp.
             JMP      ERROR_WCH             ; goto error routine.
OK_WCH1:
             MOV      HPCB.HLPCB_FUNC, HA_CHANGE_WINDOW_NAME
                                            ; Issue change window name
             MOV      HPCB.HLPCB_LENGTH, (TEST_NAME_LEN + 3)
                                            ; Set length
             MOV      AL, DFT_SESS          ; Set session id
             MOV      HDATA_STR.CHLT_SHORTNAME, AL
             MOV      HDATA_STR.CHLT_OPTION,1
             MOV      SI, OFFSET TEST_NAME
             MOV      DI, OFFSET HDATA_STR.CHLT_LTNAME
             MOV      CX, TEST_NAME_LEN
REP          MOVS     HDATA_STR.CHLT_LTNAME, TEST_NAME
             MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
             MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                            ; DS already set
             CALL     HLLMASM               ; Call EHLLAPI.
             CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
             JE       OK_WCH2               ; If GOOD rc, jmp.
             JMP      ERROR_WCH             ; goto error routine.
OK_WCH2:
             DISPLAY  <'Window Title Changed.'>
             CALL     NEW_LINE
             DISPLAY  <'(press CTRL-ESC to verify)'>
             CALL     NEW_LINE
             CALL     NEW_LINE
             CALL     NEW_LINE
             JMP      DONE_WCH
ERROR_WCH:
             MOV      AX,HPCB.HLPCB_RETCODE ; Put bad rc in RC.
             MOV      RC, AX
             XOR      BX,BX                 ; Set function number.
             MOV      BL,HPCB.HLPCB_FUNC
             CALL     ERROR_HAND
DONE_WCH:
             MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                            ; Reset the orig EHLLAPI
                                            ; data_str.
             POP      ES
             POP      BP
             POP      DI
             POP      SI
             POP      DX
             POP      CX
             POP      BX
             POP      AX

             RET

CHANGE_WINDOW_NAME    ENDP


;**************************************************************************
;CHANGE SWITCH LIST TITLE - Change logical terminal name on the           *
;                           switch list                                   *
;                                                                         *
;                                                                         *
;**************************************************************************

CHANGE_SWITCH_LIST_TITLE      PROC     NEAR


           PUSH     AX
           PUSH     BX
           PUSH     CX
           PUSH     DX
           PUSH     SI
           PUSH     DI
           PUSH     BP
           PUSH     ES

           MOV      AL, DFT_SESS          ; Set session id to
           MOV      HDATA_STR, AL         ; connect.

           MOV      HPCB.HLPCB_FUNC, HA_CONNECT_PM_SRVCS
                                          ; Issue Connect
                                          ; Presentation Space.

           MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.

           CALL     HLLMASM               ; Call EHLLAPI.

           CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
           JE       OK_WSL1               ; If GOOD rc, jmp.
           JMP      ERROR_WSL             ; goto error routine.
OK_WSL1:
           MOV      HPCB.HLPCB_FUNC, HA_CHANGE_SWITCH_NAME
                                          ; Issue change window name
           MOV      HPCB.HLPCB_LENGTH, (TEST_NAME_LEN + 3)
                                          ; Set length
           MOV      AL, DFT_SESS          ; Set session id
           MOV      HDATA_STR.CHSW_SHORTNAME, AL
           MOV      HDATA_STR.CHSW_OPTION,1
           MOV      SI, OFFSET TEST_NAME
           MOV      DI, OFFSET HDATA_STR.CHSW_SWNAME
           MOV      CX, TEST_NAME_LEN
REP        MOVS     HDATA_STR.CHSW_SWNAME, TEST_NAME

           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
           MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.
           CALL     HLLMASM               ; Call EHLLAPI.

           CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
           JE       OK_WSL2               ; If GOOD rc, jmp.
           JMP      ERROR_WSL             ; goto error routine.
OK_WSL2:
           DISPLAY  <'Switch List Title Changed.'>
           CALL     NEW_LINE
           DISPLAY  <'(press CTRL-ESC to verify)'>
           CALL     NEW_LINE
           CALL     NEW_LINE
           CALL     NEW_LINE
           JMP      DONE_WSL
ERROR_WSL:
           MOV      AX,HPCB.HLPCB_RETCODE ; Put bad rc in RC.
           MOV      RC, AX
           XOR      BX,BX                 ; Set function number.
           MOV      BL,HPCB.HLPCB_FUNC
           CALL     ERROR_HAND
DONE_WSL:
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                          ; Reset the orig EHLLAPI
                                          ; data_str.
           POP      ES
           POP      BP
           POP      DI
           POP      SI
           POP      DX
           POP      CX
           POP      BX
           POP      AX

           RET

CHANGE_SWITCH_LIST_TITLE    ENDP




;**************************************************************************
;QUERY PM WINDOW STATUS - Query PM window status and report               *
;                         back the results.                               *
;                                                                         *
;                                                                         *
;**************************************************************************

QUERY_PM_WINDOW_STATUS      PROC     NEAR

          PUSH     AX
          PUSH     BX
          PUSH     CX
          PUSH     DX
          PUSH     SI
          PUSH     DI
          PUSH     BP
          PUSH     ES

          MOV      AL, DFT_SESS          ; Set session id to
          MOV      HDATA_STR, AL         ; connect.
          MOV      HPCB.HLPCB_FUNC, HA_CONNECT_PM_SRVCS
                                         ; Issue Connect
                                         ; Presentation Space.
          MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                         ; DS already set up.
          CALL     HLLMASM               ; Call EHLLAPI.
          CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
          JE       OK_WQPM1              ; If GOOD rc, jmp.
          JMP      ERROR_WQPM            ; goto error routine.
OK_WQPM1:
          MOV      HPCB.HLPCB_FUNC, HA_PM_WINDOW_STATUS
          MOV      AL, DFT_SESS          ; Set session id
          MOV      HDATA_STR.CWIN_SHORTNAME, AL
          MOV      HDATA_STR.CWIN_OPTION,2
          MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                         ; DS already set up.
          CALL     HLLMASM               ; Call EHLLAPI.
          CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
          JE       OK_WQPM2              ; If GOOD rc, jmp.
          JMP      ERROR_WQPM            ; goto error routine.
OK_WQPM2:
          CALL     NEW_LINE
          CALL     NEW_LINE
          DISPLAY  <'                  PM WINDOW STATUS:'>
          CALL     NEW_LINE
          CALL     NEW_LINE
          MOV      AX,HDATA_STR.CWIN_FLAGS
          TEST     AX,HEX8
          JE       T1
          DISPLAY  <'The Window Is Visible. '>
          CALL     NEW_LINE

T1:       TEST     AX,HEX10
          JE       T2
          DISPLAY  <'The Window Is Invisible. '>
          CALL     NEW_LINE

T2:       TEST     AX,HEX80
          JE       T3
          DISPLAY  <'The Window Is Activated. '>
          CALL     NEW_LINE

T3:       TEST     AX,HEX100
          JE       T4
          DISPLAY  <'The Window Is Deactivated. '>
          CALL     NEW_LINE

T4:       TEST      AX,HEX400
          JE               T5
          DISPLAY   <'The Window Is Minimized. '>
          CALL      NEW_LINE

T5:       TEST      AX,HEX800
          JE        COORDS
          DISPLAY   <'The Window Is Maximized. '>
          CALL      NEW_LINE
COORDS:
          MOV       HPCB.HLPCB_DADDR, OFFSET HDATA_STR
          MOV       HPCB.HLPCB_FUNC, HA_QUERY_WINDOW_COORDS
                                   ; Issue query window coordinates

          MOV       AL, DFT_SESS
          MOV       HDATA_STR.GCOR_SHORTNAME, AL

          MOV       SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.
          CALL      HLLMASM               ; Call EHLLAPI.

          CMP       HPCB.HLPCB_RETCODE, HARC_SUCCESS
          JE        DONE_WQPM             ; If GOOD rc, jmp.
ERROR_WQPM:
          MOV      AX,HPCB.HLPCB_RETCODE  ; Put bad rc in RC.
          MOV      RC, AX
          XOR      BX,BX                  ; Set function number.
          MOV      BL,HPCB.HLPCB_FUNC
          CALL     ERROR_HAND
DONE_WQPM:
          MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                          ; Reset the orig EHLLAPI
                                          ; data_str.
        POP      ES
        POP      BP
        POP      DI
        POP      SI
        POP      DX
        POP      CX
        POP      BX
        POP      AX
        RET

QUERY_PM_WINDOW_STATUS      ENDP





;**************************************************************************
;QUERY PM WINDOW COORDINATES - Query PM window coordinates and report     *
;                              back the coordinates to OS/2 session.      *
;                                                                         *
;                                                                         *
;                                                                         *
;**************************************************************************

QUERY_PM_WINDOW_COORDS      PROC     NEAR

        PUSH     AX
        PUSH     BX
        PUSH     CX
        PUSH     DX
        PUSH     SI
        PUSH     DI
        PUSH     BP
        PUSH     ES


         MOV      AL, DFT_SESS          ; Set session id to
         MOV      HDATA_STR, AL         ; connect.
         MOV      HPCB.HLPCB_FUNC, HA_CONNECT_PM_SRVCS
                                        ; Issue Connect
                                        ; Presentation Space.
         MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                        ; DS already set up.

         CALL     HLLMASM               ; Call EHLLAPI.

         CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
         JE       OK_CRD                ; If GOOD rc, jmp.
         JMP      ERROR_COORDS          ; goto error routine.

OK_CRD:

         MOV      HPCB.HLPCB_FUNC, HA_QUERY_WINDOW_COORDS
         MOV      AL, DFT_SESS
         MOV      HDATA_STR.GCOR_SHORTNAME, AL
         MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                        ; DS already set up.
         CALL     HLLMASM               ; Call EHLLAPI.
         CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
         JE       COORD1                ; If GOOD rc, jmp.
         JMP      ERROR_COORDS          ; goto error routine.

COORD1:  CALL     NEW_LINE
         CALL     NEW_LINE
         DISPLAY  <'PM WINDOW COORDINATES :'>
         CALL     NEW_LINE

         CALL     NEW_LINE
         DISPLAY  <'XLEFT COORDINATE      :  '>
         MOV      SI, OFFSET HDATA_STR.GCOR_SHORTNAME
         ADD      SI,3
         CALL     PCOORD

         MOV      SI, OFFSET HDATA_STR.GCOR_SHORTNAME
         ADD      SI,1
         CALL     PCOORD

         CALL     NEW_LINE
         DISPLAY  <'YBOTTOM COORDINATE    :  '>
         MOV      SI, OFFSET HDATA_STR.GCOR_SHORTNAME
         ADD      SI,7
         CALL     PCOORD

         MOV      SI, OFFSET HDATA_STR.GCOR_SHORTNAME
         ADD      SI,5
         CALL     PCOORD

         CALL     NEW_LINE
         DISPLAY  <'XRIGHT COORDINATE     :  '>
         MOV      SI, OFFSET HDATA_STR.GCOR_SHORTNAME
         ADD      SI,11
         CALL     PCOORD

         MOV      SI, OFFSET HDATA_STR.GCOR_SHORTNAME
         ADD      SI,9
         CALL     PCOORD

         CALL     NEW_LINE
         DISPLAY  <'YTOP COORDINATE       :  '>
         MOV      SI, OFFSET HDATA_STR.GCOR_SHORTNAME
         ADD      SI,15
         CALL     PCOORD

         MOV      SI, OFFSET HDATA_STR.GCOR_SHORTNAME
         ADD      SI,13
         CALL     PCOORD
         CALL     NEW_LINE
         CALL     NEW_LINE
         DISPLAY <'(press CTRL-ESC to verify)'>
         CALL     NEW_LINE
         CALL     NEW_LINE

         JMP      DONE_COORDS

ERROR_COORDS:

         MOV      AX,HPCB.HLPCB_RETCODE ; Put bad rc in RC.
         MOV      RC, AX
         XOR      BX,BX                 ; Set function number.
         MOV      BL,HPCB.HLPCB_FUNC
         CALL     ERROR_HAND

DONE_COORDS:
        MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                         ; Reset the orig EHLLAPI
                                         ; data_str.
        POP      ES
        POP      BP
        POP      DI
        POP      SI
        POP      DX
        POP      CX
        POP      BX
        POP      AX
        RET

QUERY_PM_WINDOW_COORDS     ENDP



;*************************************************************************
; PCOORD - Display a window coordinate in hex.                           *
;                                                                        *
;INPUT                                                                   *
;  SI    - index to coordinate from QUERY_PM_WINDOW_COORDS               *
;                                                                        *
;                                                                        *
;*************************************************************************

PCOORD  PROC     NEAR


         MOV      DI,OFFSET DOUBLE_BUFF
         MOV      CX,2
REP      MOVS     DOUBLE_BUFF,HDATA_STR.GCOR_SHORTNAME
         MOV      AH, DOUBLE_BUFF [1]
         MOV      AL, DOUBLE_BUFF
         MOV      BX, OFFSET DIGIT_BUFF
         CALL     ITOH
         MOV      AX,4
         CALL     DSP_MSG_LEN

         RET
PCOORD   ENDP




;**************************************************************************
;MAKE WINDOW INVISIBLE - Make the PM emulator window invisible            *
;                                                                         *
;                                                                         *
;                                                                         *
;**************************************************************************

MAKE_WINDOW_INVISIBLE      PROC     NEAR

           PUSH     AX
           PUSH     BX
           PUSH     CX
           PUSH     DX
           PUSH     SI
           PUSH     DI
           PUSH     BP
           PUSH     ES

           MOV      AL, DFT_SESS          ; Set session id to
           MOV      HDATA_STR, AL         ; connect.

           MOV      HPCB.HLPCB_FUNC, HA_CONNECT_PM_SRVCS
                                          ; Issue Connect
                                          ; Presentation Space.
           MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.
           CALL     HLLMASM               ; Call EHLLAPI.
           CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
           JE       OK_WIN1               ; If GOOD rc, jmp.
           JMP      ERROR_WIN             ; goto error routine.
OK_WIN1:
           MOV      HPCB.HLPCB_FUNC, HA_PM_WINDOW_STATUS
                                          ; Issue change window name
           MOV      AL, DFT_SESS          ; Set session id
           MOV      HDATA_STR.CWIN_SHORTNAME, AL
           MOV      HDATA_STR.CWIN_OPTION,1
           MOV      HDATA_STR.CWIN_FLAGS,HEX10
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
           MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.
           CALL     HLLMASM               ; Call EHLLAPI.
           CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
           JE       OK_WIN2               ; If GOOD rc, jmp.
           JMP      ERROR_WIN             ; goto error routine.
OK_WIN2:
           DISPLAY  <'The PM Window Is Now Invisible.'>
           CALL     NEW_LINE
           DISPLAY  <'(press CTRL-ESC to verify)'>
           CALL     NEW_LINE
           CALL     NEW_LINE
           JMP      DONE_WIN
ERROR_WIN:
           MOV      AX,HPCB.HLPCB_RETCODE ; Put bad rc in RC.
           MOV      RC, AX
           XOR      BX,BX                 ; Set function number.
           MOV      BL,HPCB.HLPCB_FUNC
           CALL     ERROR_HAND
DONE_WIN:
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                          ; Reset the orig EHLLAPI
                                          ; data_str.
           POP      ES
           POP      BP
           POP      DI
           POP      SI
           POP      DX
           POP      CX
           POP      BX
           POP      AX

           RET
MAKE_WINDOW_INVISIBLE    ENDP




;**************************************************************************
;DISCONNECT PM WINDOW SERVICE - Disconnect from the PM emulator window    *
;                                                                         *
;                                                                         *
;                                                                         *
;                                                                         *
;**************************************************************************

DISCONNECT_PM_WINDOW_SERVICE     PROC     NEAR


            PUSH     AX
            PUSH     BX
            PUSH     CX
            PUSH     DX
            PUSH     SI
            PUSH     DI
            PUSH     BP
            PUSH     ES
            MOV      AL, DFT_SESS          ; Set session id to
            MOV      HDATA_STR, AL         ; connect.
            MOV      HPCB.HLPCB_FUNC, HA_CONNECT_PM_SRVCS
                                           ; Issue Connect
                                           ; Presentation Space.
            MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                           ; DS already set up.
            CALL     HLLMASM               ; Call EHLLAPI.
            CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
            JE       OK_WDS1               ; If GOOD rc, jmp.
            JMP      ERROR_WDS             ; goto error routine.
OK_WDS1:
            MOV      HPCB.HLPCB_FUNC, HA_DISCONNECT_PM_SRVCS
                                           ; Issue disconnect from PM
            MOV      AL, DFT_SESS          ; Set session id
            MOV      HDATA_STR.STPM_SHORTNAME, AL
            MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
            MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                           ; DS already set up.
            CALL     HLLMASM               ; Call EHLLAPI.
            CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
            JE       OK_WDS2               ; If GOOD rc, jmp.
            JMP      ERROR_WDS             ; goto error routine.
OK_WDS2:
            DISPLAY  <'The PM Window Is Now Disconnected.'>
            DISPLAY  <' '>
            CALL     NEW_LINE
            CALL     NEW_LINE
            JMP      DONE_WIN
ERROR_WDS:
            MOV      AX,HPCB.HLPCB_RETCODE ; Put bad rc in RC.
            MOV      RC, AX
            XOR      BX,BX                 ; Set function number.
            MOV      BL,HPCB.HLPCB_FUNC
            CALL     ERROR_HAND
DONE_WDS:
            MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                           ; Reset the orig EHLLAPI
                                           ; data_str.
            POP      ES
            POP      BP
            POP      DI
            POP      SI
            POP      DX
            POP      CX
            POP      BX
            POP      AX

            RET

DISCONNECT_PM_WINDOW_SERVICE   ENDP



;**************************************************************************
;RESET SWITCH AND WINDOW NAME - Reset the switch list name, the PM window *
;                               window name and restore the window size   *
;                                                                         *
;                                                                         *
;                                                                         *
;**************************************************************************

RESET_SWITCH_AND_WINDOW_NAME     PROC     NEAR


           PUSH     AX
           PUSH     BX
           PUSH     CX
           PUSH     DX
           PUSH     SI
           PUSH     DI
           PUSH     BP
           PUSH     ES

           MOV      AL, DFT_SESS          ; Set session id to
           MOV      HDATA_STR, AL         ; connect.
           MOV      HPCB.HLPCB_FUNC, HA_CONNECT_PM_SRVCS
                                          ; Issue Connect
                                          ; Presentation Space.
           MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.
           CALL     HLLMASM               ; Call EHLLAPI.
           CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
           JE       OK_WRS1               ; If GOOD rc, jmp.
           JMP      ERROR_WRS             ; goto error routine.
OK_WRS1:
           MOV      HPCB.HLPCB_FUNC, HA_PM_WINDOW_STATUS

           MOV      AL, DFT_SESS          ; Set session id
           MOV      HDATA_STR.CWIN_SHORTNAME, AL
           MOV      HDATA_STR.CWIN_OPTION,1
           MOV      AX,HEX1000
           MOV      HDATA_STR.CWIN_FLAGS ,AX
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
           MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.
           CALL     HLLMASM               ; Call EHLLAPI.
           CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
           JE       OK_WRS2               ; If GOOD rc, jmp.
           JMP      ERROR_WRS             ; goto error routine.
OK_WRS2:
           DISPLAY <'The PM Window Is Now Restored.'>
           CALL     NEW_LINE
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
           MOV      HPCB.HLPCB_FUNC, HA_CHANGE_WINDOW_NAME
                                          ; Issue change window name
           MOV      HPCB.HLPCB_LENGTH, 3
           MOV      AL, DFT_SESS          ; Set session id
           MOV      HDATA_STR.CHLT_SHORTNAME, AL
           MOV      HDATA_STR.CHLT_OPTION,2
           MOV      HDATA_STR.CHLT_LTNAME,0
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
           MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.
           CALL     HLLMASM               ; Call EHLLAPI.
           CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
           JE       OK_WRS3               ; If GOOD rc, jmp.
           JMP      ERROR_WRS             ; goto error routine.
OK_WRS3:
           DISPLAY <'The PM Window Title Is Now Restored.'>
           CALL       NEW_LINE
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
           MOV      HPCB.HLPCB_FUNC, HA_CHANGE_SWITCH_NAME
                                          ; Issue change switch list name
           MOV      HPCB.HLPCB_LENGTH, 2
           MOV      AL, DFT_SESS          ; Set session id
           MOV      HDATA_STR.CHSW_SHORTNAME, AL
           MOV      HDATA_STR.CHSW_OPTION,2
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
           MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.
           CALL     HLLMASM               ; Call EHLLAPI.
           CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
           JE       OK_WRS4               ; If GOOD rc, jmp.
           JMP      ERROR_WRS             ; goto error routine.
OK_WRS4:
           DISPLAY  <'The Switch List Name Is Restored.'>
           CALL     NEW_LINE
           DISPLAY  <'(Press CTRL-ESC To Verify)'>
           CALL     NEW_LINE
           CALL     NEW_LINE
           JMP      DONE_WRS
ERROR_WRS:
           MOV      AX,HPCB.HLPCB_RETCODE ; Put bad rc in RC.
           MOV      RC, AX
           XOR      BX,BX                 ; Set function number.
           MOV      BL,HPCB.HLPCB_FUNC
           CALL     ERROR_HAND
DONE_WRS:
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                          ; Reset the orig EHLLAPI
                                          ; data_str.
           POP      ES
           POP      BP
           POP      DI
           POP      SI
           POP      DX
           POP      CX
           POP      BX
           POP      AX
           RET
RESET_SWITCH_AND_WINDOW_NAME    ENDP




;**************************************************************************
;MAKE WINDOW VISIBLE - Make the PM window visible and maximized           *
;                                                                         *
;                                                                         *
;                                                                         *
;                                                                         *
;**************************************************************************

MAKE_WINDOW_VISIBLE      PROC     NEAR

           PUSH     AX
           PUSH     BX
           PUSH     CX
           PUSH     DX
           PUSH     SI
           PUSH     DI
           PUSH     BP
           PUSH     ES

           MOV      AL, DFT_SESS          ; Set session id to
           MOV      HDATA_STR, AL         ; connect.
           MOV      HPCB.HLPCB_FUNC, HA_CONNECT_PM_SRVCS
                                          ; Issue Connect
                                          ; Presentation Space.
           MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.
           CALL     HLLMASM               ; Call EHLLAPI.

           CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
           JE       OK_WVS1               ; If GOOD rc, jmp.
           JMP      ERROR_WVS             ; goto error routine
OK_WVS1:
           MOV      HPCB.HLPCB_FUNC, HA_PM_WINDOW_STATUS
                                          ; Issue change window name
           MOV      AL, DFT_SESS          ; Set session id
           MOV      HDATA_STR.CWIN_SHORTNAME, AL
           MOV      HDATA_STR.CWIN_OPTION,1
           MOV      HDATA_STR.CWIN_FLAGS,HEX808
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
           MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                          ; DS already set up.
           CALL     HLLMASM               ; Call EHLLAPI.
           CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
           JE       OK_WVS2               ; If GOOD rc, jmp.
           JMP      ERROR_WVS             ; goto error routine.
OK_WVS2:
           DISPLAY  <'The PM Window Is Now Visible And Maximized.'>
           CALL     NEW_LINE
           DISPLAY  <'(Press CTRL-ESC To Verify)'>
           CALL     NEW_LINE
           CALL     NEW_LINE
           JMP      DONE_WIN
ERROR_WVS:
           MOV      AX,HPCB.HLPCB_RETCODE ; Put bad rc in RC.
           MOV      RC, AX
           XOR      BX,BX                 ; Set function number.
           MOV      BL,HPCB.HLPCB_FUNC
           CALL     ERROR_HAND
DONE_WVS:
           MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                          ; Reset the orig EHLLAPI
                                          ; data_str.
           POP      ES
           POP      BP
           POP      DI
           POP      SI
           POP      DX
           POP      CX
           POP      BX
           POP      AX
           RET

MAKE_WINDOW_VISIBLE    ENDP

;**************************************************************************
;RESET EHLLAPI - Reset EHLLAPI to to its original condition               *
;                                                                         *
;                                                                         *
;                                                                         *
;**************************************************************************

RESET_SYSTEM     PROC     NEAR


       PUSH     AX
       PUSH     BX
       PUSH     CX
       PUSH     DX
       PUSH     SI
       PUSH     DI
       PUSH     BP
       PUSH     ES

       MOV      HPCB.HLPCB_FUNC, HA_RESET_SYSTEM
       MOV      HPCB.HLPCB_LENGTH, 1
       MOV      SI, OFFSET HPCB           ; DS:SI-> PCB
                                          ; DS already set up.
       CALL     HLLMASM                   ; Call EHLLAPI.
       CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
       JE       DONE_RS                   ; If GOOD rc, jmp.
       MOV      AX,HPCB.HLPCB_RETCODE     ; Put bad rc in RC.
       MOV      RC, AX
       XOR      BX,BX                     ; Set function number.
       MOV      BL,HPCB.HLPCB_FUNC
       CALL     ERROR_HAND
DONE_RS:
       LEA      BX, BLANK_SCR             ; Blank the screen.
       CALL     DSP_MSG
       DISPLAY  <'Ehllapi Reset To Its Original Conditions. '>
       CALL     NEW_LINE
       CALL     NEW_LINE
       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                          ; Reset the orig EHLLAPI
                                          ; data_str.
       POP      ES
       POP      BP
       POP      DI
       POP      SI
       POP      DX
       POP      CX
       POP      BX
       POP      AX

      RET

RESET_SYSTEM   ENDP



;**************************************************************************
;QUERY COM BUFFER SIZE - Query the communications buffer size.            *
;                        Collect buffer size information.                 *
;                                                                         *
;                                                                         *
;**************************************************************************

QUERY_COM_BUFFER_SIZE     PROC     NEAR


       PUSH     AX
       PUSH     BX
       PUSH     CX
       PUSH     DX
       PUSH     SI
       PUSH     DI
       PUSH     BP
       PUSH     ES

       MOV      HPCB.HLPCB_FUNC, HA_QUERY_BUFFER_SIZE
       MOV      HPCB.HLPCB_LENGTH, 9
       MOV      AL, DFT_SESS                  ; Put session id in data string
       MOV      HDATA_STR.QBUF_SHORTNAME, AL
       MOV      SI, OFFSET HPCB               ; DS:SI-> PCB
                                              ; DS already set up.
       CALL     HLLMASM                       ; Call EHLLAPI.
       CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
       JE       DONE_QCB                      ; If GOOD rc, jmp.
       MOV      AX,HPCB.HLPCB_RETCODE         ; Put bad rc in RC.
       MOV      RC, AX
       XOR      BX,BX                         ; Set function number.
       MOV      BL,HPCB.HLPCB_FUNC
       CALL     ERROR_HAND
DONE_QCB:
       DISPLAY  <'Query The Communications Buffer.'>
       CALL     NEW_LINE
       DISPLAY  <'Optimal Inbound Buffer Size  : '>
       MOV      AX,HDATA_STR.QBUF_OPT_INB
       MOV      OPTIMAL_INB,AX
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG
       CALL     NEW_LINE

       DISPLAY  <'Maximum Inbound Buffer Size  : '>
       MOV      AX,HDATA_STR.QBUF_MAX_INB
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG
       CALL     NEW_LINE

       DISPLAY  <'Optimal Outbound Buffer Size : '>
       MOV      AX,HDATA_STR.QBUF_OPT_OUTB
       MOV      OPTIMAL_OUTB,AX
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG
       CALL     NEW_LINE

       DISPLAY  <'Maximum Outbound Buffer Size : '>
       MOV      AX,HDATA_STR.QBUF_MAX_OUTB
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG
       CALL     NEW_LINE

       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                                 ; Reset the orig EHLLAPI
                                                 ; data_str.
       POP      ES
       POP      BP
       POP      DI
       POP      SI
       POP      DX
       POP      CX
       POP      BX
       POP      AX

      RET

QUERY_COM_BUFFER_SIZE   ENDP




;**************************************************************************
;ALLOCATE READ COM BUFFER - Allocate a communication buffer for to be     *
;                           used by read structured field                 *
;                                                                         *
;                                                                         *
;**************************************************************************

ALLOCATE_READ_COM_BUFFER     PROC     NEAR


       PUSH     AX
       PUSH     BX
       PUSH     CX
       PUSH     DX
       PUSH     SI
       PUSH     DI
       PUSH     BP
       PUSH     ES

       CALL     NEW_LINE
       DISPLAY  <'Allocate Read Buffer.'>
       MOV      HPCB.HLPCB_FUNC, HA_ALLOCATE_COMMO_BUFF
       MOV      HPCB.HLPCB_LENGTH, 6
       MOV      AX, OPTIMAL_INB
       MOV      HDATA_STR.ABUF_LENGTH, AX
       MOV      SI, OFFSET HPCB                    ; DS:SI-> PCB
                                                   ; DS already set up.
       CALL     HLLMASM                            ; Call EHLLAPI.
       CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
       JE       DONE_ACRB                          ; If GOOD rc, jmp.
       MOV      AX,HPCB.HLPCB_RETCODE              ; Put bad rc in RC.
       MOV      RC, AX
       XOR      BX,BX                              ; Set function number.
       MOV      BL,HPCB.HLPCB_FUNC
       CALL     ERROR_HAND
       JMP      END_ACRB
DONE_ACRB:
       CALL     NEW_LINE
       DISPLAY  <'Read Buffer Length   : '>
       MOV      AX,HDATA_STR.ABUF_LENGTH
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG
       CALL     NEW_LINE
       LES      AX,HDATA_STR.ABUF_ADDRESS
       MOV      RD_ADDRESS_SEG,ES
       MOV      RD_ADDRESS_OFF,AX
       DISPLAY  <'Allocate Read Buffer Complete.'>
       CALL     NEW_LINE
END_ACRB:
       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                                   ; Reset the orig EHLLAPI
                                                   ; data_str.
       POP      ES
       POP      BP
       POP      DI
       POP      SI
       POP      DX
       POP      CX
       POP      BX
       POP      AX

       RET

ALLOCATE_READ_COM_BUFFER  ENDP


;**************************************************************************
;ALLOCATE WRITE COM BUFFER - Allocate a communication buffer to be used   *
;                            by write structured field                    *
;                                                                         *
;                                                                         *
;**************************************************************************

ALLOCATE_WRITE_COM_BUFFER     PROC     NEAR


       PUSH     AX
       PUSH     BX
       PUSH     CX
       PUSH     DX
       PUSH     SI
       PUSH     DI
       PUSH     BP
       PUSH     ES
       CALL     NEW_LINE
       DISPLAY  <'Allocate Write Buffer.'>
       MOV      HPCB.HLPCB_FUNC, HA_ALLOCATE_COMMO_BUFF
       MOV      HPCB.HLPCB_LENGTH, 6
       MOV      AX, OPTIMAL_OUTB
       MOV      HDATA_STR.ABUF_LENGTH, AX
       MOV      SI, OFFSET HPCB                      ; DS:SI-> PCB
                                                     ; DS already set up.
       CALL     HLLMASM                              ; Call EHLLAPI.
       CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
       JE       DONE_ACWB                            ; If GOOD rc, jmp.
       MOV      AX,HPCB.HLPCB_RETCODE                ; Put bad rc in RC.
       MOV      RC, AX
       XOR      BX,BX                                ; Set function number.
       MOV      BL,HPCB.HLPCB_FUNC
       CALL     ERROR_HAND
       JMP      END_ACWB
DONE_ACWB:
       CALL     NEW_LINE
       DISPLAY  <'Write Buffer Length   : '>
       MOV      AX,HDATA_STR.ABUF_LENGTH
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG
       CALL     NEW_LINE
       LES      AX,HDATA_STR.ABUF_ADDRESS
       MOV      WR_ADDRESS_OFF,AX
       MOV      WR_ADDRESS_SEG,ES
       DISPLAY  <'Allocate Write Buffer Complete.'>
END_ACWB:
       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                                   ; Reset the orig EHLLAPI
                                                   ; data_str.
       POP      ES
       POP      BP
       POP      DI
       POP      SI
       POP      DX
       POP      CX
       POP      BX
       POP      AX

       RET

ALLOCATE_WRITE_COM_BUFFER  ENDP



;**************************************************************************
;CONNECT STRUCTURED FIELD - Initiate a structured field connection        *
;                                                                         *
;                                                                         *
;                                                                         *
;**************************************************************************

CONNECT_TO_SF   PROC     NEAR


       PUSH     AX
       PUSH     BX
       PUSH     CX
       PUSH     DX
       PUSH     SI
       PUSH     DI
       PUSH     BP
       PUSH     ES

       CALL     NEW_LINE
       CALL     NEW_LINE
       DISPLAY  <'Initiate Structured Field Connection.'>

       MOV      HPCB.HLPCB_FUNC, HA_START_STRUCTURED_FLD
       MOV      HPCB.HLPCB_LENGTH, 11

       MOV      AL,   DFT_SESS
       MOV      HDATA_STR.STSF_SHORTNAME,AL
       MOV      AX,   OFFSET QUERY_REPLY
       MOV      WORD PTR HDATA_STR.STSF_QUERY,AX
       MOV      AX,   SEG QUERY_REPLY
       MOV      WORD PTR HDATA_STR.STSF_QUERY + 2,AX


       MOV      SI, OFFSET HPCB                      ; DS:SI-> PCB
                                                     ; DS already set up.

       CALL     HLLMASM                              ; Call EHLLAPI.
       CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
       JE       DONE_CTSF                            ; If GOOD rc, jmp.
       MOV      AX,HPCB.HLPCB_RETCODE                ; Put bad rc in RC.
       MOV      RC, AX
       XOR      BX,BX                                ; Set function number.
       MOV      BL,HPCB.HLPCB_FUNC
       CALL     ERROR_HAND
       JMP      END_CTSF
DONE_CTSF:
       CALL     NEW_LINE
       DISPLAY  <'Destination / Origin ID  : '>
       MOV      AX,HDATA_STR.STSF_DOID
       MOV      DOID,AX
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG
       CALL     NEW_LINE
       DISPLAY  <'Connection To Structured Field Complete.'>
END_CTSF:
       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                                     ; Reset the orig EHLLAPI
                                                     ; data_str.
       POP      ES
       POP      BP
       POP      DI
       POP      SI
       POP      DX
       POP      CX
       POP      BX
       POP      AX

       RET

CONNECT_TO_SF  ENDP



;**************************************************************************
;READ SF ASYNC - Perform an asynchronus read of a structured field        *
;                                                                         *
;                                                                         *
;                                                                         *
;**************************************************************************

READ_SF_ASYNC    PROC     NEAR


       PUSH     AX
       PUSH     BX
       PUSH     CX
       PUSH     DX
       PUSH     SI
       PUSH     DI
       PUSH     BP
       PUSH     ES

       MOV      HPCB.HLPCB_FUNC, HA_READ_STRUCTURED_FLD
       MOV      HPCB.HLPCB_LENGTH, 14
       MOV      AL, DFT_SESS
       MOV      HDATA_STR.RDSF_SHORTNAME, AL
       MOV      HDATA_STR.RDSF_OPTION, 65
       MOV      AX, DOID
       MOV      HDATA_STR.RDSF_DOID, AX
       MOV      DI, RD_ADDRESS_OFF
       MOV      WORD PTR HDATA_STR.RDSF_BUFFER,DI
       MOV      ES, RD_ADDRESS_SEG
       MOV      WORD PTR HDATA_STR.RDSF_BUFFER+2,ES
       MOV      WORD PTR ES:[DI],0
       MOV      WORD PTR ES:[DI][1],0
       MOV      WORD PTR ES:[DI][2],0
       MOV      WORD PTR ES:[DI][3],0
       MOV      WORD PTR ES:[DI][4],0
       MOV      BYTE PTR ES:[DI][5],5           ; length field

       CALL     NEW_LINE
       DISPLAY  <'Perform An Asynchronus Read Structured Field'>
       CALL     NEW_LINE
       DISPLAY  <'Session Shortname : '>
       MOV      AX,0
       MOV      AL,HDATA_STR.RDSF_SHORTNAME
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG

       CALL     NEW_LINE
       DISPLAY  <'Destination / Origin ID  : '>
       MOV      AX,HDATA_STR.RDSF_DOID
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG

       CALL     NEW_LINE
       DISPLAY  <'Control Option   : '>
       MOV      AX,0
       MOV      AL,HDATA_STR.RDSF_OPTION
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG
       CALL     NEW_LINE

       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
       MOV      SI, OFFSET HPCB                      ; DS:SI-> PCB
                                                     ; DS already set up.
       CALL     HLLMASM                              ; Call EHLLAPI.

       CMP      HPCB.HLPCB_RETCODE, HARC_INBOUND_DISABLED
       JE       DONE_RSF                             ; If GOOD rc, jmp.
       MOV      AX,HPCB.HLPCB_RETCODE                ; Put bad rc in RC.
       MOV      RC, AX
       XOR      BX,BX                                ; Set function number.
       MOV      BL,HPCB.HLPCB_FUNC
       CALL     ERROR_HAND
       JMP      END_RSF
DONE_RSF:
       DISPLAY  <'Read Asynchronus Complete.'>
       LES      AX,HDATA_STR.RDSF_ASEM
       MOV      ASEM_OFF,AX
       MOV      ASEM_SEG,ES
END_RSF:
       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                                   ; Reset the orig EHLLAPI
                                                   ; data_str.
       POP      ES
       POP      BP
       POP      DI
       POP      SI
       POP      DX
       POP      CX
       POP      BX
       POP      AX

       RET

READ_SF_ASYNC  ENDP

;**************************************************************************
;CREATE A STRUCTURED FIELD  - Connects to first session and writes        *
;                             home_key and a ind$file command             *
;                             (This will generate a structured field.)    *
;                                                                         *
;                                                                         *
;**************************************************************************

CREATE_SF      PROC     NEAR

        PUSH     AX
        PUSH     BX
        PUSH     CX
        PUSH     DX
        PUSH     SI
        PUSH     DI
        PUSH     BP
        PUSH     ES

        CALL     NEW_LINE
        CALL     NEW_LINE
        DISPLAY  <'Create a Structured Field.'>
        CALL     NEW_LINE
        MOV      AL, DFT_SESS          ; Set session id to
        MOV      HDATA_STR, AL         ; connect.
        MOV      HPCB.HLPCB_FUNC, HA_CONNECT_PS
                                       ; Issue Connect
                                       ; Presentation Space.
        MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                       ; DS already set up.
        CALL     HLLMASM               ; Call EHLLAPI.
        CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
        JE       OK_CASF               ; If GOOD rc, jmp.
        JMP      ERROR_CASF            ; goto error routine.
OK_CASF:
        MOV      HPCB.HLPCB_FUNC, HA_SENDKEY
                                       ; Issue sendkey
        MOV      HPCB.HLPCB_LENGTH, HOME_KEY_LEN
                                       ; Set length
        MOV      HPCB.HLPCB_DADDR, OFFSET HOME_KEY
                                       ; Set new data_str.
        MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                       ; DS already set up.
        CALL     HLLMASM               ; Call EHLLAPI.
        CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
        JE       OK_CASF1              ; If GOOD rc, jmp.
        JMP      ERROR_CASF            ; goto error routine.
OK_CASF1:
        MOV      HPCB.HLPCB_LENGTH, COMMAND_LEN
                                       ; Set length
        MOV      HPCB.HLPCB_DADDR, OFFSET COMMAND_STR
                                       ; ind$file command
        MOV      SI, OFFSET HPCB       ; DS:SI-> PCB
                                       ; DS already set up.
        CALL     HLLMASM               ; Call EHLLAPI.
        CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
        JE       OK_CASF2              ; If GOOD rc, jmp.
        JMP      ERROR_CASF            ; goto error routine.
OK_CASF2:
        JMP      DONE_CASF
ERROR_CASF:
        MOV      AX,HPCB.HLPCB_RETCODE ; Put bad rc in RC.
        MOV      RC, AX
        XOR      BX,BX                 ; Set function number.
        MOV      BL,HPCB.HLPCB_FUNC
        CALL     ERROR_HAND
DONE_CASF:
        MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                       ; Reset the orig EHLLAPI
                                       ; data_str.
        POP      ES
        POP      BP
        POP      DI
        POP      SI
        POP      DX
        POP      CX
        POP      BX
        POP      AX

        RET

CREATE_SF      ENDP


;**************************************************************************
;GET ASYNC COMPLETE - Check if asynchronus read of a structured field     *
;                     is complete                                         *
;                                                                         *
;                                                                         *
;                                                                         *
;**************************************************************************

GET_ASYNC_COMPLETE PROC NEAR


       PUSH     AX
       PUSH     BX
       PUSH     CX
       PUSH     DX
       PUSH     SI
       PUSH     DI
       PUSH     BP
       PUSH     ES

       MOV      HPCB.HLPCB_FUNC, HA_GET_ASYNC_COMPLETION
       MOV      HPCB.HLPCB_LENGTH, 14
       MOV      AL, DFT_SESS
       MOV      HDATA_STR.GCMP_SHORTNAME , AL
       MOV      HDATA_STR.GCMP_OPTION, 87     ; ASCII - 'W' for wait option
       MOV      HDATA_STR.GCMP_REQUESTID, 0
       MOV      HDATA_STR.GCMP_RET_FUNCTID, 0
       MOV      WORD PTR HDATA_STR.GCMP_RET_DATASTR, 0
       MOV      WORD PTR HDATA_STR.GCMP_RET_DATASTR+2, 0
       MOV      HDATA_STR.GCMP_RET_LENGTH, 0
       MOV      HDATA_STR.GCMP_RET_RETCODE, 0

       CALL     NEW_LINE
       DISPLAY  <'Perform An Asynchronus Get Completion Request.'>
       CALL     NEW_LINE
       DISPLAY  <'Session Shortname : '>
       MOV      AX,0
       MOV      AL,HDATA_STR.GCMP_SHORTNAME
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG

       CALL     NEW_LINE
       DISPLAY  <'Request Option  : '>
       MOV      AX,0
       MOV      AL,HDATA_STR.GCMP_OPTION
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG
       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
       MOV      SI, OFFSET HPCB                      ; DS:SI-> PCB
                                                     ; DS already set up.
       CALL     HLLMASM                              ; Call EHLLAPI.
       CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
       JE       DONE_GAC                             ; If GOOD rc, jmp.
       MOV      AX,HPCB.HLPCB_RETCODE                ; Put bad rc in RC.
       MOV      RC, AX
       XOR      BX,BX                                ; Set function number.
       MOV      BL,HPCB.HLPCB_FUNC
       CALL     ERROR_HAND
       JMP      END_GAC
DONE_GAC:
       CALL     NEW_LINE
       DISPLAY  <'Get Asynchronus Request Completed.'>
END_GAC:
       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                                     ; Reset the orig EHLLAPI
                                                     ; data_str.
       POP      ES
       POP      BP
       POP      DI
       POP      SI
       POP      DX
       POP      CX
       POP      BX
       POP      AX

       RET

GET_ASYNC_COMPLETE ENDP



;**************************************************************************
;WRITE SF SYNC - Perform a synchronus write of a structured field         *
;                                                                         *
;                                                                         *
;                                                                         *
;**************************************************************************

WRITE_SF_SYNC    PROC     NEAR


       PUSH     AX
       PUSH     BX
       PUSH     CX
       PUSH     DX
       PUSH     SI
       PUSH     DI
       PUSH     BP
       PUSH     ES

       CALL     NEW_LINE


       MOV      HPCB.HLPCB_FUNC, HA_WRITE_STRUCTURED_FLD
       MOV      HPCB.HLPCB_LENGTH, 8

       MOV      AL, DFT_SESS
       MOV      HDATA_STR.WRSF_SHORTNAME, AL
       MOV      HDATA_STR.WRSF_OPTION, 83   ; ASCII - 'S' for synchronus
       MOV      AX, DOID
       MOV      HDATA_STR.WRSF_DOID, AX
       MOV      BX, WR_ADDRESS_OFF          ; write address buffer
       MOV      WORD PTR HDATA_STR.WRSF_BUFFER,BX
       MOV      ES,WR_ADDRESS_SEG
       MOV      WORD PTR HDATA_STR.WRSF_BUFFER+2,ES
       MOV      DI, BX
       MOV      AL,0
       MOV      CX,540                      ; Fill next 540 words with AL
REP    STOS     ES:WORD PTR [DI]

       MOV      BYTE PTR ES:[BX][2],5       ; put length in header block
       MOV      BYTE PTR ES:[BX][9],5
       MOV      BYTE PTR ES:[BX][10],208    ; DOID
       MOV      BYTE PTR ES:[BX][12],9

       CALL     NEW_LINE
       DISPLAY  <'Perform A Synchronus Write Structured Field.'>
       CALL     NEW_LINE
       DISPLAY  <'Session Shortname : '>
       MOV      AX,0
       MOV      AL,HDATA_STR.WRSF_SHORTNAME
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG

       CALL     NEW_LINE
       DISPLAY  <'Destination / Origin ID  : '>
       MOV      AX,HDATA_STR.WRSF_DOID
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG

       CALL     NEW_LINE
       DISPLAY  <'Control Option  : '>
       MOV      AX,0
       MOV      AL,HDATA_STR.WRSF_OPTION
       MOV      BX,OFFSET DIGIT_BUFF
       CALL     ITOH
       CALL     DSP_MSG

       MOV      HPCB.HLPCB_DADDR,OFFSET HDATA_STR
       MOV      SI, OFFSET HPCB                      ; DS:SI-> PCB
                                                     ; DS already set up.
       CALL     HLLMASM                              ; Call EHLLAPI.
       CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
       JE       DONE_WSF                             ; If GOOD rc, jmp.
       MOV      AX,HPCB.HLPCB_RETCODE                ; Put bad rc in RC.
       MOV      RC, AX
       XOR      BX,BX                                ; Set function number.
       MOV      BL,HPCB.HLPCB_FUNC
       CALL     ERROR_HAND
       JMP      END_WSF
DONE_WSF:
       CALL     NEW_LINE
       DISPLAY  <'Synchronus Write Complete.'>
       MOV      AX,5000
       MOV      DX,0
       PUSH     DX
       PUSH     AX
       CALL     DOSSLEEP
END_WSF:
       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                                   ; Reset the orig EHLLAPI
                                                   ; data_str.
       POP      ES
       POP      BP
       POP      DI
       POP      SI
       POP      DX
       POP      CX
       POP      BX
       POP      AX

       RET

WRITE_SF_SYNC  ENDP




;**************************************************************************
;FREE READ COM BUFFER - De-allocate the communication buffer used for     *
;                       read structured field                             *
;                                                                         *
;                                                                         *
;**************************************************************************

FREE_READ_COM_BUFFER     PROC     NEAR


       PUSH     AX
       PUSH     BX
       PUSH     CX
       PUSH     DX
       PUSH     SI
       PUSH     DI
       PUSH     BP
       PUSH     ES

       CALL     NEW_LINE
       CALL     NEW_LINE
       DISPLAY  <'De-allocate The Communications Buffer.'>
       CALL     NEW_LINE

       MOV      HPCB.HLPCB_FUNC, HA_FREE_COMMO_BUFF
       MOV      HPCB.HLPCB_LENGTH, 6
       MOV      AX, OPTIMAL_INB
       MOV      HDATA_STR.FBUF_LENGTH, AX
       MOV      ES, RD_ADDRESS_SEG
       MOV      AX, RD_ADDRESS_OFF
       MOV      WORD PTR HDATA_STR.FBUF_ADDRESS,AX
       MOV      WORD PTR HDATA_STR.FBUF_ADDRESS+2,ES

       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
       MOV      SI, OFFSET HPCB                      ; DS:SI-> PCB
                                                     ; DS already set up.

       CALL     HLLMASM                              ; Call EHLLAPI.

       CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
       JE       DONE_FRB                             ; If GOOD rc, jmp.

       MOV      AX,HPCB.HLPCB_RETCODE ;Put bad rc in RC.
       MOV      RC, AX
       XOR      BX,BX                 ;Set function number.
       MOV      BL,HPCB.HLPCB_FUNC
       CALL     ERROR_HAND
       JMP      END_FRB

DONE_FRB:
       DISPLAY  <'De-allocate Read Buffer Complete.'>
END_FRB:
       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                                   ; Reset the orig EHLLAPI
                                                   ; data_str.
       POP      ES
       POP      BP
       POP      DI
       POP      SI
       POP      DX
       POP      CX
       POP      BX
       POP      AX

       RET

FREE_READ_COM_BUFFER  ENDP




;**************************************************************************
;FREE WRITE COM BUFFER - De-allocate the communication buffer used for    *
;                        write structured field                           *
;                                                                         *
;                                                                         *
;**************************************************************************

FREE_WRITE_COM_BUFFER     PROC     NEAR


       PUSH     AX
       PUSH     BX
       PUSH     CX
       PUSH     DX
       PUSH     SI
       PUSH     DI
       PUSH     BP
       PUSH     ES


       MOV      HPCB.HLPCB_FUNC, HA_FREE_COMMO_BUFF
       MOV      HPCB.HLPCB_LENGTH, 6
       MOV      AX, OPTIMAL_OUTB
       MOV      HDATA_STR.FBUF_LENGTH, AX
       MOV      ES, WR_ADDRESS_SEG
       MOV      AX, WR_ADDRESS_OFF
       MOV      WORD PTR HDATA_STR.FBUF_ADDRESS,AX
       MOV      WORD PTR HDATA_STR.FBUF_ADDRESS+2,ES
       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
       MOV      SI, OFFSET HPCB                       ; DS:SI-> PCB
                                                      ; DS already set up.
       CALL     HLLMASM                               ; Call EHLLAPI.
       CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
       JE       DONE_FWRB                             ; If GOOD rc, jmp.
       MOV      AX,HPCB.HLPCB_RETCODE                 ; Put bad rc in RC.
       MOV      RC, AX
       XOR      BX,BX                                 ; Set function number.
       MOV      BL,HPCB.HLPCB_FUNC
       CALL     ERROR_HAND
       JMP      END_FWRB

DONE_FWRB:

       CALL     NEW_LINE
       DISPLAY  <'De-allocate Write Buffer Complete.'>
       CALL     NEW_LINE
END_FWRB:
       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                                      ; Reset the original
                                                      ; EHLLAPI data_str.
       POP      ES
       POP      BP
       POP      DI
       POP      SI
       POP      DX
       POP      CX
       POP      BX
       POP      AX

       RET

FREE_WRITE_COM_BUFFER  ENDP


;**************************************************************************
;DISCONNECT STRUCTURED FIELD - Disconnect From Structured Field           *
;                                                                         *
;                                                                         *
;                                                                         *
;**************************************************************************

DISCONNECT_SF    PROC     NEAR


       PUSH     AX
       PUSH     BX
       PUSH     CX
       PUSH     DX
       PUSH     SI
       PUSH     DI
       PUSH     BP
       PUSH     ES

       CALL     NEW_LINE
       DISPLAY  <'Initiate Disconnect From Structured Field.'>
       MOV      HPCB.HLPCB_FUNC, HA_STOP_STRUCTURED_FLD
       MOV      HPCB.HLPCB_LENGTH, 3
       MOV      AL,DFT_SESS
       MOV      HDATA_STR.SPSF_SHORTNAME,AL
       MOV      AX,DOID
       MOV      HDATA_STR.SPSF_DOID,AX
       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
       MOV      SI, OFFSET HPCB                    ; DS:SI-> PCB
                                                   ; DS already set up.
       CALL     HLLMASM                            ; Call EHLLAPI.
       CMP      HPCB.HLPCB_RETCODE, HARC_SUCCESS
       JE       DONE_DIS                           ; If GOOD rc, jmp.

       MOV      AX,HPCB.HLPCB_RETCODE              ; Put bad rc in RC.
       MOV      RC, AX
       XOR      BX,BX                              ; Set function number.
       MOV      BL,HPCB.HLPCB_FUNC
       CALL     ERROR_HAND
DONE_DIS:
       CALL     NEW_LINE
       DISPLAY  <'Structured Field Disconnected. '>
       CALL     NEW_LINE
       MOV      HPCB.HLPCB_DADDR, OFFSET HDATA_STR
                                                   ; Reset the orig EHLLAPI
                                                   ; data_str.
      POP      ES
      POP      BP
      POP      DI
      POP      SI
      POP      DX
      POP      CX
      POP      BX
      POP      AX

      RET

DISCONNECT_SF   ENDP










;***********************************************************************
;* ERROR HAND - Error Handler                                          *
;*                                                                     *
;*     INPUT                                                           *
;*          Ax - RETURN CODE                                           *
;*          Bx - FUNCTION                                              *
;*                                                                     *
;*     OUTPUT                                                          *
;*                                                                     *
;***********************************************************************

ERROR_HAND PROC     NEAR

           PUSH     AX
           PUSH     BX
           PUSH     CX

           MOV      CX,BX                  ; Save BX.
           DISPLAY  <'UNEXPECTED RETURN CODE '>
           MOV      BX, OFFSET DIGIT_BUFF
           CALL     ITOA
           CALL     DSP_MSG
           DISPLAY  <' from FUNCTION #'>
           MOV      AX, CX;               ; Get function number.
           CALL     ITOA
           CALL     DSP_MSG
           DISPLAY  <'.'>

           POP      CX
           POP      BX
           POP      AX

           RET
ERROR_HAND ENDP




;**************************************************************************
;NEW_LINE - Writes new line char.                                         *
;                                                                         *
;INPUT                                                                    *
;  DS:BX -> MSG                                                           *
;                                                                         *
;OUTPUT                                                                   *
;                                                                         *
;                                                                         *
;**************************************************************************

NEW_LINE      PROC     NEAR

              PUSH     AX
              PUSH     BX

              LEA      BX,CRLF_VAR
              MOV      AX, 2
              CALL     DSP_MSG_LEN

              POP      BX
              POP      AX

              RET
NEW_LINE      ENDP


;**************************************************************************
;DSP_MSG_LEN - Display msg.                                               *
;                                                                         *
;INPUT                                                                    *
;  DS:BX -> MSG                                                           *
;  AX    - length                                                         *
;                                                                         *
;OUTPUT                                                                   *
;                                                                         *
;                                                                         *
;**************************************************************************

DSP_MSG_LEN    PROC     NEAR

               PUSH     AX
               PUSH     DS
               PUSH     BX
               PUSH     AX
               PUSH     0
               CALL     VIOWRTTTY
               POP      AX

               RET
DSP_MSG_LEN    ENDP





;**************************************************************************
;DSP_MSG - Display NULL terminated msg.                                   *
;                                                                         *
;INPUT                                                                    *
;  DS:BX -> MSG                                                           *
;                                                                         *
;OUTPUT                                                                   *
;                                                                         *
;                                                                         *
;**************************************************************************

DSP_MSG      PROC     NEAR


             PUSH     AX
             PUSH     BX
TOP_DM:
             CMP      BYTE PTR [BX], NULL
             JE       DONE_DM
             PUSH     DS
             PUSH     BX
             PUSH     1
             PUSH     0
             CALL     VIOWRTTTY
             INC      BX
             JMP      TOP_DM
DONE_DM:
             POP      BX
             POP      AX

             RET
DSP_MSG      ENDP




;**************************************************************************
;FGETCHAR - Calls DOS to read the keyboard.                               *
;                                                                         *
;                                                                         *
;                                                                         *
;**************************************************************************
FGETCHAR              PROC     NEAR

                      PUSH     AX



                      PUSH     DS
                      PUSH     OFFSET DUMMY_BUFF
                      PUSH     0
                      PUSH     0
                      CALL     KBDCHARIN

                      POP      AX

                      RET
FGETCHAR              ENDP




;**************************************************************************
;ITOA - Convert binary to decimal digits.                                 *
;                                                                         *
;INPUT                                                                    *
;  AX    - BINARY NUMBER TO CONVERTS TO HEX.                              *
;  DS:BX - WHERE DIGITS WILL BE PLACED.  Buffer min is 6 bytes.           *
;                                                                         *
;OUTPUT                                                                   *
;  DS:BX - WHERE DIGITS WILL BE PLACED. Last byte will be set to NULL.    *
;                                                                         *
;                                                                         *
;**************************************************************************

ITOA         PROC     NEAR
             PUSH     AX
             PUSH     BX
             PUSH     CX
             PUSH     DX
             PUSH     SI
             PUSH     DI
             PUSH     BP

             MOV      BP,0                  ; BP flag to indicate
                                            ; if written once.
             MOV      DI,BX                 ; Save where to put
             LEA      BX,HEX_TABLE          ; address of translate tab.
                                            ; digits.
             MOV      SI,AX                 ; Save AX
             MOV      CX, 10000             ; Divide by 10000
             MOV      DX,0
             DIV      CX
             CMP      AX,0
             JE       NEXT_ITOA

             MOV      BP,1                  ; Written once.
             PUSH     AX
             XLAT     HEX_TABLE             ; convert - result in AL
             MOV      [DI], AL
             POP      AX
             INC      DI
             MOV      CX,10000
             MUL      CX                    ; Mul CX x AX
             SUB      SI,AX                 ; Subtract out 10000
                                            ; numbers
NEXT_ITOA:
             MOV      AX, SI                ; Divide by 1000
             MOV      CX, 1000
             MOV      DX,0
             DIV      CX
             CMP      BP,0                  ; Written once?
             JNE      WRT_DIG               ; If written digit, jmp
             CMP      AX,0
             JE       NEXT_ITOA1
WRT_DIG:
             MOV      BP,1                  ; Written once.
             PUSH     AX
             XLAT     HEX_TABLE             ; convert - result in AL
             MOV      [DI], AL
             POP      AX
             INC      DI
             MOV      CX,1000
             MUL      CX                    ; Mul CX x AX
             SUB      SI,AX                 ; Subtract out 1000
                                            ; numbers
NEXT_ITOA1:

             MOV      AX, SI                ; Divide by 100
             MOV      CX, 100
             MOV      DX,0
             DIV      CX
             CMP      BP,0                  ; Written once?
             JNE      WRT_DIG1              ; If written digit, jmp
             CMP      AX,0
             JE       NEXT_ITOA2
WRT_DIG1:
             MOV      BP,1                  ; Written once.
             PUSH     AX
             XLAT     HEX_TABLE             ; convert - result in AL
             MOV      [DI], AL
             POP      AX
             INC      DI
             MOV      CX,100
             MUL      CX                    ; Mul CX x AX
             SUB      SI,AX                 ; Subtract out 100
                                            ; numbers
NEXT_ITOA2:
             MOV      AX, SI                ; Divide by 10
             MOV      CX, 10
             MOV      DX,0
             DIV      CX
             CMP      BP,0                  ; Written once?
             JNE      WRT_DIG2              ; If written digit, jmp
             CMP      AX,0
             JE       NEXT_ITOA3
WRT_DIG2:
             MOV      BP,1                  ; Written once.
             PUSH     AX
             XLAT     HEX_TABLE             ; convert - result in AL
             MOV      [DI], AL
             POP      AX
             INC      DI
             MOV      CX,10
             MUL      CX                    ; Mul CX x AX
             SUB      SI,AX                 ; Subtract out 10
                                            ; numbers
NEXT_ITOA3:
             MOV      AX, SI
             XLAT     HEX_TABLE             ; convert - result in AL
             MOV      [DI], AL
             INC      DI
             MOV      BYTE PTR [DI], NULL   ; NULL last char.

             POP      BP
             POP      DI
             POP      SI
             POP      DX
             POP      CX
             POP      BX
             POP      AX
             RET
ITOA         ENDP




;******************************************************************************
;ITOH - Convert binary to hex digits.                                         *
;                                                                             *
;INPUT                                                                        *
;  AX    - BINARY NUMBER TO CONVERTS TO HEX.                                  *
;  DS:BX - WHERE DIGITS WILL BE PLACED.  Buffer min is 4 bytes.               *
;                                                                             *
;OUTPUT                                                                       *
;  DS:BX - WHERE DIGITS WILL BE PLACED.                                       *
;                                                                             *
;                                                                             *
;******************************************************************************
ITOH     PROC     NEAR
         PUSH     AX
         PUSH     BX
         PUSH     DI
         PUSH     DX

         MOV      DI,BX                 ; Save where to put
         LEA      BX,HEX_TABLE          ; address of translate tab.
                                        ; digits.
         MOV      DX,AX

         SHR      AX, 12
         XLAT     HEX_TABLE             ; convert - result in AL
         MOV      [DI], AL
         INC      DI
         MOV      AX, DX                ; Restore AX

         SHR      AX, 8
         AND      AL,0FH
         XLAT     HEX_TABLE             ; convert - result in AL
         MOV      [DI], AL
         INC      DI
         MOV      AX, DX                ; Restore AX

         SHR      AX, 4
         AND      AL,0FH
         XLAT     HEX_TABLE             ; convert - result in AL
         MOV      [DI], AL
         INC      DI
         MOV      AX, DX                ; Restore AX
         AND      AL,0FH
         XLAT     HEX_TABLE             ; convert - result in AL
         MOV      [DI], AL
                                        ; number to the screen.
         POP      DX
         POP      DI
         POP      BX
         POP      AX

         RET
ITOH     ENDP


CSEG     ENDS
                      END      MAIN
