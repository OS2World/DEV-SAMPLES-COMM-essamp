/*****************************************************************************/
/*                                                                           */
/*  IBM Communications Manager                                               */
/*                                                                           */
/*  Module Name: APPCTELL.C                                                  */
/*                                                                           */
/*  Descriptive                                                              */
/*         Name: Send a VioPopUp message to the specified partner LU.        */
/*                                                                           */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991                          */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM           */
/*                     ALL RIGHTS RESERVED                                   */
/*                                                                           */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                        */
/*                    (compiles with any memory model)                       */
/*                   IBM C Set/2 Compiler for 32 bit applications.           */
/*                                                                           */
/*        Usage: APPCTELL partner message                                    */
/*        Where: partner is either a configured partner LU alias, or         */
/*                       a fully qualified partner LU name.                  */
/*                                                                           */
/*        Notes: 1. For purposes of simplicity, this program assumes that a  */
/*                  partner LU name containing a period (i.e., '.') is a     */
/*                  fully qualified partner LU name.                         */
/*                                                                           */
/*               2. This program does not do thorough error recovery.  If,   */
/*                  for example, an OS/2 error is "detected", then the       */
/*                  program exits, unceremoniously.  Why?  It is quite       */
/*                  possible that the receiver has been defined to the       */
/*                  Attach Manager as type(Background), thus no screen or    */
/*                  keyboard is associated with the program.                 */
/*                                                                           */
/*               3. This program can be compiled as a 16 or a 32 bit         */
/*                  application.                                             */
/*                                                                           */
/*****************************************************************************/

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define  INCL_BASE
#include <os2.h>
#include <acssvcc.h>
#define  INCL_APPC_C
#include <appn.h>

#define KBD_HANDLE        0
#define VIO_HANDLE        0
#define EBCDIC_AT         (char) '\x7C'            /* EBCDIC '@'             */
#define EBCDIC_PERIOD     (char) '\x4B'            /* EBCDIC '.'             */
#define MODE_NAME         "#INTER"
#define TP_NAME           "APPCTELL"
#define SEND_RECEIVE_SIZE  1024
#define AP_DCCD           ( AP_DATA_COMPLETE | AP_CONFIRM_DEALLOCATE )

#ifdef ES32TO16
#define FRPTR   *
#define TYPEC PVOID
#define INCL_DOSMEMMGR
#include <bsememf.h>
#else
#define FRPTR   far *
#define TYPEC ULONG
#endif
/*****************************************************************************/
/* function prototypes                                                       */
/*****************************************************************************/
void ascii_name( UCHAR * dest, const UCHAR * src,
                 const USHORT dest_size );
void sender( const char * partner , const PCH buffer_ptr,
             const USHORT dlen );
void convert( UCHAR * name, const USHORT size, const UCHAR dir,
              const UCHAR char_set );
void do_verb( void FRPTR vcb, const char verb_name[] );
void ebcdic_name( UCHAR * dest, const UCHAR * src, const USHORT dest_size,
                  const UCHAR char_set );
void fp_memcpy( PCH dest, PCH src , USHORT cnt );
PCH  get_shared_buffer( const USHORT buffer_length );
void init_vcb( void * vcb, const USHORT size, const USHORT opcode );
int  invalid_PLU_name( char * PLU_name );
void receiver( PCH buffer_ptr );
void today( char datestamp[], char timestamp[] );
void Usage( void );
BOOL  test;

