/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : FILECREQ.C                                                */
/*                                                                          */
/*  DESCRIPTIVE : APPC FILE REQUESTER "C" SAMPLE PROGRAM                    */
/*  NAME          FOR IBM Communications Manager                            */
/*                                                                          */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991                         */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*                                                                          */
/*  FUNCTION:   Issues a request for a file to a partner ("server") and     */
/*              transfers the file to the current directory (if no          */
/*              local filespec is specified).  This program is started      */
/*              as follows, where the local filespec and partner LU are     */
/*              optional parameters:                                        */
/*                                                                          */
/*              FILECREQ remote_filespec                                    */
/*                       [local_filespec                                    */
/*                       [plu_alias | fqplu_name ] ]                        */
/*                                                                          */
/*              The filespecs are valid OS/2 filenames, including drive and */
/*              path.  If no "local_filespec" is specified, both the local  */
/*              requester program and the partner server program use the    */
/*              remote_filespec in their DosOpen--so, any subdirectory      */
/*              specified must be valid at both the local and remote LU.    */
/*                                                                          */
/*              Uses the following APPC Verbs:                              */
/*                                                                          */
/*                 TP_STARTED                                               */
/*                 MC_ALLOCATE                                              */
/*                 MC_SEND_DATA                                             */
/*                 MC_RECEIVE_AND_WAIT                                      */
/*                 MC_CONFIRMED                                             */
/*                 TP_ENDED                                                 */
/*                                                                          */
/*              Uses the following ACSSVC (Common Services) Verbs:          */
/*                                                                          */
/*                 CONVERT                                                  */
/*                                                                          */
/*   MODULE:    Microsoft C Compiler                                        */
/*   TYPE       (Compiles with Small Memory Model)                          */
/*                                                                          */
/*              Requires message file "APX.MSG" at runtime, unless the      */
/*              message file is bound to the ".EXE" file.                   */
/*              Files APPC.DLL and ACSSVC.DLL must be on your LIBPATH at    */
/*              runtime.                                                    */
/*                                                                          */
/*                                                                          */
/*              The previous versions of the sample programs shipped in     */
/*              OS/2 EE 1.0 through 1.3 do not communicate correctly with   */
/*              these new sample programs.  The previous versions did not   */
/*              handle the case where they received DATA_INCOMPLETE on a    */
/*              RECEIVE_AND_WAIT verb because the partner was using a       */
/*              larger buffer size.  The previous versions can be compiled  */
/*              and run as they were originally designed and communicate    */
/*              between themselves using this level of Communications       */
/*              Manager.                                                    */
/*                                                                          */
/* Changes since OS/2 EE 1.2 and 1.3                                        */
/*                                                                          */
/* 1)  Added #define for MAX_SPEED.  If defined, no screen I/O will take    */
/*     place.                                                               */
/* 2)  Added #define for RCV_BUFFER_SIZE to allow this to be changed and    */
/*     experimented with easily.                                            */
/* 3)  Added #define for MAX_FILENAME_LENGTH as part of the long filename   */
/*     support.                                                             */
/* 4)  Added #defines for many APPC parameters.                             */
/* 5)  Distinguished between remote and local filenames.  Needed to add     */
/*     another global variable.                                             */
/* 6)  Handled AP_DATA_INCOMPLETE (treat as DATA_COMPLETE and get the       */
/*     rest of the data on the next receive).                               */
/* 7)  Restructured the main loop of the program for clarity.  Removed      */
/*     CLEAR_VCB from the send_data loop.                                   */
/* 8)  Removed unnecessary CLEAR_VCB's and resetting of the same fields     */
/*     in convert calls.                                                    */
/* 9)  Added processing in PARSE_FILENAMES to set the local filespec,       */
/*     the plu_alias, and in APPN, the fqplu_name if specified.             */
/*10)  Changed the #include statements to use the new include files         */
/*     in the best way.                                                     */
/*11)  Changed variable declarations to use the OS/2 typedefs.              */
/*12)  Added "cdecl" to the main() procedure.                               */
/*13)  Added support for source and destination file names.                 */
/*14)  Added support for specifying the Partner LU alias or the Fully       */
/*     Qualified LU Name on the command line.                               */
/*15)  Restructured the subroutines to pass parameters instead of using     */
/*     global variables.  This should make it easier to re-use this code.   */
/*16)  Added "const"                                                        */
/*                                                                          */
/****************************************************************************/

