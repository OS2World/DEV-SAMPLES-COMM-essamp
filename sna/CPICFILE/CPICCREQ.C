/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : CPICCREQ.C                                                */
/*                                                                          */
/*  DESCRIPTIVE : CPI-C FILE REQUESTER "C" SAMPLE PROGRAM                   */
/*  NAME          FOR IBM COMMUNICATIONS MANAGER                            */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991                         */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*                                                                          */
/*  FUNCTION:   Issues a request for a file to a server and transfers the   */
/*              file to the default local disk (current directory).  All    */
/*              data transfer is to a peer server via CPI-C calls.  The     */
/*              program is invoked:                                         */
/*                                                                          */
/*              CPICCREQ remote_filespec                                    */
/*                       [local_filespec                                    */
/*                       [symbolic_destination_name] ]                      */
/*                                                                          */
/*              The filespecs are valid OS/2 filenames, including drive and */
/*              path.  If no "local_filespec" is specified, both the local  */
/*              requester program and the partner server program use the    */
/*              remote_filespec in their DosOpen--so, any subdirectory      */
/*              specified must be valid at both the local and remote LU.    */
/*                                                                          */
/*              Uses the following CPI-C Calls:                             */
/*                                                                          */
/*                 CMINIT - Initialize_Conversation                         */
/*                 CMALLC - Allocate                                        */
/*                 CMSEND - Send_Data                                       */
/*                 CMRCV  - Receive_Data                                    */
/*                 CMCFMD - Confirmed                                       */
/*                                                                          */
/*              Uses symbolic_dest_name from DEFINE_CPIC_SIDE_INFO verb     */
/*              to set partner_LU, TP_Name at partner_LU, and mode name     */
/*              to be used by the CPI-C conversation.                       */
/*                                                                          */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
/*                   (compiles with small memory model)                     */
/*                   IBM C Set/2 Compiler for 32 bit applications.          */
/*                                                                          */
/*              Requires message file "APC.MSG" at runtime.                 */
/*                                                                          */
/****************************************************************************/

#define LINT_ARGS

#define INCL_DOSINFOSEG
#define INCL_DOSMISC
#define INCL_DOSSESMGR
#include <os2.h>
#include <cmc.h>                            /* CPI-C include file           */

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

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

#define PROGRAM_NAME        "CPICCREQ"      /* used by "do_usage()"         */
#define SERVER_TP_NAME      "FileServer"    /* used by set tp name          */
#define MODE_NAME           "#INTER"        /* used by set mode name        */

#define MESSAGE_FILENAME    "APC.MSG"       /* used by DosGetMessage        */

#define BUFFER_SIZE         (30920)         /* use a large buffer size for  */
                                            /* peak performance.            */
#define FILE_NAME_MAX_LEN   (261)           /* use longer file name buffers */
                                            /* to support HPFS              */

#define MAX_MESSAGE_LEN     (100)           /* length of the longest        */
                                            /* message in the message file  */

#define DEFAULT_SYM_DEST_NAME    "CPICSVR"  /* used if none is specified on */
                                            /* the command line             */

#define SYM_DEST_NAME_LEN   (9)             /* length of symbolic dest name */

/* These are the message numbers of specific messages in file APC.MSG */
#define MSG_SAMPLE_REQUESTER (2)
#define MSG_SAMPLE_COMPLETE  (3)
#define MSG_BYTES_RECEIVED   (6)
#define MSG_OS2_RETCODE      (7)
#define MSG_FILE_OPEN_ERROR  (8)
#define MSG_FILE_NOT_FOUND   (9)
#define MSG_UNEXPECTED_RCVD  (10)
#define MSG_DATA_INCOMPLETE  (11)
#define MSG_NO_SEND_PERMIT   (12)
#define MSG_VERB_OPCODE      (13)
#define MSG_CPIC_ERROR_RC    (15)
#define MSG_CPIC_ERROR_VERB  (16)
#define MSG_CMINIT_FAILURE   (17)


UCHAR local_filespec[FILE_NAME_MAX_LEN];    /* filespec for DosOpen         */
UCHAR remote_filespec[FILE_NAME_MAX_LEN];   /* filespec for REMOTE DosOpen  */
                                            /* If one filespec is specified */
                                            /* on the cmd line, these two   */
                                            /* strings will be the same.    */
                                            /* If there are two filespecs,  */
                                            /* the source (remote) filespec */
                                            /* is first filespec specfied   */

