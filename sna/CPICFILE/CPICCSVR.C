/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : CPICCSVR.C                                                */
/*                                                                          */
/*  DESCRIPTIVE : CPI-C FILE SERVER "C" SAMPLE PROGRAM                      */
/*  NAME          FOR IBM Communications Manager                            */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991                         */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*                                                                          */
/*  STATUS      : Extended Services 1.0                                     */
/*                                                                          */
/*  FUNCTION:   Receives  a  request for a  file from a  peer               */
/*              requester  program.  This  program  opens the               */
/*              file  and  transfers  the  data ( or an error               */
/*              indicator if the file does not exist) back to               */
/*              the requester.                                              */
/*                                                                          */
/*              Uses the following CPI-C subroutines:                       */
/*                                                                          */
/*                 CMACCP - Accept_Conversation                             */
/*                 CMSEND - Send_Data                                       */
/*                 CMRCV  - Receive                                         */
/*                 CMSRT  - Set_Receive_Type                                */
/*                 CMSERR - Send_Error                                      */
/*                 CMDEAL - Deallocate                                      */
/*                                                                          */
/*                                                                          */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
/*                   (compiles with small memory model)                     */
/*                   IBM C Set/2 Compiler for 32 bit applications.          */
/*                                                                          */
/*              Requires message file "APC.MSG" at runtime.                 */
/*                                                                          */
/****************************************************************************/
#define LINT_ARGS

#define INCL_DOSPROCESS

#define  INCL_DOSMISC
#define  INCL_DOSSESMGR
#include <os2.h>
#include <appc_c.h>
#include <acssvcc.h>
#include <cmc.h>                            /* CPI-C include file           */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#ifdef ES32TO16
#define SIZE   ULONG
#define PSIZE  PULONG
#define INCL_DOSMEMMGR
#include <bsememf.h>
#define BUFF_SIZE  ULONG
#define FRPTR   *
#define TYPEC PVOID
#else
#define SIZE   USHORT
#define PSIZE  PUSHORT
#define FRPTR   far *
#define BUFF_SIZE  UINT
#define TYPEC ULONG
#endif

#define PROGRAM_NAME        "CPICCSVR"      /* used by LOG_MESSAGE verb     */

#define MESSAGE_FILENAME    "APC.MSG"       /* used by DosGetMessage        */
#define LOG_MESSAGE_FILE    "APC"           /* used by LOG_MESSAGE verb     */

#define BUFFER_SIZE         (31920)         /* use a large buffer size for  */
                                            /* peak performance.            */
#define FILE_NAME_MAX_LEN   (261)           /* use longer file name buffers */
                                            /* to support HPFS              */

#define MAX_MESSAGE_LEN     (100)           /* length of the longest        */
                                            /* message in the message file  */


/* These are the message numbers of specific messages in file APC.MSG */
#define MSG_SAMPLE_SERVER    (1)
#define MSG_SAMPLE_COMPLETE  (3)
#define MSG_OS2_ERROR        (4)
#define MSG_BYTES_SENT       (5)
#define MSG_OS2_RETCODE      (7)
#define MSG_FILE_OPEN_ERROR  (8)
#define MSG_UNEXPECTED_RCVD  (10)
#define MSG_DATA_INCOMPLETE  (11)
#define MSG_NO_SEND_PERMIT   (12)
#define MSG_VERB_OPCODE      (13)
#define MSG_LOG_VERB_ERROR   (14)
#define MSG_CPIC_ERROR_RC    (15)
#define MSG_CPIC_ERROR_VERB  (16)



#ifdef LINT_ARGS
/*--------------------------------------------------------------------------*/
/*                       Function Prototypes                                */
/*--------------------------------------------------------------------------*/
void show_cpic_err (char * cpic_verb, CM_RETURN_CODE conv_rc);
void log_cpic_err (char * cpic_verb, CM_RETURN_CODE conv_rc);
void show_and_log_cpic_err (char * cpic_verb, CM_RETURN_CODE conv_rc);

PCH  alloc_shared_buffer (const BUFF_SIZE display_buffer_size);
void cdecl main(void);
void log_msg(const USHORT msgno, const USHORT length, const UCHAR * text);
void log_os2_error(const USHORT dos_rc);
void show_msg(const USHORT msgno);
void show_os2_retcode (const USHORT retcode);
void show_and_log_os2_error (const USHORT retcode);

#endif


