/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : AM.C                                                      */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format Attach Manager information returned by    */
/*              the DISPLAY verb.                                           */
/*                                                                          */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
/*              (compiles with large memory model)                          */
/*                                                                          */
/*  ASSOCIATED FILES:  See DISPLAY.MAK, PMDSPLAY.MAK                        */
/*                                                                          */
/****************************************************************************/
#include "DISPLAY.H"         /* global typedefs, prototypes, and #includes  */

extern BOOL appn;                       /*  Flag for APPN vs. APPC code     */

void DISP_AM (AM_INFO_SECT far * am_ptr)
{
   AM_OVERLAY far * am_ov_ptr;          /* Pointer to AM overlay(s)   */
   unsigned i;                          /* Loop variable              */
   unsigned short ctp_count;            /* Counter for configured tps */

   print_desc_s (MSG_AM_ACTIVE,
                 (am_ptr->am_active == AP_NO) ? MSG_NO : MSG_YES);

   /*-----------------------------------------------------------------------*/
   /* Display each TP overlay                                               */
   /*-----------------------------------------------------------------------*/
   ctp_count = am_ptr->num_tps;         /* Get the number of overlays       */
   print_desc_u (MSG_CTP_COUNT, ctp_count);
   for (am_ov_ptr = (AM_OVERLAY far *)
           ((UCHAR far *)am_ptr + am_ptr->am_init_sect_len),
        i = 0; i < ctp_count; i++,
        am_ov_ptr = (AM_OVERLAY far *)
           ((UCHAR far *)am_ov_ptr + am_ov_ptr->am_entry_len)) {

      print_index1 (i+1);

      print_desc_ebcdic (MSG_TP_NAME,
                         sizeof(am_ov_ptr->tp_name),
                         am_ov_ptr->tp_name);

      print_desc_ls (MSG_FILESPEC,
                     sizeof(am_ov_ptr->filespec), am_ov_ptr->filespec);
      print_desc_ls (MSG_PROG_PARAM_STRING,
                     sizeof(am_ov_ptr->parm_string), am_ov_ptr->parm_string);

      print_desc_s (MSG_SYNC_LEVEL_NONE,
                    (am_ov_ptr->sync_level_none == AP_NOT_SUPPORTED) ?
                    MSG_NOT_SUPPORTED : MSG_SUPPORTED);

      print_desc_s (MSG_SYNC_LEVEL_CONF,
                    (am_ov_ptr->sync_level_conf == AP_NOT_SUPPORTED) ?
                    MSG_NOT_SUPPORTED : MSG_SUPPORTED);

      print_desc (MSG_CONV_TYPE);
      if (am_ov_ptr->conv_type == AP_BASIC_CONVERSATION)
           myprintf (MSG_BASIC_CONVERSATION);
      else if (am_ov_ptr->conv_type == AP_MAPPED_CONVERSATION)
           myprintf (MSG_MAPPED_CONVERSATION);
      else if (am_ov_ptr->conv_type == AP_EITHER)
           myprintf (MSG_EITHER);
      else myprintf (MSG_ERROR_VALUE, am_ov_ptr->conv_type);

      print_desc (MSG_INCOM_ALLOC_QDEPTH_LIM);
      if (am_ov_ptr->in_all_qdpth_lim == 0)
           myprintf (MSG_NON_QUEUED_PROG);
      else print_u (am_ov_ptr->in_all_qdpth_lim);

      print_desc (MSG_INCOM_ALLOC_QDEPTH);
      if (am_ov_ptr->in_all_qdpth == 0)
           myprintf (MSG_NON_QUEUED_PROG);
      else print_u (am_ov_ptr->in_all_qdpth);

      print_desc (MSG_INCOM_ALLOC_TIMEOUT);
      if (am_ov_ptr->in_all_timeout == 0)
           myprintf(MSG_NON_QUEUED_PROG);
      else if (am_ov_ptr->in_all_timeout == AP_HOLD_FOREVER)
           myprintf(MSG_HOLD_FOREVER);
      else myprintf("%u %s\n", am_ov_ptr->in_all_timeout, MSG_SECONDS);

      print_desc (MSG_NUM_RECEIV_ALLOC_PENDING);
      if (am_ov_ptr->num_rcv_all_pend == 0)
           myprintf (MSG_NON_QUEUED_PROG);
      else print_u (am_ov_ptr->num_rcv_all_pend);

      print_desc (MSG_RECEIV_ALLOC_TIMEOUT);
      if (am_ov_ptr->rcv_all_timeout == 0)
           myprintf (MSG_NON_QUEUED_PROG);
      else if (am_ov_ptr->rcv_all_timeout == AP_HOLD_FOREVER)
           myprintf (MSG_HOLD_FOREVER);
      else myprintf ("%u %s\n", am_ov_ptr->rcv_all_timeout, MSG_SECONDS);

      print_desc (MSG_TP_TYPE);
      if (am_ov_ptr->tp_type == AP_QUEUED_OPERATOR_STARTED)
           myprintf (MSG_QUEUED_OPERATOR_STARTED);
      else if (am_ov_ptr->tp_type == AP_QUEUED_AM_STARTED)
           myprintf (MSG_QUEUED_AM_STARTED);
      else if (am_ov_ptr->tp_type == AP_NONQUEUED_AM_STARTED)
           myprintf (MSG_NONQUEUED_AM_STARTED);
      else if ((am_ov_ptr->tp_type == AP_QUEUED_OPERATOR_PRELOADED) && appn)
           myprintf (MSG_QUEUED_OPERATOR_PRELOADED);
      else myprintf (MSG_ERROR_VALUE, am_ov_ptr->tp_type);

      print_desc (MSG_PROGRAM_STATE);
      if (am_ov_ptr->pgm_state == AP_INACTIVE)
           myprintf (MSG_INACTIVE);
      else if (am_ov_ptr->pgm_state == AP_LOADED)
           myprintf (MSG_LOADED);
      else if (am_ov_ptr->pgm_state == AP_LOADING)
           myprintf (MSG_LOADING);
      else if (am_ov_ptr->pgm_state == AP_RUNNING)
           myprintf (MSG_RUNNING);
      else myprintf (MSG_ERROR_VALUE, am_ov_ptr->pgm_state);

      print_desc_s (MSG_CONV_SEC_INFO_REQUIRED,
                    (am_ov_ptr->conv_sec == AP_NO) ? MSG_NO : MSG_YES);

      print_desc (MSG_PROCESS_ID);
      if (am_ov_ptr->process_id == 0)
           myprintf(MSG_NOT_KNOWN_TO_AM);
      else myprintf("X'%04X'\n", am_ov_ptr->process_id);

      if (appn) {
         print_desc (MSG_PROGRAM_TYPE);
         if (am_ov_ptr->program_type == AP_BACKGROUND) {
              myprintf (MSG_BACKGROUND);
         } else if (am_ov_ptr->program_type == AP_FULL_SCREEN) {
              myprintf (MSG_FULL_SCREEN);
         } else if (am_ov_ptr->program_type == AP_PRESENTATION_MANAGER) {
              myprintf (MSG_PRESENTATION_MANAGER);
         } else if (am_ov_ptr->program_type == AP_VIO_WINDOWABLE) {
              myprintf (MSG_VIO_WINDOWABLE);
         } else
              myprintf (MSG_ERROR_VALUE, am_ov_ptr->program_type);

         print_desc_s (MSG_TP_INITIATED,
                       (am_ov_ptr->tp_initiated == AP_LOCALLY) ?
                       MSG_LOCALLY : MSG_REMOTELY) ;

         print_desc_ls (MSG_ICON_FILESPEC,
                        sizeof(am_ov_ptr->icon_filespec),
                        am_ov_ptr->icon_filespec);
      } /* endif */
   } /* endfor */
}
