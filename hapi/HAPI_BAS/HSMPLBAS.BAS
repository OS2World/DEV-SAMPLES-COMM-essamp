'*********************************************************************
'*                                                                   *
'* FILE NAME: HSMPLBAS.BAS                                           *
'*                                                                   *
'* MODULE NAME= HSMPLBAS.BAS                                         *
'*                                                                   *
'* DESCRIPTIVE NAME= BASIC COMPILER SAMPLE PROGRAM FOR EHLLAPI       *
'*                                                                   *
'* Displays EHLLAPI and session information.                         *
'* Writes string to host.                                            *
'* Searches for written string on host.                              *
'* Displays host session screen.                                     *
'* Manipulates the Presentation Manager properties of                *
'* the emulator session to: change window title name, switch         *
'* list name, make window invisible, query window status,            *
'* window coordinates, change window size, and restore the           *
'* emulator session window to its original conditions.               *
'* Next, the structured field functions are used.  The               *
'* communications buffer is queried, the read and write buffers      *
'* allocated, a connection is initiated to the communications        *
'* buffer, and an asynchronus read structured field is issued        *
'* disabling the inbound host.  Then, the sendkey function is        *
'* used to send the command 'IND$FILE PUT SF_TEST EXEC A'            *
'* to the host which puts a non-existent file from the               *
'* PC to the host using a structured field.  Next, a get             *
'* completion request is issued to determine if the                  *
'* previous asynchronus read structured field is completed,          *
'* Upon completion, a synchronus write structured field is           *
'* issued, the communications buffers are de-allocated, and          *
'* then a disconnect from structured field is issued.                *
'*                                                                   *
'*                                                                   *
'*                                                                   *
'*   COPYRIGHT:  XXXXXXXXX  (C) COPYRIGHT IBM CORP. 1987,1988,1989   *
'*               LICENSED MATERIAL - PROGRAM PROPERTY OF IBM         *
'*               ALL RIGHTS RESERVED                                 *
'*                                                                   *
'*                                                                   *
'* NOTES=                                                            *
'*                                                                   *
'**********************-END OF SPECIFICATIONS-************************
'
'*********************************************************************
'********************** BEGIN INCLUDE FILES **************************
'*********************************************************************



'$INCLUDE: 'HAPI_BAS.INC'


'*********************************************************************
'*************************** BEGIN CODE ******************************
'*********************************************************************

'*********************************************************************
'* MAIN - Main code calls routines to do real work.                  *
'*                                                                   *
'*                                                                   *
'*                                                                   *
'*********************************************************************


' The following structures are used to order data for EHLLAPI calls

     DIM QRYoSTR        AS QBUFoSTRUCT  'Query Communications Buffer
     DIM ALLOCATEoSTR   AS ABUFoSTRUCT  'Allocate Communications Buffer
     DIM CONNECToSTR    AS STSFoSTRUCT  'Connect To Communications Buffer
     DIM READoSTR       AS RDSFoSTRUCT  'Read Structured Field
     DIM GREQoSTR       AS GCMPoSTRUCT  'Get Asynchronus Complete
     DIM WRITEoSTR      AS WRSFoSTRUCT  'Write Strucured Field
     DIM FREEoBUFFoSTR  AS FBUFoSTRUCT  'Free Communications Buffer
     DIM DISCONNECToSTR AS SPSFoSTRUCT  'Disconnect From Com Buffer

     DIM QRSTR (1 TO 12) AS STRING*1    'String with DDM query reply data
     DIM WRoADDRESS AS LONG             'Write Buffer Address
     DIM RDoADDRESS AS LONG             'Read Buffer Address
     DIM BUFFERoLENGTH  AS LONG         'Buffer Length

     CLS                                'Clear the screen.

     KEY OFF
     GOSUB BUILDTABLES
     EHLLAPI$ = "EHLLAPI"
     TESToNAME$ = "Sample_Test_Name"
     INVISoTEXT$  = "INVISIBLE_WRITE_TEST"
     COMMANDoTEXT$ ="IND$FILE PUT SF_TEST EXEC A@E@0"
     DFToSESS$ = " "
     HDATAoSTR$ = SPACE$(3840)
     MAXoDATAoSIZE = 3840
     MAXoSIZE% = MAXoDATAoSIZE
     ZERO% = 0


     GOSUB DISPoEHLLAPIoINFO

     IF HRC% <> 0 THEN GOTO MRET

     PRINT
     INPUT "Press ENTER to continue...", X$
     GOSUB DISPoSESSIONoINFO                 ' Call routine to
                                             ' display Host session

     IF HRC% <> 0 THEN GOTO MRET

     IF DFToSESS$ = " "  THEN                ' If at least 1 dft sess
       PRINT "NO DFT SESSION SESSION STARTED.":GOTO MRET
     ENDIF

     CLS
     HOSToTEXT$ = EHLLAPI$
     PRINT "Press ENTER to send string '";HOSToTEXT$;"' to session short name ";
     PRINT DFToSESS$;"...";
MI1: X$ = INKEY$: IF X$ = "" THEN GOTO MI1
     GOSUB WRITEoSTRo2oHOST

     IF HRC% <> 0 THEN GOTO MRET

     PRINT "Press ENTER to search for string '";HOSToTEXT$;
     PRINT "' on Host Presentation Space...";
MI2: X$ = INKEY$: IF X$ = "" THEN GOTO MI2
     GOSUB SEARCHoSTRoONoHOST

     IF HRC% <> 0 THEN GOTO MRET

     PRINT "Press ENTER to display first 1920 bytes of Host ";
     PRINT "presentation space...";
MI3: X$ = INKEY$: IF X$ = "" THEN GOTO MI3
     GOSUB DISPoHOSToSCR

     IF HRC% <> 0 THEN GOTO MRET
     PRINT
     PRINT "Press ENTER to change the PM window title. ";
MI4: X$ = INKEY$: IF X$ = "" THEN GOTO MI4
     GOSUB CHANGEoWINDOWoNAME

     IF HRC% <> 0 THEN GOTO MRET

     PRINT "Press ENTER to change the switch list name. "
MI5: X$ = INKEY$: IF X$ = "" THEN GOTO MI5
     GOSUB CHANGEoSWITCHoLISToNAME

     IF HRC% <> 0 THEN GOTO MRET

     PRINT "Press ENTER to query the PM session. "
MI6: X$ = INKEY$: IF X$ = "" THEN GOTO MI6
     GOSUB QUERYoPMoSESSION

     IF HRC% <> 0 THEN GOTO MRET


     PRINT "Press ENTER to make the PM session invisible. ";
MI7: X$ = INKEY$: IF X$ = "" THEN GOTO MI7
     GOSUB MAKEoPMoINVISIBLE

     HOSToTEXT$ = INVISoTEXT$
     PRINT "Press ENTER to send string '";HOSToTEXT$;"' to session short name ";
     PRINT DFToSESS$;"...";
