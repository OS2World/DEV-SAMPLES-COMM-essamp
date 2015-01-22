       IDENTIFICATION DIVISION.
       PROGRAM-ID.        SALESRPT.
      *************************************************************
      * THIS IS THE SALESRPT PROGRAM THAT SENDS DATA TO THE       *
      * CREDRPT PROGRAM FOR CREDIT BALANCE PROCESSING.            *
      *                                                           *
      * 86F2503 (C) COPYRIGHT IBM CORP. 1990, 1991                *
      * LICENSED MATERIAL - PROGRAM PROPERTY OF IBM               *
      * ALL RIGHTS RESERVED                                       *
      *                                                           *
      * PURPOSE: SEND A SALES-RECORD TO THE CREDRPT PROGRAM FOR   *
      *          CREDIT BALANCE PROCESSING, THEN RECEIVE AND      *
      *          DISPLAY A STATUS INDICATION FROM CREDRPT.        *
      *                                                           *
      * INPUT:   PROCESSING-RESULTS-RECORD FROM CREDRPT.          *
      *                                                           *
      * OUTPUT:  SALES-RECORD TO THE CREDRPT PROGRAM.             *
      *                                                           *
      *                                                           *
      * NOTE:    SALES-RECORD PROCESSING HAS BEEN GREATLY         *
      *          SIMPLIFIED IN THIS EXAMPLE.                      *
      *************************************************************
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
 
       01  BUFFER                     PIC  X(52)  VALUE SPACES.
 
       01  CM-ERROR-DISPLAY-MSG       PIC  X(40)  VALUE SPACES.
 
      ****************
      * SALES-RECORD *
      ****************
       01  SALES-RECORD.
           05  CUST-NUM                PIC X(4)      VALUE "0010".
           05  CUST-NAME               PIC X(20)     VALUE "XYZ INC.".
           05  FILLER                  PIC X(5)      VALUE SPACES.
           05  CREDIT-BALANCE          PIC S9(7)V99  VALUE 4275.50.
           05  CREDIT-LIMIT            PIC S9(7)V99  VALUE 5000.
           05  CREDIT-FLAG             PIC X         VALUE "1".
 
      *****************************
      * PROCESSING-RESULTS-RECORD *
      *****************************
       01  PROCESSING-RESULTS-RECORD   PIC X(25)  VALUE SPACES.
 
      **********************************************
      * USE THE CPI-COMMUNICATIONS PSEUDONYM FILE  *
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
 
           PERFORM APPC-INITIALIZE
              THRU APPC-INITIALIZE-EXIT.
           DISPLAY "SALESRPT CONVERSATION INITIALIZED".
 
           PERFORM APPC-ALLOCATE
              THRU APPC-ALLOCATE-EXIT.
           DISPLAY "SALESRPT CONVERSATION ALLOCATED".
 
           PERFORM APPC-SEND
              THRU APPC-SEND-EXIT.
           DISPLAY "SALESRPT DATA RECORD SENT".
 
           PERFORM APPC-RECEIVE
              THRU APPC-RECEIVE-EXIT
              UNTIL NOT CM-OK.
           DISPLAY "SALESRPT RESULTS RECORD RECEIVED".
 
           PERFORM CLEANUP
              THRU CLEANUP-EXIT.
           STOP RUN.
      ******************************************************************
      ***************************  END OF MAINLINE  ********************
      ******************************************************************
      *
       APPC-INITIALIZE.
           MOVE "CREDRPT" TO SYM-DEST-NAME.
      ****************************************************
      ** ESTABLISH DEFAULT CONVERSATION CHARACTERISTICS **
      ****************************************************
           CALL "CMINIT" USING CONVERSATION-ID
                               SYM-DEST-NAME
                               CM-RETCODE.
           IF CM-OK
              NEXT SENTENCE
           ELSE
              MOVE "INITIALIZATION PROCESSING TERMINATED"
                   TO CM-ERROR-DISPLAY-MSG
              PERFORM CLEANUP
                 THRU CLEANUP-EXIT.
       APPC-INITIALIZE-EXIT. EXIT.
      *****************************************************************
      *
       APPC-ALLOCATE.
      **********************************
      * ALLOCATE THE APPC CONVERSATION *
      **********************************
           CALL "CMALLC" USING CONVERSATION-ID
                               CM-RETCODE
           IF CM-OK
              NEXT SENTENCE
           ELSE
              MOVE "ALLOCATION PROCESSING TERMINATED"
                   TO CM-ERROR-DISPLAY-MSG
              PERFORM CLEANUP
                 THRU CLEANUP-EXIT.
       APPC-ALLOCATE-EXIT. EXIT.
      *****************************************************************
      *
       APPC-SEND.
           MOVE SALES-RECORD TO BUFFER.
           MOVE 52 TO SEND-LENGTH.
 
      *************************************
      * SEND THE SALES-RECORD DATA RECORD *
      *************************************
           CALL "CMSEND" USING CONVERSATION-ID
                               BUFFER
                               SEND-LENGTH
                               REQUEST-TO-SEND-RECEIVED
                               CM-RETCODE.
           IF CM-OK
              NEXT SENTENCE
           ELSE
              MOVE "SEND PROCESSING TERMINATED"
                   TO CM-ERROR-DISPLAY-MSG
              PERFORM CLEANUP
                 THRU CLEANUP-EXIT.
       APPC-SEND-EXIT. EXIT.
      *****************************************************************
      *
       APPC-RECEIVE.
      ****************************************************
      * PERFORM THIS CALL UNTIL A "NOT" CM-OK            *
      * RETURN CODE IS RECEIVED.  ALLOWING RECEPTION OF: *
      * - PROCESSING-RESULTS-RECORD FROM CREDRPT PROGRAM *
      * - CONVERSATION DEALLOCATION RETURN CODE          *
      *     FROM THE CREDRPT PROGRAM                     *
      ****************************************************
           MOVE 25 TO REQUESTED-LENGTH.
           CALL "CMRCV" USING CONVERSATION-ID
                              BUFFER
                              REQUESTED-LENGTH
                              DATA-RECEIVED
                              RECEIVED-LENGTH
                              STATUS-RECEIVED
                              REQUEST-TO-SEND-RECEIVED
                              CM-RETCODE.
      *
           IF CM-COMPLETE-DATA-RECEIVED
              MOVE BUFFER TO PROCESSING-RESULTS-RECORD
              DISPLAY PROCESSING-RESULTS-RECORD
           END-IF.
 
           IF CM-OK OR CM-DEALLOCATED-NORMAL
              NEXT SENTENCE
           ELSE
              MOVE "RECEIVE PROCESSING TERMINATED"
                   TO CM-ERROR-DISPLAY-MSG.
       APPC-RECEIVE-EXIT. EXIT.
      *
       CLEANUP.
      ***********************************************
      * DISPLAY EXECUTION COMPLETE OR ERROR MESSAGE *
      * NOTE: CREDRPT WILL DEALLOCATE CONVERSATION  *
      ***********************************************
           IF CM-ERROR-DISPLAY-MSG  = SPACES
               DISPLAY "PROGRAM: SALESRPT EXECUTION COMPLETE"
           ELSE
               DISPLAY "SALESRPT PROGRAM - ",
                       CM-ERROR-DISPLAY-MSG, " RC= ", CM-RETCODE.
           STOP RUN.
       CLEANUP-EXIT. EXIT.
      *****************************************************************
