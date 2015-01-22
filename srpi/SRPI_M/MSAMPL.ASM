;*************************PROLOGUE*******************************
;                                                               *
; MODULE NAME = MSAMPL.ASM                                      *
;                                                               *
; DESCRIPTIVE NAME = Macro Assembler Sample Program             *
;                                                               *
; STATUS=    Extended Services Version 1.0 Modification 0       *
;                                                               *
; COPYRIGHT=  (C) COPYRIGHT IBM CORP. 1988, 1991                *
;             LICENSED MATERIAL - PROGRAM PROPERTY OF IBM       *
;             ALL RIGHTS RESERVED                               *
;                                                               *
; FUNCTION = Invoke a hypothetical server via the Macro         *
;            Assembler interface macros.                        *
;                                                               *
;            This sample program reads a customer record        *
;            from a host computer, examines the customer's      *
;            balance, and writes the customer record if         *
;            the balance is greater than zero.                  *
;                                                               *
; NOTES =                                                       *
;                                                               *
;   RESTRICTIONS = This sample program is provided solely       *
;                  as an example of how the Macro Assembler     *
;                  macros can be used to invoke a server.       *
;                                                               *
; MODULE TYPE = IBM MACRO Assembler/2 Version 1.00              *
;*************************END PROLOGUE***************************
;*************************DEFINITIONS****************************
extrn DosExit:far
        INCLUDE uuminfac.mac
        SUBTTL  'CPRB Mapping'
        PAGE
        INCLUDE uumcprb.inc
;----------------------------------------------------------------
        SUBTTL  'Customer Record Mapping'
        PAGE
mcustrec        STRUC
mcusname        db      25 dup (?)      ;name
mcusaddr        db      25 dup (?)      ;street address
mcuscity        db      15 dup (?)      ;city
mcusstat        db      15 dup (?)      ;state
mcuszip         db       9 dup (?)      ;zip
mcusacct        db      16 dup (?)      ;account number
mcusbal         dd      ?               ;balance
mcustrec        ENDS
;----------------------------------------------------------------
        SUBTTL  'Request Parameters Mapping'
        PAGE
mqparms         STRUC
mqpaflags       db      ?               ;Processing flags
mqpaoper        db      8 dup (?)       ;Requesting operator

mqparms         ENDS
;
;Equates for processing flags defined in STRUC mqparms
mqpalog         equ     01H             ;Log the transaction
mqpacom         equ     02H             ;Commit the transaction
;
;----------------------------------------------------------------
        SUBTTL  'MWORK - Work Area Segment'
        PAGE
mwork           SEGMENT 'data'
mdabuf          db      SIZE mcustrec dup (?)  ;Allocate
;                                       buffer for customer
;                                       records
mdabuf@         dd      mdabuf          ;Vector to customer
;                                        record buffer
mdabufl         equ     SIZE mcustrec   ;Length of a customer
;                                        record

mqprmbuf        db      SIZE mqparms dup (?) ;Allocate a buffer
;                                       for request parms
mqprmbuf@       dd      mqprmbuf        ;Vector to request
;                                        parameters buffer
mqprmbufl       equ     SIZE mqparms    ;Length of a request
;                                        parameters

mserver_1$      equ     $               ;First character of
;                                        server name
mserver         db      'IBMabase'      ;Server name
mserver_len$    equ     $-mserver_1$    ;Length of server name

mfunc1          equ     1               ;Func code: Get Record
mfunc2          equ     2               ;Func code: Update AR file

mrcok           equ     0000H           ;Server Return Code: OK
mlstrh          equ     00H             ;Last Record high byte
mlstrl          equ     04H             ;Last Record low byte

moper_1$        equ     $               ;First byte - operator
;                                        name
moper           db      'ADMIN   '      ;Default operator name
moper_len$      equ     $-moper_1$      ;Length - operator name

mretcode        dd      ?               ;SRPI Return Code
                org     mretcode-mwork
mrclow          dw      0               ;Low word of return code
mrchigh         dw      0               ;High word of return code

mservrc         dd      ?               ;Server Return Code
                org     mservrc-mwork
