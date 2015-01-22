/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : MODE_DEF.C                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format Mode Defintions information returned by   */
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

void DISP_MODE_DEF (MODE_DEF_INFO_SECT far * mode_def_ptr)
{
   unsigned i;
   unsigned mode_count;                    /* Counter for modes */
   MODE_DEF_OVERLAY far * mode_def_ov_ptr; /* Pointer to mode def overlay */

   /*-----------------------------------------------------------------------*/
   /* Display each mode definition overlay                                  */
   /*-----------------------------------------------------------------------*/
   print_total (MSG_NUMBER_MODE_TOTAL,
                mode_def_ptr->total_mode_def,
                mode_def_ptr->num_mode_def);
   mode_count = mode_def_ptr->num_mode_def;
   for (mode_def_ov_ptr = (MODE_DEF_OVERLAY far *)
           ((UCHAR far *)mode_def_ptr + mode_def_ptr->mode_def_init_sect_len),
        i = 0; i < mode_count; i++,
        mode_def_ov_ptr = (MODE_DEF_OVERLAY far *)
           ((UCHAR far *)mode_def_ov_ptr +
            mode_def_ov_ptr->mode_def_entry_len)) {

      print_index1 (i+1);

      print_desc_ebcdic (MSG_MODE_NAME,
                         sizeof(mode_def_ov_ptr->mode_name),
                         mode_def_ov_ptr->mode_name);

      print_desc_ebcdic (MSG_COS_NAME,
                         sizeof(mode_def_ov_ptr->cos_name),
                         mode_def_ov_ptr->cos_name);

      print_desc_u (MSG_MAX_RU_SIZE_UPPER, mode_def_ov_ptr->rusize_upper);
      print_desc_u (MSG_RCV_PACING_WINDOW, mode_def_ov_ptr->rcv_window);

      print_desc_s (MSG_DEFAULT_RU_SIZE,
                    (mode_def_ov_ptr->default_ru_size == AP_NO) ?
                    MSG_NO : MSG_YES);

      print_desc_u (MSG_MAX_SESS_LIMIT, mode_def_ov_ptr->max_neg_sess_lim);
      print_desc_u (MSG_CURR_SESS_LIMIT, mode_def_ov_ptr->curr_sess_lim);
      print_desc_u (MSG_MIN_WINNER_LIMIT, mode_def_ov_ptr->min_win_lim);

   } /* endfor */
}
