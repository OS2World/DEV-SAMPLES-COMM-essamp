/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : APPCUTIL.C                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utilities for the DISPLAY and PMDSPLAY sample programs.     */
/*                                                                          */
/*              Uses the following APPC Verbs:                              */
/*                                                                          */
/*                 TP_STARTED                                               */
/*                 MC_ALLOCATE                                              */
/*                 RECEIVE_ALLOCATE                                         */
/*                 MC_SEND_DATA                                             */
/*                 MC_RECEIVE_AND_WAIT                                      */
/*                 MC_DEALLOCATE                                            */
/*                 MC_CONFIRMED                                             */
/*                 TP_ENDED                                                 */
/*                                                                          */
/*              Uses the following ACSSVC (Common Services) Verbs:          */
/*                                                                          */
/*                 CONVERT                                                  */
/*                                                                          */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
/*              (compiles with large memory model)                          */
/*                                                                          */
/*   ASSOCIATED FILES:  See also DISPLAY.MAK, PMDSPLAY.MAK                  */
/*                                                                          */
/*      APD.TXT      - Messages (English)                                   */
/*      MSGID.H      - #defines for messages                                */
/*                                                                          */
/****************************************************************************/
#define INCL_DOS          /* Using OS/2 DOS functions                       */
#define INCL_DOSNLS
#define INCL_DOSMISC
 
#include "DISPLAY.H"      /* Program definitions and declarations           */
 
/* MSC 6.0 handles multi-thread includes differently than IBM C/2           */
#if _MSC_VER >= 600
   #include <CTYPE.H>     /* Standard C library macros                      */
   #include <STDIO.H>     /* Standard C library I/O functions               */
   #include <STDLIB.H>    /* Standard C library miscellaneous functions     */
#else
   #include <MT\CTYPE.H>  /* Standard C library macros                      */
   #include <MT\STDIO.H>  /* Standard C library I/O functions               */
   #include <MT\STDLIB.H> /* Standard C library miscellaneous functions     */
   #endif
#include <APPC_C.H>       /* APPC definitions and declarations              */
#include <ACSSVCC.H>      /* Common Services definitions and declarations   */
 
#define  PROGRAM_NAME      "DISPLAY"            /* used by LOG_MESSAGE verb */
#define  LOG_MESSAGE_FILE  "APX"                /* used by LOG_MESSAGE verb */
#define  EBCDIC_AT         (char) '\x7C'            /* EBCDIC '@'           */
#define  EBCDIC_PERIOD     (char) '\x4B'            /* EBCDIC '.'           */
#define NUMBUFF 4         /* Allow for multiple message buffers             */
#define BUFFSIZE 500      /* Allow for long message strings                 */
#define FILENAME "APD.MSG"
 
/* Used by NLS separation code                                              */
BOOL DBCSCodeRangesInitialized = FALSE; /* not initialized                  */
int nextbuff = 0;
CHAR msgbuff[NUMBUFF][BUFFSIZE];
PUCHAR DBCSCodeRanges ;
UCHAR DBCSCR[10] ;
BOOL DBCSCodeRangesInitialized ;
BOOL file_ok;
char description[259][81];
char value[144][51];
char message[82][81];
char long_msg[6][411];
 
/* Local function prototypes for NLS separation                             */
void edit_the_message(CHAR msg[],int size);
BOOL EXPENTRY IsDBCSChar(UCHAR character);
void InitDBCSCodeRanges(void);
PCHAR get_msg(int msg_id);
void InitializeText(void);
void InitializeDescriptions1(void);
void InitializeDescriptions2(void);
void InitializeValues(void);
void InitializeMessages(void);
void InitializeLongMessages(void);
 
/*--------------------------------------------------------------------------*/
/* convert: Use the APPC Service routine to convert a field to ASCII/EBCDIC.*/
/*--------------------------------------------------------------------------*/
void convert( UCHAR * name     ,       /* TP or LU name   to be converted   */
              UINT    size     ,       /* Number of bytes to be converted   */
              UCHAR   dir      ,       /* Direction (e.g., ASCII_TO_EBCDIC) */
              UCHAR   char_set )       /* Type of conversion (e.g., A, AE)  */
{
   struct convert cvt;
   PCH    vcb_ptr = (PCH) &cvt;        /* far pointer to Verb Control Block */
   char * period =                     /* Find the first ASCII/EBCDIC period*/
          memchr( name, ( dir == SV_ASCII_TO_EBCDIC ) ? '.' : EBCDIC_PERIOD,
                  size );
 
   /*************************************************************************/
   /* Check for the special case of a Fully Qualified Partner LU name.  If a*/
   /* period (of the appropriate character set) exists, change it to a char */
   /* that exists in the type-A character set (i.e., the at sign '@')       */
   /*************************************************************************/
   if ( (char *) NULL != period ) {
     * period = ( dir == SV_ASCII_TO_EBCDIC ) ? (char)'@' : EBCDIC_AT;
   }
   CLEAR_VCB (cvt);
   cvt.opcode    = SV_CONVERT;
   cvt.direction = dir;
   cvt.char_set  = char_set;
   cvt.len       = size;
   cvt.source    = cvt.target = name;
   ACSSVC( (ULONG) vcb_ptr );
   if ( AP_OK != cvt.primary_rc ) {
      show_verb_retcode (cvt.opcode,
                         cvt.primary_rc,
                         cvt.secondary_rc,
                         0L);            /* No SNA sense data for this verb */
   }
   /*************************************************************************/
   /* Translate the period separating the Network Name and LUNAME fields    */
   /* separately.  If the period was changed to an '@' above, then we need  */
   /* to change it to a period in the appropriate character set.            */
   /*************************************************************************/
   if ( (char *) NULL != period ) {
     * period = ( dir == SV_ASCII_TO_EBCDIC ) ? EBCDIC_PERIOD : (char)'.';
   }
}
 
/*--------------------------------------------------------------------------*/
/* ascii_name: Copy specified name into target field, and pad with blanks   */
/*--------------------------------------------------------------------------*/
void ascii_name( UCHAR * dest, UCHAR * src, UINT dest_size )
{
unsigned int src_len;
 
   memset( dest, (int)' ', dest_size );
   src_len = strlen( src );
   memcpy( dest, src, min( dest_size, src_len) );
}
 
/*--------------------------------------------------------------------------*/
/* ebcdic_name: Copy the specified name, pad with blanks, and convert it.   */
/*--------------------------------------------------------------------------*/
void ebcdic_name( UCHAR * dest, UCHAR * src, UINT dest_size, UCHAR char_set )
{
   ascii_name( dest, src, dest_size );     /* Call to copy & blank pad name */
   convert( dest, dest_size, SV_ASCII_TO_EBCDIC, char_set ); /* then convert*/
}
 
/*--------------------------------------------------------------------------*/
/* invalid_PLU_name: check for an unreasonable PLU name.                    */
/*--------------------------------------------------------------------------*/
BOOL invalid_PLU_name (char * PLU_name)
{
   char * period = strchr( PLU_name, '.' ); /* Find first '.' in the name   */
   char * cp;
   UINT   netid_len, name_len;
 
   /*************************************************************************/
   /* If the PLU_name contains a period (i.e., '.'), then assume it's a     */
   /* fully qualified PLU name, otherwise, it's a PLU_Alias.                */
   /*************************************************************************/
   if ( period != NULL ) {             /* period != NULL means '.' present  */
      strupr( PLU_name );              /* FQ PLU names must be uppercase    */
      netid_len = period - PLU_name;   /* Both the netid and name portions  */
      name_len  = ( strlen( PLU_name ) /*   of fully qualified names must   */
                  - netid_len ) - 1;   /*   have from 1..8 characters.      */
      if ( ( netid_len < 1 ) || ( netid_len > 8 ) ||
           ( name_len  < 1 ) || ( name_len  > 8 ) )
         return TRUE;                  /* We have an invalid name           */
   } else {                            /*                                   */
      name_len = strlen( PLU_name );   /*                                   */
      if ( ( name_len  < 1 ) || ( name_len  > 8 ) )
         return TRUE;                  /* We have an invalid name           */
   }                                   /*                                   */
   /*************************************************************************/
   /* Verify that we have 1 or 2 type - A strings.                          */
   /*************************************************************************/
   for ( cp = PLU_name; * cp; cp++ )
      if ( cp != period )
         if ( !isalnum( * cp ) )       /* Is this 'a'..'z','A'..'Z','0'..'9'*/
            if ( ( * cp != '$' ) && ( * cp != '#' ) && ( * cp != '@' ) )
               return TRUE;            /* This char isn't valid type - A    */
   return FALSE;
}
 
