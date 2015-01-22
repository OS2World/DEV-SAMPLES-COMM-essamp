/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : EXECDISP.C                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to execute the DISPLAY and DISPLAY_APPN verbs.      */
/*                                                                          */
/*              Uses the following APPC verbs:                              */
/*                                                                          */
/*                 DISPLAY                                                  */
/*                 DISPLAY_APPN                                             */
/*                                                                          */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
/*              (compiles with large memory model)                          */
/*                                                                          */
/*  ASSOCIATED FILES:  See DISPLAY.MAK, PMDSPLAY.MAK                        */
/*                                                                          */
/****************************************************************************/
#define _MT
#include <STDDEF.H>    /* Standard C library definitions and declarations*/
#include <STRING.H>    /* Standard C library string and memory functions */
 
#include <ACSMGTC.H>      /* DISPLAY verb definitions and declarations      */
 
void pascal _saveregs call_ACSMGT (void far * display_vcb);
                          /* Local routine to call ACSMGT                   */
 
/* Indicator of type of DISPLAY information, in the same order as the info  */
/* flags and pointers in the DISPLAY and DISPLAY_APPN verbs.                */
 
typedef enum {
   DISPLAY_INFO_NONE = -1  ,
   DISPLAY_INFO_GLOBAL     ,
   DISPLAY_INFO_LU62       ,
   DISPLAY_INFO_AM         ,
   DISPLAY_INFO_TP         ,
   DISPLAY_INFO_SESSIONS   ,
   DISPLAY_INFO_LINKS      ,
   DISPLAY_INFO_LU03       ,
   DISPLAY_INFO_GW         ,
   DISPLAY_INFO_X25        ,
   DISPLAY_INFO_SYSDEF     ,
   DISPLAY_INFO_ADAPTER    ,
   DISPLAY_INFO_LUDEF      ,
   DISPLAY_INFO_PLUDEF     ,
   DISPLAY_INFO_MODES      ,
   DISPLAY_INFO_LINKDEF    ,
   DISPLAY_INFO_MS         ,
   DISPLAY_INFO_NODE       ,
   DISPLAY_INFO_DIR        ,
   DISPLAY_INFO_TOP        ,
   DISPLAY_INFO_ISR        ,
   DISPLAY_INFO_COS        ,
   DISPLAY_INFO_CN         ,
   DISPLAY_INFO_LAST
} DISPLAY_INFO_TYPE;
 
/*--------------------------------------------------------------------------*/
/*                           Global Variables                               */
/*--------------------------------------------------------------------------*/
/* Arrays of values for the init_sect_len and num_sections fields in the    */
/* DISPLAY and DISPLAY_APPN verbs for the old version of APPC (OS/2 EE 1.2) */
/* and for the new version.  Note that the old version does not support     */
/* DISPLAY_APPN.                                                            */
 
static ULONG init_sect_len[]      = {44L, 50L};   /* For DISPLAY */
static ULONG num_sections[]       = { 9L, 16L};
static ULONG appn_init_sect_len[] = { 0L, 40L};   /* For DISPLAY_APPN */
static ULONG appn_num_sections[]  = { 0L,  6L};
 
/* APPC version indicator used as an index for the above arrays.  Initial   */
/* assumption is that I'm running on the latest version.                    */
 
static enum {
   DISP_VERSION_APPC = 0,
   DISP_VERSION_APPN
} disp_version = DISP_VERSION_APPN;
 