MI8: X$ = INKEY$: IF X$ = "" THEN GOTO MI8
     GOSUB WRITEoSTRo2oHOST

     PRINT "Press ENTER to display first 1920 bytes of Host ";
     PRINT "presentation space...";
MI9: X$ = INKEY$: IF X$ = "" THEN GOTO MI9
     GOSUB DISPoHOSToSCR
     IF HRC% <> 0 THEN GOTO MRET

     PRINT
     PRINT "Press ENTER to make the PM screen visible and maximized. ";
MI10: X$ = INKEY$: IF X$ = "" THEN GOTO MI10
     GOSUB MAKEoPMoVISIBLE

     IF HRC% <> 0 THEN GOTO MRET

     PRINT "Press ENTER to disconnect from the PM session. ";
MI11: X$ = INKEY$: IF X$ = "" THEN GOTO MI11
     GOSUB DISCONNECToPM

     IF HRC% <> 0 THEN GOTO MRET

     PRINT "Press ENTER to restore the PM window settings. ";
MI12: X$ = INKEY$: IF X$ = "" THEN GOTO MI12
     GOSUB RESTOREoPMoNAMES

     IF HRC% <> 0 THEN GOTO MRET

     PRINT "The sample program continues with structured field EHLLAPI calls.";
     PRINT "The host session must be active and have access to"
     PRINT "the IND$FILE file transfer application."
     PRINT "Do you wish to continue ?   (Press 'y' or 'n')"

MI13: X$ = INKEY$: IF X$ = "" THEN GOTO MI13

     IF  X$<>"y"  AND  X$<>"Y"  AND  X$<>"n" AND  X$<>"N"  THEN GOTO MI13

     IF  X$="N" OR  X$="n"  THEN GOTO MEND

     CLS
     GOSUB RESEToSYSTEM
     IF HRC% <> 0 THEN GOTO MRET

     GOSUB QUERYoCOMoBUFFER
     IF HRC% <> 0 THEN GOTO MRET

     PRINT "Allocate The Read Buffer."
     BUFFERoLENGTH = QRYoSTR.QBUFoOPToINB
     GOSUB ALLOCATEoCOMoBUFFER
     IF HRC% <> O THEN GOTO MRET
     RDoADDRESS  = ALLOCATEoSTR.ABUFoADDRESS

     PRINT "Allocate The Write Buffer."
     BUFFERoLENGTH = QRYoSTR.QBUFoOPToOUTB
     GOSUB ALLOCATEoCOMoBUFFER
     IF HRC% <> O THEN GOTO MRET
     WRoADDRESS  = ALLOCATEoSTR.ABUFoADDRESS

     PRINT "Press ENTER to initiate a structured field connection.";
MI14: X$ = INKEY$: IF X$ = "" THEN GOTO MI14

     GOSUB CONNECToCOMoBUFFER
     IF HRC% <> O THEN GOTO MRET

     PRINT "Press ENTER to read a structured field. ";
MI15: X$ = INKEY$: IF X$ = "" THEN GOTO MI15

     GOSUB READoSFoASYNC
     IF HRC% <> O THEN GOTO MRET

     PRINT "Press ENTER to create a structured field. ";
MI16: X$ = INKEY$: IF X$ = "" THEN GOTO MI16

     GOSUB CREATEoSTRUCTUREDoFIELD
     IF HRC% <> O THEN GOTO MRET

     PRINT "Press ENTER to perform an asyncronus completion request. ";
MI17: X$ = INKEY$: IF X$ = "" THEN GOTO MI17

     GOSUB GEToASYNCoCOMPLETE
     IF HRC% <> O THEN GOTO MRET

     PRINT "Press ENTER to write a structured field.";
MI18: X$ = INKEY$: IF X$ = "" THEN GOTO MI18

     GOSUB WRITEoSFoSYNC
     IF HRC% <> O THEN GOTO MRET

     PRINT "Press ENTER to free the communication buffers.";
MI19: X$ = INKEY$: IF X$ = "" THEN GOTO MI19

     GOSUB FREEoCOMMOoBUFF
     IF HRC% <> O THEN GOTO MRET

     GOSUB DISCONNECToFROMoCOMoBUFFER
     IF HRC% <> O THEN GOTO MRET


MEND:LOCATE 25,1
     PRINT "SAMPLE PROGRAM DONE.  To Exit Program Press ENTER...";
MKEY: X$ = INKEY$: IF X$ = "" THEN GOTO MKEY
MRET:
     END