char *cpic_calls[]=                         /* CPI-C verb call in error     */
    {  "CMACCP ",     /*  0 */
       "CMALLC ",     /*  1 */
       "CMCFMD ",     /*  2 */
       "CMDEAL ",     /*  3 */
       "CMINIT ",     /*  4 */
       "CMRCV  ",     /*  5 */
       "CMSCT  ",     /*  6 */
       "CMSDT  ",     /*  7 */
       "CMSED  ",     /*  8 */
       "CMSEND ",     /*  9 */
       "CMSERR ",     /* 10 */
       "CMSLD  ",     /* 11 */
       "CMSMN  ",     /* 12 */
       "CMSPLN ",     /* 13 */
       "CMSRC  ",     /* 14 */
       "CMSRT  ",     /* 15 */
       "CMSSL  ",     /* 16 */
       "CMSST  ",     /* 17 */
       "CMSTPN " };   /* 18 */

/****************************************************************************/
/*                       Main Program Section                               */
/****************************************************************************/

void cdecl main(void)                       /* command line is ignored      */
{
   CM_RETURN_CODE conv_rc;                  /* conversation return code     */
   UCHAR cm_conv_id[8];                     /* CPI-C conversation ID        */
   CM_INT32  max_receive_len = BUFFER_SIZE; /* Max receive length on CMRCV  */
   CM_INT32  what_received;                 /* What received parm from CMRCV*/
   CM_INT32  received_len;                  /* Amount of data rcvd on CMRCV */
   CM_INT32  status_received;               /* Status from CMRCV            */
   CM_INT32  rts_received;                  /* Request to send received, Y/N*/
   CM_ERROR_DIRECTION error_direction = CM_SEND_ERROR; /* error direction   */
   CM_RECEIVE_TYPE receive_type = CM_RECEIVE_AND_WAIT; /* receive type      */
   CM_DEALLOCATE_TYPE dealloc_type;         /* deallocate type              */
   CM_SEND_TYPE send_type = CM_BUFFER_DATA; /* send type                    */
   BOOL   done = FALSE;                     /* is the transaction finished? */
   USHORT dos_rc;                           /* OS/2 return code             */
   HFILE  filehandle;                       /* returned by DosOpen          */
   SIZE   action;                           /* returned by DosOpen          */
   SIZE   bytes_read;                       /* returned by DosRead          */
   CM_INT32 length;                         /* length of received data      */
   ULONG  byte_count = 0;                   /* counts the bytes read so far */
   PCH    data_buffer;                      /* pointer to shared memory     */
                                            /* segment                      */
#ifdef ES32TO16
   ULONG  dos_rc1;                          /* Return code from DosFreeSeg  */
#endif

#ifndef MAX_SPEED
   show_msg(MSG_SAMPLE_SERVER);             /* Show the Server message      */
#endif

   /* Ask OS/2 to get the shared memory buffer.                             */
   data_buffer = alloc_shared_buffer(max((USHORT)BUFFER_SIZE,
                                         (USHORT)FILE_NAME_MAX_LEN));


   cmaccp( cm_conv_id,                      /* Accept Conversation          */
           &conv_rc);

   if (conv_rc != CM_OK)  {                 /* Quit if there was a problem  */
     show_and_log_cpic_err (cpic_calls[0], conv_rc);
   } /* endif */

   /*************************************************************************/
   /* Receive the first block of data, presumably the name of a file to send*/
   /*************************************************************************/
   cmsrt (cm_conv_id,                       /* Set Receive Type             */
          &receive_type,                    /* Receive and Wait             */
          &conv_rc);
                                            /* Handle any error             */
   if (conv_rc != CM_OK)  {show_and_log_cpic_err(cpic_calls[15],conv_rc); }

   /* Conversation type is determined by value received on CMACCP call.     */
   cmrcv (cm_conv_id,                       /* Receive Data                 */
          data_buffer,                      /* Data Pointer                 */
          &max_receive_len,                 /* Size of Data Buffer          */
          &what_received,                   /* returned - what received     */
          &received_len,                    /* returned - length of data    */
          &status_received,                 /* returned - status received   */
          &rts_received,                    /* returned - request to send   */
          &conv_rc);

   if (what_received == CM_COMPLETE_DATA_RECEIVED)  {
      if (status_received != CM_SEND_RECEIVED) {/*If Status is not SEND.....*/

         /* Try once more to get send control */
      cmrcv (cm_conv_id,                    /* Receive Data                 */
             data_buffer,                   /* Data Pointer                 */
             &max_receive_len,              /* Size of Data Buffer          */
             &what_received,                /* returned - what received     */
             &received_len,                 /* returned - length of data    */
             &status_received,              /* returned - status received   */
             &rts_received,                 /* returned - request to send   */
             &conv_rc);

         if (status_received != CM_SEND_RECEIVED) { /*If Status is not SEND.*/
            log_msg (MSG_NO_SEND_PERMIT, 0, NULL);
            exit (MSG_NO_SEND_PERMIT);
         } /* endif */
      } /* endif */
   } else {                                 /* Data was not complete        */
      log_msg (MSG_DATA_INCOMPLETE, 0, NULL);
      exit (MSG_DATA_INCOMPLETE);
   } /* endif */

   /*************************************************************************/
   /* Open the filename sent by the Requester.  The filename is an ASCIIZ   */
   /* string pointed to by data_buffer+1. (+1 to skip the Pascal type       */
   /* LL length byte).                                                      */
   /*************************************************************************/
   dos_rc = DosOpen (data_buffer + 1,       /* addr of file & path name     */
                     (PHFILE)&filehandle,   /* returned: filehandle address */
                     (PSIZE)&action,        /* returned: action address     */
                     (ULONG)0,              /* file primary allocation      */
                     FILE_NORMAL,           /* file attribute               */
                     OPEN_ACTION_OPEN_IF_EXISTS, /* function to be done     */
                     OPEN_SHARE_DENYWRITE,  /* Open mode of the file        */
                     (ULONG)0);             /* Reserved double word         */
   if (dos_rc != 0) {                       /* Non-zero OS/2 return code?   */
      if (dos_rc == 110) {                  /* RC = 110: File not found     */

         cmsed (cm_conv_id,                 /* Set Error Direction          */
                &error_direction,           /* Send Error                   */
                &conv_rc);

         cmserr(cm_conv_id,                 /* Send Error                   */
                &rts_received,              /* Request to send indicator    */
                &conv_rc);
      } /* endif */
      show_and_log_os2_error(dos_rc);       /* Exit the program.            */
   } /* endif */


   /*************************************************************************/
   /* Continue while there are still bytes to be sent from the file.        */
   /*************************************************************************/
#ifndef MAX_SPEED
   show_msg (MSG_BYTES_SENT);
#endif
   while (!done) {
      dos_rc = DosRead ((USHORT)filehandle, /* pass the filehandle          */
                        (PCH)data_buffer,    /* APPC data buffer            */
                        (USHORT)BUFFER_SIZE, /* size of the data buffer     */
                        (PSIZE)&bytes_read); /* returned: bytes read addr  */

      if (dos_rc == 0) {                    /* Non-zero OS/2 return code?   */
         if (bytes_read != 0) {             /* Did we read something?       */
            /* Send the next data buffer to the partner with type(FLUSH), to*/
            /* maximize the disk I/O time of the partner.                   */

            send_type = CM_SEND_AND_FLUSH;
            cmsst (cm_conv_id,              /* Set Send Type                */
                   &send_type,              /* Send and Flush               */
                   &conv_rc);

            length = bytes_read;

            cmsend (cm_conv_id,             /* Send Data                    */
                    data_buffer,            /* data pointer                 */
                    &length,                /* length of data sent          */
                    &rts_received,          /* request to send indicator    */
                    &conv_rc);

            if (conv_rc != CM_OK) {
               show_and_log_cpic_err(cpic_calls[9], conv_rc);
            }

         } else {
            done = TRUE;                    /* We've reached the EOF.       */
         } /* endif */

         byte_count += (ULONG) bytes_read;  /* Increment the running count  */

#ifndef MAX_SPEED
         /* Since this printf uses \r instead of \n, only one line appears--*/
         /* an example of a carriage return without a line feed.            */
         printf ("%lu\r", byte_count);
#endif
      } else {

         cmsed (cm_conv_id,                 /* Set Error Direction          */
                &error_direction,           /* Send Error                   */
                &conv_rc);

         cmserr(cm_conv_id,                 /* Send Error                   */
                &rts_received,              /* Request to send indicator    */
                &conv_rc);

         show_and_log_os2_error(dos_rc);    /* Exit the program.            */

      } /* endif */

   } /* end of the while-loop */

   /*************************************************************************/
   /* Close the file and clean up the conversation and TP.                  */
   /*************************************************************************/
   dos_rc = DosClose(filehandle);           /* Close the file               */
   if (dos_rc != 0)                         /* Non-zero OS/2 return code?   */
      show_and_log_os2_error(dos_rc);       /* Exit the program.            */

   dealloc_type = CM_DEALLOCATE_SYNC_LEVEL;

   cmsdt (cm_conv_id,                       /* Set Deallocate Type          */
          &dealloc_type,                    /* Deallocate Sync Level        */
          &conv_rc);

   if (conv_rc != CM_OK) show_and_log_cpic_err(cpic_calls[7], conv_rc);
                                            /* Handle any error             */

   cmdeal (cm_conv_id,                      /* Deallocate                   */
           &conv_rc);

   if (conv_rc != CM_OK)  show_and_log_cpic_err(cpic_calls[3], conv_rc);
                                            /* Handle any error             */

   /*************************************************************************/
   /* NO STORAGE IS FREED by the following call to DosFreeSeg!              */
   /* The following call simply decrements the OS/2 allocation count.       */
   /* For maximum performance, APPC internally locks all data segments      */
   /* passed to it until the using process ends.  See the "APPC Programming */
   /* Reference" for more details.                                          */
   /*************************************************************************/
#ifdef ES32TO16
   /*--------------------------------------------*/
   /* 32 bit memory management                   */
   /*--------------------------------------------*/

  if ( 0 != ( dos_rc1 = DosFreeMem( data_buffer ) ) ) {
      show_and_log_os2_error((USHORT) dos_rc1); /* Exit the program.        */
  }
   /*--------------------------------------------*/
   /* End of 32 bit memory management            */
   /*--------------------------------------------*/
#else
   /*--------------------------------------------*/
   /* 16 bit memory management                   */
   /*--------------------------------------------*/
  if ( 0 != ( dos_rc = DosFreeSeg(SELECTOROF( data_buffer )) ) ) {
      show_and_log_os2_error(dos_rc);       /* Exit the program.            */
  }
   /*--------------------------------------------*/
   /* End of 16 bit memory management            */
   /*--------------------------------------------*/
#endif

#ifndef MAX_SPEED
   printf("\n");
   show_msg(MSG_SAMPLE_COMPLETE);
#endif

   exit (EXIT_SUCCESS);                     /* Set the ERRORLEVEL to 0      */
}

