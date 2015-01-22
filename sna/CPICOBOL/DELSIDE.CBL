
       IDENTIFICATION DIVISION.
       PROGRAM-ID.        DELSIDE.
      ***************************************************************
      * THIS PROGRAM IS AN EXAMPLE OF THE FUNCTION AVAILABLE        *
      * THROUGH THE CPI-C EXTENSIONS PROVIDED.                      *
      *                                                             *
      * 86F2503 (C) COPYRIGHT IBM CORP. 1990, 1991                  *
      * LICENSED MATERIAL - PROGRAM PROPERTY OF IBM                 *
      * ALL RIGHTS RESERVED                                         *
      *                                                             *
      * PURPOSE: DELETE CPI-C SIDE INFORMATION AND DISPLAY RESULT   *
      *                                                             *
      * INPUT:   SIDE-INFORMATION STRUCTURE.                        *
      *                                                             *
      * OUTPUT:  CPI-C SIDE INFORMATION TABLE IS DELETED            *
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

          PERFORM DELETE-SIDE-INFO
             THRU DELETE-SIDE-INFO-EXIT.
           PERFORM CLEANUP
              THRU CLEANUP-EXIT.
           STOP RUN.
      ***********************************************
      * DELETE SIDE-INFO                            *
      ***********************************************
        DELETE-SIDE-INFO.
           MOVE "CREDRPT" TO SI-SYM-DEST-NAME.
           CALL "XCMDSI"  USING TEST-KEY
                          SI-SYM-DEST-NAME
                          CM-RETCODE.

          IF CM-OK THEN
            DISPLAY "SIDE INFO DELETED"
          ELSE
            MOVE "FAILURE TO DELETE SIDE-INFO"
                  TO CM-ERROR-DISPLAY-MSG
            PERFORM CLEANUP
             THRU CLEANUP-EXIT
          END-IF.
        DELETE-SIDE-INFO-EXIT. EXIT.
      ***********************************************
      * DISPLAY EXECUTION COMPLETE OR ERROR MESSAGE *
      ***********************************************
        CLEANUP.
          IF CM-OK THEN
            DISPLAY "PROGRAM: DELETE SIDE EXECUTION COMPLETE"
          ELSE
            DISPLAY "DELSIDE PROGRAM - ",
                      CM-ERROR-DISPLAY-MSG, " RC= ", CM-RETCODE
            IF  CM-PROGRAM-PARAMETER-CHECK
              DISPLAY "----------------------------------------------"
              DISPLAY "THIS ERROR CAN RESULT FROM RUNNING DELSIDE"
              DISPLAY "WHEN SIDE INFORMATION HAS ALREADY BEEN DELETED"
              DISPLAY "----------------------------------------------"
            END-IF
         END-IF.
           STOP RUN.
       CLEANUP-EXIT. EXIT.
      *****************************************************************