#ifdef LINT_ARGS
/*--------------------------------------------------------------------------*/
/*                       Function Prototypes                                */
/*--------------------------------------------------------------------------*/

void do_usage(void);
PCH  alloc_shared_buffer (const BUFF_SIZE display_buffer_size);
void cdecl main(int, char **);
void show_msg(const USHORT msgno);
void show_os2_retcode (const USHORT retcode);
void parse_filenames (const int argc, const char ** argv,
                      UCHAR FRPTR data_buffer,
                      UCHAR * local_filespec, UCHAR * remote_filespec,
                      UCHAR * symbolic_destination_name );
void show_cpic_err (char * cpic_verb, CM_RETURN_CODE conv_rc);

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

void cdecl main (int argc, char * argv[])   /* Need argc,argv to get        */
                                            /* Filename from command line.  */
{
  UCHAR  cm_conv_id[8];                     /* CPI-C Conversation id        */
  CM_RETURN_CODE conv_rc;                   /* Return code from CPI-C call  */
  CM_CONVERSATION_TYPE conv_type = CM_MAPPED_CONVERSATION;
  CM_SYNC_LEVEL sync_level = CM_CONFIRM;
  CM_RETURN_CONTROL return_control = CM_WHEN_SESSION_ALLOCATED;
  CM_SEND_TYPE send_type;
  CM_RECEIVE_TYPE receive_type = CM_RECEIVE_AND_WAIT;
  CM_INT32 length = 100;
  CM_INT32 max_receive_len = BUFFER_SIZE;
  CM_INT32 what_received;
  CM_INT32 received_len;
  CM_INT32 status_received;
  CM_INT32 rts_received;
  BOOL   done = FALSE;                      /* is the transaction finished? */
  USHORT dos_rc;                            /* OS/2 return code             */
  HFILE  filehandle;                        /* returned by DosOpen          */
  SIZE   action;                            /* returned by DosOpen          */
  SIZE   bytes_written;                     /* returned by DosWrite         */
  ULONG  byte_count = 0;                    /* count total bytes written    */
  PCH    data_buffer;                       /* pointer to shared memory seg */
                                            /* symbolic destination name    */
  UCHAR  symbolic_destination_name[SYM_DEST_NAME_LEN];
#ifdef ES32TO16
  ULONG  dos_rc1;                           /* Return code from DosFreeSeg  */
#endif

  /* Ask OS/2 to get the shared memory buffer.                              */
  data_buffer = alloc_shared_buffer(max((USHORT)BUFFER_SIZE,
                                        (USHORT)FILE_NAME_MAX_LEN));



  parse_filenames (argc,                    /* input - number of args       */
                   (const char **)argv,     /* input - arg list             */
                   data_buffer,             /* input - SEND_DATA from here  */
                   local_filespec,          /* output - first filespec      */
                   remote_filespec,         /* output - second filespec     */
                   symbolic_destination_name);/* output- sym dest if present*/

#ifndef MAX_SPEED
  show_msg(MSG_SAMPLE_REQUESTER);           /* Show the REQUESTER message   */
#endif

  cminit( cm_conv_id,                       /* Initialize Conversation      */
          symbolic_destination_name,
          &conv_rc);

  if (conv_rc != CM_OK) {show_cpic_err(cpic_calls[4],conv_rc); }

  cmsct( cm_conv_id,                        /* Set Conversation Type        */
          &conv_type,                       /* Mapped Conversation          */
          &conv_rc);
                                            /* Handle any errors            */
  if (conv_rc != CM_OK) {show_cpic_err(cpic_calls[6],conv_rc); }

  cmssl( cm_conv_id,                        /* Set Sync Level               */
         &sync_level,                       /* Sync level-confirm           */
         &conv_rc);
                                            /* Handle any errors            */
  if (conv_rc != CM_OK) {show_cpic_err(cpic_calls[16],conv_rc); }

  cmsrc( cm_conv_id,                        /* Set Return Control           */
         &return_control,                   /* Return when session alloc    */
         &conv_rc);
                                            /* Handle any errors            */
  if (conv_rc != CM_OK) {show_cpic_err(cpic_calls[14],conv_rc); }

  cmallc (cm_conv_id,                       /* Allocate Conversation        */
          &conv_rc);
                                            /* Handle any errors            */
  if (conv_rc != CM_OK) {show_cpic_err(cpic_calls[1],conv_rc); }


  send_type = CM_SEND_AND_FLUSH;            /* Set Send_Type to be used.    */
  cmsst (cm_conv_id,                        /* Set Send Type                */
         &send_type,
         &conv_rc);


  /* Send the remote filespec to the partner */

  length = FILE_NAME_MAX_LEN;               /* set the length of the send   */

  cmsend (cm_conv_id,                       /* Send Data                    */
          data_buffer,                      /* data pointer                 */
          &length,                          /* length of data sent          */
          &rts_received,                    /* request to send indicator    */
          &conv_rc);

  if (conv_rc != CM_OK) {show_cpic_err(cpic_calls[9], conv_rc); }

  /* Open the local file */
  dos_rc = DosOpen ((PCH)local_filespec,    /* addr. of file & path name    */
                    (PHFILE)&filehandle,    /* returned: filehandle address */
                    (PSIZE)&action,         /* returned: action address     */
                    (ULONG)0,               /* file primary allocation      */
                    FILE_NORMAL,            /* file attribute               */
                    OPEN_ACTION_REPLACE_IF_EXISTS |
                    OPEN_ACTION_CREATE_IF_NEW, /* function to be done       */
                    OPEN_ACCESS_READWRITE |
                    OPEN_SHARE_DENYWRITE,   /* Open mode of the file        */
                    (ULONG)0);              /* Reserved double word         */

  if (dos_rc != 0) {                        /* Non-zero OS/2 return code?   */
     show_os2_retcode (dos_rc);             /* Send OS/2 rc to stderr       */
     printf("\nfilespec = %s", local_filespec);
     show_msg (MSG_FILE_OPEN_ERROR);
     exit(dos_rc);
  }

  /**************************************************************************/
  /* Continue while there are still bytes to be read from the partner.      */
  /**************************************************************************/
#ifndef MAX_SPEED
  show_msg (MSG_BYTES_RECEIVED);
#endif
  while (!done) {

     cmsrt (cm_conv_id,                     /* Set Receive Type             */
            &receive_type,                  /* Receive and Wait             */
            &conv_rc);

                                            /* Handle any error             */
     if (conv_rc != CM_OK) {show_cpic_err(cpic_calls[15],conv_rc); }

     cmrcv (cm_conv_id,                     /* Receive Data                 */
            data_buffer,                    /* Data Pointer                 */
            &max_receive_len,               /* Size of Data Buffer          */
            &what_received,                 /* returned - what received     */
            &received_len,                  /* returned - length of data    */
            &status_received,               /* returned - status received   */
            &rts_received,                  /* returned - request to send   */
            &conv_rc);

     if (conv_rc != CM_OK) {
        if (conv_rc == CM_PROGRAM_ERROR_NO_TRUNC) {
           show_msg (9);
           exit(EXIT_FAILURE);
        } else {
           show_cpic_err(cpic_calls[5],conv_rc);
        } /* endif */
     } else {
     } /* endif */

                                            /* Handle any error             */
     length = (USHORT)received_len;         /* Get length we actually rcvd  */


     if ( (what_received == CM_COMPLETE_DATA_RECEIVED)  ||
          (what_received == CM_INCOMPLETE_DATA_RECEIVED) )  {

        /* Write the received data to the local file.                       */
        dos_rc = DosWrite (filehandle,
                           data_buffer,
                           (SIZE)length,
                           (PSIZE)&bytes_written);
        if (dos_rc != 0) {                  /* Non-zero OS/2 return code?   */
           show_os2_retcode (dos_rc);       /* Send OS/2 rc to stderr       */
           exit(dos_rc);
        }

        byte_count += (ULONG) bytes_written; /* Increment the count         */

#ifndef MAX_SPEED
     /* Since this printf uses \r instead of \n, only one line appears--an  */
     /* example of a carriage return without a line feed.                   */
     printf ("%lu\r", byte_count);
#endif


        /* Confirm_deallocate says EOF has been reached */
        if ( status_received == CM_CONFIRM_DEALLOC_RECEIVED ) {
           done = TRUE;
           cmcfmd (cm_conv_id,              /* Confirmed                    */
           &conv_rc);
        }
     } else {
        /* Since there was no data, we must be done.                        */
        done = TRUE;

        /* There is a chance that status did not arrive with data.          */
        if ( status_received == CM_CONFIRM_DEALLOC_RECEIVED ) {
           cmcfmd (cm_conv_id, &conv_rc);   /* And confirm that we are done */

        } else {
           show_msg(MSG_UNEXPECTED_RCVD);   /* so show Bad WHAT_RCVD msg.   */
           printf ("%04X", what_received);  /* and quit                     */
        }
     }
  }

  /**************************************************************************/
  /* Close the file and clean up the conversation and TP.                   */
  /**************************************************************************/
  dos_rc = DosClose(filehandle);            /* Close the file               */
  if (dos_rc != 0)                          /* Non-zero OS/2 return code?   */
     show_os2_retcode (dos_rc);             /* Send OS/2 rc to stderr       */

  /**************************************************************************/
  /* NO STORAGE IS FREED by the following call to DosFreeSeg!               */
  /* The following call simply decrements the OS/2 allocation count.        */
  /* For maximum performance, APPC internally locks all data segments       */
  /* passed to it until the using process ends.  See the "APPC Programming  */
  /* Reference" for more details.                                           */
  /**************************************************************************/
#ifdef ES32TO16
   /*--------------------------------------------*/
   /* 32 bit memory management                   */
   /*--------------------------------------------*/

  if ( 0 != ( dos_rc1 = DosFreeMem( data_buffer ) ) ) {
      show_os2_retcode((USHORT) dos_rc1);   /* Exit the program.            */
  }
   /*--------------------------------------------*/
   /* End of 32 bit memory management            */
   /*--------------------------------------------*/
#else
   /*--------------------------------------------*/
   /* 16 bit memory management                   */
   /*--------------------------------------------*/
  if ( 0 != ( dos_rc = DosFreeSeg(SELECTOROF( data_buffer )) ) ) {
      show_os2_retcode((USHORT) dos_rc);   /* Exit the program.            */
  }
   /*--------------------------------------------*/
   /* End of 16 bit memory management            */
   /*--------------------------------------------*/
#endif

#ifndef MAX_SPEED
  printf("\n");
  show_msg(MSG_SAMPLE_COMPLETE);
#endif

  exit (EXIT_SUCCESS);                      /* Set the ERRORLEVEL to 0      */
}