'********************************************************************
' DISPoEHLLAPIoINFO - CALLs EHLLAPI QUERYoSYSTEM and then displays  *
'                     the requested info.                           *
'                                                                   *
' INPUT                                                             *
'                                                                   *
' OUTPUT                                                            *
'                                                                   *
'********************************************************************
DISPoEHLLAPIoINFO:                      '     Routine to display
                                        '     EHLLAPI info.

  HFUNCoNUM% = HAoQUERYoSYSTEM%         '     Issue query
                                        ' system.


  CALL BLIM(HFUNCoNUM%, HDATAoSTR$, HDSoLEN%, HRC%)
                                        '     Call EHLLAPI.

  IF HRC% <> HARCoSUCCESS THEN GOTO DEIoERROR

  PRINT "                       EHLLAPI INFORMATION"
  PRINT
  PRINT "  EHLLAPI version              : ";

  PRINT MID$(HDATAoSTR$,1,1)

  PRINT "  EHLLAPI level                : ";

  PRINT MID$(HDATAoSTR$,2,2)

  PRINT "  EHLLAPI release date         : ";

  PRINT MID$(HDATAoSTR$,4,6)

  PRINT "  EHLLAPI LIM version          : ";

  PRINT MID$(HDATAoSTR$,10,1)

  PRINT "  EHLLAPI LIM level            : ";

  PRINT MID$(HDATAoSTR$,11,2)

  PRINT "  EHLLAPI hardware base        : ";

  T$ = MID$(HDATAoSTR$,13,1)
  PRINT T$;
  PRINT " = ";
  IF T$ = "Z" THEN
    PRINT "(See System model/submodel below)";
  ENDIF
  PRINT

  PRINT "  EHLLAPI CTRL program type    : ";

  T$ = MID$(HDATAoSTR$,14,1)
  PRINT T$;
  PRINT " = ";
  IF T$ = "X" THEN
    PRINT "OS/2";
  ENDIF
  PRINT

  PRINT "  EHLLAPI sequence number      : ";MID$(HDATAoSTR$,15,2)

  PRINT "  EHLLAPI CTRL program version : ";

  PRINT MID$(HDATAoSTR$,17,2)

  PRINT "  EHLLAPI PC session name      : ";

  PRINT MID$(HDATAoSTR$,19,1)

  PRINT "  EHLLAPI extended error 1     : ";

  PRINT MID$(HDATAoSTR$,20,4)

  PRINT "  EHLLAPI extended error 2     : ";

  PRINT MID$(HDATAoSTR$,24,4)

  T% = ASC(MID$(HDATAoSTR$,28,1))
  T$ = HEX$(T%)
  IF T% <= &H0F THEN T$ = "0" + T$
  T1% = ASC(MID$(HDATAoSTR$,29,1))
  T1$ = HEX$(T1%)
  IF T1% <= &H0F THEN T1$ = "0" + T1$
  PRINT "  EHLLAPI system model/submodel: ";
  PRINT T$ + T1$;
  PRINT " HEX  ";
  IF T% = &HFC AND T1% = &H00 THEN
    PRINT "= Model PC AT";
  ENDIF
  IF T% = &HFC AND T1% = &H01 THEN
    PRINT "= Model PC AT ENHANCED";
  ENDIF
  IF T% = &HFC AND T1% = &H02 THEN
    PRINT "= Model PC XT Model 286";
  ENDIF
  IF T% = &HFC AND T1% = &H04 THEN
    PRINT "= Model 50";
  ENDIF
  IF T% = &HFC AND T1% = &H05 THEN
    PRINT "= Model 60";
  ENDIF
  IF T% = &HF8 AND T1% = &H00 THEN
    PRINT "= Model 80";
  ENDIF
  IF T% = &HF8 AND T1% = &H09 THEN
    PRINT "= Model 70";
  ENDIF
  PRINT

  PRINT "  EHLLAPI National Language    : ";
  PRINT ASC(MID$(HDATAoSTR$,30,1)) + (ASC(MID$(HDATAoSTR$,31,1)) * 256)

  PRINT "  EHLLAPI monitor type         : ";

  T$ = MID$(HDATAoSTR$,32,1)
  PRINT T$;
  PRINT " = ";
  IF T$ = "M" THEN
    PRINT "PC MONOCHROME";
  ENDIF
  IF T$ = "C" THEN
  PRINT "PC CGA";
  ENDIF
  IF T$ = "E" THEN
    PRINT "PC EGA";
  ENDIF
  IF T$ = "A" THEN
    PRINT "PS MONOCHROME";
  ENDIF
  IF T$ = "V" THEN
    PRINT "PS 8512";
  ENDIF
  IF T$ = "H" THEN
    PRINT "PS 8514";
  ENDIF
  IF T$ = "U" THEN
    PRINT "UNKNOWN monitor type";
  ENDIF
  PRINT
  GOTO DEIoRET

DEIoERROR:
  GOSUB ERRORoHAND

DEIoRET:
  RETURN





'********************************************************************
' DISPoSESSIONoINFO - CALLs EHLLAPI QUERY funtions and then displays*
'                     the requested session info.                   *
'                                                                   *
'                                                                   *
'                                                                   *
'********************************************************************



DISPoSESSIONoINFO:                           ' Routine to display
                                             ' Host session info.
  NUMoSESS% = 0

  PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT

  PRINT "                           SESSION INFO";
  PRINT:PRINT

  HFUNCoNUM% = HAoQUERYoSESSIONS%            ' Issue query
                                             ' sessions.

  HDSoLEN% = MAXoDATAoSIZE / 12 * 12         ' Make sure len is
                                             ' multiple of 12.

  CALL BLIM(HFUNCoNUM%, HDATAoSTR$, HDSoLEN%, HRC%)
                                             '  Call EHLLAPI.


  IF HRC% <> HARCoSUCCESS THEN GOTO DSIoERROR
                                             ' If good rc.
  NUMoSESS% = HDSoLEN%                       ' Number of sessions
                                             ' started.

  PRINT "Number of started sessions = "; NUMoSESS%
  PRINT
  PRINT


  I% = 1
  STRT% = 0   'BASE OFFSET TO START OF ARRAY ELEMENTS.

  DO WHILE I% <= NUMoSESS%
                                             ' LOOP thru queried
                                             ' sessions.

    PRINT "Session number     : ";I%


    PRINT "Session Long name  : ";MID$(HDATAoSTR$,STRT%+2,8)

    PRINT "Session Short name : ";MID$(HDATAoSTR$,STRT%+1,1)

    T$ = MID$(HDATAoSTR$,STRT%+10,1)
    PRINT "Session Type       : ";T$;
    PRINT " = ";
    IF T$ = "H" THEN GOTO DSIoHOST
    GOTO DSIoNEXT
    DSIoHOST:

    PRINT "Host";

    IF DFToSESS$ = " " THEN                  ' If first HOST not
                                             ' set already

      DFToSESS$ = MID$(HDATAoSTR$,STRT%+1,1)
                                             ' Session to write string to.
    ENDIF

    DSIoNEXT:
    IF T$ = "P" THEN

      PRINT "PC";
    ENDIF

    PRINT



    PRINT "Session PS size    : ";
    PRINT ASC(MID$(HDATAoSTR$,STRT%+11,1))+(ASC(MID$(HDATAoSTR$,STRT%+12,1))*256)

    HFUNCoNUM% = HAoQUERYoSESSIONoSTATUS%
                                             ' Issue query
                                             ' session status.

    HDSoLEN% = 18                            ' Set length.


    QSST$ = MID$(HDATAoSTR$,STRT%+1,1) + SPACE$(17)
                                             ' Set the short name

    CALL BLIM(HFUNCoNUM%, QSST$, HDSoLEN%, HRC%)
                                             ' Call EHLLAPI.

    IF HRC% <> HARCoSUCCESS THEN GOTO DSIoERROR

    PRINT "Session PS rows    : ";
    PRINT ASC(MID$(QSST$,12,1))+(ASC(MID$(QSST$,13,1))*256)

    PRINT "Session PS columns : ";
    PRINT ASC(MID$(QSST$,14,1))+(ASC(MID$(QSST$,15,1))*256)

    T$ = MID$(QSST$,10,1)
    PRINT "Session type 2     : ";T$;
    PRINT " = ";
    IF T$ = "F" THEN
      PRINT "5250";
    ENDIF
    IF T$ = "G" THEN
      PRINT "5250 Printer Session";
    ENDIF
    IF T$ = "D" THEN
      PRINT "DFT Host";
    ENDIF
    IF T$ = "P" THEN
      PRINT "PC";
    ENDIF
    PRINT

    PRINT "Session supports Extended attributes (EABs)? : ";
    T% = ASC(MID$(QSST$,11,1))
    IF T% >= &H80 THEN
      PRINT "YES"
    ELSE
      PRINT "NO"
    ENDIF


    PRINT "Session supports Program Symbols (PSS)?      : ";

    IF T% >= &HC0 OR (T% < &H80 AND T% >= &H40) THEN
      PRINT "YES"
    ELSE
      PRINT "NO"
    ENDIF

    PRINT:PRINT
    INPUT "ENTER ENTER TO CONTINUE...", X$
    I% = I% + 1
    STRT% = STRT% + 12 'GET NEXT START OF QSES ELEMENT.
  LOOP
  GOTO DSIoRET

