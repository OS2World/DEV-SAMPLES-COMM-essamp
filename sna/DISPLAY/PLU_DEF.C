/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : PLU_DEF.C                                                 */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format Partner LU Definitions information        */
/*              return by the DISPLAY verb.                                 */
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

void DISP_PLU_DEF (PLU_DEF_INFO_SECT far * plu_def_ptr)
{
   unsigned i;
   unsigned plu_count;                     /* Counter for plu definitions */
   PLU_DEF_OVERLAY far * plu_def_ov_ptr;   /* Pointer to partner LU overlay */
   ALT_ALIAS_OVERLAY far * alt_plu_alias_ptr; /* Ptr to alternate PLU alias */

   /*-----------------------------------------------------------------------*/
   /* Display each partner LU overlay                                       */
   /*-----------------------------------------------------------------------*/
   print_total (MSG_NUMBER_PLU_TOTAL,
                plu_def_ptr->total_plu_def,
                plu_def_ptr->num_plu_def);
   plu_count = plu_def_ptr->num_plu_def;
   for (plu_def_ov_ptr = (PLU_DEF_OVERLAY far *)
           ((UCHAR far *)plu_def_ptr + plu_def_ptr->plu_def_init_sect_len),
        i = 0; i < plu_count; i++,
        plu_def_ov_ptr = (PLU_DEF_OVERLAY far *)
           ((UCHAR far *)plu_def_ov_ptr + plu_def_ov_ptr->plu_def_entry_len)) {

      print_index1 (i+1);

      print_desc_ebcdic (MSG_PLU_NAME,
                         sizeof(plu_def_ov_ptr->fqplu_name),
                         plu_def_ov_ptr->fqplu_name);

      print_desc_ls (MSG_PLU_ALIAS,
                     sizeof(plu_def_ov_ptr->plu_alias),
                     plu_def_ov_ptr->plu_alias);

      if (plu_def_ov_ptr->alt_alias_flag == 0) {
         alt_plu_alias_ptr = (ALT_ALIAS_OVERLAY far *)
            ((UCHAR far *)plu_def_ov_ptr + sizeof(PLU_DEF_OVERLAY));
         while (plu_def_ov_ptr->num_of_alt_aliases > 0) {
            print_desc_ls (MSG_ALT_PLU_ALIAS,
                           sizeof(ALT_ALIAS_OVERLAY),
                           (UCHAR *)alt_plu_alias_ptr);
            --plu_def_ov_ptr->num_of_alt_aliases;
            ++alt_plu_alias_ptr;
         } /* endwhile */
      } /* endif */

      print_desc_ebcdic (MSG_PLU_UN_NAME,
                         sizeof(plu_def_ov_ptr->plu_uninterpreted_name),
                         plu_def_ov_ptr->plu_uninterpreted_name);

      print_desc_u (MSG_MAX_MC_LL_SENDSIZE, plu_def_ov_ptr->max_mc_ll_ssize);

      print_desc_s (MSG_CONV_SEC,
                    (plu_def_ov_ptr->conv_security == AP_NO) ?
                    MSG_NO : MSG_YES);

      print_desc_s (MSG_PARA_SESS,
                    (plu_def_ov_ptr->parallel_sess == AP_NO) ?
                    MSG_NOT_SUPPORTED : MSG_SUPPORTED);
   } /* endfor */
}