/****************************************************************************/
/* exec_display:  Executes a DISPLAY or DISPLAY_APPN verb to get the        */
/*                requested information.  Although DISPLAY can return more  */
/*                than one type of information per call, this function      */
/*                requests only one type of information per call.           */
/****************************************************************************/
void far * exec_display (DISPLAY_INFO_TYPE info_type,
                         UCHAR far * info_buffer_ptr,
                         UINT info_buffer_size,
                         USHORT * primary_rc, ULONG * secondary_rc,
                         ULONG * display_length)
{
   union {
      DISPLAY      display;            /* Management verb DISPLAY      */
      DISPLAY_APPN display_appn;       /* Management verb DISPLAY_APPN */
   } vcb;
   union {                             /* Working pointer to . .            */
      UCHAR far *      info_flags;     /* . . the info flags . .            */
      void far * far * info_ptrs;      /* . . and the info_sect pointers . .*/
      void far *       info;           /* . . and the info_sect . .         */
   } ptr;                              /* . . in DISPLAY and DISPLAY_APPN   */
   int retry;                          /* Retry DISPLAY flag                */
 
/* Keep retrying the DISPLAY verb for incrementally earlier versions of     */
/* APPC as long as the DISPLAY verb keeps failing because I've built it for */
/* a later version of APPC than the one I'm running on.                     */
 
   for (retry = 1; retry; ) {
      retry = 0;
 
   /* If the requested DISPLAY info type is invalid, or if I know from      */
   /* earlier failures that it is not supported by the version of APPC on   */
   /* which I'm running, return a NULL pointer to the caller.               */
 
      if ((info_type <= DISPLAY_INFO_NONE) ||
          (info_type >= DISPLAY_INFO_LAST) ||
          ((info_type < DISPLAY_INFO_NODE) &&
           (info_type >= (DISPLAY_INFO_TYPE)num_sections[disp_version])) ||
          ((info_type >= DISPLAY_INFO_NODE) &&
           (info_type >= DISPLAY_INFO_NODE + (DISPLAY_INFO_TYPE)appn_num_sections[disp_version]))) {
         ptr.info = (void far *)NULL;
      } else {
 
      /* The requested DISPLAY info type is valid, and I do not know that   */
      /* it is not supported by the version of APPC on which I'm running.   */
      /* Build the DISPLAY or DISPLAY_APPN verb.  Note that:                */
      /* - I set init_sect_len and num_sections to the values for the most  */
      /*   recent version of APPC that I haven't tried and failed, even for */
      /*   the original DISPLAY info types; the new version of APPC returns */
      /*   more info for the original info types, but only if I specify the */
      /*   new init_sect_len and num_sections values.                       */
      /* - I use info_type as an index to set the appropriate info flag now,*/
      /*   and to get the info pointer returned by APPC when the verb       */
      /*   completes later; this avoids a couple huge switch statements.    */
      /*   Also note that the init_sect_len value determines where the info */
      /*   pointers begin.                                                  */
 
         CLEAR_VCB(vcb);                      /* Zero the verb control block*/
         if (info_type < DISPLAY_INFO_NODE) { /* Build DISPLAY verb         */
            vcb.display.init_sect_len = init_sect_len[disp_version];
            vcb.display.num_sections  = num_sections[disp_version];
            vcb.display.opcode        = AP_DISPLAY;
            vcb.display.buffer_len    = (ULONG)info_buffer_size;
            vcb.display.buffer_ptr    = info_buffer_ptr;
            ptr.info_flags = (UCHAR far *)&vcb.display.sna_global_info;
            ptr.info_flags[info_type] = AP_YES;
            ptr.info_ptrs = (void far * far *)
               ((UCHAR far *)&vcb + vcb.display.init_sect_len);
         } else {                             /* Build DISPLAY_APPN verb    */
            vcb.display_appn.opcode        = AP_DISPLAY_APPN;
            vcb.display_appn.init_sect_len = appn_init_sect_len[disp_version];
            vcb.display_appn.num_sections  = appn_num_sections[disp_version];
            vcb.display_appn.buffer_len    = (ULONG)info_buffer_size;
            vcb.display_appn.buffer_ptr    = info_buffer_ptr;
            ptr.info_flags = (UCHAR far *)&vcb.display_appn.node_info;
            ptr.info_flags[info_type - DISPLAY_INFO_NODE] = AP_YES;
            ptr.info_ptrs = (void far * far *)
               ((UCHAR far *)&vcb + vcb.display_appn.init_sect_len);
         } /* endif */
 
      /* Execute the DISPLAY or DISPLAY_APPN verb.  For the original        */
      /* DISPLAY info types, I call the original entry point, ACSMGT; the   */
      /* additional info return by the new APPC passes safely through       */
      /* ACSMGT.  Note that the new entry point, APPC, may be called for    */
      /* all the original DISPLAY info types except X.25.  Call APPC for    */
      /* all new DISPLAY info types and for DISPLAY_APPN.                   */
 
         if (info_type <= DISPLAY_INFO_X25) {
            call_ACSMGT((void far *)&vcb);
         } else {
            APPC((ULONG)((void far *)&vcb));
         } /* endif */
 
      /* Return the APPC return codes to the caller.                        */
 
         *secondary_rc = vcb.display.secondary_rc;
         *primary_rc = vcb.display.primary_rc;
         *display_length = vcb.display.display_len;
 
         if ((vcb.display.primary_rc == AP_OK) ||
             ((vcb.display.primary_rc == AP_STATE_CHECK) &&
              (vcb.display.secondary_rc == AP_DISPLAY_INFO_EXCEEDS_LEN))) {
 
         /* DISPLAY or DISPLAY_APPN executed successfully (it may have      */
         /* truncated the returned data if it was more than would fit in    */
         /* the buffer).  Use ptr.info_ptrs (pointer to the first of the    */
         /* info pointers, set above) and info_type (as an index into the   */
         /* info pointers) to to get the pointer to the info returned by    */
         /* APPC.                                                           */
 
            if (info_type < DISPLAY_INFO_NODE) { /* DISPLAY */
               ptr.info = ptr.info_ptrs[info_type];
            } else {                             /* DISPLAY_APPN */
               ptr.info = ptr.info_ptrs[info_type - DISPLAY_INFO_NODE];
            } /* endif */
         } else
         if ((vcb.display.primary_rc == AP_PARAMETER_CHECK) &&
             (vcb.display.secondary_rc == AP_DISPLAY_INVALID_CONSTANT) &&
             (disp_version != 0)) {
 
         /* I set up the DISPLAY verb for a later version of APPC than the  */
         /* one on which I'm running.  Decrement to the version of APPC     */
         /* preceeding my current assumption and try again.                 */
 
            --disp_version;
            retry = 1;
         } else {
 
         /* DISPLAY or DISPLAY_APPN failed, return NULL pointer to caller.  */
 
            ptr.info = (void far *)NULL;
         } /* endif */
      } /* endif */
   } /* endfor */
 
/* Return pointer to DISPLAY info to caller.                                */
 
   return(ptr.info);
}
 
/****************************************************************************/
/* call_ACSMGT:  Saves registers and calls the ACSMGT DLR.  This routine is */
/*               needed because some versions of ACSMGT do not save         */
/*               registers.                                                 */
/****************************************************************************/
void pascal _saveregs call_ACSMGT (void far * display_vcb)
{
   ACSMGT((long)display_vcb);
}
 
