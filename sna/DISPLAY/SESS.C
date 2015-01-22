/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : SESS.C                                                    */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format Session information returned by the       */
/*              DISPLAY verb.                                               */
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

extern BOOL appn;                        /*  Flag for APPN vs. APPC code    */

void DISP_SESSIONS (SESS_INFO_SECT far * sess_ptr)
{
   SESS_OVERLAY far * sess_ov_ptr;    /* pointer to session overlay      */
   unsigned i;                        /* loop variable                   */
   unsigned short sess_count;         /* counter for number of sessions  */

   /*-----------------------------------------------------------------------*/
   /* Display each session overlay                                          */
   /*-----------------------------------------------------------------------*/
   if (appn) {
      print_total (MSG_NUMBER_SESSIONS,
                   sess_ptr->total_sessions, sess_ptr->num_sessions);
   } else {
      print_total (MSG_NUMBER_SESSIONS,
                   sess_ptr->num_sessions, sess_ptr->num_sessions);
      } /* endif */
   sess_count = sess_ptr->num_sessions;
   for (sess_ov_ptr = (SESS_OVERLAY far *)
           ((UCHAR far *)sess_ptr + sess_ptr->sess_init_sect_len),
        i = 0; i < sess_count; i++,
        sess_ov_ptr = (SESS_OVERLAY far *)
           ((UCHAR far *)sess_ov_ptr + sess_ov_ptr->sess_entry_len)) {

      print_index1 (i+1);

      print_desc_xs (MSG_SESSION_ID,
                     sizeof(sess_ov_ptr->sess_id), sess_ov_ptr->sess_id);
      print_desc_lx (MSG_CONVERSATION_ID, sess_ov_ptr->conv_id);
      print_desc_ls (MSG_LU_ALIAS,
                     sizeof(sess_ov_ptr->lu_alias), sess_ov_ptr->lu_alias);
      print_desc_ls (MSG_PLU_ALIAS,
                     sizeof(sess_ov_ptr->plu_alias), sess_ov_ptr->plu_alias);

      print_desc_ebcdic (MSG_MODE_NAME,
                         sizeof(sess_ov_ptr->mode_name),
                         sess_ov_ptr->mode_name);

      print_desc_u (MSG_SEND_MAX_RU_SIZE, sess_ov_ptr->send_ru_size);
      print_desc_u (MSG_RCV_MAX_RU_SIZE, sess_ov_ptr->rcv_ru_size);
      print_desc_u (MSG_SEND_PACING_WINDOW, sess_ov_ptr->send_pacing_size);
      print_desc_u (MSG_RCV_PACING_WINDOW, sess_ov_ptr->rcv_pacing_size);

      if (appn) {
         print_desc_ebcdic (MSG_LINK_NAME,
                            sizeof(sess_ov_ptr->link_id),
                            sess_ov_ptr->link_id);
      } else {
         print_desc_xs (MSG_LINK_ID,
                        sizeof(sess_ov_ptr->link_id), sess_ov_ptr->link_id);
      } /* endif */

      print_desc_02x (MSG_OUTBOUND_DAF, sess_ov_ptr->daf);
      print_desc_02x (MSG_OUTBOUND_OAF, sess_ov_ptr->oaf);
      print_desc (MSG_OUTBOUND_ODAI);
      myprintf ("B'%u'\n", sess_ov_ptr->odai);

      print_desc (MSG_SESSION_TYPE);
      switch (sess_ov_ptr->sess_type) {
      case AP_SSCP_PU_SESSION: myprintf (MSG_SSCP_PU_SESSION); break;
      case AP_SSCP_LU_SESSION: myprintf (MSG_SSCP_LU_SESSION); break;
      case AP_LU_LU_SESSION:   myprintf (MSG_LU_LU_SESSION);   break;
      default: myprintf (MSG_ERROR_VALUE, sess_ov_ptr->sess_type);
      } /* endswitch */

      print_desc (MSG_CONNECTION_TYPE);
      switch (sess_ov_ptr->conn_type) {
      case AP_HOST_CONNECTION: myprintf(MSG_HOST_CONNECTION); break;
      case AP_PEER_CONNECTION: myprintf(MSG_PEER_CONNECTION); break;
      case AP_BOTH_CONNECTION: myprintf(MSG_BOTH_CONNECTION); break;
      default: myprintf(MSG_ERROR_VALUE, sess_ov_ptr->conn_type);
      } /* endswitch */

      if (appn) {
         print_desc_xs (MSG_FQPCID_ID,
                        sizeof(sess_ov_ptr->fqpcid.unique_proc_id),
                        sess_ov_ptr->fqpcid.unique_proc_id);

         print_desc_ebcdic (MSG_FQPCID_CP_NAME,
                            sess_ov_ptr->fqpcid.fq_length,
                            sess_ov_ptr->fqpcid.fq_name);

         print_desc_xs (MSG_CONVERSATION_GROUP_ID,
                        sizeof(sess_ov_ptr->cgid), sess_ov_ptr->cgid);

         print_desc_ebcdic (MSG_LU_NAME,
                            sizeof(sess_ov_ptr->fqlu_name),
                            sess_ov_ptr->fqlu_name);

         print_desc_ebcdic (MSG_PLU_NAME,
                            sizeof(sess_ov_ptr->fqplu_name),
                            sess_ov_ptr->fqplu_name);

         print_desc (MSG_PACING_TYPE);
         switch (sess_ov_ptr->pacing_type) {
         case AP_FIXED:    myprintf (MSG_FIXED);    break;
         case AP_ADAPTIVE: myprintf (MSG_ADAPTIVE); break;
         case AP_NONE:     myprintf (MSG_NONE);     break;
         default: myprintf (MSG_ERROR_VALUE, sess_ov_ptr->pacing_type);
         } /* endswitch */

      } /* endif */
   } /* endfor */
}
