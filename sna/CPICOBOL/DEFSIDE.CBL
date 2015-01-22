
       IDENTIFICATION DIVISION.
       PROGRAM-ID.        DEFSIDE.
      ***************************************************************
      * THIS PROGRAM IS AN EXAMPLE OF THE FUNCTION AVAILABLE        *
      * THROUGH THE CPI-C EXTENSIONS PROVIDED.                      *
      *                                                             *
      * 86F2503 (C) COPYRIGHT IBM CORP. 1990, 1991                  *
      * LICENSED MATERIAL - PROGRAM PROPERTY OF IBM                 *
      * ALL RIGHTS RESERVED                                         *
      *                                                             *
      * PURPOSE: DEFINE CPI-C SIDE INFORMATION AND DISPLAY RESULT   *
      *                                                             *
      * INPUT:   SIDE-INFORMATION STRUCTURE.                        *
      *                                                             *
      * OUTPUT:  CPI-C SIDE INFORMATION TABLE IS UPDATED TO         *
      *          REFLECT INPUT STRUCTURE.                           *
      *                                                             *
      * NOTE:    FOR THIS SAMPLE PROGRAM, THE KEY FIELD (TEST-KEY), *
      *          SUPPORTING THE OS/2 EE & ES COMMUNICATIONS MANAGER *
      *          KEYLOCK FEATURE, IS SET TO SPACES.  AS A RESULT,   *
      *          THIS PROGRAM WILL RUN SUCCESSFULLY ONLY WHEN       *
      *          THE KEYLOCK FEATURE IS NOT SECURED.                *
      *                                                             *
      ***************************************************************
      *
       ENVIRONMENT DIVISION.
       CONFIGURATION SECTION.
       SOURCE-COMPUTER. PS-2.
       OBJECT-COMPUTER. PS-2.
       SPECIAL-NAMES.
       INPUT-OUTPUT SECTION.
       FILE-CONTROL.
       I-O-CONTROL.
      *
       DATA DIVISION.
       FILE SECTION.
       WORKING-STORAGE SECTION.

       01  TEST-KEY                   PIC  X(1)   VALUE SPACES.
       01  TEST-ENTRY-NUMBER          PIC  9(9)   VALUE 0 COMP-4.

       01  CM-ERROR-DISPLAY-MSG       PIC  X(40)  VALUE SPACES.

      **********************************************
      * USE THE CPI-COMMUNICATIONS PSEUDONYM FILES *
      **********************************************
           COPY CMCOBOL.

       LINKAGE SECTION.

       EJECT
      *
       PROCEDURE DIVISION.
      ******************************************************************
      **************************  START OF MAINLINE  *******************
      ******************************************************************
       MAINLINE.

           PERFORM SIDE-INITIALIZE
              THRU SIDE-INITIALIZE-EXIT.
           PERFORM SIDE-DISPLAY
              THRU SIDE-DISPLAY-EXIT.
           PERFORM CLEANUP
              THRU CLEANUP-EXIT.
           STOP RUN.
      ******************************************************************
      ***************************  END OF MAINLINE  ********************
      ******************************************************************
      *
       SIDE-INITIALIZE.
           INITIALIZE SIDE-INFO-ENTRY REPLACING NUMERIC BY 0
                                           ALPHABETIC BY " ".
           MOVE "CREDRPT" TO SI-SYM-DEST-NAME.
      ******************************************************************
      * CHANGE THE SI-PARTNER-LU-NAME TO MATCH YOUR CONFIGURATION      *
      ******************************************************************
           MOVE "NET1.ENLU" TO SI-PARTNER-LU-NAME.
           SET SI-APPLICATION-TP TO TRUE.
           MOVE "CREDRPT " TO SI-TP-NAME.
           MOVE "#INTER" TO SI-MODE-NAME.
           MOVE 124 TO SIDE-INFO-LEN.
           SET SI-SECURITY-NONE TO TRUE.
           CALL "XCMSSI"  USING  TEST-KEY
                          SIDE-INFO-ENTRY
                          SIDE-INFO-LEN
                          CM-RETCODE.
      *
           IF CM-OK
             DISPLAY "SIDE-INFO CREATED"
           ELSE
              MOVE "FAILURE TO CREATE SIDE-INFO"
                   TO CM-ERROR-DISPLAY-MSG
              PERFORM CLEANUP
                 THRU CLEANUP-EXIT
           END-IF.
       SIDE-INITIALIZE-EXIT. EXIT.
      *****************************************************************
      * CLEAR THE SIDE-INFO CONTROL BLOCK FOR TESTING PURPOSES
      * THEN ISSUE DISPLAY REQUEST
      *****************************************************************
       SIDE-DISPLAY.
           INITIALIZE SIDE-INFO-ENTRY REPLACING NUMERIC BY 0
                                           ALPHABETIC BY " ".
           DISPLAY "EXTRACTING NEWLY DEFINED SIDE INFORMATION".
           MOVE "CREDRPT" TO SI-SYM-DEST-NAME.
           MOVE 124 TO SIDE-INFO-LEN.
           CALL "XCMESI"  USING TEST-ENTRY-NUMBER
                                SI-SYM-DEST-NAME
                                SIDE-INFO-ENTRY
                                SIDE-INFO-LEN
                                CM-RETCODE.
    *
          IF CM-OK THEN
            DISPLAY "-------------------------"
            DISPLAY "SIDE INFORMATION OBTAINED"
            DISPLAY "-------------------------"
            DISPLAY "PARTNER TP NAME = " SI-TP-NAME
            DISPLAY "PARTNER LU NAME = " SI-PARTNER-LU-NAME
            DISPLAY "MODE NAME       = " SI-MODE-NAME
          ELSE
            MOVE "FAILURE DURING SIDE-INFO DISPLAY"
                  TO CM-ERROR-DISPLAY-MSG
            PERFORM CLEANUP
            THRU CLEANUP-EXIT
          END-IF.
        SIDE-DISPLAY-EXIT. EXIT.
      ***********************************************
      * DISPLAY EXECUTION COMPLETE OR ERROR MESSAGE *
      * NOTE: CREDRPT WILL DEALLOCATE CONVERSATION  *
      ***********************************************
        CLEANUP.
           IF CM-ERROR-DISPLAY-MSG  = SPACES
               DISPLAY "PROGRAM: DEFSIDE EXECUTION COMPLETE"
           ELSE
               DISPLAY "DEFSIDE PROGRAM - ",
                       CM-ERROR-DISPLAY-MSG, " RC= ", CM-RETCODE.
           STOP RUN.
       CLEANUP-EXIT. EXIT.
      *****************************************************************
