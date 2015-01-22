/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : TP.C                                                      */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format information on active Transaction         */
/*              Programs returned by the DISPLAY verb.                      */
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
#include "DISPLAY.H"         /* global typedefs, prototypes, and #includes  */

extern BOOL appn;                       /*  Flag for APPN vs. APPC code     */

void DISP_TP (TP_INFO_SECT far * tp_ptr)
{
   TP_OVERLAY far * tp_ov_ptr;              /* pointer to active tp overlay */
   CONV_OVERLAY far * conv_ov_ptr;          /* pointer to conv overlay      */
   unsigned i,j;                            /* loop variables               */
   unsigned short atp_count, conv_count;    /* counters for active TPs      */
                                            /* and conversations.           */

   /*-----------------------------------------------------------------------*/
   /* Display each transaction program overlay                              */
   /*-----------------------------------------------------------------------*/
   if (appn) {
      print_total (MSG_ACTIVE_TP_COUNT, tp_ptr->total_tps, tp_ptr->num_tps);
   } else {
      print_total (MSG_ACTIVE_TP_COUNT, tp_ptr->num_tps, tp_ptr->num_tps);
      } /* endif */
   atp_count = tp_ptr->num_tps;       /* Get the number of overlays         */
   for (tp_ov_ptr = (TP_OVERLAY far *)
           ((UCHAR far *)tp_ptr + tp_ptr->tp_init_sect_len),
        i = 0; i < atp_count; i++,
        tp_ov_ptr = (TP_OVERLAY far *)
           ((UCHAR far *)tp_ov_ptr + tp_ov_ptr->tp_entry_len)) {

      print_index1 (i+1);

      print_desc_ebcdic (MSG_TP_NAME,
                         sizeof(tp_ov_ptr->tp_name),
                         tp_ov_ptr->tp_name);

      print_desc (MSG_TP_ID);
      print_hex_string (tp_ov_ptr->tp_id, sizeof(tp_ov_ptr->tp_id));

      print_desc_ebcdic (MSG_USER_ID,
                         sizeof(tp_ov_ptr->user_id),
                         tp_ov_ptr->user_id);

      print_desc_s (MSG_TP_INITIATED,
                    (tp_ov_ptr->loc_or_rem == AP_LOCAL) ?
                    MSG_LOCALLY : MSG_REMOTELY);

      print_desc_ls (MSG_LU_ALIAS,
                     sizeof(tp_ov_ptr->lu_alias), tp_ov_ptr->lu_alias);

      if (appn) {
         print_desc_ebcdic (MSG_LUW_NAME,
                            tp_ov_ptr->luw_id.fq_length,
                            tp_ov_ptr->luw_id.fq_luw_name);

         print_desc (MSG_LUW_INSTANCE);
         print_hex_string (tp_ov_ptr->luw_id.instance,
                           sizeof(tp_ov_ptr->luw_id.instance));

         print_desc (MSG_LUW_SEQUENCE);
         print_hex_string (tp_ov_ptr->luw_id.sequence,
                           sizeof(tp_ov_ptr->luw_id.sequence));
      } /* endif */

      /*--------------------------------------------------------------------*/
      /* Display each conversation overlay in this TP overlay               */
      /*--------------------------------------------------------------------*/
      conv_count = tp_ov_ptr->num_conv;
      print_desc_u (MSG_CONVERSATION_COUNT, conv_count);
      for (conv_ov_ptr = (CONV_OVERLAY far *)
              ((UCHAR far *)tp_ov_ptr +
               tp_ov_ptr->tp_overlay_len +
               sizeof(tp_ov_ptr->tp_entry_len)),
           j = 0;
           j < conv_count;
           conv_ov_ptr = (CONV_OVERLAY far *)
              ((UCHAR far *)conv_ov_ptr + conv_ov_ptr->conv_entry_len),
           j++) {

         print_index2 (i+1, j+1);

         print_desc_lx (MSG_CONVERSATION_ID, conv_ov_ptr->conv_id);

         print_desc (MSG_CONVERSATION_STATE);
         if (conv_ov_ptr->state == AP_SEND_STATE)
              myprintf (MSG_SEND_STATE);
         else if (conv_ov_ptr->state == AP_RECEIVE_STATE)
              myprintf (MSG_RECEIVE_STATE);
         else if (conv_ov_ptr->state == AP_CONFIRM_STATE)
              myprintf (MSG_CONFIRM_STATE);
         else if (conv_ov_ptr->state == AP_CONFIRM_SEND_STATE)
              myprintf (MSG_CONFIRM_SEND_STATE);
         else if (conv_ov_ptr->state == AP_CONFIRM_DEALL_STATE)
              myprintf (MSG_CONFIRM_DEALL_STATE);
         else if (conv_ov_ptr->state == AP_PEND_POST_STATE)
              myprintf (MSG_PEND_POST_STATE);
         else myprintf (MSG_ERROR_VALUE, conv_ov_ptr->state);

         print_desc (MSG_SESSION_ID);
         print_hex_string (conv_ov_ptr->sess_id,
                           sizeof(conv_ov_ptr->sess_id));

         print_desc (MSG_SYNC_LEVEL);
         if (conv_ov_ptr->sync_level == AP_NONE)
              myprintf(MSG_NONE);
         else if (conv_ov_ptr->sync_level == AP_CONFIRM)
              myprintf(MSG_CONFIRM);
         else myprintf(MSG_ERROR_VALUE, conv_ov_ptr->sync_level);

         if (appn) {
            print_desc (MSG_CONVERSATION_TYPE);
            if (conv_ov_ptr->conv_type == AP_BASIC_CONVERSATION) {
                 myprintf(MSG_BASIC_CONVERSATION);
            } else if (conv_ov_ptr->conv_type == AP_MAPPED_CONVERSATION) {
                 myprintf(MSG_MAPPED_CONVERSATION);
            } else
                 myprintf(MSG_ERROR_VALUE, conv_ov_ptr->conv_type);

            print_desc_lx (MSG_CONVERSATION_GROUP_ID,
                           conv_ov_ptr->conv_group_id);
         } /* endif */

      } /* endfor */
   } /* endfor */
}
