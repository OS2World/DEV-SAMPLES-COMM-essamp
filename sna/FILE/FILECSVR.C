/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : FILECSVR.C                                                */
/*                                                                          */
/*  DESCRIPTIVE : APPC FILE SERVER "C" SAMPLE PROGRAM                       */
/*  NAME          FOR IBM COMMUNICATIONS MANAGER                            */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991                         */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*                                                                          */
/*  FUNCTION:   Receives a request for a file from a peer requester         */
/*              program.  This "server" program opens the file and sends    */
/*              its contents--or sends an error indicator if the file       */
/*              does not exist.  This program ignores all command-line      */
/*              parameters.  The incoming drive, path, and filename must    */
/*              match identically with the file on the local machine.       */
/*                                                                          */
/*              Uses the following APPC verbs:                              */
/*                                                                          */
/*                 RECEIVE_ALLOCATE                                         */
/*                 MC_SEND_DATA                                             */
/*                 MC_RECEIVE_AND_WAIT                                      */
/*                 MC_SEND_ERROR                                            */
/*                 MC_DEALLOCATE                                            */
/*                 TP_ENDED                                                 */
/*                                                                          */
/*              Uses the following ACSSVC (Common Services) Verbs:          */
/*                                                                          */
/*                 CONVERT                                                  */
/*                 LOG_MESSAGE                                              */
/*                                                                          */
/*              Since this program is designed to be run in the background, */
/*              it logs a message whenever it gets an unexpected return     */
/*              code from OS/2, APPC, or common services.                   */
/*                                                                          */
/*              Requires message file "APX.MSG" at runtime, unless the      */
/*              message file is bound to the ".EXE" file.                   */
/*              Files APPC.DLL and ACSSVC.DLL must be on your LIBPATH at    */
/*              runtime.                                                    */
/*                                                                          */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
/*                   (compiles with small memory model)                     */
/*                                                                          */
/*              The previous versions of the sample programs shipped in     */
/*              OS/2 EE 1.0 through 1.3 do not communicate correctly with   */
/*              these new sample programs.  The previous versions did not   */
/*              handle the case where they received DATA_INCOMPLETE on a    */
/*              RECEIVE_AND_WAIT verb because the partner was using a       */
/*              larger buffer size.  The previous versions can be compiled  */
/*              and run as they were originally designed under Networking   */
/*              Services/2.                                                 */
/*                                                                          */
/*                                                                          */
/* Changes since OS/2 EE version 1.2 and 1.3:                               */
/*                                                                          */
/* 1)  Added #define for SEND_BUFFER_SIZE to allow easy modification.       */
/* 2)  Added #define for MAX_FILENAME_LENGTH to support long filenames.     */
/* 3)  Moved initializion of the SEND_DATA verb control block to outside    */
/*     the loop so it is done only once.                                    */
/* 4)  Changed the #include statements to use the new include files         */
/*     in the best way.                                                     */
/* 5)  Changed variable declarations to use the OS/2 typedefs.              */
/* 6)  Added "cdecl" to the main() procedure.                               */
/* 7)  Eliminated all global variables except tp_id and conv_id.  These     */
/*     could have been passed to each routine, but was not done for         */
/*     performance reasons.                                                 */
/* 8)  Added "const"                                                        */
/*                                                                          */
/****************************************************************************/

#define LINT_ARGS

#define  INCL_DOSMISC
#define  INCL_DOSSESMGR
#include <os2.h>
#include <appc_c.h>
#include <acssvcc.h>

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>


#define PROGRAM_NAME        "FILECSVR"      /* used by LOG_MESSAGE verb     */

#define TP_NAME             "FileServer"    /* expected by RECEIVE_ALLOCATE */
#define MESSAGE_FILENAME    "APX.MSG"       /* used by DosGetMessage        */
#define LOG_MESSAGE_FILE    "APX"           /* used by LOG_MESSAGE verb     */

#define BUFFER_SIZE         (61440)         /* use a large buffer size for  */
                                            /* peak performance.            */
#define FILE_NAME_MAX_LEN   (261)           /* use longer file name buffers */
                                            /* to support HPFS              */