/****************************************************************************/
/* alloc_shared_buffer:  Allocates a shared memory segment for use in calls */
/*                       to APPC.  APPC requires a data buffer to be in a   */
/*                       shared unnamed segment.                            */
/* NOTE:  Because of the nature of shared segments and the way APPC uses    */
/*        them, memory allocated by this function and used in a call to     */
/*        APPC will not be freed until this program terminates.  Therefore, */
/*        a program should not call this function repeatedly.  See the      */
/*        "APPC Programming Reference" for more information.                */
/****************************************************************************/
UCHAR far * alloc_shared_buffer (USHORT size)
{
   USHORT selector;                         /* selector from DosAllocSeg    */
   USHORT dos_rc;                           /* OS/2 return code             */
 
   dos_rc = DosAllocSeg ((unsigned)size,    /* size of memory to allocate   */
                         (PSEL)&selector,   /* returned: selector address   */
                         (unsigned)1);      /* shared, unnamed segment      */
   if (dos_rc != 0) {                       /* Non-zero OS/2 return code?   */
      myprintf(MSG_ALLOC_ERROR, dos_rc);    /* Show an error message.       */
      return(0L);                           /* Return null pointer.         */
   }
   return(MAKEP(selector, 0));              /* Return pointer to buffer.    */
}
 
/****************************************************************************/
/* release_shared_buffer:  Releases segment allocated by alloc_shared_buffer*/
/* NOTE:  Because of the nature of shared segments and the way APPC uses    */
/*        them, no memory is freed as result of calling this function.      */
/*        Only when this program terminates will APPC release the shared    */
/*        segment, allowing the memory occupied by the shared segment to be */
/*        freed.  Therefore, a program should not call                      */
/*        alloc_shared_buffer() and release_shared_buffer() repeatedly.     */
/*        See the "APPC Programming Reference" for more information.        */
/****************************************************************************/
void free_shared_buffer (void far * buffer_ptr)
{
   USHORT dos_rc;                           /* OS/2 return code             */
 
   dos_rc = DosFreeSeg(SELECTOROF(buffer_ptr)); /*Release the shared segment*/
   if (dos_rc != 0) {                       /* If DosFreeSeg fails          */
      myprintf(MSG_FREE_ERROR, dos_rc);     /* Show an error message.       */
   }
}
 
/****************************************************************************/
/* tp_started                                                               */
/****************************************************************************/
BOOL tp_started (UCHAR * tp_name, UCHAR tp_id[])
{
   TP_STARTED          tp_started; /* VCB for TP_STARTED                    */
 
   CLEAR_VCB (tp_started);
   tp_started.opcode = AP_TP_STARTED;
   ebcdic_name (tp_started.tp_name, tp_name,
                sizeof (tp_started.tp_name), SV_AE);
   APPC ((ULONG) &tp_started);
   if (AP_OK != tp_started.primary_rc) {
      show_verb_retcode (tp_started.opcode,
                         tp_started.primary_rc,
                         tp_started.secondary_rc,
                         0L);            /* No SNA sense data for this verb */
      return (FALSE);
   } else {
      memcpy (tp_id, tp_started.tp_id, sizeof (tp_started.tp_id));
      return (TRUE);
      } /* endif */
}
 
/****************************************************************************/
/* mc_allocate                                                              */
/****************************************************************************/
BOOL mc_allocate (UCHAR * plu_name, UCHAR * tp_name, UCHAR tp_id[],
                  UCHAR * mode_name, UCHAR synch_level, ULONG * conv_id)
{
   MC_ALLOCATE         allocate;   /* VCB for MC_ALLOCATE                   */
 
   CLEAR_VCB (allocate);
   allocate.opcode = AP_M_ALLOCATE;
   allocate.opext  = AP_MAPPED_CONVERSATION;
   memcpy (allocate.tp_id, tp_id, sizeof (allocate.tp_id));
   if ( NULL == strchr( plu_name, '.' ) ) {
      ascii_name (allocate.plu_alias, plu_name,
                  sizeof (allocate.plu_alias));
   } else {
      ebcdic_name (allocate.fqplu_name, plu_name,
                   sizeof (allocate.fqplu_name), SV_A);
      } /* endif */
   ebcdic_name (allocate.mode_name, mode_name,
                sizeof (allocate.mode_name), SV_A);
   ebcdic_name (allocate.tp_name, tp_name,
                sizeof (allocate.tp_name), SV_AE);
   allocate.sync_level = synch_level;
   allocate.rtn_ctl    = AP_WHEN_SESSION_ALLOCATED;
   allocate.security   = AP_NONE;
   APPC ((ULONG) &allocate);
 
   if (AP_OK != allocate.primary_rc) {
      show_verb_retcode (allocate.opcode,
                         allocate.primary_rc,
                         allocate.secondary_rc,
                         allocate.sense_data);
      return (FALSE);
   } else {
      *conv_id = allocate.conv_id;
      return (TRUE);
      } /* endif */
}
 
/****************************************************************************/
/* receive_allocate                                                         */
/****************************************************************************/
BOOL receive_allocate (CHAR * tp_name, UCHAR tp_id[], ULONG * conv_id)
{
   RECEIVE_ALLOCATE    rec_alloc;   /* VCB for RECEIVE_ALLOCATE             */
 
   CLEAR_VCB (rec_alloc);
   rec_alloc.opcode = AP_RECEIVE_ALLOCATE;
   ebcdic_name (rec_alloc.tp_name, tp_name,
                sizeof (rec_alloc.tp_name), SV_AE);
   APPC ((ULONG) &rec_alloc);
 
   if (AP_OK != rec_alloc.primary_rc) {
      show_verb_retcode (rec_alloc.opcode,
                         rec_alloc.primary_rc,
                         rec_alloc.secondary_rc,
                         0L);            /* No SNA sense data for this verb */
      return (FALSE);
   } else {
      memcpy (tp_id, rec_alloc.tp_id, sizeof (rec_alloc.tp_id));
      *conv_id = rec_alloc.conv_id;
      return (TRUE);
      } /* endif */
}
 
/****************************************************************************/
/* mc_send_data                                                             */
/****************************************************************************/
BOOL mc_send_data (UCHAR  far * buffer_ptr,
                   UCHAR  tp_id[],
                   ULONG  conv_id,
                   USHORT length,
                   UCHAR  type)
{
   MC_SEND_DATA        send_data;  /* VCB for MC_SEND_DATA                  */
 
   CLEAR_VCB (send_data);
   memcpy (send_data.tp_id, tp_id, sizeof (send_data.tp_id));
   send_data.opcode  = AP_M_SEND_DATA;
   send_data.opext   = AP_MAPPED_CONVERSATION;
   send_data.conv_id = conv_id;
   send_data.dlen    = length;
   send_data.dptr    = buffer_ptr;
   send_data.type    = type;
   APPC ((ULONG) &send_data);
 
   if (AP_OK != send_data.primary_rc) {
      show_verb_retcode (send_data.opcode,
                         send_data.primary_rc,
                         send_data.secondary_rc,
                         0L);            /* No SNA sense data for this verb */
      return (FALSE);
   } else {
      return (TRUE);
      } /* endif */
}
 