/*---------------------------------------------------------------------------*/
/* main: routine used to determine whether program is a sender, or a receiver*/
/*---------------------------------------------------------------------------*/
void cdecl main( int argc, char * argv[] )
{
   int    i;                           /* Loop control variable              */
   USHORT LL_field = 2;                /* Overall length of data to be sent  */
   char   buffer[ SEND_RECEIVE_SIZE ]; /* Local buffer used to build message */
   PCH    buffer_ptr = get_shared_buffer( SEND_RECEIVE_SIZE );

   /**************************************************************************/
   /* Basic conversations have a 2 byte LL in the front of the data.  So we  */
   /* build the message by first leaving room for these bytes, then appending*/
   /* each of the parameters specified on the command line.                  */
   /* The C run-time was kind enough to parse the command line arguments     */
   /* into separate "words".  In order to recreate the message, we need to   */
   /* put the words back together again (separating each word with a blank.) */
   /**************************************************************************/
   strcpy( buffer, "  " );             /* Leave room for the LL with 2 blanks*/
   for ( i = 2; i < argc; i++ ) {      /* Build the message text in buffer   */
     if ( strlen( buffer ) + strlen( argv[ i ] ) < SEND_RECEIVE_SIZE ) {
        strcat( buffer, argv[ i ] );
        strcat( buffer, " " );
        LL_field += strlen( argv[ i ] ) + 1;
     } else {
        printf( "Message truncated.\n" );
        i = argc;
     }
   }
   /**************************************************************************/
   /* Finally, we need to change the trailing blank to a null, and put the 2 */
   /* byte LL field in the front of the record (in non-Intel byte order.)    */
   /**************************************************************************/
   buffer[ LL_field - 1 ] = '\0';
   * (USHORT *) buffer = SWAP2( LL_field );

   /**************************************************************************/
   /* Now, we copy the message from the local buffer into the shared buffer  */
   /* that we had OS/2 allocate for us.  This allows us to be memory model   */
   /* independent.                                                           */
   /**************************************************************************/
   fp_memcpy( buffer_ptr, buffer, LL_field );

   if ( argc > 1 ) {                   /* Parms imply sender program         */
      if ( ( argc == 2 ) || invalid_PLU_name( argv[ 1 ] ) )
         Usage();                      /* Parameter error - tell the user    */
      else                             /* argv[ 1 ] == Partner LU name       */
         sender( argv[ 1 ], buffer_ptr, LL_field );
   } else                              /* No parms, implies receiver program */
      receiver( buffer_ptr );
   exit(EXIT_SUCCESS);
}

/*---------------------------------------------------------------------------*/
/* fp_memcpy is a memory model independent version of memcpy, because it     */
/* requires far pointers as passed parameters.                               */
/*---------------------------------------------------------------------------*/
void fp_memcpy( PCH    dest,           /* Pointer to destination             */
                PCH    src ,           /* Pointer to source                  */
                USHORT cnt )           /* Number of bytes to be copied       */
{
  for( ; cnt; cnt-- )                  /* Copy from source to target         */
    *dest++ = *src++;                  /* without any error checking         */
}                                      /* (e.g., overlaping memory).         */

/*---------------------------------------------------------------------------*/
/* init_vcb: Clear a Verb Control Block, and fill in the opcode field.       */
/*---------------------------------------------------------------------------*/
void init_vcb( void * vcb, const USHORT size, const USHORT opcode )
{
   memset( vcb, 0, size );
   ( (APPC_HDR *) vcb ) -> opcode = opcode;
}

/*---------------------------------------------------------------------------*/
/* convert: Use the APPC Service routine to convert a field to ASCII/EBCDIC. */
/*---------------------------------------------------------------------------*/
void convert( UCHAR * name       ,     /* TP or LU name   to be converted    */
              const USHORT    size ,     /* Number of bytes to be converted    */
              const UCHAR   dir  ,     /* Direction (e.g., ASCII_TO_EBCDIC)  */
              const UCHAR   char_set ) /* Type of conversion (e.g., A, AE)   */
{
   CONVERT cvt;
   PCH    vcb_ptr = (PCH) &cvt;        /* far pointer to Verb Control Block  */
   char * period =                     /* Find the first ASCII/EBCDIC period */
          strchr( name, ( dir == SV_ASCII_TO_EBCDIC ) ? '.' : EBCDIC_PERIOD );

   /**************************************************************************/
   /* Check for the special case of a Fully Qualified Partner LU name.  If a */
   /* period (of the appropriate character set) exists, change it to a char  */
   /* that exists in the type-A character set (i.e., the at sign '@')        */
   /**************************************************************************/
   if ( (char *) NULL != period ) {
     * period = ( dir == SV_ASCII_TO_EBCDIC ) ? (char)'@' : EBCDIC_AT;
   }
   init_vcb( &cvt, (USHORT) sizeof( cvt ), SV_CONVERT );
   cvt.direction = dir;
   cvt.char_set  = char_set;
   cvt.len       = (USHORT) size;
   cvt.source    = cvt.target = name;
   ACSSVC( (TYPEC) vcb_ptr );
   if ( AP_OK != cvt.primary_rc ) {
      printf( "Unexpected return code on %s verb (%04X %08lX).\n",
              "CONVERT",
              SWAP2( cvt.primary_rc ),
              SWAP4( cvt.secondary_rc ) );
      if (cvt.primary_rc == SV_COMM_SUBSYSTEM_NOT_LOADED) {
         printf("\tCommunications Manager is not started\n");
         exit(EXIT_FAILURE);
      } else {
      } /* endif */
   }
   /**************************************************************************/
   /* Translate the period separating the Network Name and LUNAME fields     */
   /* separately.  If the period was changed to an '@' above, then we need   */
   /* to change it to a period in the appropriate character set.             */
   /**************************************************************************/
   if ( (char *) NULL != period ) {
     * period = ( dir == SV_ASCII_TO_EBCDIC ) ? EBCDIC_PERIOD : (char)'.';
   }
}