#define LINT_ARGS

#define INCL_DOSINFOSEG
#define INCL_DOSMISC
#define INCL_DOSSESMGR
#include <os2.h>
#include <appc_c.h>
#include <acssvcc.h>

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#define PROGRAM_NAME        "FILECREQ"      /* used by "do_usage()"         */

#define LOCAL_TP_NAME       "FILEREQ"       /* used by the TP_STARTED verb  */
#define REMOTE_TP_NAME      "FileServer"    /* sent on the MC_ALLOCATE      */
#define MODE_NAME           "#INTER"        /* sent on the MC_ALLOCATE      */
#define DEFAULT_PLU         "FILESVR"       /* use this for the MC_ALLOCATE */
                                            /* if none is supplied on the   */
                                            /* command line                 */
#define LU_ALIAS            "\0\0\0\0\0\0\0\0"
                                            /* Set LU_ALIAS to all zeros,   */
                                            /* which indicates we want to   */
                                            /* use the default LU.  This    */
                                            /* eliminates the need to       */
                                            /* configure a special local LU */
                                            /* for FILECREQ.  It also       */
                                            /* eliminates the possibility   */
                                            /* of defining the FILEREQ LU   */
                                            /* twice in the network, which  */
                                            /* could cause an alert in APPN */
#define MESSAGE_FILENAME    "APX.MSG"       /* used by DosGetMessage        */

#define BUFFER_SIZE         (61440)         /* use a large buffer size for  */
                                            /* peak performance.            */
#define FILE_NAME_MAX_LEN   (261)           /* use longer file name buffers */
                                            /* to support HPFS              */
#define FQ_LEN              (17)            /* fully-qualified name length  */
#define PLU_ALIAS_LEN       (8)             /* partner LU alias length      */

#define STATUS_MASK         (0x00FF)        /* Masks to obtain status, data */
#define DATA_MASK           (0xFF00)        /* from variable what_rcvd      */

#define MAX_MESSAGE_LEN     (100)           /* length of the longest        */
                                            /* message in the message file  */

#define TRUE                1
#define FALSE               0

/* These are the message numbers of specific messages in file APX.MSG */
#define MSG_SAMPLE_REQUESTER (2)
#define MSG_SAMPLE_COMPLETE  (3)
#define MSG_BYTES_RECEIVED   (8)
#define MSG_PRIMARY_RC       (10)
#define MSG_SECONDARY_RC     (11)
#define MSG_OS2_RETCODE      (12)
#define MSG_FILE_OPEN_ERROR  (13)
#define MSG_UNEXPECTED_RCVD  (15)
#define MSG_DATA_INCOMPLETE  (16)
#define MSG_NO_SEND_PERMIT   (17)
#define MSG_VERB_OPCODE      (18)
#define MSG_SENSE_DATA       (19)

/* Macro BLANK_STRING sets string to all blanks */
#define BLANK_STRING(str)  memset(str,(int)' ',sizeof(str))


UCHAR tp_id[8];                             /* APPC transaction program ID  */
ULONG conv_id;                              /* APPC conversation ID         */

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
USHORT do_mc_receive_and_wait(UCHAR far * buffer,
                              const USHORT max_len, PUSHORT length);
void ascii2ebcdic(UCHAR * string, const UINT length, const UCHAR type);
void do_tp_started(const UCHAR * lu_alias, const UCHAR * tp_name);
void do_mc_allocate(const UCHAR * plu_alias, const UCHAR * fqplu_name,
                    const UCHAR * tp_name, const UCHAR * mode_name,
                    const UCHAR sync_level);
void do_mc_send_data(UCHAR far * buffer, const USHORT length,
                     const UCHAR type);
UCHAR far * alloc_shared_buffer(const USHORT size);
void convert_fqplu_name(UCHAR * fqplu_name);
void cdecl main(int, char **);
void show_msg(const USHORT msgno);
void show_verb_retcode(const USHORT opcode, const USHORT primary_rc,
                       const ULONG secondary_rc, const ULONG sense_data);