/****************************************************************************/
/* mc_receive_and_wait                                                      */
/****************************************************************************/
USHORT mc_receive_and_wait (UCHAR  far * buffer_ptr,
                            UCHAR  tp_id[],
                            ULONG  conv_id,
                            USHORT max_length)
{
   MC_RECEIVE_AND_WAIT rec_n_wait;    /* VCB for MC_RECEIVE_AND_WAIT        */
 
   CLEAR_VCB (rec_n_wait);
   memcpy (rec_n_wait.tp_id, tp_id, sizeof (rec_n_wait.tp_id));
   rec_n_wait.opcode     = AP_M_RECEIVE_AND_WAIT;
   rec_n_wait.opext      = AP_MAPPED_CONVERSATION;
   rec_n_wait.conv_id    = conv_id;
   rec_n_wait.rtn_status = AP_YES;
   rec_n_wait.max_len    = max_length;
   rec_n_wait.dptr       = buffer_ptr;
   APPC ((ULONG) &rec_n_wait);
 
   if (AP_OK != rec_n_wait.primary_rc) {
      show_verb_retcode (rec_n_wait.opcode,
                         rec_n_wait.primary_rc,
                         rec_n_wait.secondary_rc,
                         0L);            /* No SNA sense data for this verb */
      } /* endif */
   return (rec_n_wait.what_rcvd);
}
 
/****************************************************************************/
/* mc_confirmed                                                             */
/****************************************************************************/
void mc_confirmed (UCHAR tp_id[], ULONG conv_id)
{
   MC_CONFIRMED      confirmed;       /* VCB for MC_CONFIRMED               */
 
   CLEAR_VCB (confirmed);
   memcpy (confirmed.tp_id, tp_id, sizeof (confirmed.tp_id));
   confirmed.opcode  = AP_M_CONFIRMED;
   confirmed.opext   = AP_MAPPED_CONVERSATION;
   confirmed.conv_id = conv_id;
   APPC ((ULONG) &confirmed);
 
   if (AP_OK != confirmed.primary_rc) {
      show_verb_retcode (confirmed.opcode,
                         confirmed.primary_rc,
                         confirmed.secondary_rc,
                         0L);            /* No SNA sense data for this verb */
      } /* endif */
}
 
/****************************************************************************/
/* mc_deallocate                                                            */
/****************************************************************************/
void mc_deallocate (UCHAR tp_id[], ULONG conv_id, UCHAR type)
{
   MC_DEALLOCATE       dealloc;       /* VCB for MC_DEALLOCATE              */
 
   CLEAR_VCB (dealloc);
   memcpy (dealloc.tp_id, tp_id, sizeof (dealloc.tp_id));
   dealloc.opcode       = AP_M_DEALLOCATE;
   dealloc.opext        = AP_MAPPED_CONVERSATION;
   dealloc.conv_id      = conv_id;
   dealloc.dealloc_type = type;
   APPC ((ULONG) &dealloc);
 
   if (AP_OK != dealloc.primary_rc) {
      show_verb_retcode (dealloc.opcode,
                         dealloc.primary_rc,
                         dealloc.secondary_rc,
                         0L);            /* No SNA sense data for this verb */
      } /* endif */
}
 
/****************************************************************************/
/* tp_ended                                                                 */
/****************************************************************************/
void tp_ended (UCHAR tp_id[])
{
   TP_ENDED            tp_done;
 
   CLEAR_VCB (tp_done);
   memcpy (tp_done.tp_id, tp_id, sizeof (tp_done.tp_id));
   tp_done.opcode = AP_TP_ENDED;
   tp_done.type   = AP_SOFT;
   APPC ((ULONG) &tp_done);
 
   if (AP_OK != tp_done.primary_rc) {
      show_verb_retcode (tp_done.opcode,
                         tp_done.primary_rc,
                         tp_done.secondary_rc,
                         0L);            /* No SNA sense data for this verb */
      } /* endif */
}
 
/****************************************************************************/
/* show_verb_retcode                                                        */
/****************************************************************************/
void show_verb_retcode (USHORT opcode,
                        USHORT primary_rc,
                        ULONG  secondary_rc,
                        ULONG  sense_data)
{
   switch (opcode) {
      case AP_TP_STARTED:
         myprintf (MSG_TP_STARTED_FAILED);
         break;
      case AP_M_ALLOCATE:
         myprintf (MSG_MC_ALLOCATE_FAILED);
         break;
      case AP_RECEIVE_ALLOCATE:
         myprintf (MSG_RECEIVE_ALLOCATE_FAILED);
         break;
      case AP_M_SEND_DATA:
         myprintf (MSG_MC_SEND_DATA_FAILED);
         break;
      case AP_M_RECEIVE_AND_WAIT:
         myprintf (MSG_MC_RECEIVE_AND_WAIT_FAILED);
         break;
      case AP_M_CONFIRMED:
         myprintf (MSG_MC_CONFIRMED_FAILED);
         break;
      case AP_M_DEALLOCATE:
         myprintf (MSG_MC_DEALLOCATE_FAILED);
         break;
      case AP_TP_ENDED:
         myprintf (MSG_TP_ENDED_FAILED);
         break;
      default:
         myprintf (MSG_OPCODE);
         myprintf (" %04X", SWAP2 (opcode));
      } /* endswitch */
   switch (primary_rc) {
      case AP_PARAMETER_CHECK:
         if (AP_UNKNOWN_PARTNER_MODE == secondary_rc) {
            myprintf (MSG_UNKNOWN_PARTNER_MODE);
            } /* endif */
         myprintf (MSG_CRLF);
         break;
      case AP_ALLOCATION_ERROR:
         myprintf (MSG_ALLOCATION_FAILURE);
         myprintf (" ");
/* MSC 6.0 can handle 4 byte constants in switch statements, IBM C/2 can't  */
#if _MSC_VER >= 600
         switch (secondary_rc) {
            case AP_ALLOCATION_FAILURE_NO_RETRY:
               myprintf (MSG_NO_RETRY);
               break;
            case AP_ALLOCATION_FAILURE_RETRY:
               myprintf (MSG_RETRY);
               break;
            case AP_TRANS_PGM_NOT_AVAIL_RETRY:
               myprintf (MSG_TP_NOT_AVAIL_RETRY);
               break;
            case AP_TRANS_PGM_NOT_AVAIL_NO_RETRY:
            case AP_TP_NAME_NOT_RECOGNIZED:
               myprintf (MSG_TP_NOT_AVAIL_NO_RETRY);
               break;
            default:
               myprintf (MSG_SECONDARY_RC);
               myprintf (" %08lX", SWAP4 (secondary_rc));
               } /* endswitch */
#else
         if (AP_ALLOCATION_FAILURE_NO_RETRY == secondary_rc) {
            myprintf (MSG_NO_RETRY);
         } else {
            if (AP_ALLOCATION_FAILURE_RETRY == secondary_rc) {
               myprintf (MSG_RETRY);
            } else {
               if (AP_TRANS_PGM_NOT_AVAIL_RETRY == secondary_rc) {
                  myprintf (MSG_TP_NOT_AVAIL_RETRY);
               } else {
                  if ((AP_TRANS_PGM_NOT_AVAIL_NO_RETRY == secondary_rc) ||
                      (AP_TP_NAME_NOT_RECOGNIZED == secondary_rc)) {
                     myprintf (MSG_TP_NOT_AVAIL_NO_RETRY);
                  } else {
                     myprintf (MSG_SECONDARY_RC);
                     myprintf (" %08lX", SWAP4 (secondary_rc));
                     } /* endif */
                  } /* endif */
               } /* endif */
            } /* endif */
#endif
         if (sense_data) {
            myprintf (MSG_SENSE_DATA);
            myprintf (" %08lX", SWAP4 (sense_data));
         }
         myprintf (MSG_CRLF);
         break;
      default:
         myprintf (MSG_PRIMARY_RC);
         myprintf (" %04X", SWAP2 (primary_rc));
         myprintf (MSG_SECONDARY_RC);
         myprintf (" %08lX", SWAP4 (secondary_rc));
         myprintf (MSG_CRLF);
      } /* endswitch */
}
 
