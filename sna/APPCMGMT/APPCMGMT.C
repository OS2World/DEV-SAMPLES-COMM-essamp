/****************************************************************************/
/*                                                                          */
/*   MODULE NAME: APPCMGMT.C                                                */
/*                                                                          */
/*   DESCRIPTIVE NAME: APPC Management verb C sample program for            */
/*                     IBM Communications Manager                           */
/*                                                                          */
/*   COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*                                                                          */
/*   FUNCTION = Deactivates the users of APPC and the resources being used. */
/*                                                                          */
/*                 APPCMGMT [ managment_key ]                               */
/*                                                                          */
/*              Uses the following APPC Management Verbs:                   */
/*                 CNOS                                                     */
/*                 DEACTIVATE_LOGICAL_LINK                                  */
/*                 DEACTIVATE_SESSION                                       */
/*                 DISPLAY                                                  */
/*                 STOP_AM                                                  */
/*                                                                          */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
/*                    (compiles with any memory model).                     */
/*                   IBM C Set/2 Compiler for 32 bit applications.          */
/*                                                                          */
/*   Changes:                                                               */
/*               Version 0.00 (rewrite of OS/2 1.2 EE version )             */
/*               Make the code compilable using small memory model          */
/*               Removed global variables                                   */
/*               Changed clear_vcb macro to more general CLEAR version      */
/*               Added pointer structure for release independent display    */
/*                 pointer retrieval                                        */
/*               Separated verb control block union, so each routine uses   */
/*                 its own local vcb                                        */
/*               Released the dynamically allocated OS/2 segment            */
/*               Output the APPC return codes in documented byte order      */
/*               Changed indentation to more accurately reflect code        */
/*                 association                                              */
/*               Updated the prologue to more accurately reflect program    */
/*                 content                                                  */
/*               Updated the code to more accurately define variable use    */
/*                 (i.e., use signed and unsigned values, rather than the   */
/*                 generic "int") via the type definitions from OS2DEFS     */
/*               Changes ACSMGT entry point to APPC                         */
/*               Supported 32 bit IBM C Set/2 Compiler                      */
/*                                                                          */
/****************************************************************************/
#define LINT_ARGS

#include <os2.h>                           /* OS/2 type definitions & calls */

#include <acsmgtc.h>                       /* APPC Management verbs         */

#include <stdio.h>                         /* For NULL & printf             */
#include <stdlib.h>                        /* For exit                      */
#include <string.h>                        /* For memset, strncpy, strupr   */

#ifdef ES32TO16
#define INCL_DOSMEMMGR
#include <bsememf.h>
#pragma stack16 (8192)
#define BUFF_SIZE  ULONG
#define FRPTR   *
#pragma seg16(PTRSTR)
#define TYPEC PVOID
#else
#define FRPTR   far *
#define BUFF_SIZE  UINT
#define TYPEC ULONG
#endif

/*--------------------------------------------------------------------------*/
/* Define global types and structures                                       */
/*--------------------------------------------------------------------------*/
typedef enum { False, True } BOOLEAN;

typedef enum { SNA_GLOBAL_INFO        ,
               LU62_INFO              ,
               AM_INFO                ,
               TP_INFO                ,
               SESS_INFO              ,
               LINK_INFO              ,
               LU_0_3_INFO            ,
               GW_INFO                ,
               X25_PHYSICAL_LINK_INFO
} DISPLAY_TYPE;

#define DISPLAY_BUFFER_SIZE ( (BUFF_SIZE) 65530 )

/*--------------------------------------------------------------------------*/
/* The following structure is used to dynamically address the appropriate   */
/* pointer returned in the Display buffer.                                  */
/*--------------------------------------------------------------------------*/
typedef STRUCT16 {
   SNA_GLOBAL_INFO_SECT        PTR16 sna_global_info_ptr;
   LU62_INFO_SECT              PTR16 lu62_info_ptr;
   AM_INFO_SECT                PTR16 am_info_ptr;
   TP_INFO_SECT                PTR16 tp_info_ptr;
   SESS_INFO_SECT              PTR16 sess_info_ptr;
   LINK_INFO_SECT              PTR16 link_info_ptr;
   LU_0_3_INFO_SECT            PTR16 lu_0_3_info_ptr;
   GW_INFO_SECT                PTR16 gw_info_ptr;
   X25_PHYSICAL_LINK_INFO_SECT PTR16 x25_physical_link_info_ptr;
} PTRSTR;


/*--------------------------------------------------------------------------*/
/* The following is used to set an area of memory to zeros                  */
/*--------------------------------------------------------------------------*/
#define CLEAR( area ) memset( &area, (int)'\0', sizeof( area ) )

/*--------------------------------------------------------------------------*/
/*                       Function Prototypes                                */
/*--------------------------------------------------------------------------*/
void cdecl main( const int, char *[] );

PVOID   alloc_display_buffer( const BUFF_SIZE );
void    release_display_buffer( const PVOID );
void    show_error( void );
void    print_rc( const char *, const USHORT, const ULONG );
void    fp_memcpy( PCH, PCH, USHORT );

BOOLEAN do_CNOS_to_0( const PCH, const PCH, const UCHAR [] );
BOOLEAN do_deact_link_soft( const PCH, const UCHAR [] );
BOOLEAN do_deact_sessions( const PCH, const PCH, const PCH, const UCHAR [] );
BOOLEAN stop_AM( const UCHAR [] );
BOOLEAN stop_new_sessions( const PVOID, const USHORT, const UCHAR [] );

