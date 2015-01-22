/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : ISR.C                                                     */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format Intermediate Session Routing information  */
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

void DISP_ISR (ISR_INFO_SECT far * isr_ptr)
{
   unsigned i;
   unsigned isr_count;                         /* Counter for ISR overlays */
   ISR_OVERLAY far * isr_ov_ptr;         /* Pointer to current ISR overlay  */

   /*-----------------------------------------------------------------------*/
   /* Display each intermediate session overlay                             */
   /*-----------------------------------------------------------------------*/
   print_total (MSG_NUMBER_ISR_TOTAL,
                isr_ptr->total_isr,
                isr_ptr->num_isr);
   isr_count = isr_ptr->num_isr;
   for (isr_ov_ptr = (ISR_OVERLAY far *)
           ((UCHAR far *)isr_ptr + isr_ptr->isr_init_sect_len),
        i = 0; i < isr_count; i++,
        isr_ov_ptr = (ISR_OVERLAY far *)
           ((UCHAR far *)isr_ov_ptr + isr_ov_ptr->isr_entry_len)) {

      print_index1 (i+1);

      print_desc_ebcdic (MSG_PRI_CP_NAME,
                         sizeof(isr_ov_ptr->fq_pri_nncp_name),
                         isr_ov_ptr->fq_pri_nncp_name);

      print_desc (MSG_SEC_CP_NAME);
      ebcdic_to_ascii (isr_ov_ptr->fq_sec_nncp_name,
                       sizeof(isr_ov_ptr->fq_sec_nncp_name));
      if (isr_ov_ptr->fq_sec_nncp_name[0] == ' ') {
         /* During session activation, before receiving the BIND      */
         /* response, the secondary stage CP name is returned blank.  */
         myprintf (MSG_SEC_CP_NAME_UNKNOWN);
      } else {
         print_string (isr_ov_ptr->fq_sec_nncp_name,
                       sizeof(isr_ov_ptr->fq_sec_nncp_name));
      } /* endif */

      print_desc_ebcdic (MSG_PRI_LINK_NAME,
                         sizeof(isr_ov_ptr->pri_link_name),
                         isr_ov_ptr->pri_link_name);

      print_desc_ebcdic (MSG_SEC_LINK_NAME,
                         sizeof(isr_ov_ptr->sec_link_name),
                         isr_ov_ptr->sec_link_name);

      print_desc_xs (MSG_FQPCID_ID,
                     sizeof(isr_ov_ptr->fqpcid.unique_proc_id),
                     isr_ov_ptr->fqpcid.unique_proc_id);

      print_desc_ebcdic (MSG_FQPCID_CP_NAME,
                         isr_ov_ptr->fqpcid.fq_length,
                         isr_ov_ptr->fqpcid.fq_name);

   } /* endfor (ISR overlays) */
}