/* Beginning NLS subroutine:                                                */
/****************************************************************************/
/* get_msg: Converts a msg id into a pointer to the string                  */
/*         gotten from the message file APD.MSG.                            */
/****************************************************************************/
 
  PCHAR get_msg(int msg_id)
{
   USHORT dos_rc;                      /* OS/2 return code                  */
   USHORT msg_len;                     /* returned: message length          */
 
/* Support multiple buffers to allow for 'nested' calls to get_msg          */
 
 if (++nextbuff == NUMBUFF)
      nextbuff = 0;
  if (file_ok) {
    if (dos_rc = DosGetMessage((PCHAR far *)0, /* no Ivtable                */
       0, (PCHAR)msgbuff[nextbuff],    /* place message here                */
       BUFFSIZE,                       /* buffer length; maximum message
                                          supported                         */
       msg_id,                         /* identifies the message            */
       FILENAME,                       /* Msg file name                     */
       &msg_len)) {                    /* length of message returned        */
      file_ok = FALSE;
      msgbuff[nextbuff][0] = '\0';     /* ensure a null string of data      */
    }
    else {
 
      /**********************************************************************/
      /* Make this an ASCIIZ string and back out the CR, LF                 */
      /**********************************************************************/
 
      msgbuff[nextbuff][msg_len-2] = '\0';
      edit_the_message(msgbuff[nextbuff], msg_len-2);
    }
  }
  else {
    msgbuff[nextbuff][0] = '\0';       /* ensure a null string of data      */
  }
  return (PCHAR)(msgbuff[nextbuff]);   /* return with valid address         */
}
 
/****************************************************************************/
/* Function Name: InitDBCSCodeRanges                                        */
/*                                                                          */
/* Function: Initializes the code ranges to use in testing the first byte   */
/*           of a possible DBCS character.                                  */
/*                                                                          */
/****************************************************************************/
 
void InitDBCSCodeRanges(void)
{
   COUNTRYCODE Country;
   USHORT i;
 
   DBCSCodeRangesInitialized = TRUE;
   Country.country = 0;
   Country.codepage = 0;
   for (i = 0; i < 10; i++)
      DBCSCR[i] = '\0';
   DBCSCodeRanges = DBCSCR;
   DosGetDBCSEv(sizeof(DBCSCodeRanges), &Country, DBCSCodeRanges);
}
 
 
/****************************************************************************/
/*                                                                          */
/* FUNCTION NAME: IsDBCSChar                                                */
/*                                                                          */
/* FUNCTION: Tests to determine if the passed character is the first char   */
/*           of a DBCS character.                                           */
/****************************************************************************/
 
BOOL EXPENTRY IsDBCSChar(UCHAR character)
{
   USHORT count;
 
   if (!DBCSCodeRangesInitialized)
      InitDBCSCodeRanges();
   for (count = 0; DBCSCodeRanges[count] != '\0' || DBCSCodeRanges[count+1]
      != '\0'; count += 2)
      if (character >= DBCSCodeRanges[count] && character <= DBCSCodeRanges
         [count+1])
         return (TRUE);
   return (FALSE);
}
 
 
/****************************************************************************/
/*                                                                          */
/* FUNCTION NAME: edit_the_message                                          */
/*                                                                          */
/* FUNCTION: Scans the message string and processes imbedded '\' formatting */
/*           sequences.                                                     */
/*                                                                          */
/*           Control sequences supported: '\\', '\ ', '\n'.                 */
/*           Not supported (deleted): '\x', where x is any other char.      */
/****************************************************************************/
 
void edit_the_message(CHAR msg[],int size)
{
   int i;
   int j;
 
   for (i = 0, j = 0; i <= size; ++i) {/* loop cond: i=last moved           */
                                       /* data,j=next slot                  */
      if (IsDBCSChar(msg[i])){
         msg[j++] = msg[i++];          /* Move the DBCS character bytes     */
         msg[j++] = msg[i];
      }
      else {
         if (msg[i] == '\\'){          /* Excape character?                 */
            switch (msg[i+1]){
               case '\\' :
               case '\0' :
                  msg[j++] = msg[++i]; /* Overlay excape with next char     */
                  break;
               case ' ' :
                  ++i;                /* skip over '\ ' sequence  */
                  break;
               case 0x0d :
                  ++i;                /* skip over CR/LF chars when   */
                  ++i;                /*  '\' is last char on line    */
                  break;
               case 'n' :
                  msg[j++] = 0x0a;     /* add LF                            */
                  i++;
                  break;
               default  :              /* throw away the '\x' sequence      */
                  i++;
                  break;
            }
         }
         else {
            msg[j++] = msg[i];         /* move the data                     */
         }
      }
   }
   return ;
}
 
/****************************************************************************/
/*                                                                          */
/* FUNCTION NAME: InitializeText                                            */
/*                                                                          */
/* FUNCTION: Initializes four two-dimensional arrays which store the text   */
/*           used.  This prevents unnecessary calls to get_msg during run-  */
/*           time which drastically decreases performance.  After the text  */
/*           was placed in APD.TXT for national language support, PMDSPLAY  */
/*           was slowed down due to the time it takes to extract a message  */
/*           from APD.MSG using DosGetMessage.  By getting all the messages */
/*           during initialization and placing them in arrays, some time    */
/*           is added to startup, but overall the performance is increased. */
/*                                                                          */
/****************************************************************************/
 
void InitializeText (void)
{
   int i;                              /* Counter to clear storage     */
 
   for ( i = 0 ; i < 259 ; i++ ) description[i][0] = '\0' ;
   for ( i = 0 ; i < 144 ; i++ ) value[i][0] = '\0' ;
   for ( i = 0 ; i < 82 ; i++ ) message[i][0] = '\0' ;
   for ( i = 0 ; i < 6 ; i++ ) long_msg[i][0] = '\0' ;
   InitializeDescriptions1();
   InitializeDescriptions2();
   InitializeValues();
   InitializeMessages();
   InitializeLongMessages();
}
 
/****************************************************************************/
/*                                                                          */
/* FUNCTION NAME: InitializeDescriptions1                                   */
/*                                                                          */
/* FUNCTION: Initializes a two-dimensional array which stores all the       */
/*           descriptions used.  Descriptions are the text used in the      */
/*           first column of any display.  The initialization was divided   */
/*           into two functions because of the large amount of descriptions.*/
/*           In order to take advantage of the optimization compiler        */
/*           options, the size of the functions needed to be reduced.       */
/*                                                                          */
/****************************************************************************/
 