/*---------------------------------------------------------------------------*/
/* ascii_name: Copy the specified name into target field, and pad with blanks*/
/*---------------------------------------------------------------------------*/
void ascii_name( UCHAR * dest, const UCHAR * src, const USHORT dest_size )
{
   strncpy( dest, src, dest_size );
   memset( &dest[ strlen( src ) ], (int)' ', dest_size - strlen( src ) );
}

/*---------------------------------------------------------------------------*/
/* ebcdic_name: Copy the specified name, pad with blanks, and convert it.    */
/*---------------------------------------------------------------------------*/
void ebcdic_name( UCHAR * dest, const UCHAR * src, const USHORT dest_size,
                  const UCHAR char_set )
{
   ascii_name( dest, src, dest_size ); /* Call this to copy & blank pad name */
   /* then convert */
   convert( dest, dest_size, SV_ASCII_TO_EBCDIC, char_set );
}

/*---------------------------------------------------------------------------*/
/* do_verb: Pass the specified Verb Control Block to APPC & display result.  */
/*---------------------------------------------------------------------------*/
void do_verb( void FRPTR vcb, const char verb_name[] )
{
   APPC_HDR FRPTR hdr = vcb;

   APPC( (TYPEC) vcb );
   if ( AP_OK != hdr -> primary_rc ) {
      printf( "Unexpected return code on %s verb (%04X %08lX).\n", verb_name,
              SWAP2( hdr -> primary_rc ), SWAP4( hdr -> secondary_rc ) );
   if (hdr -> primary_rc == SV_COMM_SUBSYSTEM_NOT_LOADED) {
      printf("\tCommunications Manager is not started\n");
      exit(EXIT_FAILURE);
   } else {
      if (hdr -> primary_rc == AP_COMM_SUBSYSTEM_NOT_LOADED) {
         printf("\tEither the Communications Manager is not started\n");
         printf("\tor APPC is not loaded.\n");
         exit(EXIT_FAILURE);
      } else {
         if (hdr -> primary_rc == AP_COMM_SUBSYSTEM_ABENDED) {
            printf("\tAPPC has ABENDED.\n");
            exit(EXIT_FAILURE);
         }
      } /* endif */
   } /* endif */
   }
}

/*---------------------------------------------------------------------------*/
/* get_shared_buffer: Have OS/2 allocate a shared buffer as required by APPC */
/*---------------------------------------------------------------------------*/
PCH get_shared_buffer( const USHORT buffer_length )
{
#ifdef ES32TO16
   /*--------------------------------------------*/
   /* 32 bit memory management                   */
   /*--------------------------------------------*/
  PCH    dptr;                              /* Address of allocated buffer  */
  ULONG  dos_rc;                            /* Return code from Get Mem.    */
  if ( ! ( dos_rc = DosAllocSharedMem((PPVOID)&dptr,
                                      0,
                                      buffer_length,
                                      PAG_COMMIT   |
                                      OBJ_GIVEABLE |
                                      OBJ_TILE     |
                                      PAG_WRITE) ) ) {
  } else {
    printf( "Error allocating shared buffer.  OS/2 return code = %d\n",
             dos_rc );
    exit( dos_rc );
  }
   /*--------------------------------------------*/
   /* End of 32 bit memory management            */
   /*--------------------------------------------*/
#else
   /*--------------------------------------------*/
   /* 16 bit memory management                   */
   /*--------------------------------------------*/
   int    dos_rc;
   USHORT selector;
   PCH    dptr;

   if ( ! ( dos_rc = DosAllocSeg( buffer_length, (PSEL) &selector, 1) ) ) {
      OFFSETOF( dptr ) = 0;            /* set the offset to zero             */
      SELECTOROF( dptr ) = selector;   /* address = Selector:0               */
   } else {
      printf( "Error allocating shared buffer.  OS/2 return code = %d\n",
               dos_rc );
      exit (dos_rc);
   }
   /*--------------------------------------------*/
   /* End of 16 bit memory management            */
   /*--------------------------------------------*/
#endif
   return dptr;
}