void show_os2_retcode (const USHORT retcode);
void parse_filenames (const int argc, const char ** argv,
                      UCHAR far * data_buffer,
                      UCHAR * local_filespec, UCHAR * remote_filespec,
                      UCHAR * plu_alias, UCHAR * fqplu_name );
void do_mc_confirmed(void);
void do_tp_ended(const UCHAR type);

#endif

/****************************************************************************/
/*                       Main Program Section                               */
/****************************************************************************/

void cdecl main (int argc, char * argv[])   /* Need argc,argv to get        */
                                            /* Filename from command line.  */
{
  BOOL   done = FALSE;                      /* is the transaction finished? */
  USHORT dos_rc;                            /* OS/2 return code             */
  HFILE  filehandle;                        /* returned by DosOpen          */
  USHORT action;                            /* returned by DosOpen          */
  USHORT bytes_written;                     /* returned by DosWrite         */
  USHORT length;                            /* length of received data      */
  USHORT what_rcvd;                         /* returned what_received value */
  ULONG  byte_count = 0;                    /* count total bytes written    */
  UCHAR  far *data_buffer;                  /* pointer to shared memory     */
                                            /* segment                      */
  UCHAR  plu_alias[PLU_ALIAS_LEN];          /* partner LU alias             */

#ifdef APPN
  UCHAR fqplu_name[FQ_LEN];                 /* fully qualified partner LU   */
                                            /* name                         */
#endif

#ifndef MAX_SPEED
  show_msg(MSG_SAMPLE_REQUESTER);           /* Show the REQUESTER message   */
#endif

  /* Ask OS/2 to get the shared memory buffer.                              */
  data_buffer = alloc_shared_buffer(max((USHORT)BUFFER_SIZE,
                                        (USHORT)FILE_NAME_MAX_LEN));



  parse_filenames (argc,                    /* input - number of args       */
                   argv,                    /* input - arg list             */
                   data_buffer,             /* input - SEND_DATA from here  */
                   local_filespec,          /* output - first filespec      */
                   remote_filespec,         /* output - second filespec     */
                   plu_alias,               /* output - plu alias if present*/
                   fqplu_name );            /* output - if present          */


  /* Tell APPC a transaction program has started. */
  do_tp_started (LU_ALIAS, LOCAL_TP_NAME);

  /* Allocate a session and start a conversation. */
  do_mc_allocate(plu_alias,
                 fqplu_name,
                 REMOTE_TP_NAME,
                 MODE_NAME,
                 AP_CONFIRM_SYNC_LEVEL);


  /* Send the remote filespec to the partner */
  do_mc_send_data(data_buffer, FILE_NAME_MAX_LEN, AP_FLUSH);

  /* Open the local file */
  dos_rc = DosOpen ((char far *)local_filespec,/* addr. of file & path name*/
                    (PHFILE)&filehandle,    /* returned: filehandle address */
                    (PUSHORT)&action,       /* returned: action address     */
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
  show_msg (MSG_BYTES_RECEIVED);
  while (!done) {
     what_rcvd = do_mc_receive_and_wait(data_buffer,
                                        (USHORT)BUFFER_SIZE, &length);

     /* By using a mask, look at only the "data_received" byte of the       */
     /* "what_received" parameter.  If the data is not yet complete, the    */
     /* program will get more the next time it issues MC_RECEIVE_AND_WAIT.  */
     if ((what_rcvd & DATA_MASK) == AP_DATA_COMPLETE ||
         (what_rcvd & DATA_MASK) == AP_DATA_INCOMPLETE) {

        /* Write the received data to the local file.                       */
        dos_rc = DosWrite (filehandle,
                           data_buffer,
                           length,
                           (PUSHORT)&bytes_written);
        if (dos_rc != 0) {                  /* Non-zero OS/2 return code?    */
           show_os2_retcode (dos_rc);       /* Send OS/2 rc to stderr        */
           exit(dos_rc);
        }

        byte_count += (ULONG) bytes_written; /* Increment the count          */

#ifndef MAX_SPEED
     /* Since this printf uses \r instead of \n, only one line appears--an  */
     /* example of a carriage return without a line feed.                   */
     printf ("%lu\r", byte_count);
#endif


        /* Confirm_deallocate says EOF has been reached */
        if ((what_rcvd & STATUS_MASK) == AP_CONFIRM_DEALLOCATE ) {
           done = TRUE;
           do_mc_confirmed();
        }
     } else {
        /* Since there was no data, we must be done.                     */
        done = TRUE;

        /* There is a chance that status did not arrive with data.       */
        if ((what_rcvd & STATUS_MASK) == AP_CONFIRM_DEALLOCATE ) {
           do_mc_confirmed();            /* Confirm_dealloc says EOF     */
        } else {
           show_msg(MSG_UNEXPECTED_RCVD); /* so show Bad WHAT_RCVD msg.  */
           printf ("%04X", what_rcvd);   /* and quit                     */
        }
     }
  }

  /**************************************************************************/
  /* Close the file and clean up the conversation and TP.                   */
  /**************************************************************************/
  dos_rc = DosClose(filehandle);            /* Close the file               */
  if (dos_rc != 0)                          /* Non-zero OS/2 return code?   */
     show_os2_retcode (dos_rc);             /* Send OS/2 rc to stderr       */

  /* The partner deallocates the conversation.                              */
  do_tp_ended(AP_SOFT);                     /* Free the TP's resources      */

  /**************************************************************************/
  /* NO STORAGE IS FREED by the following call to DosFreeSeg!               */
  /* The following call simply decrements the OS/2 allocation count.        */
  /* For maximum performance, APPC internally locks all data segments       */
  /* passed to it until the using process ends.  See the "APPC Programming  */
  /* Reference" for more details.                                           */
  /**************************************************************************/
  dos_rc = DosFreeSeg(SELECTOROF(data_buffer)); /* No memory gets freed!    */
  if (dos_rc != 0) {                        /* Non-zero OS/2 return code?   */
     show_os2_retcode (dos_rc);             /* Send OS/2 rc to stderr       */
     exit(dos_rc);                          /* Exit the program             */
  }

#ifndef MAX_SPEED
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
  USHORT dos_rc;                            /* OS/2 return code             */
  USHORT msg_len;                           /* returned: message length     */
  UCHAR msg_buff[MAX_MESSAGE_LEN];          /* Buffer for message data      */

  /* Read the requested message from the message file.                      */
  dos_rc = DosGetMessage ((char far *)0,    /* no IvTable in use            */
                          0,                /* no IvTable in use            */
                          (char far *)msg_buff, /* Place the message here   */
                          sizeof(msg_buff), /* Length of the buffer         */
                          msgno,            /* Which Message?               */
                          (char far *)MESSAGE_FILENAME, /* Msg filename     */
                          (PUSHORT)&msg_len); /* length of message returned */
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
                 UCHAR far * data_buffer,   /* input - send from this buffer*/
                 UCHAR * local_filespec,    /* output - first filespec      */
                 UCHAR * remote_filespec,   /* output - second filespec     */
                 UCHAR * plu_alias,         /* output - plu alias if present*/
                 UCHAR * fqplu_name )       /* output - if present          */

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

     if (strchr(argv[3], (int)'.') == NULL) {/* was there a period?         */
        memset (plu_alias,(int)' ',PLU_ALIAS_LEN);/* init to blanks         */
        filespec_len = min(strlen(argv[3]), PLU_ALIAS_LEN);
        for ( ;filespec_len; ) {
           filespec_len--;                  /* copy the arg into plu alias  */
           plu_alias[filespec_len] = (UCHAR)argv[3][filespec_len];
        } /* endfor */
#ifdef APPN
        memset(fqplu_name,(int)'\0',FQ_LEN);/* zero out fqplu name          */
#endif
     } else {                               /* must be fully qualified!     */
        memset(plu_alias,(int)'\0',PLU_ALIAS_LEN);/* zero out plu alias     */
#ifdef APPN
        memset(fqplu_name,(int)' ',FQ_LEN); /* blank out fqplu name         */
        filespec_len = min(strlen(argv[3]), FQ_LEN);
        for ( ;filespec_len; ) {
           filespec_len--;                  /* copy the arg                 */
           fqplu_name[filespec_len] = (UCHAR)toupper(argv[3][filespec_len]);
        } /* endfor */

#endif

     } /* endif */
  } else {
     /* use the default */
     memcpy(plu_alias, DEFAULT_PLU, PLU_ALIAS_LEN); /* use the default      */
     fqplu_name[0] = '\0';                  /* set fully qualified to zero  */
  } /* endif */

}