DSIoERROR:

  GOSUB ERRORoHAND

DSIoRET:
  RETURN







'********************************************************************
' WRITEoSTRo2oHOST  - Connects to first session and writes homeokey *
'                     and string to host.                           *
'                                                                   *
'                                                                   *
'********************************************************************
WRITEoSTRo2oHOST:                            ' Call routine to
                                             ' write string to host.



  HFUNCoNUM% = HAoCONNECToPS%                 ' Issue Connect
                                              ' Presentation Space.

  CALL BLIM(HFUNCoNUM%, DFToSESS$, HDSoLEN%, HRC%)
                                              ' Call EHLLAPI.

  IF HRC% <> HARCoSUCCESS THEN GOTO WS2HoERROR


  HFUNCoNUM% = HAoSENDKEY%                    ' Issue sendkey.

  HOMEoKEY$ = "@L@0"                          ' String to send to host

  HDSoLEN% = LEN(HOMEoKEY$)                   ' Set length of string
                                              ' string. minus null char

  CALL BLIM(HFUNCoNUM%, HOMEoKEY$, HDSoLEN%, HRC%)
                                              ' Call EHLLAPI.

  IF HRC% <> HARCoSUCCESS THEN GOTO WS2HoERROR


  HDSoLEN% = LEN(HOSToTEXT$)                  ' Set length of string
                                              ' minus null char

  CALL BLIM(HFUNCoNUM%, HOSToTEXT$, HDSoLEN%, HRC%)
                                              ' Call EHLLAPI.

  IF HRC% <> HARCoSUCCESS THEN GOTO WS2HoERROR

  PRINT "Sent String to Host."
  PRINT
  PRINT
  GOTO WS2HoRET

WS2HoERROR:

  GOSUB ERRORoHAND

WS2HoRET:
  RETURN





'********************************************************************
' SEARCHoSTRoONoHOST- Searches for string on host.                  *
'                                                                   *
'                                                                   *
'                                                                   *
'********************************************************************
SEARCHoSTRoONoHOST:                          ' Routine to search
                                             ' for string on host session

  HFUNCoNUM% = HAoSEARCHoPS%                 ' Issue search PS.


  HDSoLEN% = LEN(HOSToTEXT$)                 ' Set length of
                                             ' string. minus null char

  CALL BLIM(HFUNCoNUM%, HOSToTEXT$, HDSoLEN%, HRC%)
                                             ' Call EHLLAPI.

  IF HRC% <> HARCoSUCCESS THEN GOTO SSOHoERROR

  PRINT "Found string '";
  PRINT HOSToTEXT$;
  PRINT "' at PS position ";HDSoLEN%
  PRINT "(press CONTROL-ESCAPE to verify)"
  PRINT
  PRINT
  PRINT

  GOTO SSOHoRET

SSOHoERROR:

  GOSUB ERRORoHAND

SSOHoRET:
  RETURN



'********************************************************************
' DISPoHOSToSCR - Displays first 1920 bytes of host screen.         *
'                                                                   *
'                                                                   *
'                                                                   *
'********************************************************************
DISPoHOSToSCR:                               ' Routine to
                                             ' display host screen.

  HFUNCoNUM% = HAoSEToSESSIONoPARMS%         ' Issue Set session
                                             ' Parms.
  T$ = "EAB NOATTRB XLATE"

  HDSoLEN% = LEN(T$)

  CALL BLIM(HFUNCoNUM%, T$, HDSoLEN%, HRC%)

  IF HRC% <> HARCoSUCCESS GOTO DHSoERROR     ' If bad rc error

  HFUNCoNUM% = HAoCOPYoPSoTOoSTR%            ' Issue Copy PS 2 str

  HDSoLEN% = MAXoDATAoSIZE                   ' Only copy the
                                             ' first 1920 bytes of the PS

  HRC% = 1                                   ' Set PS position to
                                             ' top,left corner.
  HDATAoSTR$ = SPACE$(3840)

  CALL BLIM(HFUNCoNUM%, HDATAoSTR$, HDSoLEN%, HRC%)

  IF HRC% <> HARCoSUCCESS GOTO DHSoERROR

  CLS

' CALL VIOWRTCELLSTR(BYVAL VARSEG(HDATAoSTR$),BYVAL SADD(HDATAoSTR$),BYVAL MAXoSIZE%,BYVAL ZERO%,BYVAL ZERO%,BYVAL ZERO%)

' PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT
' PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT:PRINT

  J% = 2
  FOR I% = 1 TO 3839 STEP 2

    COLOR ASC(MID$(HDATAoSTR$,J%,1))
    PRINT MID$(HDATAoSTR$,I%,1);

    J% = J% + 2

  NEXT
  COLOR 7   'Get back white forground color.

  GOTO DHSoRET

DHSoERROR:

  GOSUB ERRORoHAND

DHSoRET:
  RETURN




'********************************************************************
' CHANGE WINDOW NAME - Change the title name on the PM window       *
'                                                                   *
'                                                                   *
'                                                                   *
'********************************************************************
CHANGEoWINDOWoNAME:                          ' Call routine to
                                             ' change PM window name




  HFUNCoNUM% = HAoCONNECToPMoSRVCS%          ' Issue Connect
                                             ' Presentation Space
  CALL BLIM(HFUNCoNUM%, DFToSESS$, HDSoLEN%, HRC%)
                                             ' Call EHLLAPI.
  IF HRC% <> HARCoSUCCESS THEN GOTO WNERROR

  HFUNCoNUM% = HAoCHANGEoWINDOWoNAME%        ' Issue change window name.

  HDATAoSTR$ = DFToSESS$ + CHR$(1) + TESToNAME$                                                                                     '     String to send to host.

  HDSoLEN% = LEN(TESToNAME$) + 3             ' Set length of string.

  CALL BLIM(HFUNCoNUM%, HDATAoSTR$, HDSoLEN%, HRC%)
                                             ' Call EHLLAPI.
WNERROR:

  IF HRC% = HARCoSUCCESS THEN

    PRINT "Window Title Changed."
    PRINT "(press CONTROL-ESCAPE to verify)"
    PRINT
    PRINT

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN



'********************************************************************
' CHANGE SWITCH LIST NAME - Change the window's name on the         *
'                                               switch list         *
'                                                                   *
'                                                                   *
'********************************************************************
CHANGEoSWITCHoLISToNAME:                     ' Call routine to
                                             ' change switch list name.



  HFUNCoNUM% = HAoCONNECToPMoSRVCS%          ' Issue Connect
                                             ' Presentation Space.
  CALL BLIM(HFUNCoNUM%, DFToSESS$, HDSoLEN%, HRC%)
                                             ' Call EHLLAPI.
  IF HRC% <> HARCoSUCCESS THEN GOTO SLNERROR

  HFUNCoNUM% = HAoCHANGEoSWITCHoNAME%        ' Issue change switch name.

  HDATAoSTR$ = DFToSESS$ + CHR$(1) + TESToNAME$ ' String to send to host.

  HDSoLEN% = LEN(TESToNAME$) + 3             ' Set length of string.

  CALL BLIM(HFUNCoNUM%, HDATAoSTR$, HDSoLEN%, HRC%)
                                             ' Call EHLLAPI.
SLNERROR:

  IF HRC% = HARCoSUCCESS THEN

    PRINT "Switch List Title Changed."
    PRINT "(press CONTROL-ESCAPE to verify)"
    PRINT
    PRINT

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN





'********************************************************************
' QUERY PM SESSION - Obtain PM session information                  *
'                                                                   *
'                                                                   *
'                                                                   *
'********************************************************************
QUERYoPMoSESSION:                            ' Call routine to
                                             ' query PM session.

  HFUNCoNUM% = HAoCONNECToPMoSRVCS%          ' Issue Connect
                                             ' Presentation Space.
  CALL BLIM(HFUNCoNUM%, DFToSESS$, HDSoLEN%, HRC%)

  IF HRC% <> HARCoSUCCESS THEN GOTO QPMERROR

  HFUNCoNUM% = HAoPMoWINDOWoSTATUS%          ' Issue query status.

  HDATAoSTR$ = DFToSESS$ + CHR$(2)+ SPACE$(15) ' String to send to host.

  HDSoLEN% = 17

  CALL HLLAPI(SEG HFUNCoNUM%,BYVAL VARSEG(HDATAoSTR$),BYVAL SADD(HDATAoSTR$), SEG HDSoLEN%,SEG HRC%)

  POS3% = ASC(MID$(HDATAoSTR$,3,1))

  POS4% = ASC(MID$(HDATAoSTR$,4,1))

  IF HRC% <> HARCoSUCCESS GOTO QPMERROR

  PRINT
  PRINT "                   PM SESSION STATUS "
  PRINT

  IF ( POS3% AND &H08 )   THEN PRINT "STATUS :      The window is visible. "

  IF ( POS3% AND &H10 )   THEN PRINT "STATUS :      The window is invisible. "

  IF ( POS3% AND &H80 )   THEN PRINT "STATUS :      The window is activated. "

  IF ( POS4% AND &H01 )   THEN PRINT "STATUS :      The window is deactivated. "

  IF ( POS4% AND &H04 )   THEN PRINT "STATUS :      The window is minimized."

  IF ( POS4% AND &H08 )   THEN PRINT "STATUS :      The window is maximized."

  HFUNCoNUM% = HAoQUERYoWINDOWoCOORDS%       ' Issue change switch name.

  HDATAoSTR$ = DFToSESS$   + SPACE$(16)

  HDSoLEN% = 17

  CALL HLLAPI(SEG HFUNCoNUM%,BYVAL VARSEG(HDATAoSTR$),BYVAL SADD(HDATAoSTR$), SEG HDSoLEN%,SEG HRC%)

  IF HRC% = HARCoSUCCESS THEN

  PRINT
  PRINT "                   PM WINDOW COORDINATES "
  PRINT

  SDATA$=HDATAoSTR$                          ' initialize strings for
  DLEN%=HDSoLEN%                             ' conversion
  GOSUB CVBIN2HEX                            ' convert binary to hex

  PRINT"XLEFT   : "; MID$(HXSDATA$,9,2);  MID$(HXSDATA$,7,2); MID$(HXSDATA$,5,2);  MID$(HXSDATA$,3,2)

  PRINT"YBOTTOM : "; MID$(HXSDATA$,17,2); MID$(HXSDATA$,15,2); MID$(HXSDATA$,13,2); MID$(HXSDATA$,11,2)

  PRINT"XRIGHT  : "; MID$(HXSDATA$,25,2); MID$(HXSDATA$,23,2); MID$(HXSDATA$,21,2); MID$(HXSDATA$,19,2)

  PRINT"YTOP    : "; MID$(HXSDATA$,33,2); MID$(HXSDATA$,31,2); MID$(HXSDATA$,29,2); MID$(HXSDATA$,27,2)

  PRINT
  PRINT
  PRINT "(press CONTROL-ESCAPE to verify)"
  PRINT
  PRINT
  ELSE

QPMERROR:
    GOSUB ERRORoHAND

ENDIF

RETURN




'********************************************************************
' MAKE PM INVISIBLE - Make the screen invisible                     *
'                                                                   *
'                                                                   *
'                                                                   *
'********************************************************************
MAKEoPMoINVISIBLE:                           ' Call routine to
                                             ' change switch list name.


  HFUNCoNUM% = HAoCONNECToPMoSRVCS%          ' Issue Connect
                                             ' Presentation Space.
  CALL BLIM(HFUNCoNUM%, DFToSESS$, HDSoLEN%, HRC%)  ' Call EHLLAPI

  IF HRC% <> HARCoSUCCESS THEN GOTO MPMERROR

  HFUNCoNUM% = HAoPMoWINDOWoSTATUS%          ' Issue change pm status.

  HDATAoSTR$ = DFToSESS$+ CHR$(1)+ CHR$(16)+ CHR$(0) ' Set invisible bit.

  CALL BLIM(HFUNCoNUM%, HDATAoSTR$, HDSoLEN%, HRC%)  ' Call EHLLAPI.

MPMERROR:

  IF HRC% = HARCoSUCCESS THEN
    PRINT
    PRINT "The PM session is now invisible."
    PRINT "(press CONTROL-ESCAPE to verify)"
    PRINT
    PRINT

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN





'********************************************************************
' MAKE PM WINDOW VISIBLE - Change window status to visible          *
'                                            and maximized          *
'                                                                   *
'                                                                   *
'********************************************************************
MAKEoPMoVISIBLE:                             ' Call routine to make
                                             ' session visible and maximized.


  HFUNCoNUM% = HAoCONNECToPMoSRVCS%          ' Issue Connect
                                             ' Presentation Space.
  CALL BLIM(HFUNCoNUM%, DFToSESS$, HDSoLEN%, HRC%)
                                             ' Call EHLLAPI.
  IF HRC% <> HARCoSUCCESS THEN GOTO MWVERROR

  HFUNCoNUM% = HAoPMoWINDOWoSTATUS%          ' Issue change pm status.

  HDATAoSTR$ = DFToSESS$ + CHR$(1)+ CHR$(8)+ CHR$(8) ' Set visible bit.

  CALL BLIM(HFUNCoNUM%, HDATAoSTR$, HDSoLEN%, HRC%)  ' Call EHLLAPI.