msrvrclow       dw      0               ;Low word of return code
msrvrchigh      dw      0               ;High word of return code

mwork           ENDS
;----------------------------------------------------------------
mcprbseg        SEGMENT 'data'
mcprb           db      SIZE uercprb dup (0FFH) ;Allocate space
;                                       for CPRB
mcprbseg        ENDS
dgroup          group   mwork, mcprbseg
;----------------------------------------------------------------
mstack          SEGMENT stack 'stack'
                dw      255 dup (0FFFFH) ;Allocate a stack
mstaktop        dw      0FFFFH           ;First stack entry
mstack          ENDS
;**************************END DEFINITIONS***********************
        SUBTTL  'Main procedure'
        PAGE
;***************************PSEUDOCODE***************************
;                           PROC (MAIN)
;                       1. ESTABLISH A STACK
;                       1. SET DS TO POINT TO WORK AREA
;                       1. GET ADDRESS OF REQUEST PARAMETERS
;                       1. SET PROCESSING OPTION = COMMIT
;                            TRANSACTION
;                       1. SET REQUESTING OPERATOR ID
;                       1. GET ADDRESS OF CPRB INTO ES:DI
;                       1. DO WHILE SERVER RETURN CODE IS NOT LAST
;                            RECORD AND SRPI RETURN CODE IS GOOD
;                       2. INITIALIZE THE CPRB <SEND_REQ_INIT>
;                       2. . MOVE SERVER NAME AND FUNCTION (GET
;                             RECORD) INTO CPRB <SET_REQ_PARMS>
;                       2. . SET CPRB REQUEST PARAMETERS BUFFER
;                             INFORMATION <SET_REQ_BUFFERS>
;                       2. . SET CPRB REPLY DATA BUFFER INFORMATION
;                             <SET_REPLY_PARMS>
;                       2. . SEND THE REQUEST TO THE SERVER
;                             <SEND_REQUEST>
;                       2. . GET THE SRPI RETURN CODE AND SERVER RETURN
;                             CODE <GET_REPLY>
;                       2. . IF THE SRPI RETURN CODE IS GOOD
;                       3. . . IF THE SERVER RETURN CODE IS GOOD
;                       4. . . . IF THE ACCOUNT BALANCE IS POSITIVE
;                       5. . . . . SET CPRB FUNCTION = UPDATE
;                                       ACCOUNTS RECEIVABLE
;                                       <SET_REQ_PARMS>
;                       5. . . . . SET CPRB REQUEST DATA = CUSTOMER
;                                       RECORD <SET_REQ_BUFFERS>
;                       5. . . . . UPDATE THE ACCOUNTS RECEIVABLE
;                                       FILE <SEND_REQUEST>
;                       4. . . . ENDIF
;                       3. . . ENDIF
;                       2. . ENDIF
;                       1. ENDWHILE
;                       1. RETURN TO DOS
;                        ENDPROC (MAIN)
;************************END PSEUDOCODE**************************
msampl          segment 'code'

        assume  cs:msampl
;**************************PROCEDURE*****************************
;****************************************************************
;                       PROC (MAIN)
mentry:
;                       1. ESTABLISH A STACK
        assume  ss:mstack
        mov     ax,seg mstack
        mov     ss,ax
        mov     sp,offset mstaktop

;                       1. SET DS TO POINT TO WORK AREA
        assume  ds:mwork
        mov     ax,seg mwork
        mov     ds,ax
;                       1. GET ADDRESS OF REQUEST PARAMETERS
        assume  es:mwork
        les     di,mqprmbuf@            ;ES:DI  ->  request
;                                        parameters buffer

;                       1. SET PROCESSING OPTION = COMMIT
;                            TRANSACTION
        mov     BYTE PTR es:[di+mqpaflags],mqpacom

;                       1. SET REQUESTING OPERATOR ID
        mov     cx,moper_len$           ;length of operator name
        add     di,OFFSET mqpaoper      ;ES:DI  ->  operator name
;                                        field in req parms buf
        mov     si,OFFSET moper_1$      ;DS:SI  -> operator name
