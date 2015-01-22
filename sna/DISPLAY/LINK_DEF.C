/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : LINK_DEF.C                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format Link Definitions information returned by  */
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
 
void DISP_LINK_DEF (LINK_DEF_INFO_SECT far * link_def_ptr)
{
   unsigned i;
   unsigned link_count;                    /* Counter for active links */
   LINK_DEF_OVERLAY far * link_def_ov_ptr; /* Pointer to link def overlay */
 
   /*-----------------------------------------------------------------------*/
   /* Display each link definition overlay                                  */
   /*-----------------------------------------------------------------------*/
   print_total (MSG_NUMBER_LINK_TOTAL,
                link_def_ptr->total_link_def,
                link_def_ptr->num_link_def);
   link_count = link_def_ptr->num_link_def;
   for (link_def_ov_ptr = (LINK_DEF_OVERLAY far *)
           ((UCHAR far *)link_def_ptr + link_def_ptr->link_def_init_sect_len),
        i = 0; i < link_count; i++,
        link_def_ov_ptr = (LINK_DEF_OVERLAY far *)
           ((UCHAR far *)link_def_ov_ptr +
                         link_def_ov_ptr->link_def_entry_len)) {
 
      print_index1 (i+1);
 
      print_desc_ebcdic (MSG_LINK_NAME,
                         sizeof(link_def_ov_ptr->link_name),
                         link_def_ov_ptr->link_name);
 
      print_desc_ebcdic (MSG_ADJACENT_CP_NAME,
                         sizeof(link_def_ov_ptr->adj_fq_cp_name),
                         link_def_ov_ptr->adj_fq_cp_name);
 
      print_desc (MSG_ADJACENT_NODE_TYPE);
      switch (link_def_ov_ptr->adj_node_type) {
      case AP_LEARN: myprintf (MSG_LEARN); break;
      case AP_LEN:   myprintf (MSG_LEN);   break;
      case AP_NN:    myprintf (MSG_NN);    break;
      default:
           myprintf(MSG_ERROR_VALUE, link_def_ov_ptr->adj_node_type);
      } /* endswitch */
 
      print_desc_ls (MSG_DLC_NAME,
                     sizeof(link_def_ov_ptr->dlc_name),
                     link_def_ov_ptr->dlc_name);
 
      if (!(memcmp(link_def_ov_ptr->dlc_name, "X25DLC", 6)))
        {
        link_def_ov_ptr->adapter_num++;
        print_desc_u (MSG_ADAPTER_NUMBER, link_def_ov_ptr->adapter_num);
 
        if (link_def_ov_ptr->dest_addr[0] == 0xFF)
          {
          print_desc_xs (MSG_DEST_ADDR,
                         (link_def_ov_ptr->dest_addr_len)-1,
                         &link_def_ov_ptr->dest_addr[1]);
          }
        else
          {
          print_desc_ls (MSG_DEST_ADDR,
                         link_def_ov_ptr->dest_addr_len,
                         link_def_ov_ptr->dest_addr);
          }
        }
      else
        {
        print_desc_u (MSG_ADAPTER_NUMBER, link_def_ov_ptr->adapter_num);
 
        print_desc_xs (MSG_DEST_ADDR,
                       link_def_ov_ptr->dest_addr_len,
                       link_def_ov_ptr->dest_addr);
        }
 
      print_desc_s (MSG_CP_CP_SESSION_SUPPORT,
                    (link_def_ov_ptr->cp_cp_sess_spt == AP_NO) ?
                    MSG_NO : MSG_YES);
 
      print_desc_s (MSG_PREFERRED_NN_SERVER,
                    (link_def_ov_ptr->preferred_nn_server == AP_NO) ?
                    MSG_NO : MSG_YES);
 
      print_desc_s (MSG_AUTO_ACTIVATE_LINK,
                    (link_def_ov_ptr->auto_act_link == AP_NO) ?
                    MSG_NO : MSG_YES);
 
      print_desc_u (MSG_TG_NUMBER, link_def_ov_ptr->tg_number);
 
      print_desc (MSG_LIMITED_RESOURCE);
      switch (link_def_ov_ptr->lim_res) {
      case AP_NO:
         myprintf(MSG_NO);
         myprintf(MSG_CRLF);
         break;
      case AP_YES:
         myprintf(MSG_YES);
         myprintf(MSG_CRLF);
         break;
      case AP_USE_ADAPTER_DEF_CHAR:
         myprintf (MSG_USE_ADAPTER_DEF_CHAR);
         break;
      default:
         myprintf (MSG_ERROR_VALUE, link_def_ov_ptr->lim_res);
         break;
      } /* endswitch */
 
      print_desc_s (MSG_SOLICIT_SSCP_SESS,
                    (link_def_ov_ptr->solicit_sscp_session == AP_NO) ?
                    MSG_NO : MSG_YES);
 
      print_desc_s (MSG_INIT_SELF,
                    (link_def_ov_ptr->initself == AP_NO) ?
                    MSG_NO : MSG_YES);
 
      print_desc_s (MSG_BIND_SUPPORT,
                    (link_def_ov_ptr->bind_support == AP_NO) ?
                    MSG_NO : MSG_YES);
 
      print_desc (MSG_LINK_STATION_ROLE);
      switch (link_def_ov_ptr -> ls_role) {
      case AP_NEGOTIABLE: myprintf(MSG_NEGOTIABLE); break;
      case AP_PRIMARY:    myprintf(MSG_PRIMARY);    break;
      case AP_SECONDARY:  myprintf(MSG_SECONDARY);  break;
      case AP_USE_ADAPTER_DEF_CHAR:
         myprintf(MSG_USE_ADAPTER_DEF_CHAR);
         break;
      default:
         myprintf(MSG_ERROR_VALUE, link_def_ov_ptr->ls_role);
      } /* endswitch */
 
      print_desc (MSG_LINE_TYPE);
      switch (link_def_ov_ptr -> line_type) {
      case AP_SWITCHED:    myprintf(MSG_SWITCHED);    break;
      case AP_NONSWITCHED: myprintf(MSG_NONSWITCHED); break;
      default:
         myprintf(MSG_ERROR_VALUE, link_def_ov_ptr->line_type);
      } /* endswitch */
 
      print_desc (MSG_EFFECTIVE_CAPACITY);
      myprintf ("%lu %s\n", link_def_ov_ptr->eff_capacity, MSG_BPS);
 
      print_desc_u (MSG_CONNECT_COST, link_def_ov_ptr->conn_cost);
      print_desc_u (MSG_BYTE_COST, link_def_ov_ptr->byte_cost);
 
      print_tg_prop_delay (MSG_PROPAGATION_DELAY,
                           link_def_ov_ptr -> propagation_delay);
 
      print_desc_u (MSG_USER_DEFINED_1, link_def_ov_ptr->user_def_1);
      print_desc_u (MSG_USER_DEFINED_2, link_def_ov_ptr->user_def_2);
      print_desc_u (MSG_USER_DEFINED_3, link_def_ov_ptr->user_def_3);
 
      print_security (MSG_SECURITY, link_def_ov_ptr -> security);
 
   } /* endfor */
}