/****************************************************************************/
/*                                                                          */
/*                        UTILITY FUNCTIONS                                 */
/*                                                                          */
/****************************************************************************/

void show_msg (const USHORT msgno)
/* Displays the requested message number from the message file.             */
{
   USHORT dos_rc;                           /* OS/2 return code             */
   SIZE  msg_len;                           /* returned: message length     */
   UCHAR msg_buff[MAX_MESSAGE_LEN];         /* Buffer for message data      */

   /* Read the requested message from the message file.                     */
   dos_rc = DosGetMessage ((PCH FRPTR)0,    /* no IvTable in use            */
                           0,               /* no IvTable in use            */
                           (PCH)msg_buff,   /* Place the message here  */
                           sizeof(msg_buff),/* Length of the buffer         */
                           msgno,           /* Which Message?               */
                           (PCH)MESSAGE_FILENAME, /* Msg filename           */
                           (PSIZE)&msg_len);/* length of message returned   */
   if (dos_rc != 0) {                       /* Non-zero OS/2 return code?   */
      fprintf(stderr,
              "\nCPI-C Sample Program Error: Unable to process message file");
      fprintf(stderr,
              "\nDosGetMessage return code = %04d, message number = %04d",
              dos_rc, msgno);
   }
   else {
      msg_buff[msg_len] = '\0';             /* Make this an ASCIIZ string   */
      printf("\n");                         /* Start on a new line          */
      printf("%s", msg_buff);               /* Display the message          */
   }
}