void show_os2_retcode(const USHORT retcode)
{
   show_msg (MSG_OS2_RETCODE);              /* OS/2 return code message     */
   fprintf (stderr, "%d", retcode);         /* Show return code in decimal  */
}

/****************************************************************************/
/*                                                                          */
/*                        APPC RELATED FUNCTIONS                            */
/*                                                                          */
/****************************************************************************/


void do_tp_started(const UCHAR * lu_alias, const UCHAR * tp_name)
{
  TP_STARTED tp_started;                    /* Declare a verb control block */
  TP_STARTED far *ptr_tp_started = (TP_STARTED far *)&tp_started;
  USHORT length;                            /* length of lu_alias           */

  CLEAR_VCB(tp_started);                    /* Zero the verb control block  */
  tp_started.opcode = AP_TP_STARTED;        /* APPC verb - TP_STARTED       */

  if ((length = (USHORT)strlen(lu_alias)) > 0) {  /* Use a real LU alias?   */
     BLANK_STRING(tp_started.lu_alias);     /* Set it to ASCII blanks       */
     memcpy(tp_started.lu_alias, lu_alias,
                                 min(length, sizeof(tp_started.lu_alias)));
  } else {                                  /* Zero length; use the default */
     memset(tp_started.lu_alias, (int)'\0', sizeof(tp_started.lu_alias));
  }

  BLANK_STRING(tp_started.tp_name);         /* Set 64-byte string to blanks */
  memcpy (tp_started.tp_name, tp_name,
                            min(strlen(tp_name), sizeof(tp_started.tp_name)));
  ascii2ebcdic(tp_started.tp_name,
             sizeof(tp_started.tp_name),
             SV_AE);

  APPC ((ULONG) ptr_tp_started);            /* Issue the verb               */

  if (tp_started.primary_rc != AP_OK) {
     show_verb_retcode(tp_started.opcode,
                       tp_started.primary_rc,
                       tp_started.secondary_rc,
                       0L);               /* No SNA sense data on this verb */
     exit (tp_started.primary_rc);
  } else {  /* Save the returned tp_id in global variable                   */
     memcpy (tp_id, tp_started.tp_id, sizeof(tp_id));
  }
}


