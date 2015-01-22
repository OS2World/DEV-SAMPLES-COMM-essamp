/*********************-PROLOGUE-*********************************
*                                                               *
* MODULE NAME = CSAMPL.C                                        *
*                                                               *
* DESCRIPTIVE NAME = C Sample Program                           *
*                                                               *
* STATUS=    Extended Services Version 1.0 Modification 0       *
*                                                               *
* COPYRIGHT=  (C) COPYRIGHT IBM CORP. 1988, 1991                *
*             LICENSED MATERIAL - PROGRAM PROPERTY OF IBM       *
*             ALL RIGHTS RESERVED                               *
*                                                               *
* FUNCTION = Invoke a hypothetical server via the C interface   *
*            routines.                                          *
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
*                  an example of how the C interface routines   *
*                  can be used to invoke a server.              *
*                                                               *
* MODULE TYPE = Microsoft C Compiler Version 6.0                *
*********************-END PROLOGUE-*****************************/
/*********************-DEFINITIONS-*****************************/
#include <uuccprb.h>
char    cserver[9] = "IBMabase"; /* Server Name                 */

char    coper[9] = "ADMIN   ";   /* Default operator name       */

main()                           /* PROC (MAIN)                 */
{
UERCPRB ccprb;                   /* CPRB structure              */
struct  {                        /* Customer Record Structure   */

        char     cusname[25];    /* Customer Name               */
        char     cusaddr[25];    /* Street Address              */
        char     cuscity[15];    /* City                        */
        char     cusstat[15];    /* State                       */
        char     cuszip[9];      /* Zip Code                    */
        char     cusacct[16];    /* Account Number              */
        long int cusbal;         /* Balance                     */

        }       ccustrec;

struct  {                       /* Request Parameters Structure */

        char    qpaflags;       /* Processing Flags             */
#define QPALOG 0x01             /* Log the transaction          */
#define QPACOM 0x02             /* Commit the transaction       */
        char    qpaoper[8];     /* Requesting operator's        */
                                /* sign-on ID                   */
        }       cqparms;
#define CFUNC1 1              /* Func Code: Get Record          */
#define CFUNC2 2              /* Func Code: Update accounts     */
                              /* receivable file                */
#define CRCOK 0x00000000      /* Server Return Code OK          */
#define CLSTR 0x00000004      /* Last Record                    */

        int     cctr;         /* general purpose counter        */
        long int cretcod;     /* SRPI return code               */

/*******************-END DEFINITIONS-***************************/
/**********************-PSEUDOCODE-*****************************/
/*+                     PROC (MAIN)                                   */
/*+                    1. INITIALIZE SERVER RETURN CODE               */
/*+                    1. INITIALIZE SRPI RETURN CODE                 */
/*+                    1. SET PROCESSING OPTION = COMMIT              */
/*+                         TRANSACTION                               */
/*+                    1. SET REQUESTING OPERATOR ID                  */
/*+                    1. DO WHILE SERVER RETURN CODE IS NOT LAST     */
/*+                         RECORD AND SRPI RETURN CODE IS GOOD       */
/*+                    2. . INITIALIZE THE CPRB STRUCTURE             */
/*+                           <INIT_SEND_REQ_PARMS>                   */
/*+                    2. . MOVE SERVER NAME ADDRESS AND FUNCTION     */
/*+                           (GET RECORD) INTO CPRB STRUCTURE        */
/*+                    2. . SET CPRB REQUEST PARAMETERS BUFFER        */
/*+                           INFORMATION                             */
/*+                    2. . SET CPRB REPLY DATA BUFFER INFORMATION    */
/*+                    2. . SEND THE REQUEST TO THE SERVER            */
/*+                    2. . IF THE SRPI RETURN CODE IS GOOD           */
/*+                    3. . . IF THE SERVER RETURN CODE IS GOOD       */
/*+                    4. . . . IF THE ACCOUNT BALANCE IS POSITIVE    */
/*+                    5. . . . . SET CPRB FUNCTION = UPDATE          */
/*+                                 ACCOUNTS RECEIVABLE               */
/*+                    5. . . . . SET CPRB REQUEST DATA = CUSTOMER    */
/*+                                 RECORD                            */
/*+                    5. . . . . UPDATE THE ACCOUNTS RECEIVABLE      */
/*+                                 FILE <SEND_REQUEST>               */
/*+                    4. . . . ENDIF                                 */
/*+                    3. . . ENDIF                                   */
/*+                    2. . ENDIF                                     */
/*+                    1. ENDWHILE                                    */
/*+                     ENDPROC (MAIN)                                */
/********************-END PSEUDOCODE-***************************/
/*************************-PROCEDURE****************************/

ccprb.uerservrc = CRCOK;            /* INITIALIZE SERVER RETURN CODE  */

cretcod = UERERROK;                 /* INITIALIZE SRPI RETURN CODE    */

cqparms.qpaflags = QPACOM;          /* SET PROCESSING OPTION =        */
                                    /* COMMIT TRANSACTION             */

for (cctr = 0; cctr <= (sizeof cqparms.qpaoper) - 1; cctr++)
  cqparms.qpaoper[cctr] = coper[cctr];
                                    /* SET REQUESTING OPERATOR ID */

while (ccprb.uerservrc != CLSTR && cretcod == UERERROK)
{                                   /* DOWHILE SERVER RETURN CODE IS  */
                                    /* NOT LAST RECORD AND SRPI       */
                                    /* RETURN CODE IS GOOD            */

  init_send_req_parms(&ccprb);      /* INITIALIZE CPRB STRUCTURE      */

  ccprb.uerserver = cserver;        /* MOVE SERVER NAME & FUNCTION    */
  ccprb.uerfunct = CFUNC1;          /* INTO CPRB STRUCTURE            */

  ccprb.uerqparml = sizeof cqparms; /* SET CPRB REQUEST PARAMETER     */
  ccprb.uerqparmad = (char far *) &cqparms;/* BUFFER INFORMATION      */

  ccprb.uerrdatal = sizeof ccustrec;/* SET CPRB REPLY DATA BUFFER     */
  ccprb.uerrdataad =  (char far *) &ccustrec;/* INFORMATION           */

  cretcod = send_request(&ccprb);   /* SEND REQUEST TO SERVER         */

  if (cretcod == UERERROK)          /* IF THE SRPI RETURN CODE IS GOOD*/

    if (ccprb.uerservrc == CRCOK)   /* IF THE SERVER RETURN CODE      */
                                    /* IS GOOD                        */
    {

      if (ccustrec.cusbal > 0)      /* IF THE ACCOUNT BALANCE IS      */
                                    /* POSITIVE                       */
      {

        ccprb.uerfunct = CFUNC2;    /* SET CPRB FUNCTION = UPDATE     */
                                    /* ACCOUNTS RECEIVABLE            */
        ccprb.uerqdatal = sizeof ccustrec; /* SET CPRB REQUEST        */
        ccprb.uerqdataad = (char far *) &ccustrec; /* DATA = CUSTOMER RECORD*/

        cretcod = send_request(&ccprb); /* UPDATE ACCNT RECEIVABLE    */
                                        /* FILE <SEND_REQUEST>        */
      }                             /* ENDIF                          */

    }                               /* ENDIF                          */

  }                                 /* ENDIF                          */

}                                   /* ENDWHILE                       */

                                    /* ENDPROC (MAIN)                 */
/***********************-END PROCEDURE-*************************/