MWVERROR:

  IF HRC% = HARCoSUCCESS THEN

    PRINT "The PM session is now visible and maximized."
    PRINT "(press CONTROL-ESCAPE to verify)"
    PRINT
    PRINT

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN





'********************************************************************
' DISCONNECT PM - Disconnect from the Presentation Manager session  *
'                                                                   *
'                                                                   *
'                                                                   *
'********************************************************************
DISCONNECToPM:                               ' Call routine to
                                             ' change switch list name.


  HFUNCoNUM% = HAoCONNECToPMoSRVCS%          ' Issue Connect
                                             ' Presentation Space.
  CALL BLIM(HFUNCoNUM%, DFToSESS$, HDSoLEN%, HRC%)

  IF HRC% = HARCoSUCCESS THEN

     HFUNCoNUM% = HAoDISCONNECToPMoSRVCS%    ' Issue change pm status.

     CALL BLIM(HFUNCoNUM%, DFToSESS$, HDSoLEN%, HRC%)  ' Call EHLLAPI.

  ENDIF

  IF HRC% = HARCoSUCCESS THEN
    PRINT
    PRINT "PM Window Disconnected."
    PRINT

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN





'********************************************************************
' RESTORE PM NAMES - Restore window name & size and switch name     *
'                                                                   *
'                                                                   *
'                                                                   *
'********************************************************************
RESTOREoPMoNAMES:


  HFUNCoNUM% = HAoCONNECToPMoSRVCS%          ' Issue Connect PS

  CALL BLIM(HFUNCoNUM%, DFToSESS$, HDSoLEN%, HRC%)

  IF HRC% <> HARCoSUCCESS THEN GOTO RSTERROR

  HFUNCoNUM% = HAoCHANGEoSWITCHoNAME%        ' Issue change pm status.

  HDATAoSTR$ = DFToSESS$ + CHR$(2)           ' Set reset bit.

  HDSoLEN% = 2

  CALL BLIM(HFUNCoNUM%, HDATAoSTR$, HDSoLEN%, HRC%)  ' Call EHLLAPI.

  IF HRC% <> HARCoSUCCESS THEN GOTO RSTERROR

  PRINT
  PRINT "Switch Name Restored."

  HFUNCoNUM% = HAoCHANGEoWINDOWoNAME%        ' Issue change window status.

  HDATAoSTR$ = DFToSESS$ + CHR$(2)           ' Set reset title bit.

  HDSoLEN% = 2

  CALL BLIM(HFUNCoNUM%, HDATAoSTR$, HDSoLEN%, HRC%) ' Call EHLLAPI

  IF HRC% <> HARCoSUCCESS THEN GOTO RSTERROR

  PRINT "PM Window Name Restored."

  HFUNCoNUM% = HAoPMoWINDOWoSTATUS%          ' Issue change pm status.

  HDATAoSTR$ = DFToSESS$ + CHR$(1)  + CHR$(0) + CHR$(16)
                                             ' Set restore window
  HDSoLEN% = 4

  CALL BLIM(HFUNCoNUM%, HDATAoSTR$, HDSoLEN%, HRC%)  ' Call EHLLAPI.

RSTERROR:

  IF HRC% = HARCoSUCCESS THEN

    PRINT "PM Window Size Restored."
    PRINT "(press CONTROL-ESCAPE to verify)"
    PRINT
    PRINT

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN




'********************************************************************
' RESET SYSTEM - Reset EHLLAPI to its original conditions           *
'                                                                   *
'                                                                   *
'                                                                   *
'********************************************************************
RESEToSYSTEM :                                ' Call routine to
                                              ' reset EHLLAPI


  HFUNCoNUM% = HAoRESEToSYSTEM%               ' Issue Reset System

  CALL BLIM(HFUNCoNUM%, HDATAoSTR$, HDSoLEN%, HRC%)

  IF HRC% = HARCoSUCCESS THEN
    PRINT "EHLLAPI Reset To Original Conditions."
    PRINT

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN




'********************************************************************
' QUERY COMMUNICATIONS BUFFER - Determine the optimal and maximum   *
'                               buffer sizes                        *
'                                                                   *
'                                                                   *
'********************************************************************
QUERYoCOMoBUFFER :


  HFUNCoNUM% = HAoQUERYoBUFFERoSIZE%         ' Issue Query Buffer

  HDSoLEN% = 9

  QRYoSTR.QBUFoSHORTNAME = STRING$(1,DFToSESS$)

  PRINT "Query The Communication Buffer Size."

  CALL HLLAPI (SEG HFUNCoNUM%,BYVAL VARSEG (QRYoSTR),BYVAL VARPTR (QRYoSTR), SEG HDSoLEN%, SEG HRC% )

  IF HRC% = HARCoSUCCESS THEN
    PRINT "Optimal Inbound Buffer Size:  ";QRYoSTR.QBUFoOPToINB
    PRINT "Maximum Inbound Buffer Size:  ";QRYoSTR.QBUFoMAXoINB
    PRINT "Optimal Outbound Buffer Size: ";QRYoSTR.QBUFoOPToOUTB
    PRINT "Maximum Outbound Buffer Size: ";QRYoSTR.QBUFoMAXoOUTB
    PRINT "Query Communications Buffer Complete."
    PRINT

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN



'********************************************************************
' ALLOCATE COMMUNICATIONS BUFFER - Allocate a communciations buffer *
'                                  for structured field transfer    *
'                                                                   *
'                                                                   *
'********************************************************************
ALLOCATEoCOMoBUFFER :                        ' Call routine to allocate
                                             ' a communications buffer


  HFUNCoNUM% = HAoALLOCATEoCOMMOoBUFF%

  HDSoLEN% = 6

  ALLOCATEoSTR.ABUFoLENGTH  = BUFFERoLENGTH

  ALLOCATEoSTR.ABUFoADDRESS = 0

  PRINT "Buffer Length : "; ALLOCATEoSTR.ABUFoLENGTH

  CALL HLLAPI (SEG HFUNCoNUM%,BYVAL VARSEG (ALLOCATEoSTR),BYVAL VARPTR (ALLOCATEoSTR), SEG HDSoLEN%, SEG HRC% )

  IF HRC% = HARCoSUCCESS THEN
    PRINT "Allocate Communications Buffer Complete."
    PRINT

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN





'********************************************************************
' CONNECT TO THE COMMUNICATIONS BUFFER - Connect to the             *
'                                        communication buffer       *
'                                                                   *
'                                                                   *
'********************************************************************
CONNECToCOMoBUFFER :                         ' Call routine to connect
                                             ' to the communications buffer


  QRSTR(1) = CHR$(0)                         ' DDM Query Reply Code
  QRSTR(2) = CHR$(12)
  QRSTR(3) = CHR$(129)
  QRSTR(4) = CHR$(149)
  QRSTR(5) = CHR$(0)
  QRSTR(6) = CHR$(0)
  QRSTR(7) = CHR$(1)
  QRSTR(8) = CHR$(0)
  QRSTR(9) = CHR$(1)
  QRSTR(10) = CHR$(0)
  QRSTR(11) = CHR$(1)
  QRSTR(12) = CHR$(1)

  HFUNCoNUM% = HAoSTARToSTRUCTUREDoFLD%

  HDSoLEN% = 11

  CONNECToSTR.STSFoSHORTNAME =  STRING$(1,DFToSESS$)

  QRoSEG& = CLNG (VARSEG(QRSTR(1)))
  QRoOFF& = CLNG (VARPTR(QRSTR(1)))

  QRoSEG1& = QRoSEG&  * CLNG(256)
  QRoSEG2& = QRoSEG1& * CLNG(256)

  CONNECToSTR.STSFoQUERY = QRoSEG2&  + QRoOFF&

  PRINT
  PRINT "Session Shortname : "; CONNECToSTR.STSFoSHORTNAME


  CALL HLLAPI (SEG HFUNCoNUM%,BYVAL VARSEG (CONNECToSTR),BYVAL VARPTR (CONNECToSTR), SEG HDSoLEN%, SEG HRC% )

  IF HRC% = HARCoSUCCESS THEN
    DOID% = CONNECToSTR.STSFoDOID
    PRINT "Destination / Origin ID :",HEX$(DOID%)
    PRINT "Connection made to the Communications Buffer."
    PRINT

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN



'********************************************************************
' READ ASYNCHRONUS STRUCTURED FIELD - Read asynchronusly from the   *
'                                     communications buffer         *
'                                                                   *
'********************************************************************
READoSFoASYNC :

  HFUNCoNUM% = HAoREADoSTRUCTUREDoFLD%

  HDSoLEN% = 14

  READoSTR.RDSFoSHORTNAME =  STRING$(1,DFToSESS$)

  READoSTR.RDSFoOPTION = CHR$(65)            ' A for asynchronus

  READoSTR.RDSFoDOID   = DOID%

  READoSTR.RDSFoBUFFER = RDoADDRESS

  READoSTR.RDSFoREQUESTID = 0

  READoSTR.RDSFoASEM      = 0



  TWOo16&  = CLNG(65536)                    ' Calculate segment and offset
                                            ' to read buffer
  RDoSEG&  = RDoADDRESS / TWOo16&

  RDoSEG%  = RDoSEG&

  RDoOFF&  = RDoADDRESS - (RDoSEG& * TWOo16&)

  RDoOFF%  = RDoOFF&


  DEF SEG  = RDoSEG%

  FOR INDEX%  = 0 TO 9
     RDoADD%  = RDoOFF% + INDEX%
     POKE  RDoADD%, 0
  NEXT

  POKE (RDoOFF% + 5 ),  5                      ' buffer size 5(00)

  PRINT "Initiate Read Asynchronus Structured Field."
  PRINT "Session Shortname : ";  READoSTR.RDSFoSHORTNAME
  PRINT "Destination / Origin ID : "; HEX$(READoSTR.RDSFoDOID)
  PRINT "Read Option :  ";  READoSTR.RDSFoOPTION

  CALL HLLAPI (SEG HFUNCoNUM%,BYVAL VARSEG (READoSTR),BYVAL VARPTR (READoSTR), SEG HDSoLEN%, SEG  HRC% )


  IF (HRC% = HARCoINBOUNDoDISABLED%) THEN   ' successful case
    REQUESTID% = READoSTR.RDSFoREQUESTID
    SFoASEM&   = READoSTR.RDSFoASEM
    PRINT "Read Structured Field Completed."
    PRINT
    HRC% = 0

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN




'********************************************************************
' CREATEoSTRUCTUREDoFIELD  - Connects to first session and writes   *
'                            homekey and ind$file command to host   *
'                                                                   *
'                                                                   *
'********************************************************************
CREATEoSTRUCTUREDoFIELD:


  HFUNCoNUM% = HAoCONNECToPS%                ' Issue Connect
                                             ' Presentation Space.

  CALL BLIM(HFUNCoNUM%, DFToSESS$, HDSoLEN%, HRC%)
                                             ' Call EHLLAPI.

  IF HRC% <> HARCoSUCCESS THEN GOTO CNSFoERROR

  HFUNCoNUM% = HAoSENDKEY%                   ' Issue sendkey.

  HOMEoKEY$ = "@L@0"                         ' String to send to host

  HDSoLEN% = LEN(HOMEoKEY$)                  ' Set length of string
                                             ' minus null char

  CALL BLIM(HFUNCoNUM%, HOMEoKEY$, HDSoLEN%, HRC%)
                                             ' Call EHLLAPI.

  IF HRC% <> HARCoSUCCESS THEN GOTO CNSFoERROR


  HDSoLEN% = LEN(COMMANDoTEXT$)              ' Set length of string
                                             ' minus null char

  CALL BLIM(HFUNCoNUM%, COMMANDoTEXT$, HDSoLEN%, HRC%)

  IF HRC% = HARCoSUCCESS THEN GOTO CNSFoRET

CNSFoERROR:
  GOSUB ERRORoHAND

CNSFoRET:
  PRINT
  PRINT "Structured Field Created."
  PRINT
  RETURN




'********************************************************************
' GEToCOMPLETIONoREQUEST - Check for completion of an asynchronus   *
'                                                         process   *
'                                                                   *
'********************************************************************
GEToASYNCoCOMPLETE :

  HFUNCoNUM% = HAoGEToASYNCoCOMPLETION%

  HDSoLEN% = 14

  GREQoSTR.GCMPoSHORTNAME =   STRING$(1,DFToSESS$)

  GREQoSTR.GCMPoOPTION    =   CHR$(87)      ' W for wait option

  GREQoSTR.GCMPoREQUESTID =   REQUESTID%

  GREQoSTR.GCMPoREToFUNCTID = 0

  GREQoSTR.GCMPoREToDATASTR = 0

  GREQoSTR.GCMPoREToLENGTH  = 0

  GREQoSTR.GCMPoREToRETCODE = 0

  PRINT "Checking For Asynchronus Completion."

  PRINT "Session Shortname : "; GREQoSTR.GCMPoSHORTNAME

  CALL HLLAPI (SEG HFUNCoNUM%,BYVAL VARSEG (GREQoSTR),BYVAL VARPTR (GREQoSTR), SEG HDSoLEN%, SEG HRC% )

  IF HRC% = HARCoSUCCESS THEN

    PRINT "Completion Request Successful."
    PRINT

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN




