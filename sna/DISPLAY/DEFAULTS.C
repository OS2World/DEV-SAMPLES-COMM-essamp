/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : DEFAULTS.C                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format System Defaults information returned by   */
/*              the DISPLAY verb.                                           */
/*                                                                          */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
/*              (compiles with large memory model)                          */
/*                                                                          */
/*  ASSOCIATED FILES:  See also DISPLAY.MAK, PMDSPLAY.MAK                   */
/*                                                                          */
/*      DISPLAY.H    - Global typedefs, prototypes, and #includes           */
/*      APD.TXT      - Messages (English)                                   */
/*      MSGID.H      - #defines for messages                                */
/*                                                                          */
/****************************************************************************/
#include "DISPLAY.H"      /* global typedefs, prototypes, and #includes  */

void DISP_SYS_DFLT (SYS_DEF_INFO_SECT far * sd_ptr)
{
   print_desc_ebcdic (MSG_IMPLICIT_MODE_NAME,
                      sizeof(sd_ptr->default_mode_name),
                      sd_ptr->default_mode_name);

   print_desc_ebcdic (MSG_IMPLICIT_LU_NAME,
                      sizeof(sd_ptr->default_local_lu_name),
                      sd_ptr->default_local_lu_name);

   print_desc_s (MSG_IMPLICIT_PLU_SUPPORT,
                 (sd_ptr->implicit_inb_rlu_supp == AP_NO) ?
                 MSG_NO : MSG_YES);

   print_desc_u (MSG_HELD_ALERTS, sd_ptr->max_held_alerts);

   print_desc_s (MSG_CONV_SEC_INFO_REQUIRED,
                 (sd_ptr->tp_conv_sec_rqd == AP_NO) ? MSG_NO : MSG_YES);

   print_desc_u (MSG_MAX_MC_LL_SENDSIZE, sd_ptr->max_mc_ll_send_size);

   print_desc_ls (MSG_DIR_FOR_ATTACHES,
                  sizeof(sd_ptr->dir_for_attaches),
                  sd_ptr->dir_for_attaches);

   print_desc (MSG_TP_OPERATION);
   switch (sd_ptr->tp_operation) {
   case AP_QUEUED_OPERATOR_STARTED:
      myprintf (MSG_QUEUED_OPERATOR_STARTED);
      break;
   case AP_QUEUED_AM_STARTED:
      myprintf (MSG_QUEUED_AM_STARTED);
      break;
   case AP_NONQUEUED_AM_STARTED:
      myprintf (MSG_NONQUEUED_AM_STARTED);
      break;
   case AP_QUEUED_OPERATOR_PRELOADED:
      myprintf (MSG_QUEUED_OPERATOR_PRELOADED);
      break;
   default:
      myprintf (MSG_ERROR_VALUE, sd_ptr->tp_operation);
   } /* endswitch */

   print_desc (MSG_TP_PROGRAM_TYPE);
   switch (sd_ptr->tp_program_type) {
   case AP_BACKGROUND:           myprintf (MSG_BACKGROUND);           break;
   case AP_FULL_SCREEN:          myprintf (MSG_FULL_SCREEN);          break;
   case AP_PRESENTATION_MANAGER: myprintf (MSG_PRESENTATION_MANAGER); break;
   case AP_VIO_WINDOWABLE:       myprintf (MSG_VIO_WINDOWABLE);       break;
   default: myprintf(MSG_ERROR_VALUE, sd_ptr->tp_program_type);
   } /* endswitch */
}
