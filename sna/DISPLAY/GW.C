/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : GW.C                                                      */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format SNA Gateway information returned by the   */
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

void DISP_GW (GW_INFO_SECT far * gw_ptr)
{
   GW_OVERLAY far * gw_ov_ptr;            /* pointer to gw overlay          */
   unsigned i;                            /* loop variable                  */
   unsigned short gw_count;               /* counter for number of gateways */

   /*-----------------------------------------------------------------------*/
   /* Display each gateway LU overlay                                       */
   /*-----------------------------------------------------------------------*/
   gw_count = gw_ptr->num_gw_lus;
   print_desc_u (MSG_NUMBER_GATEWAY_LUS, gw_count);
   for (gw_ov_ptr = (GW_OVERLAY far *)
           ((UCHAR far *)gw_ptr + gw_ptr->gw_init_sect_len),
        i = 0; i < gw_count; i++,
        gw_ov_ptr = (GW_OVERLAY far *)
           ((UCHAR far *)gw_ov_ptr + gw_ov_ptr->gw_entry_len)) {

      print_index1 (i+1);

      print_desc_ebcdic (MSG_WORKSTATION_LU_NAME,
                         sizeof(gw_ov_ptr->ws_lu_name),
                         gw_ov_ptr->ws_lu_name);
      print_desc_ebcdic (MSG_WORKSTATION_PU_NAME,
                         sizeof(gw_ov_ptr->ws_pu_name),
                         gw_ov_ptr->ws_pu_name);

      print_desc (MSG_WS_POOL_CLASS);
      if (gw_ov_ptr->ws_pool_class == 0)
         myprintf (MSG_DEDICATED_LU);
      else
         print_u (gw_ov_ptr->ws_pool_class);

      print_desc (MSG_WS_LOCAL_ADDR);
      if (gw_ov_ptr->ws_local_addr == 0)
         myprintf (MSG_NOT_ASSIGNED);
      else
         print_02x (gw_ov_ptr->ws_local_addr);

      print_desc_02x (MSG_HOST_LOCAL_ADDR, gw_ov_ptr->host_local_addr);

      print_desc (MSG_WORKSTATION_LU_TYPE);
      switch (gw_ov_ptr->ws_lu_type) {
      case AP_LU62:    myprintf (MSG_LU_TYPE_62); break;
      case AP_UNKNOWN: myprintf (MSG_UNKNOWN);    break;
      default:         print_u (gw_ov_ptr->ws_lu_type);
      } /* endswitch */

      print_desc_ebcdic (MSG_HOST_LU_NAME,
                         sizeof(gw_ov_ptr->host_lu_name),
                         gw_ov_ptr->host_lu_name);

      print_desc_ls (MSG_WS_DLC_NAME,
                     sizeof(gw_ov_ptr->ws_dlc_name),
                     gw_ov_ptr->ws_dlc_name);
      print_desc_u (MSG_WS_ADAPTER_NUM, gw_ov_ptr->ws_adapter_num);

      print_desc_xs (MSG_WS_DEST_ADDR,
                     gw_ov_ptr->ws_dest_addr_len,
                     gw_ov_ptr->ws_dest_addr);

      print_desc_s (MSG_WS_LINK_ACTIVE,
                    (gw_ov_ptr->ws_link_act == AP_NO) ?
                    MSG_NO : MSG_YES);

      print_desc (MSG_SSCP_LU_SESS_STATE);
      if      (gw_ov_ptr->lu_cp_pend_term)   myprintf (MSG_DEACTIVATING);
      else if (gw_ov_ptr->lu_cp_pend_init)   myprintf (MSG_ACTIVATING);
      else if (gw_ov_ptr->lu_cp_act_offline) myprintf (MSG_ACTIVE_OFFLINE);
      else if (gw_ov_ptr->lu_cp_act_online)  myprintf (MSG_ACTIVE_ONLINE);
      else                                   myprintf (MSG_DEACTIVATED);

      print_desc (MSG_LU_LU_SESS_STATE);
      if      (gw_ov_ptr->lu_lu_pend_term)   myprintf (MSG_DEACTIVATING);
      else if (gw_ov_ptr->lu_lu_pend_init)   myprintf (MSG_ACTIVATING);
      else if (gw_ov_ptr->lu_lu_act)         myprintf (MSG_ACTIVATED);
      else                                   myprintf (MSG_DEACTIVATED);

   } /* endfor */
}
