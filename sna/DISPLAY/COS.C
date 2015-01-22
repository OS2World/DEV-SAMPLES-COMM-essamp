/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : COS.C                                                     */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format APPN Class of Service information         */
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
void DISP_COS (COS_INFO_SECT far * cos_ptr);
void DISP_A_COS_NODE_ROW (COS_NODE_ROW_OVERLAY far * cos_node_ov_ptr,
                          unsigned i, unsigned j);
void DISP_A_COS_TG_ROW (COS_TG_ROW_OVERLAY far * cos_tg_ov_ptr,
                        unsigned i, unsigned k);

void DISP_COS (COS_INFO_SECT far * cos_ptr)
{
   unsigned i,j;
   unsigned cos_count;              /* Counter for COS overlays      */
   unsigned node_count;             /* Counter for NODE ROW overlays */
   unsigned tg_count;               /* Counter for TG ROW overlays   */
   COS_OVERLAY          far * cos_ov_ptr;      /* Pointer to COS overlay    */
   COS_NODE_ROW_OVERLAY far * cos_node_ov_ptr; /*  to COS node row overlay  */
   COS_TG_ROW_OVERLAY   far * cos_tg_ov_ptr;   /*  to COS TG row overlay    */

   /*--------------------------------------------------*/
   /* Display each Class of Service overlay            */
   /*--------------------------------------------------*/
   print_total (MSG_NUMBER_COS_TOTAL,
                cos_ptr->total_cos,
                cos_ptr->num_cos);
   cos_count = cos_ptr->num_cos;
   for (cos_ov_ptr = (COS_OVERLAY far *)
           ((UCHAR far *)cos_ptr + cos_ptr->cos_init_sect_len),
        i = 0; i < cos_count; i++,
        cos_ov_ptr = (COS_OVERLAY far *)
           ((UCHAR far *)cos_ov_ptr + cos_ov_ptr->cos_entry_len)) {

      print_index1 (i+1);

      print_desc_ebcdic (MSG_COS_NAME,
                         sizeof(cos_ov_ptr->cos_name),
                         cos_ov_ptr->cos_name);

      print_desc (MSG_TRANSMISSION_PRIORITY);
      switch (cos_ov_ptr->trans_priority) {
      case AP_NETWORK: myprintf (MSG_NETWORK); break;
      case AP_HIGH:    myprintf (MSG_HIGH);    break;
      case AP_MEDIUM:  myprintf (MSG_MEDIUM);  break;
      case AP_LOW:     myprintf (MSG_LOW);     break;
      default: myprintf (MSG_ERROR_VALUE, cos_ov_ptr->trans_priority);
      } /* endswitch */

      print_desc_u (MSG_NUMBER_NODE_ROWS, cos_ov_ptr->num_of_node_rows);
      print_desc_u (MSG_NUMBER_TG_ROWS, cos_ov_ptr->num_of_tg_rows);

      /*---------------------------------------------------*/
      /* Display each node row overlay in this COS overlay */
      /*---------------------------------------------------*/
      node_count = cos_ov_ptr->num_of_node_rows;
      for (cos_node_ov_ptr = (COS_NODE_ROW_OVERLAY far *)
              ((UCHAR far *)cos_ov_ptr +
               cos_ov_ptr->cos_info_len +
               sizeof(cos_ov_ptr->cos_entry_len)),
           j = 0; j < node_count; j++,
           cos_node_ov_ptr = (COS_NODE_ROW_OVERLAY far *)
              ((UCHAR far *)cos_node_ov_ptr +
               cos_node_ov_ptr->cos_node_row_entry_len)) {

         DISP_A_COS_NODE_ROW(cos_node_ov_ptr, i, j);
      } /* endfor */

      /*--------------------------------------------------*/
      /* Display each TG row overlay in this COS overlay  */
      /*--------------------------------------------------*/
      tg_count = cos_ov_ptr->num_of_tg_rows;
      for (cos_tg_ov_ptr = (COS_TG_ROW_OVERLAY far *)cos_node_ov_ptr,
           j = 0; j < tg_count; j++,
           cos_tg_ov_ptr = (COS_TG_ROW_OVERLAY far *)
              ((UCHAR far *)cos_tg_ov_ptr +
               cos_tg_ov_ptr->cos_tg_row_entry_len)) {

        DISP_A_COS_TG_ROW(cos_tg_ov_ptr, i, j);
      } /* endfor */
   } /* endfor (COS overlays) */
}