void
do_mc_allocate(const UCHAR * plu_alias,
               const UCHAR * fqplu_name,
               const UCHAR * tp_name,
               const UCHAR * mode_name,
               const UCHAR sync_level)
{
  MC_ALLOCATE allocate;                     /* Declare a verb control block */
  MC_ALLOCATE far *ptr_allocate = (MC_ALLOCATE far *)&allocate;

  CLEAR_VCB(allocate);                      /* Zero the vcb                 */
  allocate.opcode = AP_M_ALLOCATE;          /* Verb - MC_ALLOCATE           */
  allocate.opext = AP_MAPPED_CONVERSATION;  /* Mapped Conversation type     */

  memcpy (allocate.tp_id, tp_id, sizeof(tp_id)); /* Set the TP_ID           */
  allocate.sync_level = sync_level;         /* Sync level-confirm           */
  allocate.rtn_ctl = AP_WHEN_SESSION_ALLOCATED;/* Return when ses allocated */
  allocate.security = AP_NONE;              /* No security                  */

  if (strlen(plu_alias)) {
     BLANK_STRING(allocate.plu_alias);
     memcpy (allocate.plu_alias, plu_alias,
                          min(strlen(plu_alias), sizeof(allocate.plu_alias)));
                                            /* Set PLU_ALIAS                */
  } else {
     memset (allocate.plu_alias,(int)'\0',sizeof(allocate.plu_alias));
  } /* endif */

#ifdef APPN
  if (strlen(fqplu_name)) {
     BLANK_STRING(allocate.fqplu_name);     /* Set FQ PLU NAME              */
     memcpy (allocate.fqplu_name, fqplu_name,
                        min(strlen(fqplu_name), sizeof(allocate.fqplu_name)));
     convert_fqplu_name(allocate.fqplu_name);
  } else {
  } /* endif */

#endif

  BLANK_STRING(allocate.tp_name);           /* Set 64-byte string to blanks */
  memcpy (allocate.tp_name, tp_name,
                            min(strlen(tp_name), sizeof(allocate.tp_name)));
  ascii2ebcdic(allocate.tp_name, sizeof(allocate.tp_name), SV_AE);

  BLANK_STRING(allocate.mode_name);         /* Set 8-byte string to blanks  */
  memcpy (allocate.mode_name,mode_name,
                          min(strlen(mode_name), sizeof(allocate.mode_name)));
  ascii2ebcdic(allocate.mode_name, sizeof(allocate.mode_name), SV_A);

  APPC((ULONG) ptr_allocate);               /* Issue the verb               */

  if (allocate.primary_rc != AP_OK) {
     show_verb_retcode(allocate.opcode,
                       allocate.primary_rc,
                       allocate.secondary_rc,
                       allocate.sense_data);
     exit (allocate.primary_rc);
  } else {
     conv_id = allocate.conv_id;           /* Save the conversation ID     */
  } /* endif */
}


