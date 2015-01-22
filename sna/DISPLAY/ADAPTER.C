/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : ADAPTER.C                                                 */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format Adapter information returned by the       */
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
#include "DISPLAY.H"      /* global typedefs, prototypes, and #includes  */
 
void DISP_ADAPTER (ADAPTER_INFO_SECT far * ad_ptr)
{
   unsigned i;
   unsigned adapter_count;                         /* Counter for adapters  */
   ADAPTER_OVERLAY far * ad_ov_ptr;         /* Pointer to adapter overlay   */
 
   /*-----------------------------------------------------------------------*/
   /* Display each adapter overlay                                          */
   /*-----------------------------------------------------------------------*/
   print_total (MSG_NUMBER_ADAPTERS_TOTAL,
                ad_ptr->total_adapters,
                ad_ptr->num_adapters);
   adapter_count = ad_ptr->num_adapters;
   for (ad_ov_ptr = (ADAPTER_OVERLAY far *)
           ((UCHAR far *)ad_ptr + ad_ptr->adapter_init_sect_len),
        i = 0; i < adapter_count; i++,
        ad_ov_ptr = (ADAPTER_OVERLAY far *)
           ((UCHAR far *)ad_ov_ptr + ad_ov_ptr->adapter_entry_len)) {
 
      print_index1 (i+1);
 
      print_desc_ls (MSG_DLC_NAME,
                     sizeof(ad_ov_ptr->dlc_name),
                     ad_ov_ptr->dlc_name);
 
      if (!(memcmp(ad_ov_ptr->dlc_name, "X25DLC", 6)))
        ad_ov_ptr->adapter_number++;
      print_desc_u (MSG_ADAPTER_NUMBER, ad_ov_ptr->adapter_number);
 
      print_desc (MSG_LINK_STATION_ROLE);
      switch (ad_ov_ptr->ls_role) {
      case AP_NEGOTIABLE: myprintf(MSG_NEGOTIABLE); break;
      case AP_PRIMARY:    myprintf(MSG_PRIMARY);    break;
      case AP_SECONDARY:  myprintf(MSG_SECONDARY);  break;
      default:
         myprintf(MSG_ERROR_VALUE, ad_ov_ptr->ls_role);
      } /* endswitch */
 
      print_desc (MSG_LINE_TYPE);
      switch (ad_ov_ptr->line_type) {
      case AP_SWITCHED:    myprintf(MSG_SWITCHED);    break;
      case AP_NONSWITCHED: myprintf(MSG_NONSWITCHED); break;
      default:
         myprintf(MSG_ERROR_VALUE, ad_ov_ptr->line_type);
      } /* endswitch */
 
      print_desc_s (MSG_LIMITED_RESOURCE,
                    (ad_ov_ptr->lim_res == AP_NO) ? MSG_NO : MSG_YES);
 
      print_desc_u (MSG_LIMITED_RESOURCE_TIMEOUT, ad_ov_ptr->lim_res_timeout);
      print_desc_u (MSG_MAX_BTU_SIZE, ad_ov_ptr->max_btu_size);
      print_desc_u (MSG_RECEIVE_WINDOW, ad_ov_ptr->rcv_window);
      print_desc_u (MSG_SEND_WINDOW, ad_ov_ptr->send_window);
      print_desc_u (MSG_MAX_LINK_STATIONS, ad_ov_ptr->max_ls_used);
 
      print_desc_s (MSG_ASYNC_BALANCED_MODE_USED,
                    (ad_ov_ptr->abm_support == AP_NO) ? MSG_NO : MSG_YES);
 
      print_desc (MSG_EFFECTIVE_CAPACITY);
      myprintf ("%lu %s\n", ad_ov_ptr->eff_capacity, MSG_BPS);
 
      print_desc_u (MSG_CONNECT_COST, ad_ov_ptr->conn_cost);
      print_desc_u (MSG_BYTE_COST, ad_ov_ptr->byte_cost);
 
      print_tg_prop_delay (MSG_PROPAGATION_DELAY,
                           ad_ov_ptr -> propagation_delay);
 
      print_desc_u (MSG_USER_DEFINED_1, ad_ov_ptr->user_def_1);
      print_desc_u (MSG_USER_DEFINED_2, ad_ov_ptr->user_def_2);
      print_desc_u (MSG_USER_DEFINED_3, ad_ov_ptr->user_def_3);
 
      print_security (MSG_SECURITY, ad_ov_ptr -> security);
   } /* endfor */
}