/*---------------------------------------------------------------------------*/
/* Usage: simple program description.                                        */
/*---------------------------------------------------------------------------*/
void Usage( void )
{
   printf( "\nUsage: %s Partner_LU_name Message\n\n", TP_NAME );
   printf( "Where: Partner_LU_name is either:\n" );
   printf( "       a configured partner LU alias, (case sensitive) or\n" );
   printf( "       a fully qualified partner LU name (not case sensitive)." );
   printf( "\n\n Note: This program assumes that if the Partner_LU_name " );
   printf( "contains a period\n       (i.e., '.'), then it is a fully " );
   printf( "qualified partner LU name.\n" );
}

/*---------------------------------------------------------------------------*/
/* today: Return the current date and time in the supplied parameter strings.*/
/*---------------------------------------------------------------------------*/
void today( char datestamp[], char timestamp[] )
{
   DATETIME dt;                        /* date_time of struct DateTime       */
   USHORT   dos_rc;                    /* Return code from Dos routine       */

   /**************************************************************************/
   /* Get the the current date & time from the operating system.             */
   /**************************************************************************/
   if ( NO_ERROR == ( dos_rc = (USHORT)DosGetDateTime( (PDATETIME) &dt ) ) ) {
      sprintf( datestamp, "%d/%d/%d"      , dt.month, dt.day, dt.year % 100  );
      sprintf( timestamp, "%02d:%02d:%02d", dt.hours, dt.minutes, dt.seconds );
   } else {
      printf( "Error requesting date and time.  OS/2 return code = %d\n",
               dos_rc );
   }
}

