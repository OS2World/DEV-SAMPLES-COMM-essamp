      *****************************************************************
      *                                                               *
      *  MODULE NAME  : LUASAMP.COB                                   *
      *                                                               *
      *  DESCRIPTIVE  : LUA COBOL SAMPLE PROGRAM FOR IBM EXTENDED     *
      *  NAME           SERVICES FOR OS/2                             *
      *                                                               *
      *  COPYRIGHT    : (C) COPYRIGHT IBM CORP. 1989, 1990, 1991      *
      *                 LICENSED MATERIAL - PROGRAM PROPERTY OF IBM   *
      *                 ALL RIGHTS RESERVED                           *
      *                                                               *
      *  FUNCTION     : This program issues                           *
      *                 - an SLI_OPEN to establish an LU_LU session.  *
      *                 - an SLI_SEND to transmit data to the host.   *
      *                 - an SLI_RECEIVE to get data from the host.   *
      *                 - an SLI_SEND to transmit a response.         *
      *                 - an SLI_CLOSE to end the LU_LU session.      *
      *                                                               *
      *  GENERAL SERVICE                                              *
      *    VERBS USED : CONVERT - Translates data between ASCII       *
      *                           and EBCDIC.                         *
      *  MODULE TYPE  : COBOL                                         *
      *                 (Compiles with large memory model)            *
      *                                                               *
      *****************************************************************

       IDENTIFICATION DIVISION.
       PROGRAM-ID. LUASAMP.

       ENVIRONMENT DIVISION.

       DATA DIVISION.
       WORKING-STORAGE SECTION.
       77  OPEN-DISPLAY           PIC X(46)    VALUE
                " Opening communication with SLI interface.... ".
       77  SEND-DATA-DISPLAY      PIC X(53)    VALUE
                " SLI interface opened and Init_self sent to the host ".
       77  RECEIVE-DISPLAY        PIC X(53)    VALUE
                " Test data sent to host. Waiting for host data ".
       77  SEND-RESPONSE-DISPLAY  PIC X(41)    VALUE
                " Host data received. Responding to host ".
       77  CLOSE-DISPLAY          PIC X(43)    VALUE
                " Response sent. Preparing to close session ".
       77  CLOSE0-DISPLAY         PIC X(37)    VALUE
                " SLI interface closed with no errors ".
       77  CLOSE1-DISPLAY         PIC X(53)    VALUE
                " Quit from LUA conversation due to an error. Ab-ended".
       77  SESSION-FAIL-DISPLAY   PIC X(38)    VALUE
                " LU-LU session failed due to an error ".

       77  SEM-TIMEOUT            PIC X(4)     VALUE  X'FFFFFFFF'.

       78  LUA-SIZE               VALUE    64.
       78  LUA-AND-EXT-SIZE       VALUE    98.
       78  SEND-SIZE              VALUE    66.
       78  SEND-AND-EXT-SIZE      VALUE    100.
       78  OPEN-SIZE              VALUE    68.
       78  OPEN-AND-EXT-SIZE      VALUE    102.
       78  CONVERT-LEN            VALUE    8.
       78  INITSELF-RU-LEN        VALUE    25.
       78  TEST-DATA-LEN          VALUE    25.
       78  DATA-BUFFER-LEN        VALUE    256.
       78  BIN-ZERO               VALUE    X'00'.
       78  BIN-ONE                VALUE    X'01'.

       01  INITSELF-RU.
           05  ISELF-RQ-01-HDR.
               10  HDR-1        PIC  X     VALUE   X'01'.
               10  HDR-2        PIC  X     VALUE   X'06'.
               10  HDR-3        PIC  X     VALUE   X'81'.
           05  ISELF-RQ-01-F0   PIC  X     VALUE   X'00'.
           05  ISELF-RQ-01-MODE PIC  X(8)  VALUE   "LUA768RU".
           05  ISELF-RQ-01-TY   PIC  X     VALUE   X'F3'.
           05  ISELF-RQ-01-N-1  PIC  X     VALUE   X'08'.
           05  ISELF-RQ-01-PLU  PIC  X(8)  VALUE   "VTAMPGM ".
           05  ISELF-RQ-01-RID  PIC  X     VALUE   X'00'.
           05  ISELF-RQ-01-PW   PIC  X     VALUE   X'00'.
           05  ISELF-RQ-01-UD   PIC  X     VALUE   X'00'.


       01  TEST-DATA     PIC  X(25)  VALUE "TEST#SENDING#DATA#TO#HOST".
       01  DATA-BUFFER          PIC  X(256).

       01  USER-RAM-SEM-ADDR    POINTER.
       01  USER-RAM-SEM         PIC  X(4).
       01  LU-SESSION-ID        PIC  X(4).
       01  SAVED-SEQ-NUM        PIC  X(2).

      *******************************************************
      *    Include files from OS/2 Comms Mgr                *
      *******************************************************
           COPY ACSSVCB.CBL.
      *  LUA Constants
           COPY LUA_C_C.CBL.
      *  LUA verb record
       01  LUA-VERB-RECORD.
           COPY LUA_C.CBL.
      *  OPEN specific area
           03 LUA-OPEN.
           COPY LUA_C_O.CBL.
           COPY LUA_C_E.CBL.
      *  SEND specific area
           03 LUA-SEND   REDEFINES  LUA-OPEN.
           COPY LUA_C_S.CBL.
           COPY LUA_C_E.CBL.
      *  RECEIVE, CLOSE specific area
           03 LUA-COB-EXT  REDEFINES  LUA-SEND.
           COPY LUA_C_E.CBL.


       PROCEDURE DIVISION.
       MAIN-PARAGRAPH.
      ************************************************************
      *    Main function      LUA Sample conversation code       *
      ************************************************************
      *
           SET USER-RAM-SEM-ADDR TO ADDRESS OF USER-RAM-SEM.
           DISPLAY OPEN-DISPLAY.
           PERFORM SLI-OPEN.
           IF LUA-PRIM-RC NOT EQUAL LUA-OK OR
              PRIMARY-RC IN CONVERT NOT EQUAL SV-OK THEN
              DISPLAY SESSION-FAIL-DISPLAY
              STOP RUN.


           DISPLAY SEND-DATA-DISPLAY
           PERFORM SLI-SEND-DATA
      *
           IF LUA-PRIM-RC EQUAL LUA-OK AND
              PRIMARY-RC IN CONVERT EQUAL SV-OK THEN
              DISPLAY RECEIVE-DISPLAY
              PERFORM SLI-RECEIVE
      *
              IF LUA-PRIM-RC EQUAL LUA-OK AND
                 PRIMARY-RC IN CONVERT EQUAL SV-OK THEN
                 DISPLAY SEND-RESPONSE-DISPLAY
                 PERFORM SLI-SEND-RESPONSE
      *
                 IF LUA-PRIM-RC EQUAL LUA-OK THEN
                    DISPLAY CLOSE-DISPLAY
                    MOVE LOW-VALUES TO LUA-VERB-RECORD
                    MOVE BIN-ZERO TO LUA-FLAG1-CLOSE-ABEND
                                     IN LUA-COB-EXT
                    PERFORM SLI-CLOSE.


           IF LUA-PRIM-RC EQUAL LUA-OK THEN
              DISPLAY CLOSE0-DISPLAY
           ELSE
              IF LUA-PRIM-RC NOT EQUAL LUA-SESSION-FAILURE THEN
                 MOVE LOW-VALUES TO LUA-VERB-RECORD
                 MOVE BIN-ONE TO LUA-FLAG1-CLOSE-ABEND IN LUA-COB-EXT
                 PERFORM SLI-CLOSE
                 DISPLAY CLOSE1-DISPLAY
              ELSE
                 DISPLAY SESSION-FAIL-DISPLAY.

           STOP RUN.

      ****************************************************************
      * Function : SLI-OPEN                                          *
      * Purpose  : Open a Session with the host using the SLI_OPEN   *
      *            verb and an INITSELF command.                     *
      * Actions  : Set the required fields for SLI_OPEN and convert  *
      *            the ACSII fields in the INITSELF to EBCDIC.       *
      *            If there are no conversion errors, call the       *
      *            SLI_API and wait for the SLI_OPEN to complete.    *
      *            Save the session ID for use in issuing other      *
      *            verbs for this session.                           *
      ****************************************************************
       SLI-OPEN.
           MOVE LOW-VALUES           TO CONVERT.
           MOVE LOW-VALUES           TO LUA-VERB-RECORD.
           MOVE LUA-OPCODE-SLI-OPEN  TO LUA-OPCODE.
           MOVE LUA-VERB-SLI         TO LUA-VERB.
           MOVE OPEN-AND-EXT-SIZE    TO LUA-VERB-LENGTH.
           MOVE OPEN-SIZE            TO LUA-COBOL-OFFSET.
           MOVE "LUA1    "           TO LUA-LUNAME.
           MOVE LUA-INIT-TYPE-SEC-IS TO LUA-INIT-TYPE IN LUA-OPEN.
           MOVE INITSELF-RU-LEN      TO LUA-DATA-LENGTH.
           SET LUA-DATA-PTR          TO ADDRESS OF INITSELF-RU.
           SET LUA-POST-HANDLE       TO ADDRESS OF USER-RAM-SEM.

           MOVE SV-ASCII-TO-EBCDIC  TO DIRECTION.
           MOVE CONVERT-LEN         TO LENGTH-TO-BE-CONVERTED.
           SET  SOURCE-ADDR         TO ADDRESS OF ISELF-RQ-01-PLU.
           SET  TARGET-ADDR         TO ADDRESS OF ISELF-RQ-01-PLU.

           PERFORM CONVERT-ROUTINE.
           IF PRIMARY-RC IN CONVERT EQUAL SV-OK THEN
              MOVE SV-ASCII-TO-EBCDIC  TO DIRECTION
              MOVE CONVERT-LEN         TO LENGTH-TO-BE-CONVERTED
              SET  SOURCE-ADDR         TO ADDRESS OF ISELF-RQ-01-MODE
              SET  TARGET-ADDR         TO ADDRESS OF ISELF-RQ-01-MODE

              PERFORM CONVERT-ROUTINE
              IF PRIMARY-RC IN CONVERT EQUAL SV-OK THEN

                 CALL '__SLI' USING LUA-VERB-RECORD
                 IF LUA-PRIM-RC EQUAL LUA-IN-PROGRESS THEN
                    CALL '__DOSSEMWAIT' USING
                                        BY VALUE SEM-TIMEOUT
                                        USER-RAM-SEM-ADDR
                 IF LUA-PRIM-RC NOT EQUAL LUA-OK THEN
                    PERFORM ERROR2
                 ELSE
                    MOVE LUA-SID TO LU-SESSION-ID.


      ****************************************************************
      * Function : SLI-SEND-DATA                                     *
      * Purpose  : Send data to the host on LU Normal Flow.          *
      * Actions  : Set the required fields for SLI_SEND and convert  *
      *            the ACSII test data to EBCDIC.                    *
      *            If there are no conversion errors, call the       *
      *            SLI_API and wait for the SLI_SEND to complete.    *
      ****************************************************************
       SLI-SEND-DATA.
           MOVE LOW-VALUES          TO LUA-VERB-RECORD.
           MOVE LUA-OPCODE-SLI-SEND TO LUA-OPCODE.
           MOVE LUA-VERB-SLI        TO LUA-VERB.
           MOVE SEND-AND-EXT-SIZE   TO LUA-VERB-LENGTH.
           MOVE SEND-SIZE           TO LUA-COBOL-OFFSET.
           MOVE LU-SESSION-ID       TO LUA-SID.
           MOVE TEST-DATA-LEN       TO LUA-DATA-LENGTH.
           SET LUA-DATA-PTR         TO ADDRESS OF TEST-DATA.
           SET LUA-POST-HANDLE      TO ADDRESS OF USER-RAM-SEM.
           MOVE  BIN-ONE            TO LUA-RH-RI IN LUA-SEND.
           MOVE  BIN-ONE            TO LUA-RH-DR1I IN LUA-SEND.
           MOVE  BIN-ONE            TO LUA-RH-BBI IN LUA-SEND.
           MOVE  BIN-ONE            TO LUA-RH-CDI IN LUA-SEND.
           MOVE LUA-MESSAGE-TYPE-LU-DATA TO LUA-MESSAGE-TYPE.

           MOVE SV-ASCII-TO-EBCDIC  TO DIRECTION.
           MOVE LUA-DATA-LENGTH     TO LENGTH-TO-BE-CONVERTED.
           SET  SOURCE-ADDR         TO LUA-DATA-PTR.
           SET  TARGET-ADDR         TO LUA-DATA-PTR.

           PERFORM CONVERT-ROUTINE.
           IF PRIMARY-RC IN CONVERT EQUAL SV-OK THEN

              CALL '__SLI' USING LUA-VERB-RECORD
              IF LUA-PRIM-RC EQUAL LUA-IN-PROGRESS THEN
                 CALL '__DOSSEMWAIT' USING
                                     BY VALUE SEM-TIMEOUT
                                     USER-RAM-SEM-ADDR
              IF LUA-PRIM-RC NOT EQUAL LUA-OK THEN
                 PERFORM ERROR2.


      ****************************************************************
      * Function : SLI-RECEIVE                                       *
      * Purpose  : Receive a message from the host on LU normal flow.*
      * Actions  : Set the required fields for SLI_RECEIVE, call the *
      *            SLI_API and wait for the SLI_RECEIVE to complete. *
      *            If there is no error from the SLI, convert the    *
      *            data from EBCDIC to ASCII.                        *
      ****************************************************************
       SLI-RECEIVE.
           MOVE LOW-VALUES             TO LUA-VERB-RECORD.
           MOVE LUA-OPCODE-SLI-RECEIVE TO LUA-OPCODE.
           MOVE LUA-VERB-SLI           TO LUA-VERB.
           MOVE LUA-AND-EXT-SIZE       TO LUA-VERB-LENGTH.
           MOVE LUA-SIZE               TO LUA-COBOL-OFFSET.
           MOVE LU-SESSION-ID          TO LUA-SID.
           MOVE DATA-BUFFER-LEN        TO LUA-MAX-LENGTH.
           SET LUA-DATA-PTR            TO ADDRESS OF DATA-BUFFER.
           SET LUA-POST-HANDLE         TO ADDRESS OF USER-RAM-SEM.
           MOVE BIN-ONE            TO LUA-FLAG1-LU-NORM IN LUA-COB-EXT.

           CALL '__SLI' USING LUA-VERB-RECORD.
           IF LUA-PRIM-RC EQUAL LUA-IN-PROGRESS THEN
              CALL '__DOSSEMWAIT' USING
                                  BY VALUE SEM-TIMEOUT
                                  USER-RAM-SEM-ADDR.
           IF LUA-PRIM-RC NOT EQUAL LUA-OK THEN
              PERFORM ERROR2
           ELSE
              MOVE LUA-TH-SNF          TO   SAVED-SEQ-NUM
              MOVE SV-EBCDIC-TO-ASCII  TO   DIRECTION
              MOVE LUA-DATA-LENGTH     TO   LENGTH-TO-BE-CONVERTED
              SET  SOURCE-ADDR         TO   LUA-DATA-PTR
              SET  TARGET-ADDR         TO   LUA-DATA-PTR

              PERFORM CONVERT-ROUTINE.


      ****************************************************************
      * Function : SLI-SEND-RESPONSE                                 *
      * Purpose  : Send a response to LU Normal data to the host.    *
      * Actions  : Set the required fields for SLI_SEND to send a    *
      *            positive response.  Call the SLI_API and wait for *
      *            the SLI_SEND to complete.                         *
      ****************************************************************
       SLI-SEND-RESPONSE.
           MOVE LOW-VALUES            TO LUA-VERB-RECORD.
           MOVE LUA-OPCODE-SLI-SEND   TO LUA-OPCODE.
           MOVE LUA-VERB-SLI          TO LUA-VERB.
           MOVE SEND-AND-EXT-SIZE     TO LUA-VERB-LENGTH.
           MOVE SEND-SIZE             TO LUA-COBOL-OFFSET.
           MOVE LU-SESSION-ID         TO LUA-SID.
           SET LUA-POST-HANDLE        TO ADDRESS OF USER-RAM-SEM.
           MOVE SAVED-SEQ-NUM         TO LUA-TH-SNF.
           MOVE BIN-ONE               TO LUA-RH-DR1I IN LUA-SEND.
           MOVE BIN-ONE               TO LUA-FLAG1-LU-NORM IN LUA-SEND.
           MOVE LUA-MESSAGE-TYPE-RSP  TO LUA-MESSAGE-TYPE.

           CALL '__SLI' USING LUA-VERB-RECORD.
           IF LUA-PRIM-RC EQUAL LUA-IN-PROGRESS THEN
              CALL '__DOSSEMWAIT' USING
                                  BY VALUE SEM-TIMEOUT
                                  USER-RAM-SEM-ADDR.
           IF LUA-PRIM-RC NOT EQUAL LUA-OK THEN
              PERFORM ERROR2.


      ****************************************************************
      * Function : SLI-CLOSE                                         *
      * Purpose  : Issue an SLI-CLOSE to end the session.            *
      * Actions  : Set the required fields for SLI_CLOSE.  Call the  *
      *            SLI_API and wait for the SLI_CLOSE to complete.   *
      ****************************************************************
       SLI-CLOSE.
           MOVE LOW-VALUES           TO LUA-VERB-RECORD.
           MOVE LUA-OPCODE-SLI-CLOSE TO LUA-OPCODE.
           MOVE LUA-VERB-SLI         TO LUA-VERB.
           MOVE LUA-AND-EXT-SIZE     TO LUA-VERB-LENGTH.
           MOVE LUA-SIZE             TO LUA-COBOL-OFFSET.
           MOVE LU-SESSION-ID        TO LUA-SID.
           SET LUA-POST-HANDLE       TO ADDRESS OF USER-RAM-SEM.

           CALL '__SLI' USING LUA-VERB-RECORD.
           IF LUA-PRIM-RC EQUAL LUA-IN-PROGRESS THEN
              CALL '__DOSSEMWAIT' USING
                                  BY VALUE SEM-TIMEOUT
                                  USER-RAM-SEM-ADDR.
           IF LUA-PRIM-RC NOT EQUAL LUA-OK THEN
              PERFORM ERROR2.


      *****************************************************************
      * Function : CONVERT-ROUTINE.                                   *
      * Purpose  : Convert selected data from ASCII to EBCDIC         *
      *            or from EBCDIC to ASCII.                           *
      *****************************************************************
       CONVERT-ROUTINE.
           MOVE SV-CONVERT          TO OPCODE IN CONVERT.
           MOVE SV-AE               TO CHARACTER-SET.
           CALL "__ACSSVC" USING CONVERT.
           IF PRIMARY-RC IN CONVERT NOT EQUAL SV-OK THEN
              PERFORM ERROR1.


      *****************************************************************
      * Procedure : ERROR1                                            *
      * Purpose   : Display return codes for conversion errors.       *
      *****************************************************************
       ERROR1.
           DISPLAY " An error has occurred during conversion process".
           DISPLAY " The primary return code is: ",
                     PRIMARY-RC IN CONVERT.
           DISPLAY " The secondary return code is: ",
                     SECONDARY-RC IN CONVERT.


      ****************************************************************
      * Procedure : ERROR2                                           *
      * Purpose   : Display return codes for unsuccessful SLI verbs. *
      ****************************************************************
       ERROR2.
           DISPLAY " An error occurred with SLI interface. Verb op: ",
                     LUA-OPCODE.
           DISPLAY " The primary return code is: ", LUA-PRIM-RC.
           DISPLAY " The secondary return code is: ", LUA-SEC-RC.