void InitializeDescriptions1 (void)
{
   strncat ( description[  0], get_msg( 78), 80 ) ;
   strncat ( description[  1], get_msg( 79), 80 ) ;
   strncat ( description[  2], get_msg( 80), 80 ) ;
   strncat ( description[  3], get_msg( 81), 80 ) ;
   strncat ( description[  4], get_msg( 82), 80 ) ;
   strncat ( description[  5], get_msg( 83), 80 ) ;
   strncat ( description[  6], get_msg( 84), 80 ) ;
   strncat ( description[  7], get_msg( 85), 80 ) ;
   strncat ( description[  8], get_msg( 86), 80 ) ;
   strncat ( description[  9], get_msg( 87), 80 ) ;
   strncat ( description[ 10], get_msg( 90), 80 ) ;
   strncat ( description[ 11], get_msg( 93), 80 ) ;
   strncat ( description[ 12], get_msg( 94), 80 ) ;
   strncat ( description[ 13], get_msg( 95), 80 ) ;
   strncat ( description[ 14], get_msg( 96), 80 ) ;
   strncat ( description[ 15], get_msg( 97), 80 ) ;
   strncat ( description[ 16], get_msg( 98), 80 ) ;
   strncat ( description[ 17], get_msg( 99), 80 ) ;
   strncat ( description[ 18], get_msg(100), 80 ) ;
   strncat ( description[ 19], get_msg(102), 80 ) ;
   strncat ( description[ 20], get_msg(103), 80 ) ;
   strncat ( description[ 21], get_msg(105), 80 ) ;
   strncat ( description[ 22], get_msg(107), 80 ) ;
   strncat ( description[ 23], get_msg(108), 80 ) ;
   strncat ( description[ 24], get_msg(109), 80 ) ;
   strncat ( description[ 25], get_msg(110), 80 ) ;
   strncat ( description[ 26], get_msg(111), 80 ) ;
   strncat ( description[ 27], get_msg(112), 80 ) ;
   strncat ( description[ 28], get_msg(113), 80 ) ;
   strncat ( description[ 29], get_msg(114), 80 ) ;
   strncat ( description[ 30], get_msg(115), 80 ) ;
   strncat ( description[ 31], get_msg(116), 80 ) ;
   strncat ( description[ 32], get_msg(117), 80 ) ;
   strncat ( description[ 33], get_msg(118), 80 ) ;
   strncat ( description[ 34], get_msg(119), 80 ) ;
   strncat ( description[ 35], get_msg(120), 80 ) ;
   strncat ( description[ 36], get_msg(121), 80 ) ;
   strncat ( description[ 37], get_msg(122), 80 ) ;
   strncat ( description[ 38], get_msg(123), 80 ) ;
   strncat ( description[ 39], get_msg(124), 80 ) ;
   strncat ( description[ 40], get_msg(125), 80 ) ;
   strncat ( description[ 41], get_msg(126), 80 ) ;
   strncat ( description[ 42], get_msg(127), 80 ) ;
   strncat ( description[ 43], get_msg(128), 80 ) ;
   strncat ( description[ 44], get_msg(129), 80 ) ;
   strncat ( description[ 45], get_msg(130), 80 ) ;
   strncat ( description[ 46], get_msg(131), 80 ) ;
   strncat ( description[ 47], get_msg(132), 80 ) ;
   strncat ( description[ 48], get_msg(133), 80 ) ;
   strncat ( description[ 49], get_msg(134), 80 ) ;
   strncat ( description[ 50], get_msg(135), 80 ) ;
   strncat ( description[ 51], get_msg(137), 80 ) ;
   strncat ( description[ 52], get_msg(138), 80 ) ;
   strncat ( description[ 53], get_msg(139), 80 ) ;
   strncat ( description[ 54], get_msg(140), 80 ) ;
   strncat ( description[ 55], get_msg(141), 80 ) ;
   strncat ( description[ 56], get_msg(142), 80 ) ;
   strncat ( description[ 57], get_msg(143), 80 ) ;
   strncat ( description[ 58], get_msg(144), 80 ) ;
   strncat ( description[ 59], get_msg(148), 80 ) ;
   strncat ( description[ 60], get_msg(149), 80 ) ;
   strncat ( description[ 61], get_msg(150), 80 ) ;
   strncat ( description[ 62], get_msg(153), 80 ) ;
   strncat ( description[ 63], get_msg(154), 80 ) ;
   strncat ( description[ 64], get_msg(155), 80 ) ;
   strncat ( description[ 65], get_msg(160), 80 ) ;
   strncat ( description[ 66], get_msg(165), 80 ) ;
   strncat ( description[ 67], get_msg(166), 80 ) ;
   strncat ( description[ 68], get_msg(168), 80 ) ;
   strncat ( description[ 69], get_msg(169), 80 ) ;
   strncat ( description[ 70], get_msg(174), 80 ) ;
   strncat ( description[ 71], get_msg(177), 80 ) ;
   strncat ( description[ 72], get_msg(178), 80 ) ;
   strncat ( description[ 73], get_msg(179), 80 ) ;
   strncat ( description[ 74], get_msg(180), 80 ) ;
   strncat ( description[ 75], get_msg(181), 80 ) ;
   strncat ( description[ 76], get_msg(182), 80 ) ;
   strncat ( description[ 77], get_msg(183), 80 ) ;
   strncat ( description[ 78], get_msg(184), 80 ) ;
   strncat ( description[ 79], get_msg(185), 80 ) ;
   strncat ( description[ 80], get_msg(186), 80 ) ;
   strncat ( description[ 81], get_msg(193), 80 ) ;
   strncat ( description[ 82], get_msg(194), 80 ) ;
   strncat ( description[ 83], get_msg(197), 80 ) ;
   strncat ( description[ 84], get_msg(198), 80 ) ;
   strncat ( description[ 85], get_msg(199), 80 ) ;
   strncat ( description[ 86], get_msg(200), 80 ) ;
   strncat ( description[ 87], get_msg(201), 80 ) ;
   strncat ( description[ 88], get_msg(202), 80 ) ;
   strncat ( description[ 89], get_msg(203), 80 ) ;
   strncat ( description[ 90], get_msg(204), 80 ) ;
   strncat ( description[ 91], get_msg(205), 80 ) ;
   strncat ( description[ 92], get_msg(206), 80 ) ;
   strncat ( description[ 93], get_msg(207), 80 ) ;
   strncat ( description[ 94], get_msg(208), 80 ) ;
   strncat ( description[ 95], get_msg(209), 80 ) ;
   strncat ( description[ 96], get_msg(213), 80 ) ;
   strncat ( description[ 97], get_msg(217), 80 ) ;
   strncat ( description[ 98], get_msg(218), 80 ) ;
   strncat ( description[ 99], get_msg(219), 80 ) ;
   strncat ( description[100], get_msg(222), 80 ) ;
   strncat ( description[101], get_msg(223), 80 ) ;
   strncat ( description[102], get_msg(224), 80 ) ;
   strncat ( description[103], get_msg(231), 80 ) ;
   strncat ( description[104], get_msg(232), 80 ) ;
   strncat ( description[105], get_msg(233), 80 ) ;
   strncat ( description[106], get_msg(234), 80 ) ;
   strncat ( description[107], get_msg(235), 80 ) ;
   strncat ( description[108], get_msg(238), 80 ) ;
   strncat ( description[109], get_msg(239), 80 ) ;
   strncat ( description[110], get_msg(243), 80 ) ;
   strncat ( description[111], get_msg(246), 80 ) ;
   strncat ( description[112], get_msg(247), 80 ) ;
   strncat ( description[113], get_msg(252), 80 ) ;
   strncat ( description[114], get_msg(253), 80 ) ;
   strncat ( description[115], get_msg(254), 80 ) ;
   strncat ( description[116], get_msg(265), 80 ) ;
   strncat ( description[117], get_msg(266), 80 ) ;
   strncat ( description[118], get_msg(267), 80 ) ;
   strncat ( description[119], get_msg(268), 80 ) ;
   strncat ( description[120], get_msg(277), 80 ) ;
   strncat ( description[121], get_msg(278), 80 ) ;
   strncat ( description[122], get_msg(281), 80 ) ;
   strncat ( description[123], get_msg(282), 80 ) ;
   strncat ( description[124], get_msg(283), 80 ) ;
   strncat ( description[125], get_msg(289), 80 ) ;
   strncat ( description[126], get_msg(290), 80 ) ;
   strncat ( description[127], get_msg(291), 80 ) ;
   strncat ( description[128], get_msg(292), 80 ) ;
   strncat ( description[129], get_msg(293), 80 ) ;
}
 
/****************************************************************************/
/*                                                                          */
/* FUNCTION NAME: InitializeDescriptions2                                   */
/*                                                                          */
/* FUNCTION: Initializes a two-dimensional array which stores all the       */
/*           descriptions used.  Descriptions are the text used in the      */
/*           first column of any display.  See InitializeDescriptions1 for  */
/*           an explanation of why two functions were used.                 */
/*                                                                          */
/****************************************************************************/
 