void log_msg (const USHORT msgno, const USHORT length, const UCHAR *text)
/* This function logs the requested message from the message file.          */
/* The server presumably logs its errors, since it is designed to run       */
/* unattended--no one would see when it writes to the screen.               */
{
    LOG_MESSAGE vcb_lmg;                    /* Common Services LOG_MESSAGE  */
    LOG_MESSAGE FRPTR vcbptr = (LOG_MESSAGE FRPTR)&vcb_lmg;

   CLEAR_VCB(vcb_lmg);                      /* Zero the verb control block  */
   vcb_lmg.opcode = SV_LOG_MESSAGE;         /* LOG_MESSAGE opcode           */
   vcb_lmg.msg_num = msgno;                 /* Number in the message file   */
   memcpy(vcb_lmg.msg_file_name,            /* Set the message file name    */
          LOG_MESSAGE_FILE,
          min(sizeof(vcb_lmg.msg_file_name), strlen(LOG_MESSAGE_FILE)));
   memcpy(vcb_lmg.origntr_id,               /* Set originator of message    */
          PROGRAM_NAME,                     /* Use this program's name      */
          min(sizeof(vcb_lmg.origntr_id), strlen(PROGRAM_NAME))); /* <= 8   */
   vcb_lmg.msg_act = SV_NO_INTRV;           /* No intervention action type  */
   vcb_lmg.msg_ins_len = length;            /* Insertion text length        */
   vcb_lmg.msg_ins_ptr = (PCH) text;        /* Insertion text               */

   ACSSVC ((TYPEC) vcbptr);                 /* Go log the message           */
   if (vcb_lmg.primary_rc != SV_OK) {       /* If bad, try to display error */
      fprintf(stderr,
            "\nCPI-C Sample Program Error: Unable to log message number %04d",
            msgno);
   }
}