void    bring_down_links( const PVOID, const USHORT, const UCHAR [] );
void    bring_down_sessions( const PVOID, const USHORT, const UCHAR [] );
PVOID   get_display_info( const DISPLAY_TYPE, const PVOID, const USHORT );
BOOL  test;
/****************************************************************************/
/*                                                                          */
/*                       Main Program Section                               */
/*                                                                          */
/****************************************************************************/

/*--------------------------------------------------------------------------*/
/*  The main procedure consists of the following steps:                     */
/*  1.  Retrieve the management key, if one exists, passed on command line. */
/*      This key prevents unauthorized usage of APPC management verbs.      */
/*  2.  Allocate a large buffer for the DISPLAY verb.                       */
/*      This buffer is re-used every time a DISPLAY is issued.              */
/*  3.  Stop the Attach Manager.                                            */
/*      This rejects all queued and incoming Allocation requests.           */
/*  4.  For each LU and partner LU, do a CNOS to 0.                         */
/*      This prevents any new sessions from starting.                       */
/*  5.  For each LU, partner LU, and mode, do a DEACTIVATE_SESSION.         */
/*      This terminates any active sessions.                                */
/*  6.  For each link, do a DEACTIVATE_LOGICAL_LINK.                        */
/*      This brings down the active SNA logical links.                      */
/*  7.  Release the buffer allocated in step 2.                             */
/*      Even though APPC continues to have access to this buffer, until     */
/*      process termination, it is good programming practice to release     */
/*      when we are finished with it.                                       */
/*--------------------------------------------------------------------------*/
/* Note: The management key, referenced in step 1 above, is the password    */
/*       protection mechanism provided by APPC and the Communications       */
/*       Manager.  If a management key (i.e., password) has been configured */
/*       for the system, then all trace, dump, and configuration requests   */
/*       must contain the key. Only the DISPLAY verb does not require the   */
/*       management key.                                                    */
/*--------------------------------------------------------------------------*/

void cdecl main ( const int argc,           /*                              */
                        char *argv[] )      /* argv[1] = optional           */
{                                           /*   management key             */
  USHORT       key_len;                     /* Length of user specified key */
  signed int pad_size;                      /* # of blanks at end of key    */
  UCHAR      input_key[ 8 ];                /* User specified APPC keyword  */
  PVOID      data_buffer_ptr;               /* OS/2 segment ptr for DISPLAY */
  BOOLEAN    sys_error;                     /* True if error is encountered */

  /*------------------------------------------------------------------------*/
  /* Decode the management key that was passed on the command line.  This   */
  /* is used to open the keylock for the APPC management verbs.             */
  /* Note: If the management key isn't present, then we use memset to set   */
  /* the storage to null because CLEAR() doesn't expect (support) an array. */
  /*------------------------------------------------------------------------*/

  if ( argc == 1 ) {
    memset( input_key, (int)'\0', sizeof( input_key ) ); /* Default key = 0 */
  }
  else {
    strupr( argv[ 1 ] );                        /* Convert key to uppercase */
    strncpy( input_key, argv[ 1 ], sizeof( input_key ) );
    key_len = strlen( argv[ 1 ] );              /* Copy it, and blank pad   */
    if ( 0 < ( pad_size = sizeof( input_key ) - key_len ) )
      memset( &input_key[ key_len ], (int)' ', pad_size );
  }

  /*------------------------------------------------------------------------*/
  /* Allocate an OS/2 segment (as required by the APPC DISPLAY verb).       */
  /*------------------------------------------------------------------------*/
  data_buffer_ptr = alloc_display_buffer( DISPLAY_BUFFER_SIZE );

  /*------------------------------------------------------------------------*/
  /* Stop the Attach Manager.  Thus rejecting queued attaches and any       */
  /* inbound attaches that might arrive during termination.                 */
  /*------------------------------------------------------------------------*/
  sys_error = stop_AM( input_key );

  if ( !sys_error )
    /*----------------------------------------------------------------------*/
    /* For each LU and partner LU, issue a CNOS to 0, with mode_name_select */
    /* set to ALL.  This will prevent any new sessions from starting.       */
    /*----------------------------------------------------------------------*/
    sys_error =
      stop_new_sessions( data_buffer_ptr, DISPLAY_BUFFER_SIZE, input_key );

  if ( !sys_error ) {
    /*----------------------------------------------------------------------*/
    /* For each LU, partner LU, and mode, issue a DEACTIVATE_SESSION verb   */
    /* to stop all existing sessions.                                       */
    /*----------------------------------------------------------------------*/
    bring_down_sessions( data_buffer_ptr, DISPLAY_BUFFER_SIZE, input_key );

    /*----------------------------------------------------------------------*/
    /* For each active link, issue a DEACTIVATE_LINK verb with TYPE(SOFT)   */
    /* to bring down the link when it's not in use.                         */
    /*----------------------------------------------------------------------*/
    bring_down_links( data_buffer_ptr, DISPLAY_BUFFER_SIZE, input_key );
  }

  /*------------------------------------------------------------------------*/
  /* Release the OS/2 segment (if it exists).                               */
  /*------------------------------------------------------------------------*/
  if ( NULL != data_buffer_ptr )
    release_display_buffer( data_buffer_ptr );

}

/****************************************************************************/
/*                                                                          */
/*                        UTILITY FUNCTIONS                                 */
/*                                                                          */
/****************************************************************************/

