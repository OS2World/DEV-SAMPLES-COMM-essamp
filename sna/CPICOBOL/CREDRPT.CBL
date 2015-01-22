       IDENTIFICATION DIVISION.
       PROGRAM-ID.        CREDRPT.
      *************************************************************
      * THIS IS THE CREDRPT PROGRAM THAT RECIEVES DATA FROM THE   *
      * SALESRPT PROGRAM FOR CREDIT BALANCE PROCESSING.           *
      *                                                           *
      * 86F2503 (C) COPYRIGHT IBM CORP. 1990, 1991                *
      * LICENSED MATERIAL - PROGRAM PROPERTY OF IBM               *
      * ALL RIGHTS RESERVED                                       *
      *                                                           *
      * PURPOSE: RECEIVE A SALES-RECORD FROM THE SALESRPT PROGRAM *
      *          AND COMPUTE AND DISPLAY A NEW CREDIT BALANCE,    *
      *          THEN SEND A STATUS INDICATION TO SALESRPT.       *
      *                                                           *
      * INPUT:   SALES-RECORD FROM SALESRPT PROGRAM.              *
      *                                                           *
      * OUTPUT:  DISPLAY OUTPUT-RECORD.                           *
      *          PROCESSING-RESULTS-RECORD TO SALESRPT.           *
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
 
       01  CM-ERROR-DISPLAY-MSG       PIC X(40)  VALUE SPACES.
 
       01  BUFFER                     PIC X(52).
 
       01  CURRENT-CREDIT-BALANCE     PIC S9(7)V99.
 
       01  CONVERSATION-STATUS    PIC 9(9)    COMP-4.
           88 CONVERSATION-ACCEPTED           VALUE 1.
           88 CONVERSATION-NOT-ESTABLISHED    VALUE 0.
 
      ****************
      * SALES-RECORD *
      ****************
       01  SALES-RECORD.
           05  CUST-NUM                PIC X(4).
           05  CUST-NAME               PIC X(20).
           05  FILLER                  PIC X(5).
           05  CREDIT-BALANCE          PIC S9(7)V99.
           05  CREDIT-LIMIT            PIC S9(7)V99.
           05  CREDIT-FLAG             PIC X.
 
      *****************
      * OUTPUT-RECORD *
      *****************
       01  OUTPUT-RECORD.
           05 FILLER                   PIC X.
           05 OP-CUST-NUM              PIC X(4).
           05 FILLER                   PIC X(3)   VALUE SPACES.
           05 OP-CUST-NAME             PIC X(20).
           05 FILLER                   PIC X(5)   VALUE SPACES.
           05 OP-CREDIT-LIMIT          PIC Z(6)9.99-.
           05 FILLER                   PIC X(5)   VALUE SPACES.
           05 OP-CREDIT-BALANCE        PIC Z(6)9.99-.
           05 FILLER                   PIC X(5)   VALUE SPACES.
           05 OP-TEXT-FIELD            PIC X(25).
           05 FILLER                   PIC X(5)   VALUE SPACES.
 
      *****************************
      * PROCESSING-RESULTS-RECORD *
      *****************************
       01  PROCESSING-RESULTS-RECORD   PIC X(25)  VALUE SPACES.
 
      **********************************************
      * CPI-COMMUNICATIONS PSEUDONYM COPYBOOK FILE *
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
 
           PERFORM APPC-ACCEPT
              THRU APPC-ACCEPT-EXIT.
           DISPLAY "CREDRPT CONVERSATION ACCEPTED".
 
           PERFORM APPC-RECEIVE
              THRU APPC-RECEIVE-EXIT
              UNTIL CM-SEND-RECEIVED.
           DISPLAY "CREDRPT RECORD RECEIVED".
 
           PERFORM PROCESS-RECORD
              THRU PROCESS-RECORD-EXIT.
           DISPLAY "CREDRPT DATA PROCESSED".
 
           PERFORM APPC-SEND
              THRU APPC-SEND-EXIT.
           DISPLAY "CREDRPT RESULTS RECORD SENT".
 
           PERFORM CLEANUP
              THRU CLEANUP-EXIT.
           STOP RUN.
      ******************************************************************
      ***************************  END OF MAINLINE  ********************
      ******************************************************************
      *
       APPC-ACCEPT.
      **************************************************
      * ACCEPT INCOMING APPC CONVERSATION ESTABLISHING *
      * DEFAULT CONVERSATION CHARACTERISTICS           *
      **************************************************
           CALL "CMACCP" USING CONVERSATION-ID
                               CM-RETCODE.
           IF CM-OK
              SET CONVERSATION-ACCEPTED TO TRUE
           ELSE
              MOVE "ACCEPT PROCESSING TERMINATED"
                   TO CM-ERROR-DISPLAY-MSG
              PERFORM CLEANUP
                 THRU CLEANUP-EXIT
           END-IF.
       APPC-ACCEPT-EXIT. EXIT.
      *****************************************************************
      *
       APPC-RECEIVE.
      *************************************************************
      * PERFORM THIS CALL UNTIL A CM-SEND-RECEIVE INDICATION IS   *
      * RECEIVED. THIS INDICATES A CONVERSATION STATE CHANGE FROM *
      * RECEIVE TO SEND OR SEND-PENDING STATE, THUS "CMRCV"       *
      * (RECEIVE) HAS COMPLETED. ALLOWING RECEPTION OF:           *
      * - SALES-RECORD FROM SALESRPT PROGRAM                      *
      *************************************************************
           MOVE 52 TO REQUESTED-LENGTH.
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
              MOVE BUFFER TO SALES-RECORD
           END-IF.
      *
           IF CM-OK
              NEXT SENTENCE
           ELSE
              PERFORM APPC-SET-DEALLOCATE-TYPE
                 THRU APPC-SET-DEALLOCATE-TYPE-EXIT
              MOVE "RECEIVE PROCESSING TERMINATED"
                   TO CM-ERROR-DISPLAY-MSG
              PERFORM CLEANUP
                 THRU CLEANUP-EXIT.
       APPC-RECEIVE-EXIT. EXIT.
      *****************************************************************
      *
       PROCESS-RECORD.
           SUBTRACT CREDIT-BALANCE FROM CREDIT-LIMIT
              GIVING CURRENT-CREDIT-BALANCE.
           IF CREDIT-FLAG = "0"
              MOVE "**CREDIT LIMIT EXCEEDED**" TO OP-TEXT-FIELD
           ELSE
              MOVE SPACES TO OP-TEXT-FIELD
           END-IF.
           MOVE CUST-NUM TO OP-CUST-NUM.
           MOVE CUST-NAME TO OP-CUST-NAME.
           MOVE CREDIT-LIMIT TO OP-CREDIT-LIMIT.
           MOVE CURRENT-CREDIT-BALANCE TO OP-CREDIT-BALANCE.
           DISPLAY OUTPUT-RECORD.
      *
           MOVE "CREDIT RECORD UPDATED" TO PROCESSING-RESULTS-RECORD.
       PROCESS-RECORD-EXIT. EXIT.
      *****************************************************************
      *
       APPC-SEND.
           MOVE PROCESSING-RESULTS-RECORD TO BUFFER.
           MOVE 25 TO SEND-LENGTH.
 
      **************************************************
      * SEND THE PROCESSING-RESULTS-RECORD TO SALESRPT *
      **************************************************
           CALL "CMSEND" USING CONVERSATION-ID
                               BUFFER
                               SEND-LENGTH
                               REQUEST-TO-SEND-RECEIVED
                               CM-RETCODE.
           IF CM-OK
              NEXT SENTENCE
           ELSE
              PERFORM APPC-SET-DEALLOCATE-TYPE
                 THRU APPC-SET-DEALLOCATE-TYPE-EXIT
              MOVE "SEND PROCESSING TERMINATED"
                   TO CM-ERROR-DISPLAY-MSG
              PERFORM CLEANUP
                 THRU CLEANUP-EXIT.
       APPC-SEND-EXIT. EXIT.
      *****************************************************************
      *
       APPC-SET-DEALLOCATE-TYPE.
           SET CM-DEALLOCATE-ABEND TO TRUE.
 
      *****************************************
      * ON ERROR SET DEALLOCATE-TYPE TO ABEND *
      *****************************************
           CALL "CMSDT" USING CONVERSATION-ID
                              DEALLOCATE-TYPE
                              CM-RETCODE.
           IF CM-OK
              NEXT SENTENCE
           ELSE
              DISPLAY "ERROR SETTING CONVERSATION DEALLOCATE TYPE".
       APPC-SET-DEALLOCATE-TYPE-EXIT. EXIT.
      *****************************************************************
      *
       CLEANUP.
           IF CONVERSATION-ACCEPTED
      ********************************
      * DEALLOCATE APPC CONVERSATION *
      ********************************
               CALL "CMDEAL" USING CONVERSATION-ID
                                   CM-RETCODE
               DISPLAY "CREDRPT DEALLOCATED CONVERSATION"
           END-IF.
           IF CM-ERROR-DISPLAY-MSG  = SPACES
               DISPLAY "PROGRAM: CREDRPT EXECUTION COMPLETE"
           ELSE
               DISPLAY "CREDRPT PROGRAM - ",
                       CM-ERROR-DISPLAY-MSG, " RC= ", CM-RETCODE
           END-IF.
           STOP RUN.
       CLEANUP-EXIT. EXIT.
      *****************************************************************