#define STATUS_MASK         (0x00FF)        /* Masks to obtain status, data */
#define DATA_MASK           (0xFF00)        /* from variable what_rcvd      */

#define MAX_MESSAGE_LEN     (100)           /* length of the longest        */
                                            /* message in the message file  */

#define TRUE                1
#define FALSE               0

/* These are the message numbers of specific messages in file APX.MSG */
#define MSG_SAMPLE_SERVER   (1)
#define MSG_SAMPLE_COMPLETE (3)
#define MSG_BYTES_SENT      (7)
#define MSG_PRIMARY_RC      (10)
#define MSG_SECONDARY_RC    (11)
#define MSG_OS2_RETCODE     (12)
#define MSG_DATA_INCOMPLETE (16)
#define MSG_NO_SEND_PERMIT  (17)
#define MSG_VERB_OPCODE     (18)
#define MSG_SENSE_DATA      (19)

/* Macro BLANK_STRING sets string to all blanks */
#define BLANK_STRING(str)  memset(str,(int)' ',sizeof(str))


UCHAR tp_idç8Ÿ;                             /* APPC transaction program ID  */
ULONG conv_id;                              /* APPC conversation ID         */


#ifdef LINT_ARGS
/*--------------------------------------------------------------------------*/
/*                       Function Prototypes                                */
/*--------------------------------------------------------------------------*/

void ascii2ebcdic (UCHAR * string, const UINT length, const UCHAR type);
void show_verb_retcode(const USHORT opcode, const USHORT primary_rc,
                       const ULONG secondary_rc, const ULONG sense_data);
UCHAR far * alloc_shared_buffer(const USHORT size);
void do_receive_allocate(const UCHAR * tp_name);
void display_buffer(UCHAR far * buffer);
void convert_fqplu_name(UCHAR * fqplu_name);
void do_confirmed(void);
void do_tp_ended(const UCHAR type);
USHORT do_mc_receive_and_wait(UCHAR far * buffer,
                              const USHORT max_len, PUSHORT length);
void do_mc_send_data(UCHAR far * buffer,
                     const USHORT length, const UCHAR type);
void cdecl main(void);
void log_msg(const USHORT msgno, const USHORT length, const UCHAR * text);
void log_os2_error(const USHORT dos_rc);
void log_verb_error (const USHORT primary,
                     const ULONG secondary, const USHORT opcode);
void show_msg(const USHORT msgno);
void show_os2_retcode (const USHORT retcode);
void show_and_log_os2_error (const USHORT retcode);
void show_and_log_verb_error (const USHORT opcode, const USHORT primary_rc,
                            const ULONG secondary_rc, const ULONG  sense_data);
void do_mc_send_error(void);
void do_mc_deallocate(void);

#endif

/****************************************************************************/
/*                       Main Program Section                               */
/****************************************************************************/