/*--------------------------------------------------------------------------*/
/* fp_memcpy is a memory model independent version of memcpy, because it    */
/* requires far pointers as passed parameters.                              */
/*--------------------------------------------------------------------------*/
void fp_memcpy( PCH  dest,                  /* Far pointer to destination   */
                PCH  src ,                  /* Far pointer to source        */
                USHORT cnt )                /* Number of bytes to be copied */
{
  for( ; cnt; cnt-- )                       /* Copy from source to target   */
    *dest++ = *src++;                       /* without any error checking   */
}                                           /* (e.g., overlaping memory).   */

/*--------------------------------------------------------------------------*/
/* This procedure simply indicates that an unexpected return code was       */
/* encountered.  The more extensive SHOW_ERR procedure that was illustrated */
/* in the APPC sample programs (FILECSVR and FILECREQ) could be substituted */
/* here, if, for example, logging of errors were desirable.                 */
/*--------------------------------------------------------------------------*/
void show_error()
{
  printf( "The preceding return code was unexpected.\n" );
}

/*--------------------------------------------------------------------------*/
/* This procedure is used to display a message, and the return code from an */
/* APPC verb control block.  Note that the return codes found in the vcb do */
/* not appear in Intel byte reversed order.  In order to display them as    */
/* they appear in the documentation, we use byte swapping macros (defined   */
/* in APPCDEFS.H).                                                          */
/*--------------------------------------------------------------------------*/
void print_rc( const char * message, const USHORT pri_rc, const ULONG sec_rc )
{
  printf( "%s %04X, %08lX\n", message,
                              SWAP2( pri_rc ),
                              SWAP4( sec_rc ) );
}

/*--------------------------------------------------------------------------*/
/* DISPLAY requires a data buffer in a shared unnamed segment.  The         */
/* following function allocates that segment and returns a pointer to it.   */
/*--------------------------------------------------------------------------*/
PVOID alloc_display_buffer( const BUFF_SIZE display_buffer_size )
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
  USHORT selector;                          /* Selector from DosAllocSeg    */
  PCH    result;                            /* Address of allocated buffer  */
  USHORT dos_rc;                            /* Return code from Allocate    */

  if ( ! ( dos_rc = DosAllocSeg( display_buffer_size,
                                 (PSEL) &selector,
                                 1) ) ) {
    OFFSETOF( result ) = 0;                 /* set the offset to zero       */
    SELECTOROF( result ) = selector;        /* address = Selector:0         */
  } else {
    printf( "Error allocating shared buffer.  OS/2 return code = %d\n",
             dos_rc );
    exit( dos_rc );
  }
   /*--------------------------------------------*/
   /* End of 16 bit memory management            */
   /*--------------------------------------------*/
#endif
  return result;                            /* Return the buffer address    */
}

/*--------------------------------------------------------------------------*/
/* DISPLAY requires a data buffer in a shared unnamed segment.              */
/* This procedure informs OS/2 that this program is finished with it.       */
/*                                                                          */
/* Note: APPC still has access to it until this process ends.  For details, */
/*       see the section titled "Data Buffers", in chapter 6, "Writing the  */
/*       TP and Matching Configuration Values" of the "APPC Programming     */
/*       Reference."                                                        */
/*--------------------------------------------------------------------------*/
void release_display_buffer( const PVOID display_buffer_ptr )
{
#ifdef ES32TO16
   /*--------------------------------------------*/
   /* 32 bit memory management                   */
   /*--------------------------------------------*/
  ULONG  dos_rc;                            /* Return code from DosFreeSeg  */

  if ( 0 != ( dos_rc = DosFreeMem( display_buffer_ptr ) ) ) {
    printf( "Error releasing DISPLAY buffer.\n"
            "OS/2 return code = %d\n", dos_rc );
  }
   /*--------------------------------------------*/
   /* End of 32 bit memory management            */
   /*--------------------------------------------*/
#else
   /*--------------------------------------------*/
   /* 16 bit memory management                   */
   /*--------------------------------------------*/
  USHORT selector;                          /* Selector from DosAllocSeg    */
  USHORT dos_rc;                            /* Return code from DosFreeSeg  */

  selector = SELECTOROF( display_buffer_ptr );
  if ( 0 != ( dos_rc = DosFreeSeg( selector ) ) ) {
    printf( "Error releasing DISPLAY buffer.\n"
            "OS/2 return code = %d\n", dos_rc );
  }
   /*--------------------------------------------*/
   /* End of 16 bit memory management            */
   /*--------------------------------------------*/
#endif
}

/****************************************************************************/
/*                                                                          */
/*                        APPC RELATED FUNCTIONS                            */
/*                                                                          */
/****************************************************************************/