void InitializeDescriptions2 (void)
{
   strncat ( description[130], get_msg(295), 80 ) ;
   strncat ( description[131], get_msg(297), 80 ) ;
   strncat ( description[132], get_msg(298), 80 ) ;
   strncat ( description[133], get_msg(300), 80 ) ;
   strncat ( description[134], get_msg(301), 80 ) ;
   strncat ( description[135], get_msg(302), 80 ) ;
   strncat ( description[136], get_msg(303), 80 ) ;
   strncat ( description[137], get_msg(304), 80 ) ;
   strncat ( description[138], get_msg(307), 80 ) ;
   strncat ( description[139], get_msg(308), 80 ) ;
   strncat ( description[140], get_msg(309), 80 ) ;
   strncat ( description[141], get_msg(310), 80 ) ;
   strncat ( description[142], get_msg(311), 80 ) ;
   strncat ( description[143], get_msg(314), 80 ) ;
   strncat ( description[144], get_msg(318), 80 ) ;
   strncat ( description[145], get_msg(328), 80 ) ;
   strncat ( description[146], get_msg(332), 80 ) ;
   strncat ( description[147], get_msg(333), 80 ) ;
   strncat ( description[148], get_msg(334), 80 ) ;
   strncat ( description[149], get_msg(335), 80 ) ;
   strncat ( description[150], get_msg(336), 80 ) ;
   strncat ( description[151], get_msg(337), 80 ) ;
   strncat ( description[152], get_msg(338), 80 ) ;
   strncat ( description[153], get_msg(339), 80 ) ;
   strncat ( description[154], get_msg(340), 80 ) ;
   strncat ( description[155], get_msg(341), 80 ) ;
   strncat ( description[156], get_msg(342), 80 ) ;
   strncat ( description[157], get_msg(343), 80 ) ;
   strncat ( description[158], get_msg(344), 80 ) ;
   strncat ( description[159], get_msg(345), 80 ) ;
   strncat ( description[160], get_msg(346), 80 ) ;
   strncat ( description[161], get_msg(347), 80 ) ;
   strncat ( description[162], get_msg(348), 80 ) ;
   strncat ( description[163], get_msg(349), 80 ) ;
   strncat ( description[164], get_msg(350), 80 ) ;
   strncat ( description[165], get_msg(351), 80 ) ;
   strncat ( description[166], get_msg(352), 80 ) ;
   strncat ( description[167], get_msg(353), 80 ) ;
   strncat ( description[168], get_msg(354), 80 ) ;
   strncat ( description[169], get_msg(355), 80 ) ;
   strncat ( description[170], get_msg(356), 80 ) ;
   strncat ( description[171], get_msg(357), 80 ) ;
   strncat ( description[172], get_msg(358), 80 ) ;
   strncat ( description[173], get_msg(359), 80 ) ;
   strncat ( description[174], get_msg(360), 80 ) ;
   strncat ( description[175], get_msg(362), 80 ) ;
   strncat ( description[176], get_msg(363), 80 ) ;
   strncat ( description[177], get_msg(364), 80 ) ;
   strncat ( description[178], get_msg(365), 80 ) ;
   strncat ( description[179], get_msg(366), 80 ) ;
   strncat ( description[180], get_msg(367), 80 ) ;
   strncat ( description[181], get_msg(368), 80 ) ;
   strncat ( description[182], get_msg(369), 80 ) ;
   strncat ( description[183], get_msg(370), 80 ) ;
   strncat ( description[184], get_msg(371), 80 ) ;
   strncat ( description[185], get_msg(372), 80 ) ;
   strncat ( description[186], get_msg(373), 80 ) ;
   strncat ( description[187], get_msg(374), 80 ) ;
   strncat ( description[188], get_msg(375), 80 ) ;
   strncat ( description[189], get_msg(382), 80 ) ;
   strncat ( description[190], get_msg(385), 80 ) ;
   strncat ( description[191], get_msg(388), 80 ) ;
   strncat ( description[192], get_msg(389), 80 ) ;
   strncat ( description[193], get_msg(390), 80 ) ;
   strncat ( description[194], get_msg(391), 80 ) ;
   strncat ( description[195], get_msg(392), 80 ) ;
   strncat ( description[196], get_msg(393), 80 ) ;
   strncat ( description[197], get_msg(394), 80 ) ;
   strncat ( description[198], get_msg(395), 80 ) ;
   strncat ( description[199], get_msg(396), 80 ) ;
   strncat ( description[200], get_msg(397), 80 ) ;
   strncat ( description[201], get_msg(398), 80 ) ;
   strncat ( description[202], get_msg(399), 80 ) ;
   strncat ( description[203], get_msg(400), 80 ) ;
   strncat ( description[204], get_msg(402), 80 ) ;
   strncat ( description[205], get_msg(403), 80 ) ;
   strncat ( description[206], get_msg(405), 80 ) ;
   strncat ( description[207], get_msg(406), 80 ) ;
   strncat ( description[208], get_msg(407), 80 ) ;
   strncat ( description[209], get_msg(408), 80 ) ;
   strncat ( description[210], get_msg(409), 80 ) ;
   strncat ( description[211], get_msg(411), 80 ) ;
   strncat ( description[212], get_msg(412), 80 ) ;
   strncat ( description[213], get_msg(416), 80 ) ;
   strncat ( description[214], get_msg(417), 80 ) ;
   strncat ( description[215], get_msg(418), 80 ) ;
   strncat ( description[216], get_msg(419), 80 ) ;
   strncat ( description[217], get_msg(420), 80 ) ;
   strncat ( description[218], get_msg(421), 80 ) ;
   strncat ( description[219], get_msg(422), 80 ) ;
   strncat ( description[220], get_msg(423), 80 ) ;
   strncat ( description[221], get_msg(426), 80 ) ;
   strncat ( description[222], get_msg(429), 80 ) ;
   strncat ( description[223], get_msg(430), 80 ) ;
   strncat ( description[224], get_msg(431), 80 ) ;
   strncat ( description[225], get_msg(433), 80 ) ;
   strncat ( description[226], get_msg(434), 80 ) ;
   strncat ( description[227], get_msg(435), 80 ) ;
   strncat ( description[228], get_msg(436), 80 ) ;
   strncat ( description[229], get_msg(437), 80 ) ;
   strncat ( description[230], get_msg(442), 80 ) ;
   strncat ( description[231], get_msg(443), 80 ) ;
   strncat ( description[232], get_msg(444), 80 ) ;
   strncat ( description[233], get_msg(445), 80 ) ;
   strncat ( description[234], get_msg(446), 80 ) ;
   strncat ( description[235], get_msg(447), 80 ) ;
   strncat ( description[236], get_msg(448), 80 ) ;
   strncat ( description[237], get_msg(449), 80 ) ;
   strncat ( description[238], get_msg(450), 80 ) ;
   strncat ( description[239], get_msg(451), 80 ) ;
   strncat ( description[240], get_msg(452), 80 ) ;
   strncat ( description[241], get_msg(453), 80 ) ;
   strncat ( description[242], get_msg(454), 80 ) ;
   strncat ( description[243], get_msg(455), 80 ) ;
   strncat ( description[244], get_msg(456), 80 ) ;
   strncat ( description[245], get_msg(457), 80 ) ;
   strncat ( description[246], get_msg(458), 80 ) ;
   strncat ( description[247], get_msg(459), 80 ) ;
   strncat ( description[248], get_msg(460), 80 ) ;
   strncat ( description[249], get_msg(461), 80 ) ;
   strncat ( description[250], get_msg(462), 80 ) ;
   strncat ( description[251], get_msg(463), 80 ) ;
   strncat ( description[252], get_msg(464), 80 ) ;
   strncat ( description[253], get_msg(465), 80 ) ;
   strncat ( description[254], get_msg(466), 80 ) ;
   strncat ( description[255], get_msg(467), 80 ) ;
   strncat ( description[256], get_msg(468), 80 ) ;
   strncat ( description[257], get_msg(469), 80 ) ;
   strncat ( description[258], get_msg(470), 80 ) ;
}
 
/****************************************************************************/
/*                                                                          */
/* FUNCTION NAME: InitializeValues                                          */
/*                                                                          */
/* FUNCTION: Initializes a two-dimensional array which stores all the       */
/*           values used.  Values are the text used in the second column    */
/*           of any display.                                                */
/*                                                                          */
/****************************************************************************/
 