void log_os2_error (const USHORT retcode)
/* This function logs an OS/2 return code. */
{
   UCHAR error_dos_rc[5];                   /* ASCII format OS/2 return code*/

   sprintf (error_dos_rc, "%4d", retcode);  /* Convert to ASCIIZ string.    */

   log_msg(MSG_OS2_ERROR, sizeof(error_dos_rc), error_dos_rc);
   exit(retcode);
}

void log_cpic_err (char * cpic_verb, CM_RETURN_CODE conv_rc)
{
   struct {                                 /* Data to insert in err log    */
      UCHAR error_vrb[8];                   /* ASCIIZ verb name             */
      UCHAR error_rc[9];                    /* ASCIIZ return code           */
   } log_ins_text;

   memcpy(log_ins_text.error_vrb, cpic_verb, min(7, strlen(cpic_verb))+1 );
   sprintf (log_ins_text.error_rc, "%ld", conv_rc);  /* Return Code          */

   log_msg(MSG_LOG_VERB_ERROR, sizeof(log_ins_text), (UCHAR *)&log_ins_text);

   exit((USHORT) conv_rc);
}



void show_and_log_cpic_err (char * cpic_verb, CM_RETURN_CODE conv_rc)
{
   show_cpic_err (cpic_verb, conv_rc);
   log_cpic_err (cpic_verb, conv_rc);
}

void show_and_log_os2_error (const USHORT retcode)
{
   show_os2_retcode (retcode);              /* send OS/2 rc ro stderr       */
   log_os2_error (retcode);
}


void show_os2_retcode(const USHORT retcode)
{
   show_msg (MSG_OS2_RETCODE);              /* OS/2 return code message     */
   fprintf (stderr, "%d", retcode);         /* Show return code in decimal  */
}


void
show_cpic_err (char * cpic_verb, CM_RETURN_CODE conv_rc)

/* Displays the CPI-C return code and the verb name */

{
   show_msg (MSG_CPIC_ERROR_RC);            /* CPI-C Error = msg.           */
   fprintf (stderr,"%08d\n",conv_rc);       /* Show CPI-C rc in hex         */
   show_msg (MSG_CPIC_ERROR_VERB);          /* APPC verb in error           */
   fprintf (stderr,"%s\n",cpic_verb);       /* Show verb name of verb in err*/
}

/****************************************************************************/
/*                                                                          */
/*                    OS/2 RELATED FUNCTIONS                                */
/*                                                                          */
/****************************************************************************/

PCH  alloc_shared_buffer (const BUFF_SIZE display_buffer_size)
/* APPC requires a data buffer in a shared unnamed segment.                 */
{
#ifdef ES32TO16
   /*--------------------------------------------*/
   /* 32 bit memory management                   */
   /*--------------------------------------------*/
  PCH    result;                            /* Address of allocated buffer  */
  ULONG  dos_rc;                            /* Return code from Get Mem.    */
  if ( ! ( dos_rc = DosAllocSharedMem((PPVOID)&result,
                                      0,
                                      display_buffer_size,
                                      PAG_COMMIT   |
                                      OBJ_GIVEABLE |
                                      OBJ_TILE     |
                                      PAG_WRITE) ) ) {
  } else {
      show_and_log_os2_error((USHORT) dos_rc);       /* Exit the program.  */
  }
   /*--------------------------------------------*/
   /* End of 32 bit memory management            */
   /*--------------------------------------------*/
#else
   /*--------------------------------------------*/
   /* 16 bit memory management                   */
   /*--------------------------------------------*/
  USHORT selector;                          /* Selector from DosAllocSeg    */
  PCH    result;                            /* Address of allocated buffer  */
  USHORT dos_rc;                            /* Return code from Allocate    */

  if ( ! ( dos_rc = DosAllocSeg( display_buffer_size, (PSEL) &selector, 1) ) ) {
    OFFSETOF( result ) = 0;                 /* set the offset to zero       */
    SELECTOROF( result ) = selector;        /* address = Selector:0         */
  } else {
      show_and_log_os2_error(dos_rc);       /* Exit the program.            */
  }
   /*--------------------------------------------*/
   /* End of 16 bit memory management            */
   /*--------------------------------------------*/
#endif
  return result;                            /* Return the buffer address    */
}

/* EOF CPICCSVR.C */