/*--------------------------------------------------------------------------*/
/* This function issues a STOP_AM verb to stop the APPC Attach Manager.     */
/* This routine returns True if an error is encountered, False otherwise.   */
/*--------------------------------------------------------------------------*/
BOOLEAN stop_AM( const UCHAR input_key[] )  /* Stop the Attach Manager      */
{
  static  STOP_AM vcb;                      /* STOP_AM verb control block   */
  PCH     vcbptr = ( PCH ) &vcb;            /* Pointer to vcb               */
  BOOLEAN error_found = False;              /* Function result              */
                                            /*                              */
  CLEAR( vcb );                             /* Fill vcb with '\0'           */
  vcb.opcode = AP_STOP_AM;                  /* Verb opcode = STOP_AM        */
  memcpy( vcb.key, input_key,               /* Copy in the user specified   */
          sizeof( vcb.key ) );              /*   managment key              */
                                            /*                              */
  APPC((TYPEC) vcbptr   );                  /* Issue the STOP_AM verb       */
                                            /*                              */
  print_rc( "STOP_AM:", vcb.primary_rc, vcb.secondary_rc );
                                            /*                              */
  /*------------------------------------------------------------------------*/
  /* Check the return codes and report appropriate message.  Some of the    */
  /* more common errors (e.g., INVALID_KEY) will be caught here, so they    */
  /* are not checked on subsequent verbs.                                   */
  /*------------------------------------------------------------------------*/
  if ( vcb.primary_rc == AP_OK )
    printf( "Attach Manager stopped.\n" );
  else
    if ( ( vcb.primary_rc   == AP_STATE_CHECK ) &&
         ( vcb.secondary_rc == AP_ATTACH_MGR_ALREADY_INACTIVE ) )
      printf( "Attach Manager was not active.\n" );
    else {
      error_found = True;
      if ( vcb.primary_rc == AP_INVALID_KEY )
        printf( "The Key supplied on the command line was invalid.\n" );
      else
        if ( vcb.primary_rc == AP_COMM_SUBSYSTEM_NOT_LOADED )
          printf( "APPC is not started, or the Communications Manager "
                  "is not active.\n" );
        else
          show_error();
    }
  return error_found;
}

/*--------------------------------------------------------------------------*/
/* This fuction issues a Change Number of Sessions (CNOS) to 0, with mode   */
/* name select set to ALL.  This prevents any new sessions from starting.   */
/* This routine returns True if an error is encountered, False otherwise.   */
/*--------------------------------------------------------------------------*/
BOOLEAN do_CNOS_to_0( const PCH   lu_alias    ,   /* Pointer to LU_alias    */
                      const PCH   plu_alias   ,   /* Pointer to PLU_alias   */
                      const UCHAR input_key[] )   /* Management Key         */
{                                           /*                              */
  static  CNOS vcb;                         /* verb control block           */
  PCH     vcbptr = ( PCH ) &vcb;            /* Pointer to vcb               */
  BOOLEAN error_found = False;              /* Function result              */
  char    LU_alias[ 9 ];                    /* Automatic copies of lu_alias */
  char    PLU_alias[ 9 ];                   /*   and plu_alias              */
  char    CNOS_message[ 64 ];               /*                              */
                                            /*                              */
  CLEAR( vcb );                             /* Initialize it to zero.       */
  vcb.opcode = AP_CNOS;                     /* Verb: CNOS                   */
  memcpy( vcb.key, input_key,               /* Copy in the user specified   */
          sizeof( vcb.key ) );              /*   management key             */
                                            /*                              */
  fp_memcpy( vcb.lu_alias ,                 /* Set the LU_alias from the    */
             lu_alias     ,                 /* passed pointer.              */
             sizeof( vcb.lu_alias ) );      /*                              */
                                            /*                              */
  fp_memcpy( LU_alias ,                     /* Copy the lu_alias parm into  */
             lu_alias ,                     /* an automatic variable for    */
             sizeof( LU_alias ) );          /* display via printf.          */
  LU_alias[ 8 ] = '\0';                     /*                              */
                                            /*                              */
  fp_memcpy( vcb.plu_alias ,                /* Set the partner_LU_alias     */
             plu_alias     ,                /* from the passed pointer.     */
             sizeof( vcb.plu_alias ) );     /*                              */
                                            /*                              */
  fp_memcpy( PLU_alias ,                    /* Copy the plu_alias parm into */
             plu_alias ,                    /* an automatic variable for    */
             sizeof( PLU_alias ) );         /* display via printf.          */
  PLU_alias[ 8 ] = '\0';                    /*                              */
                                            /*                              */
  /*------------------------------------------------------------------------*/
  /* Since mode_name_select = ALL, the mode_name field is OK as all zeroes. */
  /*------------------------------------------------------------------------*/
                                            /*                              */
  vcb.set_negotiable        = AP_YES;       /* set_negotiable(YES)          */
  vcb.mode_name_select      = AP_ALL;       /* mode_name_select(ALL)        */
  vcb.plu_mode_sess_lim     = 0;            /* session_limit(0)             */
  vcb.min_conwinners_source = 0;            /* min_conwinners_source(0)     */
  vcb.min_conwinners_target = 0;            /* min_conwinners_target(0)     */
  vcb.responsible           = AP_SOURCE;    /* responsible(SOURCE)          */
  vcb.drain_source          = AP_YES;       /* drain_source(YES)            */
  vcb.drain_target          = AP_YES;       /* drain_target(YES)            */
                                            /*                              */
  APPC((TYPEC)  vcbptr );                   /* Issue the CNOS verb.         */

  sprintf( CNOS_message, "CNOS: LU=%s PLU=%s  ",
           LU_alias, PLU_alias );
  print_rc( CNOS_message, vcb.primary_rc, vcb.secondary_rc );

  /*------------------------------------------------------------------------*/
  /* Check the return codes and report appropriate message.                 */
  /*------------------------------------------------------------------------*/
  if ( vcb.primary_rc == AP_OK)
    printf( "CNOS completed successfully.\n" );
  else {
    error_found = True;
    if ( ( vcb.primary_rc == AP_PARAMETER_CHECK ) &&
         ( ( vcb.secondary_rc == AP_BAD_LU_ALIAS ) ||
           ( vcb.secondary_rc == AP_BAD_PARTNER_LU_ALIAS ) ) )
      printf( "LU alias or partner LU alias was invalid.\n" );
    else if ( vcb.primary_rc == AP_CNOS_LOCAL_RACE_REJECT )
      printf( "CNOS rejected - previous CNOS still outstanding.\n" );
    else if ( vcb.primary_rc == AP_CNOS_PARTNER_LU_REJECT )
      printf( "CNOS rejected by partner.\n" );
    else if ( ( vcb.primary_rc == AP_ALLOCATION_ERROR ) &&
              ( vcb.secondary_rc == AP_ALLOCATION_FAILURE_NO_RETRY ) ) {
      printf( "CNOS rejected - Allocation Failure No retry.  " );
      printf( "See the error log for details.\n" );
    }
    else
      show_error();
  }
  return error_found;
}