/*---------------------------------------------------------------------------*/
/* receiver: Routine that actually invokes APPC to converse with the sender. */
/*---------------------------------------------------------------------------*/
void receiver( PCH buffer_ptr )        /* Dos allocated buffer for receive   */
{                                      /*                                    */
   RECEIVE_ALLOCATE ra_vcb;            /* RECEIVE_ALLOCATE Verb Control Block*/
   RECEIVE_AND_WAIT rw_vcb;            /* RECEIVE_AND_WAIT Verb Control Block*/
   CONFIRMED        cd_vcb;            /* CONFIRMED        Verb Control Block*/
   TP_ENDED         te_vcb;            /* TP_ENDED         Verb Control Block*/
   char       datestamp[ 9 ];          /* Of the form: "MM/DD/YY"            */
   char       timestamp[ 9 ];          /* Of the form: "HH:MM:SS"            */
   char       plu[ 18 ];               /* Either PLU_ALIAS or FQPLU_NAME     */
   char     * cp;                      /* Pointer for char manipulation      */
   KBDKEYINFO key;                     /* Used by KbdCharIn                  */
   USHORT     dos_rc;                  /* Return code from OS/2 requests     */
   USHORT     wait = 1;                /* Wait mode for VioPopUp = Yes       */
   USHORT     error = FALSE;           /* If an error gets detected -> 1     */
   char       buffer[ SEND_RECEIVE_SIZE ]; /* local storage into which the   */
                                           /* received message is put.  To   */
                                           /* make us memory model indepent. */

   /**************************************************************************/
   /* First, inform the user that we are acting as a receiver.  This is just */
   /* in case they typed APPCTELL at the prompt with no parameters, thus     */
   /* making it hard to distinquish whether we were ATTACH MANAGER, or user  */
   /* started.  So, the technique used by this program to make this          */
   /* determination is to issue a RECEIVE_ALLOCATE.  However, there is a     */
   /* problem with this tactic.  The problem is that APPCTELL could be       */
   /* configured such that a RECEIVE_ALLOCATE will wait forever for the      */
   /* incoming Attach.  So, what we do about this is to display the Ctrl-    */
   /* Break message on stdout, thus giving the user an opportunity to realize*/
   /* that something is "wrong", and giving them a chance to terminate the   */
   /* program.                                                               */
   /**************************************************************************/
   printf( "\a%s: Acting as an APPC receiver program.\n", TP_NAME );
   printf( "Press Ctrl-Break to terminate.\n" );

   /**************************************************************************/
   /* Next, we check for the incoming allocate.                              */
   /**************************************************************************/
   init_vcb( &ra_vcb, sizeof( ra_vcb ), AP_RECEIVE_ALLOCATE );
   ebcdic_name( ra_vcb.tp_name, TP_NAME, sizeof( ra_vcb.tp_name ), SV_AE );
   do_verb( &ra_vcb, "RECEIVE_ALLOCATE" );
   if ( AP_OK != ra_vcb.primary_rc ) {
      /***********************************************************************/
      /* If we get here, the user probably made a mistake, and invoked the   */
      /* program with no parameters.  In addition, the user is configured,   */
      /* such that the program will not wait forever for an allocate.        */
      /***********************************************************************/
      Usage();
      error = TRUE;
   } else {
      /***********************************************************************/
      /* The RECEIVE_ALLOCATE was successful.  Now, it is good form to find  */
      /* out from whom the message originated.  If it is one for which this  */
      /* LU has not been configured, then the allocate will have come in on  */
      /* the "default" LU.  We figure this out by looking at the first char  */
      /* of the PLU_ALIAS.  In OS/2, default PLU names are like @Innnnnn.    */
      /* So we assume that if an '@' is the first character of the name,     */
      /* then we display the fully qualified partner LU name.                */
      /***********************************************************************/
      /* Note that plu is large enough to hold a fully qualified PLU name,   */
      /* and still have room at the end for the null termination character.  */
      /***********************************************************************/
      memset( plu, '\0', sizeof( plu ) );
      if ( ra_vcb.plu_alias[ 0 ] == '@' ) {
         convert( ra_vcb.fqplu_name           ,
                  sizeof( ra_vcb.fqplu_name ) ,
                  SV_EBCDIC_TO_ASCII          ,
                  SV_A                        );
         memcpy( plu, ra_vcb.fqplu_name, sizeof( ra_vcb.fqplu_name ) );
      } else
         memcpy( plu, ra_vcb.plu_alias, sizeof( ra_vcb.plu_alias ) );
      /***********************************************************************/
      /* Get rid of trailing blanks, so we can make the text of the "Message */
      /* from" line more aesthetically appealing.                            */
      /***********************************************************************/
      if ( NULL != ( cp = strchr( plu, ' ' ) ) )
         * cp = '\0';

      /***********************************************************************/
      /* Get the current date and time stamps into local variables, so the   */
      /* message heading reflects the time the message was received.         */
      /***********************************************************************/
      today( datestamp, timestamp );

      /***********************************************************************/
      /* Now that we have been successfully allocated, we are in "receive"   */
      /* state.  So we need to use one of the RECEIVE verbs to see what was  */
      /* sent by the sender.  For this program, we use receive_and_wait.     */
      /***********************************************************************/
      init_vcb( &rw_vcb, sizeof( rw_vcb ), AP_B_RECEIVE_AND_WAIT );
      rw_vcb.fill       = AP_LL;
      rw_vcb.max_len    = SEND_RECEIVE_SIZE;
      rw_vcb.dptr       = buffer_ptr;
      rw_vcb.rtn_status = AP_YES;
      memcpy( rw_vcb.tp_id, ra_vcb.tp_id, sizeof( rw_vcb.tp_id ) );
      rw_vcb.conv_id    = ra_vcb.conv_id;
      do_verb( &rw_vcb, "RECEIVE_AND_WAIT" );
      if ( AP_OK == rw_vcb.primary_rc ) {
         if ( AP_DCCD == rw_vcb.what_rcvd ) {    /* Combined indicator check */
            /*****************************************************************/
            /* The combined indicator shows that, as expected, the sender    */
            /* did a SEND_DATA, followed by a DEALLOCATE TYPE=SYNC_LEVEL,    */
            /* when SYNC_LEVEL = CONFIRM. {See #define above for AP_DCCD.}   */
            /*****************************************************************/
            /* Now, we respond to the CONFIRM part of the sender's message.  */
            /*****************************************************************/
            init_vcb( &cd_vcb, sizeof( cd_vcb ), AP_B_CONFIRMED );
            memcpy( cd_vcb.tp_id, ra_vcb.tp_id, sizeof( cd_vcb.tp_id ) );
            cd_vcb.conv_id = ra_vcb.conv_id;
            do_verb( &cd_vcb, "CONFIRMED" );
         } else {
            printf( "Unexpected 'what_received' on RECEIVE_AND_WAIT (%04X)\n",
                    rw_vcb.what_rcvd );
            error = TRUE;
         }
      } else
         error = TRUE;
      /***********************************************************************/
      /* At this point, we are finished with the conversation, so we issue   */
      /* the TP_ENDED verb to release the associated LU resources.  If, for  */
      /* some reason, we have encountered a bad return code (for example),   */
      /* the conversation will be deallocated as part of this process.       */
      /***********************************************************************/
      init_vcb( &te_vcb, sizeof( te_vcb ), AP_TP_ENDED );
      memcpy( te_vcb.tp_id, ra_vcb.tp_id, sizeof( te_vcb.tp_id ) );
      do_verb( &te_vcb, "TP_ENDED" );
   }
   /**************************************************************************/
   /* If no error has been detected, display the received message.           */
   /**************************************************************************/
   if ( !error ) {
      /***********************************************************************/
      /* Now that the conversation has been deallocated, and the need for    */
      /* all LU resources has been relinquished, we start using the serial   */
      /* resource of a VioPopUp for displaying the message.                  */
      /***********************************************************************/
      if ( NO_ERROR != ( dos_rc = VioPopUp( (PUSHORT) &wait, VIO_HANDLE ) ) ) {
         printf( "Error requesting VioPopUp.  OS/2 return code = %d\n",
                  dos_rc );
         exit( dos_rc );
      }
      printf( "Message from: %s on %s at %s\n\n", plu, datestamp, timestamp );

      /***********************************************************************/
      /* Now, we copy the message to the local buffer from the shared buffer */
      /* we had OS/2 allocate for us.  This allows us to be memory model     */
      /* independent.                                                        */
      /***********************************************************************/
      fp_memcpy( buffer, buffer_ptr, rw_vcb.dlen );

      /***********************************************************************/
      /* Display the received message, skipping over the LL field.           */
      /***********************************************************************/
      printf( "%s", &buffer[ 2 ] );          /* Msg text starts after the LL */
      /***********************************************************************/
      /* Display a prompt for the user.                                      */
      /***********************************************************************/
      printf( "\n\nPress ENTER to continue." );
      do {
         dos_rc = (USHORT) KbdCharIn(&key,  /* Wait for user keypress        */
                                     0,
                                     KBD_HANDLE );
         if ( NO_ERROR != dos_rc ) {
            printf( "Error reading keyboard.  OS/2 return code = %d\n",
                     dos_rc );
            exit( dos_rc );
         }
      } while ( key.chChar != '\r' );
      /***********************************************************************/
      /* Now, terminate the VioPopUp, defined at the start of this section.  */
      /***********************************************************************/
      if ( NO_ERROR != ( dos_rc = VioEndPopUp( VIO_HANDLE ) ) ) {
         printf( "Error releasing VioPopUp.  OS/2 return code = %d\n",
                  dos_rc );
         exit( dos_rc );
      }
   }
}