void cdecl main(void)                       /* command line is ignored      */
{
  BOOL   done = FALSE;                      /* is the transaction finished? */
  USHORT dos_rc;                            /* OS/2 return code             */
  HFILE  filehandle;                        /* returned by DosOpen          */
  USHORT action;                            /* returned by DosOpen          */
  USHORT bytes_read;                        /* returned by DosRead          */
  USHORT length;                            /* length of received data      */
  USHORT what_rcvd;                         /* returned what_received value */
  ULONG  byte_count = 0;                    /* counts the bytes read so far */
  UCHAR  far *data_buffer;                  /* pointer to shared memory     */
                                            /* segment                      */

#ifndef MAX_SPEED
  show_msg(MSG_SAMPLE_SERVER);              /* Show the Server message      */
#endif

  /* Ask OS/2 to get the shared memory buffer.                              */
  data_buffer = alloc_shared_buffer(max((USHORT)BUFFER_SIZE,
                                        (USHORT)FILE_NAME_MAX_LEN));

  do_receive_allocate(TP_NAME);             /* Receive an incoming attach   */

  /**************************************************************************/
  /* Receive the first block of data, presumably the name of a file to send */
  /**************************************************************************/
  what_rcvd = do_mc_receive_and_wait(data_buffer, FILE_NAME_MAX_LEN, &length);

  if ((what_rcvd & DATA_MASK) == AP_DATA_COMPLETE)  {
     if ((what_rcvd & STATUS_MASK) != AP_SEND) {   /* If Status is not SEND */
        /* Try once more to get send control */
        what_rcvd = do_mc_receive_and_wait (data_buffer, FILE_NAME_MAX_LEN,
                                            &length);

        if ((what_rcvd & STATUS_MASK) != AP_SEND){ /* Verify status is SEND */
           log_msg (MSG_NO_SEND_PERMIT, 0, NULL);
           exit (MSG_NO_SEND_PERMIT);
        }
     }
  } else {  /* Data was not complete */
     log_msg (MSG_DATA_INCOMPLETE, 0, NULL);
     exit (MSG_DATA_INCOMPLETE);
  }

  /**************************************************************************/
  /* Open the filename sent by the Requester.  The filename is an ASCIIZ    */
  /* string pointed to by data_buffer+1. (+1 to skip the Pascal type        */
  /* LL length byte).                                                       */
  /**************************************************************************/
  dos_rc = DosOpen (data_buffer + 1,        /* addr of file & path name     */
                    (PHFILE)&filehandle,    /* returned: filehandle address */
                    (PUSHORT)&action,       /* returned: action address     */
                    (ULONG)0,               /* file primary allocation      */
                    FILE_NORMAL,            /* file attribute               */
                    OPEN_ACTION_OPEN_IF_EXISTS, /* function to be done      */
                    OPEN_SHARE_DENYWRITE,   /* Open mode of the file        */
                    (ULONG)0);              /* Reserved double word         */
  if (dos_rc != 0) {                        /* Non-zero OS/2 return code?   */
     if (dos_rc == 110)                     /* RC = 110: File not found     */
        do_mc_send_error();                 /* If no such file, SEND_ERROR  */
     show_and_log_os2_error(dos_rc);        /* Exit the program.            */
  }


  /**************************************************************************/
  /* Continue while there are still bytes to be sent from the file.         */
  /**************************************************************************/
  show_msg (MSG_BYTES_SENT);
  while (!done) {
     dos_rc = DosRead ((USHORT)filehandle,  /* pass the filehandle          */
                       (UCHAR far *)data_buffer, /* APPC data buffer        */
                       (USHORT)BUFFER_SIZE, /* size of the data buffer      */
                       (PUSHORT)&bytes_read);/* returned: bytes read addr   */

     if (dos_rc == 0) {                     /* Non-zero OS/2 return code?   */
        if (bytes_read != 0) {              /* Did we read something?       */
           /* Send the next data buffer to the partner with type(FLUSH), to */
           /* maximize the disk I/O time of the partner.                    */
           do_mc_send_data(data_buffer, bytes_read, AP_FLUSH);
        } else {
           done = TRUE;                     /* We've reached the EOF.       */
        }

        byte_count += (ULONG) bytes_read;   /* Increment the running count  */

#ifndef MAX_SPEED
        /* Since this printf uses \r instead of \n, only one line appears-- */
        /* an example of a carriage return without a line feed.             */
        printf ("%lu\r", byte_count);
#endif
     } else {

        do_mc_send_error();                 /* If error, notify requester   */
        show_and_log_os2_error(dos_rc);     /* Exit the program.            */

     } /* endif */

  } /* end of the while-loop */

  /**************************************************************************/
  /* Close the file and clean up the conversation and TP.                   */
  /**************************************************************************/
  dos_rc = DosClose(filehandle);            /* Close the file               */
  if (dos_rc != 0)                          /* Non-zero OS/2 return code?   */
     show_and_log_os2_error(dos_rc);        /* Exit the program.            */

  do_mc_deallocate();                       /* Deallocate the conversation  */

  do_tp_ended(AP_SOFT);                     /* Free the TP's resources      */

  /**************************************************************************/
  /* NO STORAGE IS FREED by the following call to DosFreeSeg!               */
  /* The following call simply decrements the OS/2 allocation count.        */
  /* For maximum performance, APPC internally locks all data segments       */
  /* passed to it until the using process ends.  See the "APPC Programming  */
  /* Reference" for more details.                                           */
  /**************************************************************************/
  dos_rc = DosFreeSeg(SELECTOROF(data_buffer)); /* No memory gets freed!    */
  if (dos_rc != 0)                          /* Non-zero OS/2 return code?   */
     show_and_log_os2_error(dos_rc);        /* Exit the program.            */

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

void show_msg (const USHORT msgno)
/* Displays the requested message number from the message file.             */
{
  USHORT dos_rc;                            /* OS/2 return code             */
  USHORT msg_len;                           /* returned: message length     */
  UCHAR msg_buffçMAX_MESSAGE_LENŸ;          /* Buffer for message data      */

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
     msg_buffçmsg_lenŸ = '\0';              /* Make this an ASCIIZ string   */
     printf("\n");                          /* Start on a new line          */
     printf("%s", msg_buff);                /* Display the message          */
  }
}