/*--------------------------------------------------------------------------*/
/* This function issues a DEACTIVATE_SESSION for all sessions using the     */
/* specified LU_alias, partner_LU_alias, and mode_name.                     */
/* This routine returns True if an error is encountered, False otherwise.   */
/*--------------------------------------------------------------------------*/
BOOLEAN do_deact_sessions( const PCH   lu_alias    ,  /* Ptr to LU_alias    */
                           const PCH   plu_alias   ,  /* Ptr to PLU_alias   */
                           const PCH   mode_name   ,  /* Ptr to Mode Name   */
                           const UCHAR input_key[] )  /* Management Key     */
{                                           /*                              */
  static  DEACTIVATE_SESSION vcb;           /* verb control block           */
  PCH     vcbptr = ( PCH ) &vcb;            /* Pointer to vcb               */
  BOOLEAN error_found = False;              /* Function result              */
                                            /*                              */
  CLEAR( vcb );                             /* Zero the verb control block. */
  vcb.opcode = AP_DEACTIVATE_SESSION;       /* Verb: DEACTIVATE_SESSION     */
  memcpy( vcb.key, input_key,               /* Copy in the user specified   */
          sizeof( vcb.key ) );              /*   management key             */
                                            /*                              */
  fp_memcpy( vcb.lu_alias,                  /* Set the LU_alias from the    */
             lu_alias,                      /* passed pointer.              */
             sizeof( vcb.lu_alias ) );      /*                              */
                                            /*                              */
  fp_memcpy( vcb.plu_alias,                 /* Set the partner_LU_alias     */
             plu_alias,                     /* from the passed pointer.     */
             sizeof( vcb.plu_alias ) );     /*                              */
                                            /*                              */
  fp_memcpy( vcb.mode_name,                 /* Set the mode_name            */
             mode_name,                     /* from the passed pointer.     */
             sizeof( vcb.mode_name ) );     /*                              */
                                            /*                              */
  /*------------------------------------------------------------------------*/
  /* Note: session_id = 0 (from CLEAR( vcb ) above)                         */
  /*------------------------------------------------------------------------*/
                                            /*                              */
  APPC((TYPEC)  vcbptr );                   /* Do DEACTIVATE_SESSION        */

  print_rc( "DEACTIVATE_SESSION:", vcb.primary_rc, vcb.secondary_rc );

  /*------------------------------------------------------------------------*/
  /* Check the return codes and report appropriate message.                 */
  /* Note: Your program will get OK even if no sessions are active.         */
  /*------------------------------------------------------------------------*/
  if ( vcb.primary_rc == AP_OK )
    printf( "All active sessions were successfully deactivated.\n" );
  else {
    error_found = True;
    if ( ( vcb.primary_rc == AP_PARAMETER_CHECK ) &&
         ( ( vcb.secondary_rc == AP_BAD_LU_ALIAS )         ||
           ( vcb.secondary_rc == AP_BAD_PARTNER_LU_ALIAS ) ||
           ( vcb.secondary_rc == AP_BAD_MODE_NAME ) ) )
      printf( "LU alias, partner LU alias, or mode name was invalid.\n" );
    else
      show_error();
  }
  return error_found;
}

/*--------------------------------------------------------------------------*/
/* This procedures issues a DEACTIVATE_LINK TYPE(SOFT) for a specified link.*/
/* This routine returns True if an error is encountered, False otherwise.   */
/*--------------------------------------------------------------------------*/
BOOLEAN do_deact_link_soft( const PCH   link_id,      /* Pointer to link_id */
                            const UCHAR input_key[] ) /* Management Key     */
{                                           /*                              */
  static  DEACTIVATE_LOGICAL_LINK vcb;      /* verb control block           */
  PCH     vcbptr = ( PCH ) &vcb;            /* Pointer to vcb               */
  BOOLEAN error_found = False;              /* Function result              */
                                            /*                              */
  CLEAR( vcb );                             /* Zero the verb control block. */
  vcb.opcode = AP_DEACTIVATE_LOGICAL_LINK;  /* Set the opcode               */
  memcpy( vcb.key, input_key,               /* Copy in the user specified   */
          sizeof( vcb.key ) );              /*   management key             */
                                            /*                              */
  fp_memcpy( vcb.link_id,                   /* Set the link_id from the     */
             link_id,                       /* passed pointer.              */
             sizeof( vcb.link_id ) );       /*                              */
  vcb.type = AP_LINK_SOFT;                  /* Set type(SOFT).              */
                                            /*                              */
  APPC((TYPEC)  vcbptr );                   /* Do DEACTIVATE_LOGICAL_LINK   */

  print_rc( "DEACTIVATE_LOGICAL_LINK:", vcb.primary_rc, vcb.secondary_rc );

  /*------------------------------------------------------------------------*/
  /* Check the return codes and report appropriate message.                 */
  /*------------------------------------------------------------------------*/
  if ( vcb.primary_rc == AP_OK )
    printf( "Link deactivated, type(SOFT)\n" );
  else {
    if ( ( vcb.primary_rc == AP_PARAMETER_CHECK ) &&
         ( vcb.secondary_rc == AP_INVALID_LINK_ID ) )
      printf( "Link was already deactivated.\n" );
    else {
      if ( ( vcb.primary_rc == AP_STATE_CHECK ) &&
           ( vcb.secondary_rc == AP_LINK_DEACT_IN_PROGRESS ) )
        printf( "Link deactivation was already in progress.\n" );
      else {
        show_error();
        error_found = True;
      }
    }
  }
  return error_found;
}