'********************************************************************
' WRITE SYNCHRONUS STRUCTURED FIELD - Write a structured field      *
'                         synchronusly to the communications buffer *
'                                                                   *
'********************************************************************
WRITEoSFoSYNC :

  HFUNCoNUM% = HAoWRITEoSTRUCTUREDoFLD%

  HDSoLEN% = 8

  WRITEoSTR.WRSFoSHORTNAME = STRING$(1,DFToSESS$)

  WRITEoSTR.WRSFoOPTION    = CHR$(83)        ' S for synchronous

  WRITEoSTR.WRSFoDOID      = DOID%           ' Destination / Origin ID

  WRITEoSTR.WRSFoBUFFER    = WRoADDRESS

  PRINT "Initiate Write Synchronus Structured Field."

  PRINT "Session Shortname : "; WRITEoSTR.WRSFoSHORTNAME

  PRINT "Write Option : ";      WRITEoSTR.WRSFoOPTION

  PRINT "Destination / Origin ID : "; HEX$(WRITEoSTR.WRSFoDOID)


  TWOo16&  = CLNG(65536)                    ' Calculate segemnt and offset
  WRoSEG&  = WRoADDRESS / TWOo16&           ' of the write buffer
  WRoSEG%  = WRoSEG&
  WRoOFF&  = WRoADDRESS - (WRoSEG& * TWOo16&)
  WRoOFF%  = WRoOFF&

  DEF SEG  = WRoSEG%

  FOR INDEX%  = 0 TO 1279
     WRoADD%  = WRoOFF% + INDEX%           ' Clear write buffer contents
     POKE  WRoADD%, 0
  NEXT
                                           ' Write buffer header
  POKE (WRoOFF% + 2),  5
  POKE (WRoOFF% + 9),  5
  POKE (WRoOFF% + 10), 208                 ' destination / origin ID
  POKE (WRoOFF% + 12), 9


  CALL HLLAPI (SEG HFUNCoNUM%,BYVAL VARSEG (WRITEoSTR),BYVAL VARPTR (WRITEoSTR), SEG HDSoLEN%, SEG HRC% )

  IF HRC% = HARCoSUCCESS% THEN
    PRINT "Write Structured Field Completed."
    PRINT

    CALL  DosSleep(BYVAL 0, BYVAL 5000 )   ' Give Host time to clear
                                           ' outstanding responses
  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN



'********************************************************************
' FREE COMMUNICATIONS BUFFER- Return to the shared memory pool the  *
'                       communications buffers no longer being used *
'                                                                   *
'********************************************************************
FREEoCOMMOoBUFF:

  HFUNCoNUM% = HAoFREEoCOMMOoBUFF%

  HDSoLEN% = 6

  FREEoBUFFoSTR.FBUFoADDRESS = RDoADDRESS

  FREEoBUFFoSTR.FBUFoLENGTH  = QRYoSTR.QBUFoOPToINB

  CALL HLLAPI (SEG HFUNCoNUM%,BYVAL VARSEG (FREEoBUFFoSTR),BYVAL VARPTR (FREEoBUFFoSTR), SEG HDSoLEN%, SEG HRC% )

  IF HRC% = HARCoSUCCESS8 THEN

    PRINT
    PRINT "Read Buffer De-Allocated."

  ELSE

    GOSUB ERRORoHAND

  ENDIF

  FREEoBUFFoSTR.FBUFoADDRESS = WRoADDRESS

  FREEoBUFFoSTR.FBUFoLENGTH  =QRYoSTR.QBUFoOPToOUTB

  CALL HLLAPI (SEG HFUNCoNUM%,BYVAL VARSEG (FREEoBUFFoSTR),BYVAL VARPTR (FREEoBUFFoSTR), SEG HDSoLEN%, SEG HRC% )

  IF HRC% = HARCoSUCCESS8 THEN

    PRINT "Write Buffer De-Allocated."
    PRINT

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN





'********************************************************************
' DISCONNECT FROM THE STRUCTURED FIELD - Disconnect from the        *
'                                        communications buffer      *
'                                                                   *
'********************************************************************
DISCONNECToFROMoCOMoBUFFER :

  HFUNCoNUM% = HAoSTOPoSTRUCTUREDoFLD%

  HDSoLEN% = 3

  DISCONNECToSTR.SPSFoSHORTNAME =  STRING$(1,DFToSESS$)

  DISCONNECToSTR.SPSFoDOID = DOID%

  PRINT "Initiate Disconnect From Structured Field."

  PRINT "Session Shortname : "; DISCONNECToSTR.SPSFoSHORTNAME

  PRINT "Destination / Origin ID : "; HEX$(DISCONNECToSTR.SPSFoDOID)

  CALL HLLAPI (SEG HFUNCoNUM%,BYVAL VARSEG (DISCONNECToSTR),BYVAL VARPTR (DISCONNECToSTR), SEG HDSoLEN%, SEG HRC% )

  IF HRC% = HARCoSUCCESS THEN

    PRINT "Communications Buffer Disconnected."
    PRINT

  ELSE

    GOSUB ERRORoHAND

  ENDIF

RETURN




'********************************************************************
' ERRORoHAND - Error handler.                                       *
'                                                                   *
'********************************************************************
ERRORoHAND:                         '     Error handler.

  PRINT "UNEXPECTED RETURN CODE "; HRC%;" from FUNCTION #";HFUNCoNUM%;"."

  INPUT "PRESS ENTER TO EXIT ...", X$


  RETURN


'***********************************************************************
'*                                                                     *
'*    BUILDTABLES :                                                    *
'*             initializes hex values at the beginning of the program. *
'*             This is done before calling CVBIN2HEX.                  *
'*                                                                     *
'***********************************************************************

BUILDTABLES:

  DIM BIN2HEX%(16)
  DATA &H30%,&H31%,&H32%,&H33%,&H34%,&H35%,&H36%,&H37%,&H38%,&H39%,&H41%
  DATA &H42%,&H43%,&H44%,&H45%,&H46%

  FOR I=1 TO 16
     READ BIN2HEX%(I)
  NEXT I

RETURN



'********************************************************************
'*                                                                  *
'*     CVBIN2HEX :                                                  *
'*            This routine is to converse a string of binary values *
'*            into hex.                                             *
'*                                                                  *
'********************************************************************

CVBIN2HEX :
   HXSDATA$ = SPACE$(2*DLEN%)
   SRC%=SADD(SDATA$)
   TRG%=SADD(HXSDATA$)
   TNDX%=0
   SNDX%=0
   WHILE SNDX% < DLEN%
         TMP1% = PEEK(SRC%+SNDX%)\16
         TMP2% = PEEK(SRC%+SNDX%) - (TMP1%*16)
         POKE TRG%+TNDX%, BIN2HEX%(TMP1%+1)
         POKE TRG%+TNDX%+1, BIN2HEX%(TMP2%+1)
         SNDX%=SNDX%+1
         TNDX%=TNDX%+2
  WEND
  DLEN%=TNDX%
RETURN
