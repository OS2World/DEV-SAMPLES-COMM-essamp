      ******************************************************************
      *                                                                *
      * FILE NAME: HSMPLCBL.CBL                                        *
      *                                                                *
      * MODULE NAME= HSMPLCBL.CBL                                      *
      *                                                                *
      * MODULE TYPE= MICROFOCUS COBOL                                  *
      *                                                                *
      * DESCRIPTIVE NAME= COBOL SAMPLE PROGRAM FOR EHLLAPI             *
      *                                                                *
      *                                                                *
      * Displays EHLLAPI and session information.                      *
      * Writes string to host.                                         *
      * Searches for written string on host.                           *
      * Displays host session screen.                                  *
      * Manipulates the Presentation Manager properties of             *
      * the emulator session to: change window title name, switch      *
      * list name, make window invisible, query window status,         *
      * window coordinates, change window size, and restore the        *
      * emulator session window to its original conditions.            *
      *                                                                *
      *                                                                *
      *   COPYRIGHT:  XXXXXXXXX  (C) COPYRIGHT IBM CORP. 1987,1988,    *
      *               1989, 1991 LICENSED MATERIAL - PROGRAM PROPERTY  *
      *               OF IBM ALL RIGHTS RESERVED                       *
      *                                                                *
      * NOTES=                                                         *
      *                                                                *
      **********************-END OF SPECIFICATIONS-*********************
       IDENTIFICATION DIVISION.
       PROGRAM-ID. EHLLAPI-SAMPLE-PROGRAM.
       ENVIRONMENT DIVISION.
       DATA DIVISION.
       WORKING-STORAGE SECTION.

      ******************************************************************
      ************************ CONSTANTS *******************************
      ******************************************************************

       78  MAX-DATA-SIZE            VALUE 3840.
      *                                      The maximum data
      *                                      size for this
      *                                      application.

       78  EABS                     VALUE 128.
      *                                      Extended attribute bit

       78  PSS                      VALUE 64.
      *                                      Programmed Symbol bit

       77  ZRO                      PIC 9(4) COMP-5 VALUE 0.

       77  MDS                      PIC 9(4) COMP-5 VALUE 3840.

       77  DUMMY                    PIC X(1).

       77  PRESS-ENT-MSG            PIC X(26) VALUE
           "Press ENTER to continue...".

       77  PRESS-CTRL-ESC           PIC X(26) VALUE
           "(press CTRL-ESC to verify)".

       77  DFT-SESS                 PIC X VALUE SPACE.

       77  HOST-TEXT                PIC X(9) VALUE
           "EHLLAPI-1".

       77  INVIS-HOST-TEXT          PIC X(9) VALUE
           "EHLLAPI-2".

       77  DISP-TEXT                PIC X(9) .

       77  COMMAND-TEXT             PIC X(29) VALUE
           "IND$FILE PUT SF-TEST EXEC A@E".

       77  HOME-KEY                 PIC X(4) VALUE
           "@L@0".

       77  SETPARM-TEXT             PIC X(17) VALUE
           "NOATTRB EAB XLATE".

       77  DISP-NUM                 PIC ZZZZ9.

       77  LOOP-COUNT               PIC 99 COMP-0.

       77  NUM-SESS                 PIC 99 COMP-0.

       77  BIN-NUM                  PIC 99 COMP-0.

       77  BIN-NUM2                 PIC 99 COMP-0.

       77  BUFF-LENGTH              PIC X(2).

       01  HEX-TABLE.
           05 FILLER PIC X(16) VALUE "0123456789ABCDEF".

       01  HEX-DIGITS REDEFINES HEX-TABLE.
           05  HEX-DIG PIC X OCCURS 16 TIMES INDEXED BY INDX.

       77  HEX-OUTPUT               PIC X(2).

       01  HEX-OUTR REDEFINES HEX-OUTPUT.
           05  HEX-OUT PIC X OCCURS 2 TIMES INDEXED BY IND.

       77  BLANK-LINE               PIC X VALUE SPACE.

       01  BINARY-NUM.
           05 B-NUM PIC 9 COMP OCCURS 4 TIMES.

       01  H-PRINT.
           05 H-OUT PIC X OCCURS 8 TIMES.

       01  BINARY-NUM-X.
           05 B-NUM-X PIC 9 OCCURS 2 TIMES.

       01  H-PRINT-X.
           05 H-OUT-X PIC X OCCURS 4 TIMES.

       01 WINDOW-DATA.
            10  WN-DATA1           PIC 9(1) USAGE COMP.
            10  WN-DATA2           PIC 9(1) USAGE COMP.


     ****** EHLLAPI variables for use with LIM ********

       77  HFUNC-NUM               PIC 99 COMP-0.
      *                                      EHLLAPI function number.
       01  HDATA-STRING.
           05  HDATA-STR           PIC X(1) OCCURS 3840 TIMES.
      *                                      EHLLAPI data string
       77  HDS-LEN                 PIC 99 COMP-0.
      *                                      EHLLAPI data string length
       77  HRC                     PIC 99 COMP-0 VALUE ZERO.
      *                                      EHLLAPI return code




      ****************************************************************
      ***************** BEGIN INCLUDE FILES **************************
      ****************************************************************

       COPY "HAPI_CBL.INC".

       SCREEN SECTION.
       01  BLANK-SCR.
          05  BLANK SCREEN.

       PROCEDURE DIVISION.




      *********************************************************************
      * MAIN - Main code calls routines to do real work.                  *
      *                                                                   *
      *                                                                   *
      *                                                                   *
      *                                                                   *
      *********************************************************************


       MAIN.

         DISPLAY BLANK-SCR.

         PERFORM  DISP-EHLLAPI-INFO.

         IF HRC = ZERO THEN

           DISPLAY PRESS-ENT-MSG WITH NO ADVANCING
           ACCEPT DUMMY
           PERFORM  DISP-SESSION-INFO.

         IF HRC = ZERO THEN
           PERFORM M-NEXT.

         STOP RUN.

       M-NEXT.

         IF DFT-SESS NOT = SPACE THEN

           DISPLAY BLANK-LINE
           MOVE HOST-TEXT TO DISP-TEXT
           DISPLAY 'Press ENTER to send string "' DISP-TEXT
               WITH NO ADVANCING
           DISPLAY '" to session short name ' DFT-SESS
               WITH NO ADVANCING
           DISPLAY '...'
               WITH NO ADVANCING
           ACCEPT DUMMY
           PERFORM WRITE-STR-2-HOST

         ELSE

           DISPLAY 'NO DFT SESSION SESSION STARTED.'
           MOVE 1 TO HRC.

         IF HRC = ZERO THEN

           DISPLAY BLANK-LINE
           DISPLAY 'Press ENTER to search for string "' DISP-TEXT
                 WITH NO ADVANCING
           DISPLAY '" on Host Presentation Space...'
                 WITH NO ADVANCING
           ACCEPT DUMMY
           PERFORM SEARCH-STR-ON-HOST.

         IF HRC = ZERO THEN

           DISPLAY 'Press ENTER to display first 1920 '
                 WITH NO ADVANCING
           DISPLAY 'bytes of Host presentation space...'
                 WITH NO ADVANCING
           ACCEPT DUMMY
           PERFORM DISP-HOST-SCR.

         IF HRC = ZERO THEN

           DISPLAY 'Press ENTER to change window title name'
                 WITH NO ADVANCING
           DISPLAY ' of session short name ' DFT-SESS
                 WITH NO ADVANCING

           ACCEPT DUMMY
           PERFORM CHANGE-PS-WINDOW-NAME.

         IF HRC = ZERO THEN

           DISPLAY BLANK-LINE
           DISPLAY 'Press ENTER to change the switch list LT name'
                 WITH NO ADVANCING
           DISPLAY ' for session short name ' DFT-SESS
           ACCEPT DUMMY
           PERFORM CHANGE-SWITCH-LIST-LT-NAME.

         IF HRC = ZERO THEN

           DISPLAY BLANK-LINE
           DISPLAY BLANK-LINE
           DISPLAY BLANK-LINE
           DISPLAY 'Press ENTER to query the PM status'
                 WITH NO ADVANCING
           DISPLAY ' of session short name ' DFT-SESS
           ACCEPT DUMMY
           PERFORM QUERY-PM-STATUS.

         IF HRC = ZERO THEN

           DISPLAY BLANK-LINE
           DISPLAY 'Press ENTER to make the PM window invisible'
                 WITH NO ADVANCING
           DISPLAY ' for session short name ' DFT-SESS
           ACCEPT DUMMY
           PERFORM MAKE-PM-WINDOW-INVISIBLE

           DISPLAY BLANK-LINE
           MOVE INVIS-HOST-TEXT TO DISP-TEXT
           DISPLAY 'Press ENTER to send string "' DISP-TEXT
               WITH NO ADVANCING
           DISPLAY '" to session short name ' DFT-SESS
               WITH NO ADVANCING
           DISPLAY '...'
               WITH NO ADVANCING
           ACCEPT DUMMY
           PERFORM WRITE-STR-2-HOST.

         IF HRC = ZERO THEN

           DISPLAY BLANK-LINE
           DISPLAY 'Press ENTER to display first 1920 '
                 WITH NO ADVANCING
           DISPLAY 'bytes of invisible Host presentation space...'
                 WITH NO ADVANCING
           ACCEPT DUMMY
           PERFORM DISP-HOST-SCR.

         IF HRC = ZERO THEN

           DISPLAY 'Press ENTER to maximize the PM window'
                 WITH NO ADVANCING
           DISPLAY ' and make visible...'
                 WITH NO ADVANCING
           ACCEPT DUMMY
           PERFORM MAKE-PM-WINDOW-VISIBLE.

         IF HRC = ZERO THEN

           DISPLAY BLANK-LINE
           DISPLAY 'Press ENTER to disconnect from'
                 WITH NO ADVANCING
           DISPLAY ' from session short name ' DFT-SESS
                 WITH NO ADVANCING
           DISPLAY '...'
           ACCEPT DUMMY
           PERFORM DISCONNECT-PM-WINDOW.

         IF HRC = ZERO THEN

           DISPLAY BLANK-LINE
           DISPLAY 'Press ENTER to restore switch name and window'
                 WITH NO ADVANCING
           DISPLAY ' name and size...'
           ACCEPT DUMMY
           PERFORM RESET-WINDOW.


         IF HRC = ZERO THEN
           DISPLAY BLANK-LINE
           DISPLAY 'SAMPLE PROGRAM DONE.  To Exit Program '
                 WITH NO ADVANCING
           DISPLAY 'Press ENTER...'
                 WITH NO ADVANCING
           ACCEPT DUMMY.

      *********************************************************************
      * DISP-EHLLAPI-INFO - CALLs EHLLAPI QUERY-SYSTEM and then displays  *
      *                     the requested info.                           *
      *                                                                   *
      *                                                                   *
      *                                                                   *
      *********************************************************************
       DISP-EHLLAPI-INFO.

         MOVE HA-QUERY-SYSTEM TO HFUNC-NUM.

         CALL 'COBLIM' USING HFUNC-NUM, QSYS-STRUCT, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS
           PERFORM DEI-DISP
         ELSE
           PERFORM ERROR-HAND.

       DEI-DISP.
         DISPLAY '                       EHLLAPI INFORMATION'.

         DISPLAY BLANK-LINE.

         DISPLAY '  EHLLAPI version              : '
             QSYS-HLLAPI-VER.

         DISPLAY '  EHLLAPI level                : '
             QSYS-HLLAPI-LVL.

         DISPLAY '  EHLLAPI release date         : '
             QSYS-HLLAPI-DATE.

         DISPLAY '  EHLLAPI LIM version          : '
             QSYS-LIM-VER.

         DISPLAY '  EHLLAPI LIM level            : '
             QSYS-LIM-LVL.

         DISPLAY '  EHLLAPI hardware base        : '
             QSYS-HARDWARE-BASE ' = '
             WITH NO ADVANCING.
         IF QSYS-HARDWARE-BASE = 'Z'
           DISPLAY '(See System model/submodel below)'
               WITH NO ADVANCING.
         DISPLAY BLANK-LINE.

         DISPLAY '  EHLLAPI CTRL program type    : '
             QSYS-CTRL-PROG-TYPE ' = '
             WITH NO ADVANCING.
         IF QSYS-CTRL-PROG-TYPE = 'X'
           DISPLAY 'OS/2' WITH NO ADVANCING.
         DISPLAY BLANK-LINE.

         DISPLAY '  EHLLAPI sequence number      : '
             QSYS-SEQ-NUM.

         DISPLAY '  EHLLAPI CTRL program version : '
             QSYS-CTRL-PROG-VER.

         DISPLAY '  EHLLAPI PC session name      : '
             QSYS-PC-SNAME.

         DISPLAY '  EHLLAPI extended error 1     : '
             QSYS-ERR1.

         DISPLAY '  EHLLAPI extended error 2     : '
             QSYS-ERR2.

         DISPLAY '  EHLLAPI system model/submodel: '
             WITH NO ADVANCING.
         MOVE QSYS-SYS-MODEL TO BIN-NUM.
         PERFORM ITOH.
         DISPLAY HEX-OUT(1) WITH NO ADVANCING.
         DISPLAY HEX-OUT(2) WITH NO ADVANCING.
         MOVE QSYS-SYS-SUBMODEL TO BIN-NUM.
         PERFORM ITOH.
         DISPLAY HEX-OUT(1) WITH NO ADVANCING.
         DISPLAY HEX-OUT(2) WITH NO ADVANCING.
         DISPLAY ' HEX  ' WITH NO ADVANCING.
         IF QSYS-SYS-MODEL = H'FC' AND QSYS-SYS-SUBMODEL = H'00'
           DISPLAY '= Model PC AT' WITH NO ADVANCING.
         IF QSYS-SYS-MODEL = H'FC' AND QSYS-SYS-SUBMODEL = H'01'
           DISPLAY '= Model PC AT ENHANCED' WITH NO ADVANCING.
         IF QSYS-SYS-MODEL = H'FC' AND QSYS-SYS-SUBMODEL = H'02'
           DISPLAY '= Model PC XT Model 286' WITH NO ADVANCING.
         IF QSYS-SYS-MODEL = H'FC' AND QSYS-SYS-SUBMODEL = H'04'
           DISPLAY '= Model 50' WITH NO ADVANCING.
         IF QSYS-SYS-MODEL = H'FC' AND QSYS-SYS-SUBMODEL = H'05'
           DISPLAY '= Model 60' WITH NO ADVANCING.
         IF QSYS-SYS-MODEL = H'F8' AND QSYS-SYS-SUBMODEL = H'00'
           DISPLAY '= Model 80' WITH NO ADVANCING.
         IF QSYS-SYS-MODEL = H'F8' AND QSYS-SYS-SUBMODEL = H'09'
           DISPLAY '= Model 70' WITH NO ADVANCING.
         DISPLAY BLANK-LINE.


         MOVE QSYS-PC-NLS TO DISP-NUM.
         DISPLAY '  EHLLAPI National Language    : '
             DISP-NUM.


         DISPLAY '  EHLLAPI monitor type         : '
             QSYS-MONITOR-TYPE ' = '
             WITH NO ADVANCING.
         IF QSYS-MONITOR-TYPE = 'M'
           DISPLAY 'PC MONOCHROME' WITH NO ADVANCING.
         IF QSYS-MONITOR-TYPE = 'C'
           DISPLAY 'PC CGA' WITH NO ADVANCING.
         IF QSYS-MONITOR-TYPE = 'E'
           DISPLAY 'PC EGA' WITH NO ADVANCING.
         IF QSYS-MONITOR-TYPE = 'A'
           DISPLAY 'PS MONOCHROME' WITH NO ADVANCING.
         IF QSYS-MONITOR-TYPE = 'V'
           DISPLAY 'PS 8512' WITH NO ADVANCING.
         IF QSYS-MONITOR-TYPE = 'H'
           DISPLAY 'PS 8514' WITH NO ADVANCING.
         IF QSYS-MONITOR-TYPE = 'U'
           DISPLAY 'UNKNOWN monitor type' WITH NO ADVANCING.
         DISPLAY BLANK-LINE.





      *********************************************************************
      * DISP-SESSION-INFO - CALLs EHLLAPI QUERY funtions and then displays*
      *                     the requested session info.                   *
      *                                                                   *
      *                                                                   *
      *                                                                   *
      *********************************************************************
       DISP-SESSION-INFO.


         DISPLAY BLANK-LINE.
         DISPLAY BLANK-LINE.
         DISPLAY BLANK-LINE.
         DISPLAY BLANK-LINE.
         DISPLAY BLANK-LINE.
         DISPLAY BLANK-LINE.
         DISPLAY BLANK-LINE.
         DISPLAY BLANK-LINE.
         DISPLAY BLANK-LINE.
         DISPLAY BLANK-LINE.
         DISPLAY BLANK-LINE.
         DISPLAY BLANK-LINE.
         DISPLAY '                           SESSION INFO'.
         DISPLAY BLANK-LINE.

         MOVE HA-QUERY-SESSIONS TO HFUNC-NUM.

         COMPUTE HDS-LEN = 12 * HAMMAX-SESSIONS.

         CALL 'COBLIM' USING HFUNC-NUM, QSES-STRUCT(1), HDS-LEN,HRC.

         IF HRC = HARC-SUCCESS
           PERFORM DSI-SUCCESS
         ELSE
           PERFORM ERROR-HAND.

       DSI-SUCCESS.

         MOVE HDS-LEN TO NUM-SESS.

         MOVE NUM-SESS TO DISP-NUM.
         DISPLAY 'Number of started sessions = ' DISP-NUM.
         DISPLAY BLANK-LINE.
         DISPLAY BLANK-LINE.

         SET HAIX TO 1.
         MOVE 1 TO LOOP-COUNT.

         PERFORM DSI-LOOP UNTIL HRC NOT = 0 OR HAIX > NUM-SESS.

       DSI-LOOP.


         MOVE LOOP-COUNT TO DISP-NUM.
         DISPLAY 'Session number     : ' DISP-NUM.

         DISPLAY 'Session Long name  : ' QSES-LONGNAME(HAIX).

         DISPLAY 'Session Short name : '
            QSES-SHORTNAME(HAIX).

         DISPLAY 'Session Type       : '
            QSES-SESTYPE(HAIX) ' = ' WITH NO ADVANCING.
         IF QSES-SESTYPE(HAIX) = 'H'
           PERFORM DSI-SET-HOST
           DISPLAY 'Host' WITH NO ADVANCING.
         IF QSES-SESTYPE(HAIX) = 'P'
           DISPLAY 'PC' WITH NO ADVANCING.
         DISPLAY BLANK-LINE.

         MOVE QSES-PSSIZE(HAIX) TO DISP-NUM.
         DISPLAY 'Session PS size    : ' DISP-NUM.


         MOVE HA-QUERY-SESSION-STATUS TO HFUNC-NUM.

         MOVE 18 TO HDS-LEN.

         MOVE QSES-SHORTNAME(HAIX) TO QSST-SHORTNAME.

         CALL 'COBLIM' USING HFUNC-NUM, QSST-STRUCT, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS
           PERFORM DSI-SUCCESS2
         ELSE
           PERFORM ERROR-HAND.

       DSI-SUCCESS2.

         MOVE QSST-PS-ROWS TO DISP-NUM.
         DISPLAY 'Session PS rows    : ' DISP-NUM.

         MOVE QSST-PS-COLS TO DISP-NUM.
         DISPLAY 'Session PS columns : ' DISP-NUM.

         DISPLAY 'Session type 2     : ' QSST-SESTYPE ' = '
             WITH NO ADVANCING.
         IF QSST-SESTYPE = 'F'
           DISPLAY '5250' WITH NO ADVANCING.
         IF QSST-SESTYPE = 'G'
           DISPLAY '5250 Printer Session' WITH NO ADVANCING.
         IF QSST-SESTYPE = 'D'
           DISPLAY 'DFT Host' WITH NO ADVANCING.
         IF QSST-SESTYPE = 'P'
           DISPLAY 'PC' WITH NO ADVANCING.
         DISPLAY BLANK-LINE.

         DISPLAY 'Session supports Extended attributes (EABs)? : '
             WITH NO ADVANCING.

         IF QSST-CHAR >= X'80'
           DISPLAY 'YES' WITH NO ADVANCING
         ELSE
           DISPLAY 'NO' WITH NO ADVANCING.
         DISPLAY BLANK-LINE.

         DISPLAY 'Session supports Program Symbols (PSS)?      : '
             WITH NO ADVANCING.
         IF QSST-CHAR >= X'C0'
            OR (QSST-CHAR < X'80' AND  QSST-CHAR >= X'40')
           DISPLAY 'YES' WITH NO ADVANCING
         ELSE
           DISPLAY 'NO' WITH NO ADVANCING.
         DISPLAY BLANK-LINE.

         DISPLAY PRESS-ENT-MSG WITH NO ADVANCING.
         ACCEPT DUMMY.



         SET HAIX UP BY 1.
         ADD 1 TO LOOP-COUNT.


       DSI-SET-HOST.

         IF DFT-SESS = SPACE
           MOVE QSES-SHORTNAME(HAIX) TO DFT-SESS.



      *********************************************************************
      * WRITE-STR-2-HOST  - Connects to first session and writes home-key *
      *                     and string to host.                           *
      *                                                                   *
      *                                                                   *
      *                                                                   *
      *********************************************************************
       WRITE-STR-2-HOST.


         MOVE HA-CONNECT-PS TO HFUNC-NUM.

         MOVE DFT-SESS TO HDATA-STR(1).

         CALL 'COBLIM' USING HFUNC-NUM, HDATA-STRING, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS
           PERFORM WS2H-SUCCESS
         ELSE
           PERFORM ERROR-HAND.

       WS2H-SUCCESS.

         MOVE HA-SENDKEY TO HFUNC-NUM.

         MOVE 4 TO HDS-LEN.

         CALL 'COBLIM' USING HFUNC-NUM, HOME-KEY, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS
           PERFORM WS2H-SUCCESS2
         ELSE
           PERFORM ERROR-HAND.

       WS2H-SUCCESS2.


         MOVE 9 TO HDS-LEN.

         CALL 'COBLIM' USING HFUNC-NUM, DISP-TEXT, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS
           DISPLAY 'Sent String to Host.'

         ELSE
           PERFORM ERROR-HAND.



      *********************************************************************
      * SEARCH-STR-ON-HOST- Searches for string on host.                  *
      *                                                                   *
      *                                                                   *
      *                                                                   *
      *********************************************************************
       SEARCH-STR-ON-HOST.

         MOVE HA-SEARCH-PS TO HFUNC-NUM.

         MOVE 7 TO HDS-LEN.

         CALL 'COBLIM' USING HFUNC-NUM, DISP-TEXT, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS
           DISPLAY 'Found string "' DISP-TEXT
               WITH NO ADVANCING
           MOVE HDS-LEN TO DISP-NUM
           DISPLAY '" at PS position ' DISP-NUM '.'
           DISPLAY BLANK-LINE
           DISPLAY BLANK-LINE

         ELSE
           PERFORM ERROR-HAND.


      *********************************************************************
      * DISP-HOST-SCR - Displays first 1920 bytes of host screen.         *
      *                                                                   *
      *                                                                   *
      *                                                                   *
      *********************************************************************
       DISP-HOST-SCR.

         MOVE HA-SET-SESSION-PARMS TO HFUNC-NUM.

         MOVE 17 TO HDS-LEN.

         CALL 'COBLIM' USING HFUNC-NUM, SETPARM-TEXT, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS
           PERFORM DHS-SUCCESS
         ELSE
           PERFORM ERROR-HAND.

       DHS-SUCCESS.

         MOVE HA-COPY-PS-TO-STR TO HFUNC-NUM.

         MOVE MAX-DATA-SIZE TO HDS-LEN.

         MOVE 1 TO HRC.

         CALL 'COBLIM' USING HFUNC-NUM, HDATA-STRING, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS

           CALL '__VIOWRTCELLSTR' USING BY VALUE ZRO
                                        BY VALUE ZRO
                                        BY VALUE ZRO
                                        BY VALUE MDS
                               BY REFERENCE HDATA-STRING

         ELSE

           PERFORM ERROR-HAND.


      *********************************************************************
      * CHANGE PS WINDOW NAME - Change the title of the PM window session *
      *                                                                   *
      *                                                                   *
      *                                                                   *
      *********************************************************************
       CHANGE-PS-WINDOW-NAME.

         MOVE HA-CONNECT-PM-SRVCS TO HFUNC-NUM.

         MOVE DFT-SESS TO STPM-SHORTNAME.

         CALL 'HLLCOB' USING HFUNC-NUM, STPM-STRUCT, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS
           PERFORM CHANGE-PS
         ELSE
           PERFORM ERROR-HAND.

       CHANGE-PS.

         MOVE HA-CHANGE-WINDOW-NAME TO HFUNC-NUM.

         MOVE DFT-SESS TO CHLT-SHORTNAME.

         MOVE 1 TO CHLT-OPTION.

         MOVE "Sample Window Name Test" TO CHLT-LTNAME.
         MOVE 26 TO HDS-LEN.

         CALL 'HLLCOB' USING HFUNC-NUM,CHLT-STRUCT,HDS-LEN,HRC.

         IF HRC = HARC-SUCCESS

           DISPLAY 'Window Title Changed.'
           DISPLAY  PRESS-CTRL-ESC

         ELSE

           PERFORM ERROR-HAND.



      *********************************************************************
      * CHANGE SWITCH LIST LT NAME - Change the session's name on         *
      *                                     the switch list               *
      *                                                                   *
      *                                                                   *
      *********************************************************************
       CHANGE-SWITCH-LIST-LT-NAME.

           MOVE HA-CONNECT-PM-SRVCS TO HFUNC-NUM.

           MOVE DFT-SESS TO  STPM-SHORTNAME.

           CALL 'HLLCOB' USING HFUNC-NUM, STPM-STRUCT, HDS-LEN, HRC.

           IF HRC = HARC-SUCCESS
             PERFORM CHANGE-SWITCH
           ELSE
             PERFORM ERROR-HAND.

       CHANGE-SWITCH.

           MOVE HA-CHANGE-SWITCH-NAME TO  HFUNC-NUM.

           MOVE DFT-SESS TO CHSW-SHORTNAME.
           MOVE 1 TO CHSW-OPTION.

           MOVE "Sample Switch List Name" TO CHSW-SWNAME.
           MOVE 26 TO HDS-LEN.

           CALL 'HLLCOB' USING HFUNC-NUM,CHSW-STRUCT,HDS-LEN,HRC.


           IF HRC = HARC-SUCCESS

             DISPLAY 'Switch List LT Name Changed.'
         DISPLAY PRESS-CTRL-ESC

           ELSE

             PERFORM ERROR-HAND.



      *********************************************************************
      * QUERY-PM-STATUS -  Query the PM window status.                    *
      *                                                                   *
      *                                                                   *
      *                                                                   *
      *********************************************************************
       QUERY-PM-STATUS.

         MOVE HA-CONNECT-PM-SRVCS TO HFUNC-NUM.

         MOVE DFT-SESS TO STPM-SHORTNAME.

         CALL 'COBLIM' USING HFUNC-NUM, STPM-STRUCT, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS
           PERFORM QUERY-SESSION
         ELSE
           PERFORM ERROR-HAND.

       QUERY-SESSION.

         MOVE HA-PM-WINDOW-STATUS TO HFUNC-NUM.

         MOVE DFT-SESS TO CWIN-SHORTNAME.

         MOVE 2 TO CWIN-OPTION.

         MOVE 4 TO HDS-LEN.

         CALL 'HLLCOB' USING HFUNC-NUM,CWIN-STRUCT,HDS-LEN,HRC.

         IF HRC >  HARC-SUCCESS
             PERFORM ERROR-HAND.

             DISPLAY '                      PM WINDOW STATUS.'.

         DISPLAY BLANK-LINE.

         MOVE CWIN-FLAGS TO WINDOW-DATA.

         IF (WN-DATA1 = 1) OR (WN-DATA1 = 5) OR (WN-DATA1 = 9)
            DISPLAY 'The Window Is Deactivated.'.
         IF (WN-DATA1 = 4) OR (WN-DATA1 = 5)
            DISPLAY 'The Window Is Minimized.'.
         IF (WN-DATA1 = 8) OR (WN-DATA1 = 9)
            DISPLAY 'The Window Is Maximized.'.
         IF (WN-DATA2 = 8) OR (WN-DATA2 = 136)
            DISPLAY 'The Window Is Visible.'.
         IF (WN-DATA2 = 16) OR (WN-DATA2 = 144)
            DISPLAY 'The Window Is Invisible.'.
         IF (WN-DATA2 = 128)
            DISPLAY 'The Window Is Activated.'.

         MOVE HA-QUERY-WINDOW-COORDS TO HFUNC-NUM.

         MOVE DFT-SESS TO GCOR-SHORTNAME.

         MOVE 17 TO HDS-LEN.

         CALL 'HLLCOB' USING HFUNC-NUM,GCOR-STRUCT,HDS-LEN,HRC.

         IF HRC = HARC-SUCCESS
           DISPLAY BLANK-LINE
           DISPLAY '                      PM WINDOW COORDINATES.'
           DISPLAY BLANK-LINE

           DISPLAY   'XLEFT       '  WITH NO ADVANCING
           MOVE GCOR-XLEFT TO BINARY-NUM
           PERFORM BIN2HEX
           DISPLAY H-PRINT

           DISPLAY   'YBOTTOM     ' WITH NO ADVANCING
           MOVE GCOR-YBOTTOM TO BINARY-NUM
           PERFORM BIN2HEX
           DISPLAY H-PRINT

           DISPLAY   'XRIGHT      ' WITH NO ADVANCING
           MOVE GCOR-XRIGHT TO BINARY-NUM
           PERFORM BIN2HEX
           DISPLAY H-PRINT

           DISPLAY   'YTOP        '  WITH NO ADVANCING
           MOVE GCOR-YTOP TO BINARY-NUM
           PERFORM BIN2HEX
           DISPLAY H-PRINT

         ELSE

           PERFORM ERROR-HAND.


      *********************************************************************
      * MAKE-PM-WINDOW-INVISIBLE -  Make the PM window invisible.         *
      *                                                                   *
      *                                                                   *
      *                                                                   *
      *********************************************************************
       MAKE-PM-WINDOW-INVISIBLE.

         MOVE HA-CONNECT-PM-SRVCS TO HFUNC-NUM.

         MOVE DFT-SESS TO HDATA-STR(1).

         CALL 'COBLIM' USING HFUNC-NUM, HDATA-STRING, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS
           PERFORM MAKE-INVIS
         ELSE
           PERFORM ERROR-HAND.

       MAKE-INVIS.

         MOVE HA-PM-WINDOW-STATUS TO HFUNC-NUM.

         MOVE DFT-SESS TO CWIN-SHORTNAME.
         MOVE 1  TO CWIN-OPTION.
         MOVE 16 TO CWIN-FLAGS.
         MOVE 0  TO CWIN-XPOS.
         MOVE 0  TO CWIN-YPOS.
         MOVE 0  TO CWIN-XSIZE.
         MOVE 0  TO CWIN-YSIZE.
         MOVE 0  TO CWIN-BEHIND.

         CALL 'HLLCOB' USING HFUNC-NUM,CWIN-STRUCT,HDS-LEN,HRC.

         IF HRC = HARC-SUCCESS

           DISPLAY 'The PM Window Is Now Invisible.'
           DISPLAY PRESS-CTRL-ESC

         ELSE

           PERFORM ERROR-HAND.


      *********************************************************************
      * MAKE-PM-WINDOW-VISIBLE -Make the PM window visible and maximized. *
      *                                                                   *
      *                                                                   *
      *                                                                   *
      *********************************************************************
       MAKE-PM-WINDOW-VISIBLE.

         MOVE HA-CONNECT-PM-SRVCS TO HFUNC-NUM.

         MOVE DFT-SESS TO  STPM-SHORTNAME.

         CALL 'COBLIM' USING HFUNC-NUM, STPM-STRUCT, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS
           PERFORM MAKE-VIS
         ELSE
           PERFORM ERROR-HAND.

       MAKE-VIS.

         MOVE HA-PM-WINDOW-STATUS TO HFUNC-NUM.

         MOVE DFT-SESS TO CWIN-SHORTNAME.
         MOVE 1  TO CWIN-OPTION.
         MOVE 2056 TO CWIN-FLAGS.
         MOVE 0  TO CWIN-XPOS.
         MOVE 0  TO CWIN-YPOS.
         MOVE 0  TO CWIN-XSIZE.
         MOVE 0  TO CWIN-YSIZE.
         MOVE 0  TO CWIN-BEHIND.

         CALL 'HLLCOB' USING HFUNC-NUM,CWIN-STRUCT,HDS-LEN,HRC.

         IF HRC = HARC-SUCCESS

           DISPLAY 'The PM Window Is Now Visible And Maximized.'
           DISPLAY PRESS-CTRL-ESC

         ELSE

           PERFORM ERROR-HAND.




      *********************************************************************
      * DISCONNECT-PM-WINDOW - Disconnect from PM window                  *
      *                                                                   *
      *                                                                   *
      *                                                                   *
      *********************************************************************
       DISCONNECT-PM-WINDOW.


         MOVE HA-CONNECT-PS TO HFUNC-NUM.

         MOVE DFT-SESS TO STPM-SHORTNAME.

         CALL 'COBLIM' USING HFUNC-NUM, STPM-STRUCT, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS
           PERFORM DISCONNECT
         ELSE
           PERFORM ERROR-HAND.

       DISCONNECT.

         MOVE HA-DISCONNECT-PM-SRVCS TO HFUNC-NUM.

         MOVE DFT-SESS TO SPPM-SHORTNAME.

         CALL 'COBLIM' USING HFUNC-NUM, SPPM-STRUCT, HDS-LEN, HRC.

         IF HRC = HARC-SUCCESS
           DISPLAY "PM Window Disconnected."
         ELSE
           PERFORM ERROR-HAND.




      *********************************************************************
      * RESET WINDOW- Reset switch name, window name and window size.     *
      *                                                                   *
      *                                                                   *
      *                                                                   *
      *********************************************************************
       RESET-WINDOW.


         MOVE HA-CONNECT-PM-SRVCS TO HFUNC-NUM.

         MOVE DFT-SESS TO STPM-SHORTNAME.

         CALL 'COBLIM' USING HFUNC-NUM, STPM-STRUCT, HDS-LEN, HRC.

         IF HRC > HARC-SUCCESS
            PERFORM ERROR-HAND.

         MOVE HA-CHANGE-SWITCH-NAME TO  HFUNC-NUM.

         MOVE DFT-SESS TO CHSW-SHORTNAME.
         MOVE 2 TO CHSW-OPTION.
         MOVE 4 TO HDS-LEN.

         CALL 'HLLCOB' USING HFUNC-NUM,CHSW-STRUCT,HDS-LEN,HRC.

         IF HRC > HARC-SUCCESS
           PERFORM ERROR-HAND.

         DISPLAY 'Switch List LT Name Reset.'

         MOVE HA-CHANGE-WINDOW-NAME TO HFUNC-NUM.

         MOVE DFT-SESS TO CHLT-SHORTNAME.
         MOVE 2 TO CHLT-OPTION.
         MOVE 4 TO HDS-LEN.

         CALL 'HLLCOB' USING HFUNC-NUM,CHLT-STRUCT,HDS-LEN,HRC.

         IF HRC > HARC-SUCCESS
           PERFORM ERROR-HAND.

         DISPLAY 'PM Window Name Reset.'

         MOVE HA-PM-WINDOW-STATUS TO HFUNC-NUM.

         MOVE DFT-SESS TO CWIN-SHORTNAME.
         MOVE 1  TO CWIN-OPTION.
         MOVE 4096 TO CWIN-FLAGS.
         MOVE 0  TO CWIN-XPOS.
         MOVE 0  TO CWIN-YPOS.
         MOVE 0  TO CWIN-XSIZE.
         MOVE 0  TO CWIN-YSIZE.
         MOVE 0  TO CWIN-BEHIND.

         CALL 'HLLCOB' USING HFUNC-NUM,CWIN-STRUCT,HDS-LEN,HRC.

         IF HRC = HARC-SUCCESS

           DISPLAY 'The PM Window Size Is Now Restored.'
           DISPLAY PRESS-CTRL-ESC
           DISPLAY BLANK-LINE

         ELSE

           PERFORM ERROR-HAND.




      *********************************************************************
      * RESET EHLLAPI - Return EHLLAPI to its original conditions         *
      *                                                                   *
      *                                                                   *
      *********************************************************************
       RESET-EHLLAPI.

         MOVE HA-RESET-SYSTEM TO HFUNC-NUM.

         MOVE DFT-SESS TO HDATA-STR(1).

         CALL 'COBLIM' USING HFUNC-NUM,HDATA-STRING,HDS-LEN,HRC.

         IF HRC > HARC-SUCCESS
              PERFORM ERROR-HAND.

         DISPLAY 'EHLLAPI Reset To Original Conditions.'.





      *********************************************************************
      * ERROR_HAND - Error handler.                                       *
      *                                                                   *
      * INPUT                                                             *
      *                                                                   *
      * OUTPUT                                                            *
      *********************************************************************
       ERROR-HAND.

           DISPLAY BLANK-LINE.
           MOVE HRC TO DISP-NUM.
           DISPLAY 'UNEXPECTED RETURN CODE ' DISP-NUM ' from '
               WITH NO ADVANCING.
           MOVE HFUNC-NUM TO DISP-NUM.
           DISPLAY 'FUNCTION #' DISP-NUM '.'
               WITH NO ADVANCING.


      ******************************************************************
      * ITOH - Convert binary to hex digits.                           *
      *                                                                *
      * INPUT                                                          *
      *                                                                *
      * OUTPUT                                                         *
      *                                                                *
      *                                                                *
      ******************************************************************
       ITOH.


           IF BIN-NUM < 0
             COMPUTE BIN-NUM = 256 + BIN-NUM.

           COMPUTE BIN-NUM2 = BIN-NUM / 16.

           ADD 1 TO BIN-NUM2.

           SET INDX TO BIN-NUM2.

           MOVE HEX-DIG(INDX) TO HEX-OUT(1).

           COMPUTE BIN-NUM2 = BIN-NUM - ((BIN-NUM2 - 1) * 16).

           ADD 1 TO BIN-NUM2.

           SET INDX TO BIN-NUM2.

           MOVE HEX-DIG(INDX) TO HEX-OUT(2).



      ******************************************************************
      * BIN2HEX - Copy a variable stored in binary as a PIC(4) to a    *
      *           variable which can be viewed as a hex number in ascii*
      *                                                                *
      * INPUT   - BINARY-NUM CONTAINS A PIC 9(4)                       *
      *         - B-NUM (4) IS A PIC 9 COMP OCCURS 4 TIMES             *
      *                                                                *
      * OUTPUT  - H-PRINT  (H-OUT X OCCURS 8 TIMES)                    *
      *                                                                *
      *                                                                *
      ******************************************************************
       BIN2HEX.


        COMPUTE BIN-NUM  = B-NUM(3) /16.
        COMPUTE BIN-NUM2 = B-NUM(3) - (BIN-NUM * 16).
        ADD 1 TO BIN-NUM.
        SET INDX TO BIN-NUM.
        MOVE HEX-DIG(INDX) TO H-OUT(1).
        ADD 1 TO BIN-NUM2.
        SET INDX TO BIN-NUM2.
        MOVE HEX-DIG(INDX) TO H-OUT(2).

        COMPUTE BIN-NUM = B-NUM(4) /16.
        COMPUTE BIN-NUM2 = B-NUM(4) - (BIN-NUM * 16).
        ADD 1 TO BIN-NUM.
        SET INDX TO BIN-NUM.
        MOVE HEX-DIG(INDX) TO H-OUT(3).
        ADD 1 TO BIN-NUM2.
        SET INDX TO BIN-NUM2.
        MOVE HEX-DIG(INDX) TO H-OUT(4).


        COMPUTE BIN-NUM = B-NUM(1) /16.
        COMPUTE BIN-NUM2 = B-NUM(1) - (BIN-NUM * 16).
        ADD 1 TO BIN-NUM.
        SET INDX TO BIN-NUM.
        MOVE HEX-DIG(INDX) TO H-OUT(5).
        ADD 1 TO BIN-NUM2.
        SET INDX TO BIN-NUM2.
        MOVE HEX-DIG(INDX) TO H-OUT(6).


        COMPUTE BIN-NUM = B-NUM(2) /16.
        COMPUTE BIN-NUM2 = B-NUM(2) - (BIN-NUM * 16).
        ADD 1 TO BIN-NUM.
        SET INDX TO BIN-NUM.
        MOVE HEX-DIG(INDX) TO H-OUT(7).
        ADD 1 TO BIN-NUM2.
        SET INDX TO BIN-NUM2.
        MOVE HEX-DIG(INDX) TO H-OUT(8).

      ******************************************************************
      * X2HEX - Copy a variable stored in binary as a PIC X(2) to a    *
      *           variable which can be viewed as a hex number in ascii*
      *                                                                *
      * INPUT   - BINARY-NUM-X CONTAINS A PIC X(2)                     *
      *         - B-NUM-X (2) IS A PIC 9 COMP OCCURS 4 TIMES           *
      *                                                                *
      * OUTPUT  - H-PRINT-X  (H-OUT-X X OCCURS 4 TIMES)                *
      *                                                                *
      *                                                                *
      ******************************************************************
       X2HEX.


        COMPUTE BIN-NUM = B-NUM(1) /16.
        COMPUTE BIN-NUM2 = B-NUM(1) - (BIN-NUM * 16).
        ADD 1 TO BIN-NUM.
        SET INDX TO BIN-NUM.
        MOVE HEX-DIG(INDX) TO H-OUT-X(3).
        ADD 1 TO BIN-NUM2.
        SET INDX TO BIN-NUM2.
        MOVE HEX-DIG(INDX) TO H-OUT-X(4).


        COMPUTE BIN-NUM = B-NUM(2) /16.
        COMPUTE BIN-NUM2 = B-NUM(2) - (BIN-NUM * 16).
        ADD 1 TO BIN-NUM.
        SET INDX TO BIN-NUM.
        MOVE HEX-DIG(INDX) TO H-OUT-X(1).
        ADD 1 TO BIN-NUM2.
        SET INDX TO BIN-NUM2.
        MOVE HEX-DIG(INDX) TO H-OUT-X(2).


