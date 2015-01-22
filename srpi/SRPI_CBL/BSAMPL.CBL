      **********************-PROLOGUE-*********************************
      *                                                               *
      * MODULE NAME = BSAMPL.CBL                                      *
      *                                                               *
      * DESCRIPTIVE NAME = COBOL Sample Program                       *
      *                                                               *
      * STATUS=    Extended Services Version 1.0 Modification 0       *
      *                                                               *
      * COPYRIGHT= (C) COPYRIGHT IBM CORP. 1988, 1991                 *
      *            LICENSED MATERIAL - PROGRAM PROPERTY OF IBM        *
      *            ALL RIGHTS RESERVED                                *
      *                                                               *
      * FUNCTION = Invoke a hypothetical server via the COBOL         *
      *            INTERFACE routines.                                *
      *                                                               *
      *            This sample program reads a customer record        *
      *            from a data base, examines the customer's          *
      *            balance and writes the customer record to          *
      *            a file containing customer records if the          *
      *            balance is greater than zero.                      *
      *                                                               *
      * NOTES =                                                       *
      *                                                               *
      *   RESTRICTIONS = This sample program is provided solely as    *
      *                  an example of how the COBOL interface        *
      *                  routines can be used to invoke a server.     *
      *                                                               *
      * MODULE TYPE = Microfocus COBOL Compiler Version xxxx          *
      *********************-END PROLOGUE-******************************

       ID DIVISION.
         PROGRAM-ID. BSAMPL.
       ENVIRONMENT DIVISION.
       DATA DIVISION.
       WORKING-STORAGE SECTION.

      *****************************************************************
      *             Working Storage Section                           *
      *****************************************************************
      *  BSERVER          Server Name                                 *
      *  BOPER            Default Operator Name                       *
      *  QPALOG           Log the Transaction                         *
      *  QPACOM           Commit the Transaction                      *
      *  BFUNCT1          Function Code : Get Record                  *
      *  BFUNCT2          Function Code : Update Accounts             *
      *  BRCOK            Server Return Code OK                       *
      *  BLSTR            Last Record                                 *
      *  BQPARMS-SIZE     Request Parameters Record Size              *
      *  CUST-REC-SIZE    Customer Record Size                        *
      *                                                               *
      *  BRETCOD          SRPI Return Code                            *
      *                                                               *
      *  BQPARMS-RECORD   Request Parameters                          *
      *  QPAFLAGS         Processing Flags                            *
      *  QPAOPER          Requesting Operator                         *
      *****************************************************************

       77  BSERVER            PIC X(8)             VALUE "IBMabase".
       77  BOPER              PIC X(8)             VALUE "ADMIN".
       77  QPALOG             PIC 9(4)     COMP-5  VALUE H'01'.
       77  QPACOM             PIC 9(4)     COMP-5  VALUE H'02'.
       77  BFUNC1             PIC 9(4)     COMP-5  VALUE  IS 1.
       77  BFUNC2             PIC 9(4)     COMP-5  VALUE  IS 2.
       77  BRCOK              PIC 9(9)     COMP-5  VALUE H'00000000'.
       77  BLSTR              PIC 9(9)     COMP-5  VALUE H'00000004'.
       77  BQPARMS-SIZE       PIC 9(4)     COMP-5  VALUE IS 9.
       77  CUST-REC-SIZE      PIC 9(4)     COMP-5  VALUE IS 109.

       01  BRETCOD            PIC S9(9)    COMP-5.

       01  BQPARMS-RECORD.
           05 QPAFLAGS         PIC X(1).
           05 QPAOPER          PIC X(8).

      *****************************************************************
      *             Working Storage Section                           *
      *****************************************************************
      *  CUSTOMER-RECORD  Customer Record                             *
      *  CUSTOMER-NAME    Customer Name                               *
      *  CUSTOMER-ADDR    Street Address                              *
      *  CUSTOMER-CITY    City                                        *
      *  CUSTOMER-STAT    State                                       *
      *  CUSTOMER-ZIP     Zip Code                                    *
      *  CUSTOMER-ACCT    Account Number                              *
      *  CUSTOMER-BAL     Balance                                     *
      *****************************************************************

       01  CUSTOMER-RECORD.
           05 CUSTOMER-NAME    PIC X(25).
           05 CUSTOMER-ADDR    PIC X(25).
           05 CUSTOMER-CITY    PIC X(15).
           05 CUSTOMER-STAT    PIC X(15).
           05 CUSTOMER-ZIP     PIC X(9).
           05 CUSTOMER-ACCT    PIC X(16).
           05 CUSTOMER-BAL     PIC S9(9) COMP-5.

        COPY UUBCPRB.
      ***********************-PSEUDOCODE-******************************
      *                     PROC (MAIN)                               *
      *                    1. SET PROCESSING OPTION = COMMIT          *
      *                          TRANSACTION                          *
      *                    1. SET REQUESTING OPERATOR ID              *
      *                    1. INITIALIZE SERVER RETURN CODE           *
      *                    1. INITIALIZE SRPI RETURN CODE             *
      *                    1. DO UNTIL SERVER RETURN CODE IS LAST     *
      *                           RECORD OR SRPI RETURN CODE NOT GOOD *
      *                    1. ENDWHILE                                *
      *                     ENDPROC (MAIN)                            *
      *********************-END PSEUDOCODE-****************************
       PROCEDURE DIVISION.
       100-MAIN.


      *******   SET PROCESSING OPTION = COMMIT TRANSACTION
           MOVE QPACOM TO QPAFLAGS.

      *******   SET REQUESTING OPERATOR ID
           MOVE BOPER TO QPAOPER.

      *******   INITIALIZE SERVER RETURN CODE
           MOVE BRCOK TO UERSERVRC.

      *******   INITIALIZE SRPI RETURN CODE
           MOVE UERERROK TO BRETCOD.

      *******   DO UNTIL SERVER RETURN CODE IS LAST
      *******       RECORD OR SRPI RETURN CODE IS NOT GOOD
           PERFORM 200-INIT-AND-SEND UNTIL
             ((UERSERVRC  EQUAL BLSTR) OR (BRETCOD NOT EQUAL UERERROK)).

       STOP RUN.

       100-EXIT.

           EXIT.

      ***********************-PSEUDOCODE-******************************
      *                     PROC (INIT-AND-SEND)                      *
      *                    1. INITIALIZE THE CPRB STRUCTURE           *
      *                           <INIT_SEND_REQ_PARMS>               *
      *                    1. MOVE SERVER NAME AND FUNCTION           *
      *                           (GET RECORD) INTO CPRB STRUCTURE    *
      *                    1. SET CPRB REQUEST PARAMETERS BUFFER      *
      *                          INFORMATION                         *
      *                    1. SET CPRB REPLY DATA BUFFER INFORMATION  *
      *                    1. SEND THE REQUEST TO THE SERVER          *
      *                    1. IF THE SERVER RETURN CODE IS GOOD AND   *
      *                           IF THE SRPI RETURN CODE IS GOOD AND *
      *                           IF THE ACCOUNT BALANCE IS POSITIVE  *
      *                    2. . SET CPRB FUNCTION = UPDATE            *
      *                           ACCOUNTS RECEIVABLE                 *
      *                    2. . SET CPRB REQUEST DATA = CUSTOMER      *
      *                           RECORD                              *
      *                    2. . UPDATE THE ACCOUNTS RECEIVABLE        *
      *                           FILE <SENDREQUEST>                  *
      *                    1. ENDIF                                   *
      *                     ENDPROC (INIT-AND-SEND)                   *
      *********************-END PSEUDOCODE-****************************

       200-INIT-AND-SEND.

      *******   INITIALIZE THE CPRB STRUCTURE <INIT_SEND_REQ_PARMS>
           CALL '__INIT_SEND_REQ_PARMS' USING UERCPRB.

      *******   MOVE SERVER NAME  INTO CPRB STRUCTURE
           MOVE BSERVER TO UERSERVER.

      *******   MOVE FUNCTION (GET RECORD)INTO CPRB STRUCTURE
           MOVE BFUNC1 TO UERFUNCT.

      *******   SET CPRB REQUEST PARAMETERS BUFFER INFORMATION
           MOVE BQPARMS-SIZE TO UERQPARML.

      *******   SET CPRB REQUEST PARAMETERS BUFFER INFORMATION
           SET UERQPARMAD TO ADDRESS OF BQPARMS-RECORD.

      *******   SET CPRB REPLY DATA BUFFER INFORMATION
           MOVE CUST-REC-SIZE TO UERRDATAL.

      *******   SET CPRB REPLY DATA BUFFER INFORMATION
           SET UERRDATAAD TO ADDRESS OF CUSTOMER-RECORD.

      *******   SEND THE REQUEST TO THE SERVER <SENDREQUEST>
           MOVE BSERVER TO UERSERVER.

           CALL '__SENDREQUEST' USING UERCPRB.


      *******   RETURN SRPI RETURN CODE TO MAIN
           MOVE UERRETCODE TO BRETCOD.

      *******   IF THE SRPI RETURN CODE IS GOOD
           IF BRETCOD = UERERROK

      *******   IF THE SERVER RETURN CODE IS GOOD
             IF UERSERVRC = BRCOK

      *******   IF THE ACCOUNT BALANCE IS POSITIVE
               IF CUSTOMER-BAL > 0

      *******   SET CPRB FUNCTION = UPDATE ACCOUNTS RECEIVABLE
                  MOVE BFUNC2 TO UERFUNCT

      *******   SET CPRB REQUEST DATA = CUSTOMER RECORD
                  MOVE CUST-REC-SIZE TO UERQDATAL

      *******   SET CPRB REQUEST DATA = CUSTOMER RECORD
                  SET UERQDATAAD TO ADDRESS OF CUSTOMER-RECORD

      *******   UPDATE THE ACCOUNTS RECEIVABLE
      *******     FILE <SENDREQUEST>

                  CALL '__SENDREQUEST' USING UERCPRB


      *******   RETURN SRPI RETURN CODE TO MAIN
                  MOVE UERRETCODE TO BRETCOD.

       200-INIT-AND-SEND-EXIT.

           EXIT.

           END PROGRAM BSAMPL.