void InitializeValues (void)
{
   strncat ( value[  0], get_msg( 66), 50 ) ;
   strncat ( value[  1], get_msg( 67), 50 ) ;
   strncat ( value[  2], get_msg( 68), 50 ) ;
   strncat ( value[  3], get_msg( 69), 50 ) ;
   strncat ( value[  4], get_msg( 70), 50 ) ;
   strncat ( value[  5], get_msg( 71), 50 ) ;
   strncat ( value[  6], get_msg( 72), 50 ) ;
   strncat ( value[  7], get_msg( 73), 50 ) ;
   strncat ( value[  8], get_msg( 74), 50 ) ;
   strncat ( value[  9], get_msg( 75), 50 ) ;
   strncat ( value[ 10], get_msg( 88), 50 ) ;
   strncat ( value[ 11], get_msg( 89), 50 ) ;
   strncat ( value[ 12], get_msg( 91), 50 ) ;
   strncat ( value[ 13], get_msg( 92), 50 ) ;
   strncat ( value[ 14], get_msg(101), 50 ) ;
   strncat ( value[ 15], get_msg(104), 50 ) ;
   strncat ( value[ 16], get_msg(106), 50 ) ;
   strncat ( value[ 17], get_msg(136), 50 ) ;
   strncat ( value[ 18], get_msg(145), 50 ) ;
   strncat ( value[ 19], get_msg(146), 50 ) ;
   strncat ( value[ 20], get_msg(147), 50 ) ;
   strncat ( value[ 21], get_msg(151), 50 ) ;
   strncat ( value[ 22], get_msg(152), 50 ) ;
   strncat ( value[ 23], get_msg(156), 50 ) ;
   strncat ( value[ 24], get_msg(157), 50 ) ;
   strncat ( value[ 25], get_msg(158), 50 ) ;
   strncat ( value[ 26], get_msg(159), 50 ) ;
   strncat ( value[ 27], get_msg(161), 50 ) ;
   strncat ( value[ 28], get_msg(162), 50 ) ;
   strncat ( value[ 29], get_msg(163), 50 ) ;
   strncat ( value[ 30], get_msg(164), 50 ) ;
   strncat ( value[ 31], get_msg(167), 50 ) ;
   strncat ( value[ 32], get_msg(170), 50 ) ;
   strncat ( value[ 33], get_msg(171), 50 ) ;
   strncat ( value[ 34], get_msg(172), 50 ) ;
   strncat ( value[ 35], get_msg(173), 50 ) ;
   strncat ( value[ 36], get_msg(175), 50 ) ;
   strncat ( value[ 37], get_msg(176), 50 ) ;
   strncat ( value[ 38], get_msg(187), 50 ) ;
   strncat ( value[ 39], get_msg(188), 50 ) ;
   strncat ( value[ 40], get_msg(189), 50 ) ;
   strncat ( value[ 41], get_msg(190), 50 ) ;
   strncat ( value[ 42], get_msg(191), 50 ) ;
   strncat ( value[ 43], get_msg(192), 50 ) ;
   strncat ( value[ 44], get_msg(195), 50 ) ;
   strncat ( value[ 45], get_msg(196), 50 ) ;
   strncat ( value[ 46], get_msg(210), 50 ) ;
   strncat ( value[ 47], get_msg(211), 50 ) ;
   strncat ( value[ 48], get_msg(212), 50 ) ;
   strncat ( value[ 49], get_msg(214), 50 ) ;
   strncat ( value[ 50], get_msg(215), 50 ) ;
   strncat ( value[ 51], get_msg(216), 50 ) ;
   strncat ( value[ 52], get_msg(220), 50 ) ;
   strncat ( value[ 53], get_msg(221), 50 ) ;
   strncat ( value[ 54], get_msg(225), 50 ) ;
   strncat ( value[ 55], get_msg(226), 50 ) ;
   strncat ( value[ 56], get_msg(227), 50 ) ;
   strncat ( value[ 57], get_msg(228), 50 ) ;
   strncat ( value[ 58], get_msg(229), 50 ) ;
   strncat ( value[ 59], get_msg(230), 50 ) ;
   strncat ( value[ 60], get_msg(236), 50 ) ;
   strncat ( value[ 61], get_msg(237), 50 ) ;
   strncat ( value[ 62], get_msg(240), 50 ) ;
   strncat ( value[ 63], get_msg(241), 50 ) ;
   strncat ( value[ 64], get_msg(242), 50 ) ;
   strncat ( value[ 65], get_msg(244), 50 ) ;
   strncat ( value[ 66], get_msg(245), 50 ) ;
   strncat ( value[ 67], get_msg(248), 50 ) ;
   strncat ( value[ 68], get_msg(249), 50 ) ;
   strncat ( value[ 69], get_msg(250), 50 ) ;
   strncat ( value[ 70], get_msg(251), 50 ) ;
   strncat ( value[ 71], get_msg(255), 50 ) ;
   strncat ( value[ 72], get_msg(256), 50 ) ;
   strncat ( value[ 73], get_msg(257), 50 ) ;
   strncat ( value[ 74], get_msg(258), 50 ) ;
   strncat ( value[ 75], get_msg(259), 50 ) ;
   strncat ( value[ 76], get_msg(260), 50 ) ;
   strncat ( value[ 77], get_msg(261), 50 ) ;
   strncat ( value[ 78], get_msg(262), 50 ) ;
   strncat ( value[ 79], get_msg(263), 50 ) ;
   strncat ( value[ 80], get_msg(264), 50 ) ;
   strncat ( value[ 81], get_msg(269), 50 ) ;
   strncat ( value[ 82], get_msg(270), 50 ) ;
   strncat ( value[ 83], get_msg(271), 50 ) ;
   strncat ( value[ 84], get_msg(272), 50 ) ;
   strncat ( value[ 85], get_msg(273), 50 ) ;
   strncat ( value[ 86], get_msg(274), 50 ) ;
   strncat ( value[ 87], get_msg(275), 50 ) ;
   strncat ( value[ 88], get_msg(276), 50 ) ;
   strncat ( value[ 89], get_msg(279), 50 ) ;
   strncat ( value[ 90], get_msg(280), 50 ) ;
   strncat ( value[ 91], get_msg(284), 50 ) ;
   strncat ( value[ 92], get_msg(285), 50 ) ;
   strncat ( value[ 93], get_msg(286), 50 ) ;
   strncat ( value[ 94], get_msg(287), 50 ) ;
   strncat ( value[ 95], get_msg(288), 50 ) ;
   strncat ( value[ 96], get_msg(294), 50 ) ;
   strncat ( value[ 97], get_msg(296), 50 ) ;
   strncat ( value[ 98], get_msg(299), 50 ) ;
   strncat ( value[ 99], get_msg(305), 50 ) ;
   strncat ( value[100], get_msg(306), 50 ) ;
   strncat ( value[101], get_msg(312), 50 ) ;
   strncat ( value[102], get_msg(313), 50 ) ;
   strncat ( value[103], get_msg(315), 50 ) ;
   strncat ( value[104], get_msg(316), 50 ) ;
   strncat ( value[105], get_msg(317), 50 ) ;
   strncat ( value[106], get_msg(319), 50 ) ;
   strncat ( value[107], get_msg(320), 50 ) ;
   strncat ( value[108], get_msg(321), 50 ) ;
   strncat ( value[109], get_msg(322), 50 ) ;
   strncat ( value[110], get_msg(323), 50 ) ;
   strncat ( value[111], get_msg(324), 50 ) ;
   strncat ( value[112], get_msg(325), 50 ) ;
   strncat ( value[113], get_msg(326), 50 ) ;
   strncat ( value[114], get_msg(327), 50 ) ;
   strncat ( value[115], get_msg(329), 50 ) ;
   strncat ( value[116], get_msg(330), 50 ) ;
   strncat ( value[117], get_msg(331), 50 ) ;
   strncat ( value[118], get_msg(361), 50 ) ;
   strncat ( value[119], get_msg(376), 50 ) ;
   strncat ( value[120], get_msg(377), 50 ) ;
   strncat ( value[121], get_msg(378), 50 ) ;
   strncat ( value[122], get_msg(379), 50 ) ;
   strncat ( value[123], get_msg(380), 50 ) ;
   strncat ( value[124], get_msg(381), 50 ) ;
   strncat ( value[125], get_msg(383), 50 ) ;
   strncat ( value[126], get_msg(384), 50 ) ;
   strncat ( value[127], get_msg(386), 50 ) ;
   strncat ( value[128], get_msg(387), 50 ) ;
   strncat ( value[129], get_msg(401), 50 ) ;
   strncat ( value[130], get_msg(404), 50 ) ;
   strncat ( value[131], get_msg(410), 50 ) ;
   strncat ( value[132], get_msg(413), 50 ) ;
   strncat ( value[133], get_msg(414), 50 ) ;
   strncat ( value[134], get_msg(415), 50 ) ;
   strncat ( value[135], get_msg(424), 50 ) ;
   strncat ( value[136], get_msg(425), 50 ) ;
   strncat ( value[137], get_msg(427), 50 ) ;
   strncat ( value[138], get_msg(428), 50 ) ;
   strncat ( value[139], get_msg(432), 50 ) ;
   strncat ( value[140], get_msg(438), 50 ) ;
   strncat ( value[141], get_msg(439), 50 ) ;
   strncat ( value[142], get_msg(440), 50 ) ;
   strncat ( value[143], get_msg(441), 50 ) ;
}
 
