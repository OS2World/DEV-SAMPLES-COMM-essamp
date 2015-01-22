/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : CN.C                                                      */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format APPN Connection Network information       */
/*              returned by the DISPLAY_APPN verb.                          */
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

/*--------------------------------------------------------------------------*/
/*                      Local Function Prototypes                           */
/*--------------------------------------------------------------------------*/
void DISP_CN (CN_INFO_SECT far * cn_ptr);
void DISP_A_CN (CN_OVERLAY far * cn_ov_ptr, unsigned i);

void DISP_CN (CN_INFO_SECT far * cn_ptr)
{
   unsigned i,j;
   unsigned cn_count;                    /* Counter for CN overlays  */
   unsigned cna_count;                   /* Counter for CNA overlays */
   CN_OVERLAY far * cn_ov_ptr;           /* Pointer to CN overlay         */
   CNA_OVERLAY far * cna_ov_ptr;         /* Pointer to CN adapter overlay */

   /*--------------------------------------------------*/
   /* Display each connection network overlay          */
   /*--------------------------------------------------*/
   print_total (MSG_NUMBER_CN_TOTAL,
                cn_ptr->total_cn,
                cn_ptr->num_cn);
   cn_count = cn_ptr->num_cn;
   for (cn_ov_ptr = (CN_OVERLAY far *)
           ((UCHAR far *)cn_ptr + cn_ptr->cn_init_sect_len),
        i = 0; i < cn_count; i++,
        cn_ov_ptr = (CN_OVERLAY far *)
           ((UCHAR far *)cn_ov_ptr + cn_ov_ptr->cn_entry_len)) {

      DISP_A_CN (cn_ov_ptr, i);

      /*-----------------------------------------------------------------*/
      /* Display each adapter overlay in this connection network overlay */
      /*-----------------------------------------------------------------*/
      cna_count = cn_ov_ptr->num_adapters;
      print_desc_u (MSG_NUMBER_CN_ADAPTERS, cna_count);
      for (cna_ov_ptr = (CNA_OVERLAY far *)
              ((UCHAR far *)cn_ov_ptr +
               cn_ov_ptr->cn_info_len +
               sizeof(cn_ov_ptr->cn_entry_len)),
           j = 0; j < cna_count; j++,
           cna_ov_ptr = (CNA_OVERLAY far *)
              ((UCHAR far *)cna_ov_ptr + cna_ov_ptr->cna_entry_len)) {

         print_index2 (i+1, j+1);

         print_desc_ls (MSG_DLC_NAME,
                        sizeof(cna_ov_ptr->dlc_name), cna_ov_ptr->dlc_name);
         print_desc_u (MSG_ADAPTER_NUMBER, cna_ov_ptr->adapter_num);

      } /* endfor (Connection network adapter overlays) */
   } /* endfor (Connection network overlays) */
}

/*--------------------------------------------------------------------------*/
/* Subroutine to display a connection network overlay                       */
/*--------------------------------------------------------------------------*/
void DISP_A_CN(CN_OVERLAY far * cn_ov_ptr, unsigned i)
{
   print_index1 (i+1);

   print_desc_ebcdic (MSG_CONNECTION_NETWORK_NAME,
                      sizeof(cn_ov_ptr->cn_name),
                      cn_ov_ptr->cn_name);

   print_desc (MSG_EFFECTIVE_CAPACITY);
   myprintf ("%lu %s\n", cn_ov_ptr->eff_capacity, MSG_BPS);

   print_desc_u (MSG_CONNECT_COST, cn_ov_ptr->conn_cost);
   print_desc_u (MSG_BYTE_COST, cn_ov_ptr->byte_cost);

   print_tg_prop_delay (MSG_PROPAGATION_DELAY,
                        cn_ov_ptr -> propagation_delay);

   print_desc_u (MSG_USER_DEFINED_1, cn_ov_ptr->user_def_1);
   print_desc_u (MSG_USER_DEFINED_2, cn_ov_ptr->user_def_2);
   print_desc_u (MSG_USER_DEFINED_3, cn_ov_ptr->user_def_3);

   print_security (MSG_SECURITY, cn_ov_ptr -> security);
}
