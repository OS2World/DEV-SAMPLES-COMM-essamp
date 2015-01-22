/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : LINKS.C                                                   */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format Active Links information returned by the  */
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
 
extern BOOL appn;                       /*  Flag for APPN vs. APPC code     */
 
void DISP_LINKS (LINK_INFO_SECT far * link_ptr)
{
   LINK_OVERLAY far * link_ov_ptr;           /* pointer to link overlay     */
   unsigned i;                               /* loop variable               */
   unsigned short link_count;                /* counter for number of links */
 
   /*-----------------------------------------------------------------------*/
   /* Display each active link overlay                                      */
   /*-----------------------------------------------------------------------*/
   link_count = link_ptr->num_links;
   print_desc_u (MSG_NUMBER_LINKS, link_count);
   for (link_ov_ptr = (LINK_OVERLAY far *)
           ((UCHAR far *)link_ptr + link_ptr->link_init_sect_len),
        i = 0; i < link_count; i++,
        link_ov_ptr = (LINK_OVERLAY far *)
           ((UCHAR far *)link_ov_ptr + link_ov_ptr->link_entry_len)) {
 
      print_index1 (i+1);
 
      if (appn) {
         print_desc_ebcdic (MSG_LINK_NAME,
                            sizeof(link_ov_ptr->link_id),
                            link_ov_ptr->link_id);
      } else {
         print_desc_xs (MSG_LINK_ID,
                        sizeof(link_ov_ptr->link_id),
                        link_ov_ptr->link_id);
      } /* endif */
 
      print_desc_ls (MSG_DLC_NAME,
                     sizeof(link_ov_ptr->dlc_name),
                     link_ov_ptr->dlc_name);
 
      if (!(memcmp(link_ov_ptr->dlc_name, "X25DLC", 6)))
        {
        link_ov_ptr->adapter_num++;
        print_desc_u (MSG_ADAPTER_NUMBER, (USHORT)link_ov_ptr->adapter_num);
 
        if (link_ov_ptr->dest_addr[0] == 0xFF)
          {
          print_desc_xs (MSG_DEST_ADDR,
                         (link_ov_ptr->dest_addr_len)-1,
                         &link_ov_ptr->dest_addr[1]);
          }
        else
          {
          print_desc_ls (MSG_DEST_ADDR,
                         link_ov_ptr->dest_addr_len,
                         link_ov_ptr->dest_addr);
          }
        }
      else
        {
        print_desc_u (MSG_ADAPTER_NUMBER, (USHORT)link_ov_ptr->adapter_num);
 
        print_desc_xs (MSG_DEST_ADDR,
                       link_ov_ptr->dest_addr_len,
                       link_ov_ptr->dest_addr);
        }
 
      print_desc_s (MSG_LINK_ACTIVATION,
                    (link_ov_ptr -> inbound_outbound == AP_OUTBOUND) ?
                    MSG_LOCALLY : MSG_REMOTELY);
 
      print_desc (MSG_LINK_STATE);
      switch (link_ov_ptr -> state) {
      case AP_CONALS_PND:  myprintf (MSG_CONALS_PND);  break;
      case AP_XID_PND:     myprintf (MSG_XID_PND);     break;
      case AP_CONTACT_PND: myprintf (MSG_CONTACT_PND); break;
      case AP_CONTACTED:   myprintf (MSG_CONTACTED);   break;
      case AP_DISC_PND:    myprintf (MSG_DISC_PND);    break;
      case AP_DISC_RQ:     myprintf (MSG_DISC_RQ);     break;
      default:
         myprintf (MSG_ERROR_VALUE, link_ov_ptr->state);
      } /* endswitch */
 
      print_desc_s (MSG_DEACTIVATE_LOGICAL_LINK,
                    (link_ov_ptr -> deact_link_flag == AP_NOT_IN_PROGRESS) ?
                    MSG_NO : MSG_YES);
 
      print_desc_u (MSG_SESSIONS_ON_LINK, link_ov_ptr->num_sessions);
      print_desc_u (MSG_MAX_BTU_SIZE, link_ov_ptr->ru_size);
 
      if (appn) {
         print_desc_ebcdic (MSG_ADJACENT_CP_NAME,
                            sizeof(link_ov_ptr->adj_fq_cp_name),
                            link_ov_ptr->adj_fq_cp_name);
 
         print_desc (MSG_ADJACENT_NODE_TYPE);
         switch (link_ov_ptr -> adj_node_type) {
         case AP_LEARN: myprintf(MSG_LEARN); break;
         case AP_LEN:   myprintf(MSG_LEN);   break;
         case AP_NN:    myprintf(MSG_NN);    break;
         case AP_EN:    myprintf(MSG_EN);    break;
         default:
            myprintf(MSG_ERROR_VALUE, link_ov_ptr->adj_node_type);
         } /* endswitch */
 
         print_desc_s (MSG_CP_CP_SESSION_SUPPORT,
                       (link_ov_ptr -> cp_cp_sess_spt == AP_NO) ?
                       MSG_NO : MSG_YES);
 
         print_desc (MSG_CONNECTION_TYPE);
         switch (link_ov_ptr -> conn_type) {
         case AP_HOST_CONNECTION: myprintf (MSG_HOST_CONNECTION); break;
         case AP_PEER_CONNECTION: myprintf (MSG_PEER_CONNECTION); break;
         case AP_BOTH_CONNECTION: myprintf (MSG_BOTH_CONNECTION); break;
         default:
            myprintf (MSG_ERROR_VALUE, link_ov_ptr->conn_type);
         } /* endswitch */
 
         print_desc (MSG_LINK_STATION_ROLE);
         switch (link_ov_ptr -> ls_role) {
         case AP_NEGOTIABLE: myprintf(MSG_NEGOTIABLE); break;
         case AP_PRIMARY:    myprintf(MSG_PRIMARY);    break;
         case AP_SECONDARY:  myprintf(MSG_SECONDARY);  break;
         default:
            myprintf(MSG_ERROR_VALUE, link_ov_ptr->ls_role);
         } /* endswitch */
 
         print_desc (MSG_LINE_TYPE);
         switch (link_ov_ptr -> line_type) {
         case AP_SWITCHED:    myprintf(MSG_SWITCHED);    break;
         case AP_NONSWITCHED: myprintf(MSG_NONSWITCHED); break;
         default:
            myprintf(MSG_ERROR_VALUE, link_ov_ptr->line_type);
         } /* endswitch */
 
         print_desc_u (MSG_TG_NUMBER, link_ov_ptr->tg_number);
 
         print_desc (MSG_EFFECTIVE_CAPACITY);
         myprintf ("%lu %s\n", link_ov_ptr->eff_capacity, MSG_BPS);
 
         print_desc_u (MSG_CONNECT_COST, link_ov_ptr->conn_cost);
         print_desc_u (MSG_BYTE_COST, link_ov_ptr->byte_cost);
 
         print_tg_prop_delay (MSG_PROPAGATION_DELAY,
                              link_ov_ptr -> propagation_delay);
 
         print_desc_u (MSG_USER_DEFINED_1, link_ov_ptr->user_def_1);
         print_desc_u (MSG_USER_DEFINED_2, link_ov_ptr->user_def_2);
         print_desc_u (MSG_USER_DEFINED_3, link_ov_ptr->user_def_3);
 
         print_security (MSG_SECURITY, link_ov_ptr -> security);
      } /* endif */
   } /* endfor */
}