void log_msg (const USHORT msgno, const USHORT length, const UCHAR *text)
/* This function logs the requested message from the message file.          */
/* The server presumably logs its errors, since it is designed to run       */
/* unattended--no one would see when it writes to the screen.               */
{
  struct log_message vcb_lmg;               /* Common Services LOG_MESSAGE  */
  struct log_message far *vcbptr = (struct log_message far *)&vcb_lmg;

  CLEAR_VCB(vcb_lmg);                       /* Zero the verb control block  */
  vcb_lmg.opcode = SV_LOG_MESSAGE;          /* LOG_MESSAGE opcode           */
  vcb_lmg.msg_num = msgno;                  /* Number in the message file   */
  memcpy(vcb_lmg.msg_file_name,             /* Set the message file name    */
         LOG_MESSAGE_FILE,
         min(sizeof(vcb_lmg.msg_file_name), strlen(LOG_MESSAGE_FILE)));/*<=3*/
  memcpy(vcb_lmg.origntr_id,                /* Set originator of message    */
         PROGRAM_NAME,                      /* Use this program's name      */
         min(sizeof(vcb_lmg.origntr_id), strlen(PROGRAM_NAME))); /* <= 8    */
  vcb_lmg.msg_act = SV_NO_INTRV;            /* No intervention action type  */
  vcb_lmg.msg_ins_len = length;             /* Insertion text length        */
  vcb_lmg.msg_ins_ptr = (UCHAR far *) text; /* Insertion text               */

  ACSSVC ((ULONG) vcbptr);                  /* Go log the message           */
  if (vcb_lmg.primary_rc != SV_OK) {        /* If bad, try to display error */
     fprintf(stderr,
             "\nAPPC Sample Program Error: Unable to log message number %04d",
             msgno);
  }
}


void log_verb_error (const USHORT primary, const ULONG secondary,
                     const USHORT opcode)
/* This procedure logs the Primary and Secondary verb return codes.         */
/* It then issues an exit(), forcing APPC's TP ExitList processing to issue */
/* the DEALLOCATE and TP_ENDED verbs.                                       */
/* The LOG_MESSAGE verb only allows 3 substitution text strings.            */
{
  struct {                                  /* Data to insert in err log    */
     UCHAR error_vrbç5Ÿ;                    /* ASCIIZ verb opcode           */
     UCHAR error_priç5Ÿ;                    /* ASCIIZ primary return code   */
     UCHAR error_secç9Ÿ;                    /* ASCIIZ secondary return code */
  } log_ins_text;

  sprintf (log_ins_text.error_pri, "%04X", primary); /* Primary return code */
  sprintf (log_ins_text.error_sec, "%08lX", secondary); /* Sec. return code */
  sprintf (log_ins_text.error_vrb, "%04X", opcode); /* verb opcode          */

  log_msg(5, sizeof(log_ins_text), (UCHAR *)&log_ins_text);

  /* APPC's internal TP exitlist processing will issue the DEALLOCATE and   */
  /* TP_ENDED verbs, if necessary, to clean up this transaction.            */
  exit(primary);
}


