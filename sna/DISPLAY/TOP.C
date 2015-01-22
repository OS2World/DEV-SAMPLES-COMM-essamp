/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : TOP.C                                                     */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format APPN Topology information returned by the */
/*              DISPLAY_APPN verb.                                          */
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
void DISP_TOP (TOPOLOGY_INFO_SECT far * top_ptr);
void DISP_A_TOP_NN (TOPOLOGY_NN_OVERLAY far * top_nn_ov_ptr, unsigned i);
void DISP_A_TG (TG_OVERLAY far * tg_ov_ptr, unsigned i, unsigned j);

void DISP_TOP (TOPOLOGY_INFO_SECT far * top_ptr)
{
   unsigned i,j;
   unsigned nn_count;                       /* Counter for NN overlays */
   unsigned tg_count;                       /* Counter for TG overlays */
   TOPOLOGY_NN_OVERLAY far * top_nn_ov_ptr; /* Pointer to NN overlay   */
   TG_OVERLAY far * tg_ov_ptr;              /* Pointer to TG overlay   */

   /*--------------------------------------------------*/
   /* Display each network node overlay                */
   /*--------------------------------------------------*/
   print_total (MSG_NUMBER_TOP_NN_TOTAL,
                top_ptr->total_nns,
                top_ptr->num_nns);
   nn_count = top_ptr->num_nns;
   for (top_nn_ov_ptr = (TOPOLOGY_NN_OVERLAY far *)
           ((UCHAR far *)top_ptr + top_ptr->topology_init_sect_len),
        i = 0; i < nn_count; i++,
        top_nn_ov_ptr = (TOPOLOGY_NN_OVERLAY far *)
           ((UCHAR far *)top_nn_ov_ptr + top_nn_ov_ptr->topology_entry_len)) {

      DISP_A_TOP_NN(top_nn_ov_ptr, i);    /* Display info about the NN */

      /*--------------------------------------------------*/
      /* Display each TG overlay in this NN overlay       */
      /*--------------------------------------------------*/
      tg_count = top_nn_ov_ptr->num_tgs;
      print_desc_u (MSG_NUMBER_TGS, tg_count);
      for (tg_ov_ptr = (TG_OVERLAY far *)
              ((UCHAR far *)top_nn_ov_ptr +
               top_nn_ov_ptr->topology_nn_info_len +
               (unsigned long)sizeof(top_nn_ov_ptr->topology_entry_len)),
           j = 0; j < tg_count; j++,
           tg_ov_ptr = (TG_OVERLAY far *)
              ((UCHAR far *)tg_ov_ptr + tg_ov_ptr->tg_entry_len)) {

         DISP_A_TG(tg_ov_ptr, i, j);   /* display info about the TG overlay */

      } /* endfor (TG overlay) */

   } /* endfor (NN overlay) */
}

/*--------------------------------------------------------------------------*/
/* Subroutine to display a network node overlay                             */
/*--------------------------------------------------------------------------*/
void DISP_A_TOP_NN (TOPOLOGY_NN_OVERLAY far * top_nn_ov_ptr, unsigned i)
{
   print_index1 (i+1);

   print_desc_ebcdic (MSG_TOPOLOGY_NNCP_NAME,
                      sizeof(top_nn_ov_ptr->fq_nncp_name),
                      top_nn_ov_ptr->fq_nncp_name);

   print_desc_u (MSG_ROUTE_RESIST, top_nn_ov_ptr->route_resist);

   print_desc_s (MSG_CONGESTED,
                 (top_nn_ov_ptr->nncp_congested == AP_NO) ?
                 MSG_NO : MSG_YES);

   print_desc_s (MSG_QUIESCING,
                 (top_nn_ov_ptr->nncp_quiescing == AP_NO) ?
                 MSG_NO : MSG_YES);

   print_desc_s (MSG_ISR_DEPLETED,
                 (top_nn_ov_ptr->nncp_isr_depleted == AP_NO) ?
                 MSG_NO : MSG_YES) ;
}

/*--------------------------------------------------------------------------*/
/* Subroutine to display a transmission group overlay                       */
/*--------------------------------------------------------------------------*/
void DISP_A_TG (TG_OVERLAY far * tg_ov_ptr, unsigned i, unsigned j)
{
   print_index2 (i+1, j+1);

   print_desc_ebcdic (MSG_TG_NNCP_NAME,
                      sizeof(tg_ov_ptr->fq_nncp_name),
                      tg_ov_ptr->fq_nncp_name);

   print_desc_u (MSG_TG_NUMBER, tg_ov_ptr -> tg_number);

   print_desc (MSG_TG_NODE_TYPE);
   switch (tg_ov_ptr -> node_type) {
   case AP_REAL:    myprintf(MSG_REAL_NODE);    break;
   case AP_VIRTUAL: myprintf(MSG_VIRTUAL_NODE); break;
   default:         myprintf(MSG_ERROR_VALUE, tg_ov_ptr -> node_type);
   } /* endswitch */

   print_desc_s (MSG_QUIESCING,
                 (tg_ov_ptr -> quiescing == AP_NO) ? MSG_NO : MSG_YES);

   print_desc_s (MSG_TG_TOPOLOGY,
                 (tg_ov_ptr -> network_topology == AP_NO) ?
                 MSG_TOPO_LOCAL : MSG_TOPO_NETWORK);

   print_tg_capacity(MSG_EFFECTIVE_CAPACITY,
                     tg_ov_ptr -> eff_capacity);

   print_desc_u (MSG_CONNECT_COST, tg_ov_ptr->conn_cost);
   print_desc_u (MSG_BYTE_COST, tg_ov_ptr->byte_cost);

   print_tg_prop_delay (MSG_PROPAGATION_DELAY,
                        tg_ov_ptr -> propagation_delay);

   print_desc_u (MSG_USER_DEFINED_1, tg_ov_ptr->user_def_1);
   print_desc_u (MSG_USER_DEFINED_2, tg_ov_ptr->user_def_2);
   print_desc_u (MSG_USER_DEFINED_3, tg_ov_ptr->user_def_3);

   print_security (MSG_SECURITY, tg_ov_ptr -> security);

}