/****************************************************************************/
/*                                                                          */
/* FUNCTION NAME: InitializeMessages                                        */
/*                                                                          */
/* FUNCTION: Initializes a two-dimensional array which stores all the       */
/*           messages used.  Messages are titles, error messages and any    */
/*           other short text other than descriptions and values.           */
/*                                                                          */
/****************************************************************************/
 
void InitializeMessages (void)
{
   strncat ( message[  0], get_msg(  0), 80 ) ;
   strncat ( message[  1], get_msg(  1), 80 ) ;
   strncat ( message[  2], get_msg(  3), 80 ) ;
   strncat ( message[  3], get_msg(  4), 80 ) ;
   strncat ( message[  4], get_msg(  5), 80 ) ;
   strncat ( message[  5], get_msg(  6), 80 ) ;
   strncat ( message[  6], get_msg(  7), 80 ) ;
   strncat ( message[  7], get_msg(  8), 80 ) ;
   strncat ( message[  8], get_msg(  9), 80 ) ;
   strncat ( message[  9], get_msg( 10), 80 ) ;
   strncat ( message[ 10], get_msg( 11), 80 ) ;
   strncat ( message[ 11], get_msg( 12), 80 ) ;
   strncat ( message[ 12], get_msg( 13), 80 ) ;
   strncat ( message[ 13], get_msg( 14), 80 ) ;
   strncat ( message[ 14], get_msg( 15), 80 ) ;
   strncat ( message[ 15], get_msg( 16), 80 ) ;
   strncat ( message[ 16], get_msg( 18), 80 ) ;
   strncat ( message[ 17], get_msg( 19), 80 ) ;
   strncat ( message[ 18], get_msg( 20), 80 ) ;
   strncat ( message[ 19], get_msg( 21), 80 ) ;
   strncat ( message[ 20], get_msg( 22), 80 ) ;
   strncat ( message[ 21], get_msg( 23), 80 ) ;
   strncat ( message[ 22], get_msg( 24), 80 ) ;
   strncat ( message[ 23], get_msg( 25), 80 ) ;
   strncat ( message[ 24], get_msg( 26), 80 ) ;
   strncat ( message[ 25], get_msg( 27), 80 ) ;
   strncat ( message[ 26], get_msg( 28), 80 ) ;
   strncat ( message[ 27], get_msg( 29), 80 ) ;
   strncat ( message[ 28], get_msg( 30), 80 ) ;
   strncat ( message[ 29], get_msg( 31), 80 ) ;
   strncat ( message[ 30], get_msg( 32), 80 ) ;
   strncat ( message[ 31], get_msg( 33), 80 ) ;
   strncat ( message[ 32], get_msg( 34), 80 ) ;
   strncat ( message[ 33], get_msg( 35), 80 ) ;
   strncat ( message[ 34], get_msg( 36), 80 ) ;
   strncat ( message[ 35], get_msg( 37), 80 ) ;
   strncat ( message[ 36], get_msg( 38), 80 ) ;
   strncat ( message[ 37], get_msg( 39), 80 ) ;
   strncat ( message[ 38], get_msg( 41), 80 ) ;
   strncat ( message[ 39], get_msg( 42), 80 ) ;
   strncat ( message[ 40], get_msg( 43), 80 ) ;
   strncat ( message[ 41], get_msg( 44), 80 ) ;
   strncat ( message[ 42], get_msg( 45), 80 ) ;
   strncat ( message[ 43], get_msg( 46), 80 ) ;
   strncat ( message[ 44], get_msg( 47), 80 ) ;
   strncat ( message[ 45], get_msg( 48), 80 ) ;
   strncat ( message[ 46], get_msg( 49), 80 ) ;
   strncat ( message[ 47], get_msg( 50), 80 ) ;
   strncat ( message[ 48], get_msg( 51), 80 ) ;
   strncat ( message[ 49], get_msg( 52), 80 ) ;
   strncat ( message[ 50], get_msg( 53), 80 ) ;
   strncat ( message[ 51], get_msg( 54), 80 ) ;
   strncat ( message[ 52], get_msg( 55), 80 ) ;
   strncat ( message[ 53], get_msg( 56), 80 ) ;
   strncat ( message[ 54], get_msg( 57), 80 ) ;
   strncat ( message[ 55], get_msg( 58), 80 ) ;
   strncat ( message[ 56], get_msg( 59), 80 ) ;
   strncat ( message[ 57], get_msg( 60), 80 ) ;
   strncat ( message[ 58], get_msg( 61), 80 ) ;
   strncat ( message[ 59], get_msg( 62), 80 ) ;
   strncat ( message[ 60], get_msg( 63), 80 ) ;
   strncat ( message[ 61], get_msg( 64), 80 ) ;
   strncat ( message[ 62], get_msg( 65), 80 ) ;
   strncat ( message[ 63], get_msg( 76), 80 ) ;
   strncat ( message[ 64], get_msg( 77), 80 ) ;
   strncat ( message[ 65], get_msg(471), 80 ) ;
   strncat ( message[ 66], get_msg(472), 80 ) ;
   strncat ( message[ 67], get_msg(473), 80 ) ;
   strncat ( message[ 68], get_msg(474), 80 ) ;
   strncat ( message[ 69], get_msg(475), 80 ) ;
   strncat ( message[ 70], get_msg(476), 80 ) ;
   strncat ( message[ 71], get_msg(477), 80 ) ;
   strncat ( message[ 72], get_msg(478), 80 ) ;
   strncat ( message[ 73], get_msg(479), 80 ) ;
   strncat ( message[ 74], get_msg(480), 80 ) ;
   strncat ( message[ 75], get_msg(481), 80 ) ;
   strncat ( message[ 76], get_msg(482), 80 ) ;
   strncat ( message[ 77], get_msg(484), 80 ) ;
   strncat ( message[ 78], get_msg(485), 80 ) ;
   strncat ( message[ 79], get_msg(486), 80 ) ;
   strncat ( message[ 80], get_msg(489), 80 ) ;
   strncat ( message[ 81], get_msg(490), 80 ) ;
}
 
/****************************************************************************/
/*                                                                          */
/* FUNCTION NAME: InitializeLongMessages                                    */
/*                                                                          */
/* FUNCTION: Initializes a two-dimensional array which stores all the       */
/*           long messages used.  Long messages are the few messages much   */
/*           longer than the others.  A separate array was used in order    */
/*           to reduce the amount of space needed to store all the messages.*/
/*                                                                          */
/****************************************************************************/
 
void InitializeLongMessages (void)
{
   strncat ( long_msg[  0], get_msg(  2), 410 ) ;
   strncat ( long_msg[  1], get_msg( 17), 410 ) ;
   strncat ( long_msg[  2], get_msg( 40), 410 ) ;
   strncat ( long_msg[  3], get_msg(483), 410 ) ;
   strncat ( long_msg[  4], get_msg(487), 410 ) ;
   strncat ( long_msg[  5], get_msg(488), 410 ) ;
}