void log_os2_error (const USHORT retcode)
/* This function logs an OS/2 return code. */
{
  UCHAR error_dos_rcç5Ÿ;                    /* ASCII format OS/2 return code*/

  sprintf (error_dos_rc, "%4d", retcode);   /* Convert to ASCIIZ string.    */

  log_msg(4, sizeof(error_dos_rc), error_dos_rc);
  exit(retcode);
}


void show_and_log_verb_error (const USHORT opcode, const USHORT primary_rc,
                              const ULONG secondary_rc, const ULONG sense_data)
{
  show_verb_retcode (opcode, primary_rc, secondary_rc, sense_data);
  log_verb_error (primary_rc, secondary_rc, opcode);
}


void show_and_log_os2_error (const USHORT retcode)
{
  show_os2_retcode (retcode);               /* send OS/2 rc ro stderr       */
  log_os2_error (retcode);
}


void show_os2_retcode(const USHORT retcode)
{
   show_msg (MSG_OS2_RETCODE);              /* OS/2 return code message     */
   fprintf (stderr, "%d", retcode);         /* Show return code in decimal  */
}

/****************************************************************************/
/*                                                                          */
/*               APPC RELATED FUNCTIONS                                     */
/*                                                                          */
/****************************************************************************/

void do_receive_allocate(const UCHAR * tp_name)
/* Issue a RECEIVE_ALLOCATE verb, to get a tp_id and conv_id.               */
{
  RECEIVE_ALLOCATE receive_allocate;        /* Declare a verb control block */
  RECEIVE_ALLOCATE far *ptr_receive_allocate =
                                 (RECEIVE_ALLOCATE far *)&receive_allocate;

  CLEAR_VCB(receive_allocate);              /* Zero the verb control block  */
  receive_allocate.opcode = AP_RECEIVE_ALLOCATE;   /* Set the APPC opcode   */

  /* Note: Other products support sending of TP names with less restrictive */
  /* character sets than EBCDIC type AE.  Since we know that our TP name    */
  /* uses this character set, we can use these translation routines.        */
  BLANK_STRING(receive_allocate.tp_name);   /* Set 64-byte string to blanks */
  memcpy (receive_allocate.tp_name, tp_name,
                      min(strlen(tp_name), sizeof(receive_allocate.tp_name)));
  ascii2ebcdic(receive_allocate.tp_name,
               sizeof(receive_allocate.tp_name),
               SV_AE);

  APPC((ULONG) ptr_receive_allocate);       /* Issue the verb               */

  if (receive_allocate.primary_rc != AP_OK) {
     show_and_log_verb_error(receive_allocate.opcode,
                             receive_allocate.primary_rc,
                             receive_allocate.secondary_rc,
                             0L);         /* No SNA sense data on this verb */
  } else {  /* Save the returned tp_id and conv_id in global variables.     */
     memcpy (tp_id, receive_allocate.tp_id, sizeof(tp_id));
     conv_id = receive_allocate.conv_id;
  }
}


void do_mc_deallocate (void)
{
  DEALLOCATE deallocate;                    /* Declare a verb control block */
  DEALLOCATE far *ptr_deallocate = (DEALLOCATE far *)&deallocate;

  CLEAR_VCB(deallocate);                    /* Zero the verb control block  */
  deallocate.opcode = AP_M_DEALLOCATE;      /* Verb-MC_DEALLOCATE           */
  deallocate.opext  = AP_MAPPED_CONVERSATION; /* Set MC ext. type           */
  deallocate.conv_id = conv_id;             /* Set conversation_id          */
  memcpy (deallocate.tp_id, tp_id, sizeof(tp_id)); /* Set tp_id             */
  deallocate.dealloc_type = AP_SYNC_LEVEL;  /* Match the allocate.          */

  APPC ((ULONG) ptr_deallocate);            /* Issue the verb.              */

  if (deallocate.primary_rc != AP_OK)  {    /* Log a non-zero return code   */
     show_and_log_verb_error(deallocate.opcode,
                             deallocate.primary_rc,
                             deallocate.secondary_rc,
                             0L);         /* No SNA sense data on this verb */
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

  if (tp_ended.primary_rc != AP_OK) {
     show_and_log_verb_error(tp_ended.opcode,
                             tp_ended.primary_rc,
                             tp_ended.secondary_rc,
                             0L);         /* No SNA sense data on this verb */
  }
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
     show_and_log_verb_error(send_data.opcode,
                             send_data.primary_rc,
                             send_data.secondary_rc,
                             0L);         /* No SNA sense data on this verb */
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
     show_and_log_verb_error(receive_and_wait.opcode,
                             receive_and_wait.primary_rc,
                             receive_and_wait.secondary_rc,
                             0L);         /* No SNA sense data on this verb */
     return 0;          /* a returned 0 indicates that nothing was received */
  } else {
     *length = receive_and_wait.dlen;       /* Get length we actually rcvd  */
     return (*(USHORT *)&receive_and_wait.what_rcvd);
  }
}


