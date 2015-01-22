/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : MS.C                                                      */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format Management Services information returned  */
/*              by the DISPLAY verb.                                        */
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
 
static struct ms_tab {
   unsigned char ebcdic_string[8];
   unsigned char *application_name;
   unsigned char *category;
   } ms_table[] = {
    "\x23\xF0\xF1\xF4\x40\x40\x40\x40", "EP_COMMON_OPS_SERVICES",    "COMMON_OPERATION_SERVICES",
    "\x23\xF0\xF1\xF5\x40\x40\x40\x40", "COMMON_OPS_SERVICES_NETOP", "COMMON_OPERATION_SERVICES",
    "\x23\xF0\xF3\xF0\x40\x40\x40\x40", "EP_ALERT",                  "PROBLEM_MANAGEMENT",
    "\x23\xF0\xF3\xF1\x40\x40\x40\x40", "ALERT_NETOP",               "PROBLEM_MANAGEMENT",
    "\x23\xF0\xF4\xF2\x40\x40\x40\x40", "EP_QUERY_PRODUCT_ID",       "CONFIGURATION_MANAGEMENT",
    "" , "", ""
   };
 
void DISP_MS (MS_INFO_SECT far * ms_ptr)
{
   unsigned i;
   unsigned fp_count;              /* Counter for focal point overlays   */
   unsigned appl_count;            /* Counter for application overlays   */
   unsigned at_count;              /* Counter for active trans. overlays */
   MS_FP_OVERLAY far *        fp_ov_ptr;   /* Pointer to focal point overlay */
   MS_APPL_OVERLAY far *      appl_ov_ptr; /*  to application overlay        */
   MS_ACT_TRANS_OVERLAY far * at_ov_ptr;   /*  to active transaction overlay */
   struct ms_tab *ms_tab_ptr;    /* Pointer to table of ms architected names */
 
   print_desc_u (MSG_HELD_MDS_MU_ALERTS, ms_ptr->held_mds_mu_alerts);
   print_desc_u (MSG_HELD_NMVT_ALERTS, ms_ptr->held_nmvt_alerts);
 
   /*-----------------------------------------------------------------------*/
   /* Display each focal point overlay                                      */
   /*-----------------------------------------------------------------------*/
   print_total (MSG_NUMBER_FPS_TOTAL,
                ms_ptr->total_fps,
                ms_ptr->num_fps);
   fp_count = ms_ptr->num_fps;
   for (fp_ov_ptr = (MS_FP_OVERLAY far *)
           ((UCHAR far *)ms_ptr + ms_ptr->ms_init_sect_len),
        i = 0; i < fp_count; i++,
        fp_ov_ptr = (MS_FP_OVERLAY far *)
           ((UCHAR far *)fp_ov_ptr + fp_ov_ptr->ms_fp_entry_len)) {
 
      print_index1 (i+1);
 
      ms_tab_ptr = ms_table;
      while (ms_tab_ptr->ebcdic_string[0] != '\0') {
         if (!strncmp(fp_ov_ptr->ms_appl_name,
                      ms_tab_ptr->ebcdic_string,
                      sizeof(fp_ov_ptr->ms_appl_name))) {
            print_desc_s (MSG_MS_APPLICATION_NAME,
                          ms_tab_ptr->application_name);
            break;
         }
         else ms_tab_ptr++;
      }
      if (ms_tab_ptr->ebcdic_string[0] == '\0')
         print_desc_ebcdic (MSG_MS_APPLICATION_NAME,
                            sizeof(fp_ov_ptr->ms_appl_name),
                            fp_ov_ptr->ms_appl_name);
 
      ms_tab_ptr = ms_table;
      while (ms_tab_ptr->ebcdic_string[0] != '\0') {
         if (!strncmp(fp_ov_ptr->ms_category,
                      ms_tab_ptr->ebcdic_string,
                      sizeof(fp_ov_ptr->ms_category))) {
            print_desc_s (MSG_MS_CATEGORY,
                          ms_tab_ptr->category);
            break;
         }
         else ms_tab_ptr++;
      }
      if (ms_tab_ptr->ebcdic_string[0] == '\0')
         print_desc_xs (MSG_MS_CATEGORY,
                        sizeof(fp_ov_ptr->ms_category),
                        fp_ov_ptr->ms_category);
 
      print_desc_ebcdic (MSG_FOCAL_PT_CP_NAME,
                         sizeof(fp_ov_ptr->fp_fq_cp_name),
                         fp_ov_ptr->fp_fq_cp_name);
 
      print_desc_ebcdic (MSG_BACKUP_APPL_NAME,
                         sizeof(fp_ov_ptr->bkup_appl_name),
                         fp_ov_ptr->bkup_appl_name);
 
      print_desc_ebcdic (MSG_BACKUP_FOCAL_PT_CP_NAME,
                         sizeof(fp_ov_ptr->bkup_fp_fq_cp_name),
                         fp_ov_ptr->bkup_fp_fq_cp_name);
 
      print_desc (MSG_FOCAL_POINT_TYPE);
      switch (fp_ov_ptr->fp_type) {
      case AP_EXPLICIT_PRIMARY_FP: myprintf (MSG_PRIMARY_FP); break;
      case AP_BACKUP_FP:           myprintf (MSG_BACKUP_FP);  break;
      case AP_DEFAULT_PRIMARY_FP:  myprintf (MSG_DEFAULT_FP); break;
      case AP_DOMAIN_FP:           myprintf (MSG_DOMAIN_FP);  break;
      case AP_HOST_FP:             myprintf (MSG_HOST_FP);    break;
      case AP_NO_FP:               myprintf (MSG_NO_FP);      break;
      default: myprintf(MSG_ERROR_VALUE, fp_ov_ptr->fp_type);
      } /* endswitch */
 
      print_desc (MSG_FOCAL_POINT_STATUS);
      switch (fp_ov_ptr->fp_status) {
      case AP_NOT_ACTIVE:    myprintf (MSG_DEACTIVATED);  break;
      case AP_ACTIVE:        myprintf (MSG_ACTIVATED);    break;
      case AP_PENDING:       myprintf (MSG_PENDING);      break;
      case AP_NEVER_ACTIVE:  myprintf (MSG_NEVER_ACTIVE); break;
      default: myprintf(MSG_ERROR_VALUE, fp_ov_ptr->fp_status);
      } /* endswitch */
 
      print_desc (MSG_FOCAL_POINT_ROUTING);
      switch (fp_ov_ptr->fp_routing) {
      case AP_DEFAULT:  myprintf (MSG_DEFAULT_ROUTING); break;
      case AP_DIRECT:   myprintf (MSG_DIRECT_ROUTING);  break;
      default: myprintf (MSG_ERROR_VALUE, fp_ov_ptr->fp_routing);
      } /* endswitch */
 
   } /* endfor */
 
   if (fp_count) myprintf (MSG_CRLF);  /* Put space between sections */
 
   /*-----------------------------------------------------------------------*/
   /* Display each Management Services application overlay                  */
   /*-----------------------------------------------------------------------*/
   print_total (MSG_NUMBER_APPLS_TOTAL,
                ms_ptr->total_ms_appls,
                ms_ptr->num_ms_appls);
   appl_count = ms_ptr->num_ms_appls;
   for (appl_ov_ptr  = (MS_APPL_OVERLAY far *)fp_ov_ptr,
        i = 0; i < appl_count; i++,
        appl_ov_ptr = (MS_APPL_OVERLAY far *)
           ((UCHAR far *)appl_ov_ptr + appl_ov_ptr->ms_appl_entry_len)) {
 
      print_index1 (i+1);
 
      ms_tab_ptr = ms_table;
      while (ms_tab_ptr->ebcdic_string[0] != '\0') {
         if (!strncmp(appl_ov_ptr->ms_appl_name,
                      ms_tab_ptr->ebcdic_string,
                      sizeof(appl_ov_ptr->ms_appl_name))) {
            print_desc_s (MSG_MS_APPL_NAME,
                          ms_tab_ptr->application_name);
            break;
         }
         else ms_tab_ptr++;
      }
      if (ms_tab_ptr->ebcdic_string[0] == '\0')
         print_desc_ebcdic (MSG_MS_APPL_NAME,
                            sizeof(appl_ov_ptr->ms_appl_name),
                            appl_ov_ptr->ms_appl_name);
 
      ms_tab_ptr = ms_table;
      while (ms_tab_ptr->ebcdic_string[0] != '\0') {
         if (!strncmp(appl_ov_ptr->ms_category,
                      ms_tab_ptr->ebcdic_string,
                      sizeof(appl_ov_ptr->ms_category))) {
            print_desc_s (MSG_MS_APPL_CATEGORY,
                          ms_tab_ptr->category);
            break;
         }
         else ms_tab_ptr++;
      }
      if (ms_tab_ptr->ebcdic_string[0] == '\0')
         print_desc_xs (MSG_MS_APPL_CATEGORY,
                        sizeof(appl_ov_ptr->ms_category),
                        appl_ov_ptr->ms_category);
 
      print_desc_ls (MSG_OS2_QUEUE_NAME,
                     sizeof(appl_ov_ptr->q_name),
                     appl_ov_ptr->q_name);
 
   } /* endfor */
 
   if (appl_count) myprintf (MSG_CRLF); /* Put space between sections */
 
   /*-----------------------------------------------------------------------*/
   /* Display each active transaction program overlay                       */
   /*-----------------------------------------------------------------------*/
   print_total (MSG_NUMBER_ACT_TRANS_TOTAL,
                ms_ptr->total_act_trans,
                ms_ptr->num_act_trans);
   at_count = ms_ptr->num_act_trans;
   for (at_ov_ptr  = (MS_ACT_TRANS_OVERLAY far *)appl_ov_ptr,
        i = 0; i < at_count; i++,
        at_ov_ptr = (MS_ACT_TRANS_OVERLAY far *)
           ((UCHAR far *)at_ov_ptr + at_ov_ptr->ms_act_trans_entry_len)) {
 
      print_index1 (i+1);
 
      print_desc_ebcdic (MSG_ORIGIN_CP_NAME,
                         sizeof(at_ov_ptr->fq_origin_cp_name),
                         at_ov_ptr->fq_origin_cp_name);
 
      ms_tab_ptr = ms_table;
      while (ms_tab_ptr->ebcdic_string[0] != '\0') {
         if (!strncmp(at_ov_ptr->origin_ms_appl_name,
                      ms_tab_ptr->ebcdic_string,
                      sizeof(at_ov_ptr->origin_ms_appl_name))) {
            print_desc_s (MSG_ORIGIN_MS_APPL_NAME,
                          ms_tab_ptr->application_name);
            break;
         }
         else ms_tab_ptr++;
      }
      if (ms_tab_ptr->ebcdic_string[0] == '\0')
         print_desc_ebcdic (MSG_ORIGIN_MS_APPL_NAME,
                            sizeof(at_ov_ptr->origin_ms_appl_name),
                            at_ov_ptr->origin_ms_appl_name);
 
      print_desc_ebcdic (MSG_DEST_CP_NAME,
                         sizeof(at_ov_ptr->fq_dest_cp_name),
                         at_ov_ptr->fq_dest_cp_name);
 
      ms_tab_ptr = ms_table;
      while (ms_tab_ptr->ebcdic_string[0] != '\0') {
         if (!strncmp(at_ov_ptr->dest_ms_appl_name,
                      ms_tab_ptr->ebcdic_string,
                      sizeof(at_ov_ptr->dest_ms_appl_name))) {
            print_desc_s (MSG_DEST_MS_APPL_NAME,
                          ms_tab_ptr->application_name);
            break;
         }
         else ms_tab_ptr++;
      }
      if (ms_tab_ptr->ebcdic_string[0] == '\0')
         print_desc_ebcdic (MSG_DEST_MS_APPL_NAME,
                            sizeof(at_ov_ptr->dest_ms_appl_name),
                            at_ov_ptr->dest_ms_appl_name);
 
      print_desc (MSG_AGENT_UNIT_OF_WORK);
      myprintf (MSG_CRLF);
 
      /* The following three fields make up the Unit of Work ID */
 
      print_desc_ebcdic (MSG_REQUESTER_CP_NAME,
                         sizeof(at_ov_ptr->fq_req_loc_cp_name),
                         at_ov_ptr->fq_req_loc_cp_name);
 
      print_desc_ebcdic (MSG_REQUESTER_APPL_NAME,
                         sizeof(at_ov_ptr->req_agent_appl_name),
                         at_ov_ptr->req_agent_appl_name);
 
      print_desc_xs (MSG_SEQUENCE_NUMBER,
                     sizeof(at_ov_ptr->seq_num_dt),
                     at_ov_ptr->seq_num_dt);
 
   } /* endfor */
}