void do_mc_send_data (UCHAR far * buffer,
                      const USHORT length, const UCHAR type)
{
  MC_SEND_DATA send_data;                   /* Declare a verb control block */
  MC_SEND_DATA far *ptr_send_data = (MC_SEND_DATA far *)&send_data;

  CLEAR_VCB(send_data);                     /* Zero the verb control block  */
  send_data.opcode = AP_M_SEND_DATA;        /* APPC Verb - MC_SEND_DATA     */
  send_data.opext = AP_MAPPED_CONVERSATION; /* Mapped Conservation type     */
  memcpy (send_data.tp_id, tp_id, sizeof(tp_id));  /* Set the tp_id         */
  send_data.conv_id = conv_id;              /* Set conversation_id          */
  send_data.dlen = length;                  /* Length of data to send       */
  send_data.dptr = buffer;                  /* Set data pointer to buffer   */
  send_data.type = type;                    /* Set the passed type          */

  APPC((ULONG) ptr_send_data);              /* Issue the verb.              */

  /* If you use this routine in another program or change the verb          */
  /* sequences, other return codes may be expected.  Your program should    */
  /* alwalys handle non-zero return codes more fully than simply informing  */
  /* to the user the fact that an error occurred.                           */
  if (send_data.primary_rc != AP_OK) {
     show_verb_retcode(send_data.opcode,
                       send_data.primary_rc,
                       send_data.secondary_rc,
                       0L);               /* No SNA sense data on this verb */
     exit (send_data.primary_rc);
  }
}


USHORT do_mc_receive_and_wait(UCHAR far * buffer,
                              const USHORT max_len,
                              PUSHORT length)
{
  static RECEIVE_AND_WAIT receive_and_wait; /* Declare a verb control block */
  static RECEIVE_AND_WAIT far *ptr_receive_and_wait =
                                 (RECEIVE_AND_WAIT far *)&receive_and_wait;

#ifdef MAX_SPEED
  static BOOL first_time = TRUE;

  if (first_time) {
#endif

     CLEAR_VCB(receive_and_wait);           /* Zero the verb control block  */
     receive_and_wait.opcode = AP_M_RECEIVE_AND_WAIT; /* Set verb opcode    */
     receive_and_wait.opext = AP_MAPPED_CONVERSATION; /* Set verb type      */
     receive_and_wait.conv_id = conv_id;    /* Set saved conversation ID    */
     memcpy(receive_and_wait.tp_id, tp_id, sizeof(tp_id)); /* Set the tp_id */
     receive_and_wait.max_len = max_len;    /* Max length of buffer         */
     receive_and_wait.dptr = buffer;        /* Set data buffer pointer      */
     receive_and_wait.rtn_status = AP_YES;  /* Return status with data      */

#ifdef MAX_SPEED
     first_time = FALSE;
  }
#endif

  APPC((ULONG) ptr_receive_and_wait);       /* Issue the verb.              */

  /* If you use this routine in another program or change the verb          */
  /* sequences, other return codes may be expected.  Your program should    */
  /* alwalys handle non-zero return codes more fully than simply informing  */
  /* to the user the fact that an error occurred.                           */
  if (receive_and_wait.primary_rc != AP_OK) {
     show_verb_retcode(receive_and_wait.opcode,
                       receive_and_wait.primary_rc,
                       receive_and_wait.secondary_rc,
                       0L);               /* No SNA sense data on this verb */
     exit (receive_and_wait.primary_rc);
  } else {
     *length = receive_and_wait.dlen;       /* Get length we actually rcvd  */
     return (*(USHORT *)&receive_and_wait.what_rcvd);
  }
}