void do_mc_send_error (void)
{
  SEND_ERROR send_error;                    /* Declare a verb control block */
  SEND_ERROR far *ptr_send_error = (SEND_ERROR far *)&send_error;

  CLEAR_VCB(send_error);                    /* Zero the verb control block  */
  send_error.opcode = AP_M_SEND_ERROR;      /* Verb = send_error            */
  send_error.opext = AP_MAPPED_CONVERSATION;/* Set MC type                  */
  send_error.conv_id = conv_id;             /* Set Conversation_id          */
  memcpy (send_error.tp_id, tp_id, sizeof(tp_id)); /* Set TP_id             */
  send_error.err_dir = AP_SEND_DIR_ERROR;   /* Set error direction          */

  APPC ((ULONG) ptr_send_error);            /* Issue the verb               */

  /* If you use this routine in another program or change the verb          */
  /* sequences, other return codes may be expected.  Your program should    */
  /* alwalys handle non-zero return codes more fully than simply informing  */
  /* to the user the fact that an error occurred.                           */
  if (send_error.primary_rc != AP_OK) {
     show_and_log_verb_error(send_error.opcode,
                             send_error.primary_rc,
                             send_error.secondary_rc,
                             0L);         /* No SNA sense data on this verb */
  }
}

/****************************************************************************/
/*                                                                          */
/*                    OS/2 RELATED FUNCTIONS                                */
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
     show_and_log_os2_error(dos_rc);        /* Exit the program.            */
  }
  else {                                    /* Set up data buffer pointer   */
     SELECTOROF(memory_pointer) = selector; /* address = Selector:0         */
     OFFSETOF(memory_pointer) = 0;          /* set the offset to zero       */
     return(memory_pointer);
  }
}


void ascii2ebcdic (UCHAR * string, const UINT length, const UCHAR type)
/* This procedure takes an ASCII string of a specified length and converts  */
/* strings to EBCDIC using the specified table (SV_A, SV_AE, or SV_G).      */
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

  ACSSVC((long)ptr_convert);                /* Issue the verb.              */

  if (vcb_convert.primary_rc != AP_OK) {
     show_and_log_verb_error(vcb_convert.opcode,
                             vcb_convert.primary_rc,
                             vcb_convert.secondary_rc,
                             0L);         /* No SNA sense data on this verb */
  }
}


void show_verb_retcode(const USHORT opcode,
                       const USHORT primary_rc,
                       const ULONG  secondary_rc,
                       const ULONG  sense_data)
{
   show_msg (MSG_PRIMARY_RC);             /* APPC Primary RC = msg.       */
   printf ("%04X", SWAP2(primary_rc));    /* Show Primary rc in hex       */
   show_msg (MSG_SECONDARY_RC);           /* Secondary RC = msg.          */
   printf ("%08lX", SWAP4(secondary_rc)); /* Show Secondary rc in hex     */
   show_msg (MSG_VERB_OPCODE);            /* APPC verb in error           */
   printf ("%04X", SWAP2(opcode));        /* Show verb in hex             */
   if (sense_data) {
      show_msg (MSG_SENSE_DATA);          /* APPC verb sense data         */
      printf ("%08lX", SWAP4(sense_data)); /* Show sense data in hex      */
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

/* EOF FILECSVR.C */