/*---------------------------------------------------------------------------*/
/* sender: Routine that actually invokes APPC to converse with the receiver. */
/*---------------------------------------------------------------------------*/
void sender( const char   * partner,   /* Partner Name, Alias/Fully Qualified*/
             const PCH      buffer_ptr,/* Dos Shared Buffer containing msg   */
             const USHORT   dlen    )  /* Number of bytes to be sent         */
{                                      /*                                    */
   TP_STARTED ts_vcb;                  /* TP_STARTED       Verb Control Block*/
   ALLOCATE   al_vcb;                  /* ALLOCATE         Verb Control Block*/
   SEND_DATA  sd_vcb;                  /* SEND_DATA        Verb Control Block*/
   TP_ENDED   te_vcb;                  /* TP_ENDED         Verb Control Block*/
   USHORT i, len;                      /* Loop control variables             */

   /**************************************************************************/
   /* As a sender, we first need to inform the LU that some of its resources */
   /* are needed by this program.  We do this with the TP_STARTED verb.      */
   /**************************************************************************/
   init_vcb( &ts_vcb, sizeof( ts_vcb ), AP_TP_STARTED );
   ebcdic_name( ts_vcb.tp_name, TP_NAME, sizeof( ts_vcb.tp_name ), SV_AE );
   do_verb( &ts_vcb, "TP_STARTED" );
   if ( AP_OK == ts_vcb.primary_rc ) {
      /***********************************************************************/
      /* Now, we identify the partner with which we want to communicate.     */
      /***********************************************************************/
      init_vcb( &al_vcb, sizeof( al_vcb ), AP_B_ALLOCATE );
      al_vcb.conv_type  = AP_BASIC_CONVERSATION;
      al_vcb.sync_level = AP_CONFIRM_SYNC_LEVEL;
      al_vcb.rtn_ctl    = AP_WHEN_SESSION_ALLOCATED;
      al_vcb.security   = AP_NONE;
      if ( NULL == strchr( partner, '.' ) )
         ascii_name ( al_vcb.plu_alias            ,
                      partner                     ,
                      sizeof( al_vcb.plu_alias  ) );
      else
         ebcdic_name( al_vcb.fqplu_name           ,
                      partner                     ,
                      sizeof( al_vcb.fqplu_name ) ,
                      SV_A                        );
      ebcdic_name( al_vcb.mode_name           ,
                   MODE_NAME                  ,
                   sizeof( al_vcb.mode_name ) ,
                   SV_A                       );
      ebcdic_name( al_vcb.tp_name             ,
                   TP_NAME                    ,
                   sizeof( al_vcb.tp_name   ) ,
                   SV_AE                      );
      memcpy( al_vcb.tp_id, ts_vcb.tp_id, sizeof( al_vcb.tp_id ) );
      do_verb( &al_vcb, "ALLOCATE" );
      if ( AP_OK == al_vcb.primary_rc ) {
         /********************************************************************/
         /* Since the ALLOCATE was successful, we can now send the data.     */
         /********************************************************************/
         init_vcb( &sd_vcb, sizeof( sd_vcb ), AP_B_SEND_DATA );
         sd_vcb.dlen = dlen;
         sd_vcb.dptr = buffer_ptr;
         sd_vcb.type = AP_SEND_DATA_DEALLOC_SYNC_LEVEL;
         memcpy( sd_vcb.tp_id, ts_vcb.tp_id, sizeof( sd_vcb.tp_id ) );
         sd_vcb.conv_id = al_vcb.conv_id;
         do_verb( &sd_vcb, "SEND_DATA" );
         /********************************************************************/
         /* Check for some of the most common error conditions.              */
         /********************************************************************/
         if ( AP_ALLOCATION_ERROR == sd_vcb.primary_rc ) {
            if ( AP_TRANS_PGM_NOT_AVAIL_RETRY == sd_vcb.secondary_rc ) {
               printf( "\nThe specified partner probably hasn't started " );
               printf( "their ATTACH_MANAGER.\n" );
            }
            if ( ( AP_TRANS_PGM_NOT_AVAIL_NO_RETRY == sd_vcb.secondary_rc ) ||
                 ( AP_TP_NAME_NOT_RECOGNIZED       == sd_vcb.secondary_rc ) ) {
               printf( "\nThe specified partner probably has been " );
               printf( "configured with no DEFINE_TP\nfor this program, " );
               printf( "or the DIRECTORY_FOR_INBOUND_ATTACHES is not where" );
               printf( "\ntheir copy of this program resides.\n" );
            }
         }
      } else {
         /********************************************************************/
         /* Check for some common Allocation error conditions.               */
         /********************************************************************/
         if ( AP_ALLOCATION_ERROR == al_vcb.primary_rc   ) {
            printf( "\nALLOCATION FAILURE " );
            if ( AP_ALLOCATION_FAILURE_NO_RETRY == al_vcb.secondary_rc )
               printf( "NO RETRY." );
            if ( AP_ALLOCATION_FAILURE_RETRY    == al_vcb.secondary_rc )
               printf( "RETRY." );
            if ( al_vcb.sense_data )
               printf( "  Sense Data = %08lX\n", SWAP4( al_vcb.sense_data ) );
            else
               printf( "\n" );
         }
         /********************************************************************/
         /* Another common error condition check - PLU_ALIAS not defined.    */
         /********************************************************************/
         if ( ( AP_PARAMETER_CHECK      == al_vcb.primary_rc   ) &&
              ( AP_UNKNOWN_PARTNER_MODE == al_vcb.secondary_rc ) ) {
            printf( "\nThe specified partner has not been configured. " );
            printf( "You can either:\n  - Reconfigure, and add this "   );
            printf( "alias, or,\n  - Specify the fully qualified name " );
            printf( "of the partner.\n\n" );
            /*****************************************************************/
            /* The following loop is used to see if the partner LU name has  */
            /* any lowercase letters.  Since this loop occurs only when an   */
            /* error has occured, its performance isn't really of concern.   */
            /*****************************************************************/
            for ( i = 0, len = strlen( partner ); i < len; i++ ) {
               if ( islower( partner[ i ] ) ) {
                  printf( "Note: Partner_LU_alias is a case sensitive " );
                  printf( "field.  You may need to specify it\n      " );
                  printf( "in uppercase.\n" );
                  i = len;
               }
            }
         }
      }
      /***********************************************************************/
      /* At this point, we are finished with the conversation, so we issue   */
      /* the TP_ENDED verb to release the associated LU resources.  If, for  */
      /* some reason, we encountered a bad return code (for example), the    */
      /* conversation will be deallocated as part of this process.           */
      /***********************************************************************/
      init_vcb( &te_vcb, sizeof( te_vcb ), AP_TP_ENDED );
      memcpy( te_vcb.tp_id, ts_vcb.tp_id, sizeof( te_vcb.tp_id ) );
      do_verb( &te_vcb, "TP_ENDED" );
   }
}