/*--------------------------------------------------------------------------*/
/* This procedure returns a pointer to one type of DISPLAY data.  DISPLAY   */
/* can return zero or more types of information on each request.  For       */
/* simplicity, this program only illustrates returning one type per request.*/
/*--------------------------------------------------------------------------*/
PVOID get_display_info( const DISPLAY_TYPE display_type ,
                        const PVOID        data_buffer_ptr,
                        const USHORT         buffer_size  )
{
  static   DISPLAY vcb;                     /* verb control block           */
  PCH      vcbptr = ( PCH ) &vcb;           /* Pointer to vcb               */
  ULONG    temp;                            /* Necessary for ptr arithmatic */
  PTRSTR   FRPTR display_ptr;               /* Display Pointer Structure    */
  CLEAR( vcb );                             /* Zero the verb control block. */
  vcb.opcode        = AP_DISPLAY;           /* Verb: DISPLAY                */
  vcb.init_sect_len = 44L;                  /* Version 1.2 constant value   */
  vcb.buffer_len    = (ULONG) buffer_size;  /* Data buffer size             */
  vcb.buffer_ptr    = data_buffer_ptr;      /* Set data buffer pointer      */
  vcb.num_sections  =  9L;                  /* Version 1.2 constant value   */

  switch ( display_type ) {                 /* What kind of data is needed? */
    case SNA_GLOBAL_INFO :
      vcb.sna_global_info = AP_YES;
      break;
    case LU62_INFO :
      vcb.lu62_info = AP_YES;
      break;
    case AM_INFO :
      vcb.am_info = AP_YES;
      break;
    case TP_INFO :
      vcb.tp_info = AP_YES;
      break;
    case SESS_INFO :
      vcb.sess_info = AP_YES;
      break;
    case LINK_INFO :
      vcb.link_info = AP_YES;
      break;
    case LU_0_3_INFO :
      vcb.lu_0_3_info = AP_YES;
      break;
    case GW_INFO :
      vcb.gw_info = AP_YES;
      break;
    case X25_PHYSICAL_LINK_INFO :
      vcb.x25_physical_link_info = AP_YES;
      break;
  }

  APPC((TYPEC)  vcbptr );                   /* Issue the DISPLAY verb.      */

  /*------------------------------------------------------------------------*/
  /*   This program may eventually be executed against newer version of     */
  /* code that have an expanded Display structure.  To remain compatible,   */
  /* the hard coded version of the Display structure must be made more      */
  /* dynamic.  Thus, the portion of the structure associated with the       */
  /* pointers to the data may not be where ACSMGTC.H has them hard coded.   */
  /* So, we need to determine the actual address of this portion of the     */
  /* table as follows.                                                      */
  /*------------------------------------------------------------------------*/

  display_ptr = (PTRSTR *) &vcb;

  /*------------------------------------------------------------------------*/
  /*   Unfortunately, the version 6.0 of the Microsoft C compiler has a bug */
  /* associated with pointer arithmetic requiring a temporary variable.     */
  /*------------------------------------------------------------------------*/
  temp = (ULONG) display_ptr + vcb.init_sect_len;
  display_ptr = (PTRSTR *) temp;

  print_rc( "DISPLAY:", vcb.primary_rc, vcb.secondary_rc );

  if ( ( vcb.primary_rc == AP_STATE_CHECK ) &&
       ( vcb.secondary_rc == AP_DISPLAY_INFO_EXCEEDS_LEN ) ) {
    printf( "  The results of the DISPLAY request exceed the available " );
    printf( "storage.  This\nprogram will process the returned data.  No " );
    printf( "attempt is made to handle\nthe information that was unable " );
    printf( "to be returned.\n\n" );
  }

  if ( ( vcb.primary_rc == AP_OK ) ||
     ( ( vcb.primary_rc == AP_STATE_CHECK ) &&
       ( vcb.secondary_rc == AP_DISPLAY_INFO_EXCEEDS_LEN ) ) ) {
    switch ( display_type ) {               /* Return the appropriate ptr   */
      case SNA_GLOBAL_INFO   :
        return  (PVOID) display_ptr -> sna_global_info_ptr;
      case LU62_INFO         :
        return  (PVOID) display_ptr -> lu62_info_ptr;
      case AM_INFO           :
        return  (PVOID)  display_ptr -> am_info_ptr;
      case TP_INFO           :
        return  (PVOID)  display_ptr -> tp_info_ptr;
      case SESS_INFO         :
        return  (PVOID)  display_ptr -> sess_info_ptr;
      case LINK_INFO         :
        return  (PVOID)  display_ptr -> link_info_ptr;
      case LU_0_3_INFO       :
        return  (PVOID)  display_ptr -> lu_0_3_info_ptr;
      case GW_INFO           :
        return  (PVOID)  display_ptr -> gw_info_ptr;
      case X25_PHYSICAL_LINK_INFO:
        return  (PVOID)  display_ptr -> x25_physical_link_info_ptr;
    }
  }
  else {
    show_error();
    return NULL;
  }
}