void do_mc_confirmed(void)
{
  CONFIRMED confirmed;                      /* Declare a verb control block */
  CONFIRMED far *ptr_confirmed = (CONFIRMED far *)&confirmed;

  CLEAR_VCB(confirmed);                     /* Zero the vcb                 */
  confirmed.opcode = AP_M_CONFIRMED;        /* Verb - CONFIRMED             */
  confirmed.opext = AP_MAPPED_CONVERSATION; /* Mapped Conversation type     */
  confirmed.conv_id = conv_id;              /* Set conversation_id          */
  memcpy (confirmed.tp_id, tp_id, sizeof(tp_id)); /* Set tp_id              */

  APPC ((ULONG) ptr_confirmed);             /* Issue the verb               */

  if (confirmed.primary_rc != AP_OK) {
     show_verb_retcode(confirmed.opcode,
                       confirmed.primary_rc,
                       confirmed.secondary_rc,
                       0L);               /* No SNA sense data on this verb */
     exit (confirmed.primary_rc);
  }
}


void do_tp_ended (const UCHAR type)
{
  TP_ENDED tp_ended;                        /* Declare a verb control block */
  TP_ENDED far *ptr_tp_ended = (TP_ENDED far *)&tp_ended;

  CLEAR_VCB(tp_ended);                      /* Zero the verb control block  */
  tp_ended.opcode = AP_TP_ENDED;            /* Set the verb opcode          */
  memcpy (tp_ended.tp_id, tp_id, sizeof(tp_id)); /* Set the tp_id           */
  tp_ended.type = type;                     /* type: AP_HARD or AP_SOFT     */

  APPC((ULONG) ptr_tp_ended);               /* Issue the verb.              */

  /* If you use this routine in another program or change the verb          */
  /* sequences, other return codes may be expected.  Your program should    */
  /* alwalys handle non-zero return codes more fully than simply informing  */
  /* to the user the fact that an error occurred.                           */
  if (tp_ended.primary_rc != AP_OK) {
     show_verb_retcode(tp_ended.opcode,
                       tp_ended.primary_rc,
                       tp_ended.secondary_rc,
                       0L);               /* No SNA sense data on this verb */
  }
}

/****************************************************************************/
/*                                                                          */
/*                  OS/2 RELATED FUNCTIONS                                  */
/*                                                                          */
/****************************************************************************/

UCHAR far * alloc_shared_buffer (const USHORT size)
/* APPC requires a data buffer in a shared unnamed segment.                 */
{
  USHORT selector;                          /* selector from DosAllocSeg    */
  USHORT dos_rc;                            /* OS/2 return code             */
  UCHAR far *memory_pointer;                /* return pointer to memory     */

  dos_rc = DosAllocSeg ((unsigned)size,     /* size of memory to allocate   */
                        (PSEL)&selector,    /* returned: selector address   */
                        (unsigned)SEG_GIVEABLE);
                                            /* shared, unmamed segment      */
  if (dos_rc != 0) {                        /* Non-zero OS/2 return code?   */
     show_os2_retcode (dos_rc);             /* Send OS/2 rc to stderr       */
     exit(dos_rc);                          /* Exit the program.            */
  }
  else {                                    /* Set up data buffer pointer   */
     SELECTOROF(memory_pointer) = selector; /* address = Selector:0         */
     OFFSETOF(memory_pointer) = 0;          /* set the offset to zero       */
     return(memory_pointer);
  }
}


void convert_fqplu_name(UCHAR * fqplu_name)
/* This special procedure is needed because a Fully Qualified Name          */
/* actually consists of two EBCDIC type A strings, concatenated with an     */
/* EBCDIC period.  The period is not in the type A translation table and    */
/* will cause the CONVERT verb to otherwise translate the ASCII period      */
/* to '\0'.                                                                 */
{
   UINT i;

   /* Find either the end of the string, or a period                        */
   for (i = 0;
        i < FQ_LEN && fqplu_name[i] != '\0' && fqplu_name[i] != '.'; i++);

   if (fqplu_name[i] == '.') {              /* if it was a period...        */
      ascii2ebcdic(fqplu_name, i, SV_A);    /* convert the first string,    */
      ascii2ebcdic(&fqplu_name[i+1], (FQ_LEN-1)-i,
                                     SV_A); /* then the second.             */
      fqplu_name[i] = 0x4B;                 /* then convert the period.     */
   } else {
      /* just convert the whole thing */
      ascii2ebcdic(fqplu_name, FQ_LEN, SV_A);
   }
}