/****************************************************************************/
/*                                                                          */
/*                        UTILITY FUNCTIONS                                 */
/*                                                                          */
/****************************************************************************/

void
show_msg (const USHORT msgno)
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
  if (dos_rc != 0) {                        /* Non-zero OS/2 return code?   */
     fprintf(stderr,
             "\nAPPC Sample Program Error: Unable to process message file");
     fprintf(stderr,
             "\nDosGetMessage return code = %04d, message number = %04d",
             dos_rc, msgno);
  }
  else {
     msg_buff[msg_len] = '\0';              /* Make this an ASCIIZ string   */
     printf("\n");                          /* Start on a new line          */
     printf("%s", msg_buff);                /* Display the message          */
  }
}


void
parse_filenames (const int argc,            /* input - number of args       */
                 const char ** argv,        /* input - arg list             */
                 PCH   data_buffer,         /* input - send from this buffer*/
                 UCHAR * local_filespec,    /* output - first filespec      */
                 UCHAR * remote_filespec,   /* output - second filespec     */
                 UCHAR * sym_dest_name)     /* output - sym dest  if present*/

/*   Convert requested filespec (from the command line) into an ASCIIZ      */
/*   string that is headed by a LL (binary string length) byte.             */
/*   Place this string in the APPC data buffer in shared unnamed storage    */
{
  register USHORT filespec_len;             /* Includes the null byte       */

  if ((argc == 1) || argv[1][0] == '?') {   /* no params or question mark   */
     do_usage();
  }

  if (argc > 2) {                           /* Save the local filespec      */
     if (strlen(argv[2]) > FILE_NAME_MAX_LEN - 1 ) {
        fprintf(stderr,"Local filespec exceeds maximum allowed.\n");
        exit(99);
     } else {
     } /* endif */
     strcpy(local_filespec, argv[2]);       /* use the second filespec for  */
                                            /* local file if it is specified*/
  } else {
     if (strlen(argv[1]) > FILE_NAME_MAX_LEN - 1 ) {
        fprintf(stderr,"Remote filespec exceeds maximum allowed.\n");
        exit(99);
     } else {
     } /* endif */
     strcpy(local_filespec, argv[1]);       /* use same filespec if not     */
  }
  strcpy (remote_filespec, argv[1]);        /* Save the remote filespec     */
  filespec_len = strlen(remote_filespec);   /* Save its length              */


  data_buffer[0] = (UCHAR)(++filespec_len + 1);
                                            /* Put length byte in string    */
                                            /* Add 1 for length byte itself */
                                            /* and 1 for null byte on end.  */

  for (;filespec_len; ) {                   /* Move filespec, starting at   */
     filespec_len--;                        /* end and working down to zero */
                                            /* this includes the null byte  */
     data_buffer[filespec_len + 1] = remote_filespec[filespec_len];
  }

  if (argc > 3) {         /* was a plu alias specified on the command line? */
        memset (sym_dest_name,(int)' ',SYM_DEST_NAME_LEN);/* init to blanks */
        filespec_len = min(strlen(argv[3]), SYM_DEST_NAME_LEN);
        for ( ;filespec_len; ) {
           filespec_len--;                  /* copy the arg into plu alias  */
           sym_dest_name[filespec_len] = (UCHAR)argv[3][filespec_len];
        } /* endfor */
  } else {
     /* use the default */
     memset (sym_dest_name,(int)' ',SYM_DEST_NAME_LEN);   /* init to blanks */
     memcpy(sym_dest_name,
            DEFAULT_SYM_DEST_NAME,
            strlen(DEFAULT_SYM_DEST_NAME));
  } /* endif */

}