/*--------------------------------------------------------------------------*/
/* For each LU and partner LU, do a CNOS to 0, thus preventing new sessions.*/
/* This routine returns True if an error is encountered, False otherwise.   */
/*--------------------------------------------------------------------------*/
BOOLEAN stop_new_sessions( const PVOID data_buffer_ptr,
                           const USHORT  buffer_size    ,
                           const UCHAR input_key[]    )
{
  LU62_INFO_SECT FRPTR lu62_buffer;         /* Pointer to DISPLAYed data    */
  LU62_OVERLAY   FRPTR lu_ptr;              /* Pointer to current LU        */
  PLU62_OVERLAY  FRPTR plu_ptr=NULL;        /* Pointer to cuurent partner   */

  USHORT  lu_count;                         /* Counter for LUs              */
  USHORT  plu_count;                        /* Counter for partner LUs      */
  ULONG   temp_length;                      /* Used to calculate pointers   */
  BOOLEAN error_found = False;              /* True => error encountered    */

  lu62_buffer = (LU62_INFO_SECT FRPTR )
                 get_display_info( LU62_INFO, data_buffer_ptr, buffer_size );
  if ( !lu62_buffer )                       /* If no buffer was returned ...*/
    printf( "DISPLAY found no LUs.\n" );
  else {
    if ( 0 != ( lu_count = lu62_buffer -> num_lu62s ) ) { /* Save LU count. */
      /*--------------------------------------------------------------------*/
      /* Point to the first LU in the display buffer.                       */
      /*--------------------------------------------------------------------*/
      temp_length = (ULONG) lu62_buffer + lu62_buffer -> lu62_init_sect_len;
      lu_ptr = (LU62_OVERLAY FRPTR ) temp_length;

      for ( ; lu_count; lu_count-- ) {      /* Now, for each LU ...         */
        if ( 0 != ( plu_count = lu_ptr -> num_plus ) ) { /* Save PLU count. */
          /*----------------------------------------------------------------*/
          /* Point to the first partner LU.                                 */
          /*----------------------------------------------------------------*/
          temp_length = (ULONG) lu_ptr + lu_ptr -> lu62_overlay_len +
                        (ULONG) sizeof( lu_ptr -> lu62_entry_len );
          plu_ptr = (PLU62_OVERLAY FRPTR ) temp_length;
        }

        for ( ; plu_count; plu_count-- ) {    /* Now, for each partner LU...*/
          error_found |=                      /* Keep track of any errors   */
            do_CNOS_to_0( lu_ptr  -> lu_alias,/* Issue the CNOS verb        */
                          plu_ptr -> plu_alias,
                          input_key           );
          /*----------------------------------------------------------------*/
          /* And, point to the next partner LU.                             */
          /*----------------------------------------------------------------*/
          temp_length = (ULONG) plu_ptr + plu_ptr -> plu62_entry_len;
          plu_ptr = (PLU62_OVERLAY FRPTR ) temp_length;
        }
        /*------------------------------------------------------------------*/
        /* Now, point to the next LU.                                       */
        /*------------------------------------------------------------------*/
        temp_length = (ULONG) lu_ptr + lu_ptr -> lu62_entry_len;
        lu_ptr = (LU62_OVERLAY FRPTR ) temp_length;
      }
    }
  }
  return error_found;
}