rep     movsb                           ;Move operator name to
;                                        request parms buffer

;                       1. GET ADDRESS OF CPRB INTO ES:DI
        assume  es:mcprbseg
        mov     ax,SEG mcprbseg
        mov     es,ax
        mov     di,OFFSET mcprb         ;ES:DI  ->  CPRB

;                       1. DO WHILE SERVER RETURN CODE IS NOT LAST
;                            RECORD AND SRPI RETURN CODE IS GOOD
loop1:
        cmp     msrvrchigh,mlstrh
        jne     nowhile
        cmp     msrvrclow,mlstrl
        je      nowhile
        cmp     mrclow,uererrokeq
        jne     nowhile
        jmp     while
nowhile:
        jmp     exit
while:
;                       2. . INITIALIZE THE CPRB <SEND_REQ_INIT>
        SEND_REQ_INIT

;                       2. . MOVE SERVER NAME AND FUNCTION (GET
;    +                          RECORD) INTO CPRB <SET_REQ_PARMS>
        SET_REQ_PARMS   mserver,mfunc1

;                       2. . SET CPRB REQUEST PARAMETERS BUFFER
;                              INFORMATION <SET_REQ_BUFFERS>
        SET_REQ_BUFFERS mqprmbuf@,mqprmbufl

;                       2. . SET CPRB REPLY DATA BUFFER INFORMATION
;                              <SET_REPLY_PARMS>
        SET_REPLY_BUFFERS ,,mdabuf@,mdabufl

;                       2. . SEND THE REQUEST TO THE SERVER
;                              <SEND_REQUEST>
        SEND_REQUEST

;                       2. . GET THE SRPI RETURN CODE AND SERVER RETURN
;                              CODE <GET_REPLY>
        GET_REPLY  mretcode,mservrc

;                       2. . IF THE SRPI RETURN CODE IS GOOD
        cmp     mrchigh,uererrokeq
        je      goodrc1                 ;exit label is >127
        jmp     endit                   ; bytes away
goodrc1:

;                       3. . . IF THE SERVER RETURN CODE IS GOOD
        cmp     msrvrchigh,mrcok        ;Compare high word of
;                                        server return code
        je      goodrc2                 ;exit label is >127
        jmp     endit                   ;bytes away
goodrc2:
        cmp     msrvrclow,mrcok         ;Compare low word of
;                                        server return code
        jne     endit
;                       4. . . . IF THE ACCOUNT BALANCE IS POSITIVE
        mov     si,WORD PTR mdabuf@     ;get offset of data buf,
;                                        DS:SI -> data buffer
        mov     ax,WORD PTR [si+mcusbal];Get low word of
;                                        balance
        mov     dx,WORD PTR [si+mcusbal+2];Get high word
;                                       of balance
        sub     dx,0                    ;Subtract zero from the
;                                        high word
        jl      endit                   ;Negative balance,quit
        jg      update                  ;Positive balance, update
;                                        the AR file
        cmp     ax,0                    ;Is low word zero?
        je      endit                   ;Yes-zero balance, quit

;                       5. . . . . SET CPRB FUNCTION = UPDATE
;                                       ACCOUNTS RECEIVABLE
;                                       <SET_REQ_PARMS>
update:
        SET_REQ_PARMS ,mfunc2

;                       5. . . . . SET CPRB REQUEST DATA = CUSTOMER
;                                       RECORD <SET_REQ_BUFFERS>
        SET_REQ_BUFFERS ,,mdabuf@,mdabufl

;                       5. . . . . UPDATE THE ACCOUNTS RECEIVABLE
;                                       FILE <SEND_REQUEST>
        SEND_REQUEST
;                       4. . . . ENDIF
;                       3. . . ENDIF
;                       2. . ENDIF
endit:  jmp loop1
;                       1. ENDWHILE
exit:
;                       1. RETURN TO DOS
        sub     ax,ax                   ;Return to DOS
        push    ax
        push    ax
        Call    DosExit
;                       ENDPROC (MAIN)
;***********************END PROCEDURE****************************
msampl          ENDS
        END     mentry