void ascii2ebcdic (UCHAR * string, const UINT length, const UCHAR type)
/* This procedure takes an ASCII string of a specified length and converts  */
/* an EBCDIC string using the specified table (SV_A, SV_AE, or SV_G).       */
{
  struct convert vcb_convert;               /* Declare a verb control block */
  struct convert far *ptr_convert  = (struct convert far *)&vcb_convert;

  if (length == 0) return;                  /* No work to be done           */

  CLEAR_VCB(vcb_convert);                   /* Zero the verb control block  */
  vcb_convert.opcode = SV_CONVERT;
  vcb_convert.direction = SV_ASCII_TO_EBCDIC;
  vcb_convert.char_set = type;              /* Type of conversion: A, AE, G */
  vcb_convert.len = length;                 /* Length to convert            */
  vcb_convert.source = vcb_convert.target = (UCHAR far *) string;
                                            /* convert in place             */
  ACSSVC((long)ptr_convert);                /* Issue the verb.              */

  if (vcb_convert.primary_rc != AP_OK) {
     show_verb_retcode(vcb_convert.opcode,
                       vcb_convert.primary_rc,
                       vcb_convert.secondary_rc,
                       0L);               /* No SNA sense data on this verb */
  }
}


void show_verb_retcode(const USHORT opcode,
                       const USHORT primary_rc,
                       const ULONG  secondary_rc,
                       const ULONG  sense_data)
{
   show_msg (MSG_PRIMARY_RC);               /* APPC Primary RC              */
   printf ("%04X", SWAP2(primary_rc));      /* Show Primary rc in hex       */
   show_msg (MSG_SECONDARY_RC);             /* Secondary RC = msg.          */
   printf ("%08lX", SWAP4(secondary_rc));   /* Show Secondary rc in hex     */
   show_msg (MSG_VERB_OPCODE);              /* APPC verb in error           */
   printf ("%04X", SWAP2(opcode));          /* Show verb in hex             */
   if (sense_data) {
      show_msg (MSG_SENSE_DATA);            /* APPC verb sense data         */
      printf ("%08lX", SWAP4(sense_data));  /* Show sense data in hex       */
   }
   if (primary_rc == SV_COMM_SUBSYSTEM_NOT_LOADED) {
      printf("\n\tCommunications Manager is not started\n");
      exit(EXIT_FAILURE);
   } else {
      if (primary_rc == AP_COMM_SUBSYSTEM_NOT_LOADED) {
         printf("\n\tEither the Communications Manager is not started\n");
         printf("\tor APPC is not loaded.\n");
         exit(EXIT_FAILURE);
      } else {
         if (primary_rc == AP_COMM_SUBSYSTEM_ABENDED) {
            printf("\n\tAPPC has ABENDED.\n");
            exit(EXIT_FAILURE);
         }
      } /* endif */
   } /* endif */
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
  "\tFilespec needs to include drive, path, and filename.\n");
  fprintf(stderr,
  "   local_filespec (optional) - a new filename for the destination.\n");
  fprintf(stderr,
  "\tThe default is to use the same as the remote_filespec.\n");
  fprintf(stderr,
  "   partner_LU_name (optional) - is either:\n");
  fprintf(stderr,
  "\ta configured partner LU alias, (case sensitive) or\n");
  fprintf(stderr,
  "\ta fully qualified partner LU name (not case sensitive).\n");
  fprintf(stderr,
  "\tThe default partner LU alias is \"FILESVR\".\n\n");
  fprintf(stderr,
  "Note: This program assumes that if the partner_LU_name contains a ");
  fprintf(stderr,
  "period\n\t(that is, '.'), then it is a fully qualified partner LU name.\n");

  exit(EXIT_FAILURE);
}

/* EOF - FILECREQ.C */