void show_os2_retcode(const USHORT retcode)
{
   show_msg (MSG_OS2_RETCODE);              /* OS/2 return code message     */
   fprintf (stderr, "%d", retcode);         /* Show return code in decimal  */
}


/****************************************************************************/
/*                                                                          */
/*                  OS/2 RELATED FUNCTIONS                                  */
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
      show_os2_retcode((USHORT) dos_rc);             /* Exit the program.  */
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
      show_os2_retcode((USHORT) dos_rc);             /* Exit the program.  */
  }
   /*--------------------------------------------*/
   /* End of 16 bit memory management            */
   /*--------------------------------------------*/
#endif
  return result;                            /* Return the buffer address    */
}


void show_cpic_err (char * cpic_verb, CM_RETURN_CODE conv_rc)


/* Displays the CPI-C return code and the verb name */

{
  show_msg (MSG_CPIC_ERROR_RC);             /* CPI-C Error = msg.           */
  fprintf (stderr,"%08d\n",conv_rc);        /* Show CPI-C rc                */
  show_msg (MSG_CPIC_ERROR_VERB);           /* APPC verb in error           */
  fprintf (stderr,"%s\n",cpic_verb);        /* Show verb name of verb in err*/
  exit((USHORT) conv_rc);
}


void do_usage(void)
{
  fprintf(stderr,
  "\nUsage:\n");
  fprintf(stderr,
  "%s ", PROGRAM_NAME);
  fprintf(stderr,
  "remote_filespec [local_filespec [partner_LU_name] ]\n\n");
  fprintf(stderr,
  "Where:\n");
  fprintf(stderr,
  "   remote_filespec - the source file to be copied.\n");
  fprintf(stderr,
  "      Filespec needs to include drive, path, and filename.\n");
  fprintf(stderr,
  "   local_filespec (optional) - a new filename for the destination.\n");
  fprintf(stderr,
  "      The default is to use the same as the remote_filespec.\n");
  fprintf(stderr,
  "   symbolic_destination_name (optional) - the name\n");
  fprintf(stderr,
  "      given in a configured CPIC_SIDE_INFO verb\n");

  exit(EXIT_FAILURE);
}

/* EOF - CPICCREQ.C */