/*--------------------------------------------------------------------------*/
/* Subroutine to display a COS node row overlay                             */
/*--------------------------------------------------------------------------*/
void DISP_A_COS_NODE_ROW (COS_NODE_ROW_OVERLAY far * cos_node_ov_ptr,
                          unsigned i, unsigned j)
{
   print_index2 (i+1, j+1);
   print_desc_u (MSG_NODE_ROW_WEIGHT, cos_node_ov_ptr->weight);

   print_desc_s (MSG_CONGESTION_MIN,
                 (cos_node_ov_ptr->congestion_min == AP_NO) ?
                 MSG_NO : MSG_YES);

   print_desc_s (MSG_CONGESTION_MAX,
                 (cos_node_ov_ptr->congestion_max == AP_NO) ?
                 MSG_NO : MSG_YES);

   print_desc_u (MSG_ROUTE_ADDITION_RES_MIN,
                 cos_node_ov_ptr->route_add_res_min);
   print_desc_u (MSG_ROUTE_ADDITION_RES_MAX,
                 cos_node_ov_ptr->route_add_res_max);
}

/*--------------------------------------------------------------------------*/
/* Subroutine to display a COS transmission group row overlay               */
/*--------------------------------------------------------------------------*/
void DISP_A_COS_TG_ROW (COS_TG_ROW_OVERLAY far * cos_tg_ov_ptr,
                        unsigned i, unsigned j)
{
   print_index2 (i+1, j+1);
   print_desc_u (MSG_TG_ROW_WEIGHT, cos_tg_ov_ptr->weight);

   print_desc_u (MSG_COST_PER_TIME_MIN, cos_tg_ov_ptr->cost_per_time_min);
   print_desc_u (MSG_COST_PER_TIME_MAX, cos_tg_ov_ptr->cost_per_time_max);
   print_desc_u (MSG_COST_PER_BYTE_MIN, cos_tg_ov_ptr->cost_per_byte_min);
   print_desc_u (MSG_COST_PER_BYTE_MAX, cos_tg_ov_ptr->cost_per_byte_max);

   print_security (MSG_SECURITY_MIN, cos_tg_ov_ptr -> security_min);
   print_security (MSG_SECURITY_MAX, cos_tg_ov_ptr -> security_max);

   print_tg_prop_delay (MSG_PROPAGATION_DELAY_MIN,
                        cos_tg_ov_ptr -> propagation_delay_min);
   print_tg_prop_delay (MSG_PROPAGATION_DELAY_MAX,
                        cos_tg_ov_ptr -> propagation_delay_max);

   print_tg_capacity (MSG_EFFECTIVE_CAPACITY_MIN,
                      cos_tg_ov_ptr -> eff_capacity_min);
   print_tg_capacity (MSG_EFFECTIVE_CAPACITY_MAX,
                      cos_tg_ov_ptr -> eff_capacity_max);

   print_desc_u (MSG_USER_DEFINED_1_MIN, cos_tg_ov_ptr->user_def_1_min);
   print_desc_u (MSG_USER_DEFINED_1_MAX, cos_tg_ov_ptr->user_def_1_max);
   print_desc_u (MSG_USER_DEFINED_2_MIN, cos_tg_ov_ptr->user_def_2_min);
   print_desc_u (MSG_USER_DEFINED_2_MAX, cos_tg_ov_ptr->user_def_2_max);
   print_desc_u (MSG_USER_DEFINED_3_MIN, cos_tg_ov_ptr->user_def_3_min);
   print_desc_u (MSG_USER_DEFINED_3_MAX, cos_tg_ov_ptr->user_def_3_max);
}