/*---------------------------------------------------------------------------*/
/* invalid_PLU_name: check for an unreasonable PLU name.                     */
/*---------------------------------------------------------------------------*/
int invalid_PLU_name( char * PLU_name )
{
   char * period = strchr( PLU_name, '.' ); /* Find first '.' in the name    */
   char * cp;
   USHORT   netid_len, name_len;

   /**************************************************************************/
   /* If the PLU_name contains a period (i.e., '.'), then assume it's a      */
   /* fully qualified PLU name, otherwise, it's a PLU_Alias.                 */
   /**************************************************************************/
   if ( period != NULL ) {             /* period != NULL means '.' present   */
      strupr( PLU_name );              /* FQ PLU names must be uppercase     */
      netid_len = period - PLU_name;   /* Both the netid and name portions   */
      name_len  = ( strlen( PLU_name ) /*   of fully qualified names must    */
                  - netid_len ) - 1;   /*   have from 1..8 characters.       */
      if ( ( netid_len < 1 ) || ( netid_len > 8 ) ||
           ( name_len  < 1 ) || ( name_len  > 8 ) )
         return 1;                     /* We have an invalid name            */
   } else {                            /*                                    */
      name_len = strlen( PLU_name );   /*                                    */
      if ( ( name_len  < 1 ) || ( name_len  > 8 ) )
         return 1;                     /* We have an invalid name            */
   }                                   /*                                    */
   /**************************************************************************/
   /* Verify that we have 1 or 2 type - A strings.                           */
   /**************************************************************************/
   for ( cp = PLU_name; * cp; cp++ )
      if ( cp != period )
         if ( !isalnum( * cp ) )       /* Is this 'a'..'z','A'..'Z','0'..'9' */
            if ( ( * cp != '$' ) && ( * cp != '#' ) && ( * cp != '@' ) )
               return 1;               /* This char isn't valid type - A     */
   return 0;
}