/*--------------------------------------------------------------------------*/
/* For each valid combination of LU, partner LU, and mode, issue a          */
/* DEACTIVATE_SESSION verb to bring down all active sessions.               */
/*--------------------------------------------------------------------------*/
void bring_down_sessions( const PVOID data_buffer_ptr,
                          const USHORT  buffer_size    ,
                          const UCHAR input_key[]    )
{
  LU62_INFO_SECT FRPTR lu62_buffer;         /* Pointer to DISPLAYed data    */
  LU62_OVERLAY   FRPTR lu_ptr;              /* Pointer to current LU        */
  PLU62_OVERLAY  FRPTR plu_ptr=NULL;        /* Pointer to current partner   */
  MODE_OVERLAY   FRPTR mode_ptr=NULL;       /* Pointer to current mode      */

  USHORT lu_count;                          /* Counter for LUs              */
  USHORT plu_count;                         /* Counter for partner LUs      */
  USHORT mode_count;                        /* Counter for modes            */
  ULONG  temp_length;                       /* Used to calculate addresses  */

  /*------------------------------------------------------------------------*/
  /* Request LU Type 6.2 information again.  This is included here to show  */
  /* how to write this procedure in a modular way.  Since this information  */
  /* was presumably already determined in the procedure stop_new_sessions(),*/
  /* it may be reasonable to perform this information extraction in only    */
  /* one place.                                                             */
  /*------------------------------------------------------------------------*/
  lu62_buffer = (LU62_INFO_SECT FRPTR )
                 get_display_info( LU62_INFO, data_buffer_ptr, buffer_size );

  if ( !lu62_buffer )                       /* If no buffer was returned ...*/
    printf( "DISPLAY found no LUs.\n" );
  else {
    if ( 0 != ( lu_count = lu62_buffer -> num_lu62s ) ) { /* Save LU count. */
      /*--------------------------------------------------------------------*/
      /* Point to the first LU.                                             */
      /*--------------------------------------------------------------------*/
      temp_length = (ULONG) lu62_buffer + lu62_buffer -> lu62_init_sect_len;
      lu_ptr = (LU62_OVERLAY FRPTR ) temp_length;

      for ( ; lu_count; lu_count-- ) {          /* For each LU ...          */
        if ( 0 != ( plu_count = lu_ptr->num_plus ) ) { /* Save PLU count.   */
          /*----------------------------------------------------------------*/
          /* Point to the first partner LU.                                 */
          /*----------------------------------------------------------------*/
          temp_length = (ULONG) lu_ptr + lu_ptr -> lu62_overlay_len +
                        (ULONG) sizeof( lu_ptr -> lu62_entry_len );
          plu_ptr = (PLU62_OVERLAY FRPTR ) temp_length;
        }

        for ( ; plu_count; plu_count-- ) {          /* For each partner LU..*/
          if ( 0 != ( mode_count = plu_ptr -> num_modes ) ) {
            /*--------------------------------------------------------------*/
            /* Point to the first mode name.                                */
            /*--------------------------------------------------------------*/
            temp_length = (ULONG) plu_ptr + plu_ptr -> plu62_overlay_len +
                          (ULONG) sizeof( plu_ptr -> plu62_entry_len );
            mode_ptr = (MODE_OVERLAY FRPTR ) temp_length;
          }
          for ( ; mode_count; mode_count-- ) {      /* For each mode ...    */
            do_deact_sessions( lu_ptr   -> lu_alias,/* Issue the verb       */
                               plu_ptr  -> plu_alias,
                               mode_ptr -> mode_name,
                               input_key            );
            /*--------------------------------------------------------------*/
            /* And, set up the pointer to the next mode.                    */
            /*--------------------------------------------------------------*/
            temp_length = (ULONG) mode_ptr + mode_ptr -> mode_entry_len;
            mode_ptr = (MODE_OVERLAY FRPTR ) temp_length;
          }
          /*----------------------------------------------------------------*/
          /* And, now we set up the pointer to the next Partner LU          */
          /*----------------------------------------------------------------*/
          temp_length = (ULONG) plu_ptr + plu_ptr -> plu62_entry_len;
          plu_ptr = (PLU62_OVERLAY FRPTR ) temp_length;
        }
        /*------------------------------------------------------------------*/
        /* And, now we set up the pointer to the next LU.                   */
        /*------------------------------------------------------------------*/
        temp_length = (ULONG) lu_ptr + lu_ptr -> lu62_entry_len;
        lu_ptr = (LU62_OVERLAY FRPTR ) temp_length;
      }
    }
  }
}

/*--------------------------------------------------------------------------*/
/* For each link, do a DEACTIVATE_LOGICAL_LINK.  This brings down the       */
/* active SNA logical links.                                                */
/*--------------------------------------------------------------------------*/
void bring_down_links( const PVOID data_buffer_ptr,
                       const USHORT  buffer_size    ,
                       const UCHAR input_key[]    )
{
  LINK_INFO_SECT FRPTR link_buffer;         /* Pointer to DISPLAYed data    */
  LINK_OVERLAY   FRPTR link_ptr;

  USHORT link_count;                        /* Counter for links            */
  ULONG  temp_length;                       /* Used to calculate pointers   */

  link_buffer = (LINK_INFO_SECT FRPTR )
                 get_display_info( LINK_INFO, data_buffer_ptr, buffer_size );
  if ( !link_buffer ) {                     /* If no buffer was returned ...*/
    /*----------------------------------------------------------------------*/
    /* The link_count is zero; there are no active logical links.  This     */
    /* could occur because the DEACTIVATE_SESSIONs were successful, and the */
    /* DLC(s) had been configured as "Free Unused Link = Yes" - the links   */
    /* have already been taken down since they were not being used.         */
    /*----------------------------------------------------------------------*/
    printf( "DISPLAY found no active links.\n" );
  } else {
    if ( 0 != ( link_count = link_buffer -> num_links ) ) { /* Save # links */
      /*--------------------------------------------------------------------*/
      /* Point to the first link.                                           */
      /*--------------------------------------------------------------------*/
      temp_length = (ULONG) link_buffer + link_buffer -> link_init_sect_len;
      link_ptr = (LINK_OVERLAY FRPTR ) temp_length;

      for ( ; link_count; link_count-- ) {          /* For each link ...    */
        do_deact_link_soft( link_ptr -> link_id,    /* Issue the verb.      */
                            input_key );            /*                      */
        /*------------------------------------------------------------------*/
        /* And, point to the next link.                                     */
        /*------------------------------------------------------------------*/
        temp_length = (ULONG) link_ptr + link_ptr -> link_entry_len;
        link_ptr = (LINK_OVERLAY FRPTR ) temp_length;
      }
    }
  }
}

/* EOF - APPCMGMT.C */
