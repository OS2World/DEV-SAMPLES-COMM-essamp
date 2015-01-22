/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : DIR.C                                                     */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format APPN Directory information returned by    */
/*              the DISPLAY_APPN verb.                                      */
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

void DISP_A_LU (LUN_OVERLAY far * lu_ov_ptr, unsigned i, unsigned j,
                int display_cp_name_flag);

void DISP_DIR (DIRECTORY_INFO_SECT far * dir_ptr)
{
   unsigned i,j;
   unsigned node_count;              /* Number of NN and EN overlays */
   unsigned lu_count;                /* Number of LU overlays        */
   DIR_NN_OVERLAY far * nn_ov_ptr;        /* Pointer to current nn overlay  */
   DIR_EN_OVERLAY far * en_ov_ptr;        /* Pointer to current en overlay  */
   LUN_OVERLAY    far * lu_ov_ptr;        /* Pointer to current lu overlay  */

   print_desc_u (MSG_NUMBER_DIR_ENTRIES, dir_ptr->num_entries);

   /*-----------------------------------------------------------------------*/
   /* Display each network node overlay                                     */
   /*-----------------------------------------------------------------------*/
   print_total (MSG_SERVING_NN_TOTAL,
                dir_ptr->total_nns,
                dir_ptr->num_nns);
   node_count = dir_ptr->num_nns;
   for (nn_ov_ptr = (DIR_NN_OVERLAY far *)
           ((UCHAR far *)dir_ptr + dir_ptr->directory_init_sect_len),
        i = 0; i < node_count; i++,
        nn_ov_ptr = (DIR_NN_OVERLAY far *)
           ((UCHAR far *)nn_ov_ptr + nn_ov_ptr->dir_nn_entry_len)) {

      print_index1 (i+1);

      print_desc_ebcdic (MSG_SERVING_NN_CP_NAME,
                         sizeof(nn_ov_ptr->fq_nncp_name),
                         nn_ov_ptr->fq_nncp_name);

      /*--------------------------------------------------------------------*/
      /* Display each LU overlay in this network node overlay               */
      /*--------------------------------------------------------------------*/
      lu_count = nn_ov_ptr->num_lus;
      print_desc_u (MSG_NUMBER_ASSOC_LUS, lu_count);
      for (lu_ov_ptr = (LUN_OVERLAY far *)
              ((UCHAR far *)nn_ov_ptr +
               nn_ov_ptr->dir_nn_info_len +
               sizeof(nn_ov_ptr->dir_nn_entry_len)),
           j = 0; j < lu_count; j++,
           lu_ov_ptr = (LUN_OVERLAY far *)
              ((UCHAR far *)lu_ov_ptr + lu_ov_ptr->lun_entry_len)) {

         DISP_A_LU (lu_ov_ptr, i, j, 1);    /* Display info about the LU    */
                                            /* including the owning CP name */

      } /* endfor (LU overlays) */

   } /* endfor (NN overlays) */

/* Put blank line between network node entries and end node entries         */
   if (node_count) myprintf(MSG_CRLF);

   /*-----------------------------------------------------------------------*/
   /* Display each local and adjacent node overlay                          */
   /*-----------------------------------------------------------------------*/
   print_total (MSG_ORPHAN_TOTAL,
                dir_ptr->total_ens,
                dir_ptr->num_ens);
   node_count = dir_ptr->num_ens;
   for (en_ov_ptr = (DIR_EN_OVERLAY far *)nn_ov_ptr,
        i = 0; i < node_count; i++,
        en_ov_ptr = (DIR_EN_OVERLAY far *)
           ((UCHAR far *)en_ov_ptr + en_ov_ptr->dir_en_entry_len)) {

      print_index1 (i+1);

      print_desc_ebcdic (MSG_ORPHAN_CP_NAME,
                         sizeof(en_ov_ptr->fq_encp_name),
                         en_ov_ptr->fq_encp_name);

      /*--------------------------------------------------------------------*/
      /* Display each LU overlay in this end node overlay                   */
      /*--------------------------------------------------------------------*/
      lu_count = en_ov_ptr->num_lus;
      print_desc_u (MSG_NUMBER_ASSOC_LUS, lu_count);
      for (lu_ov_ptr = (LUN_OVERLAY far *)
              ((UCHAR far *)en_ov_ptr +
               en_ov_ptr->dir_en_info_len +
               sizeof(en_ov_ptr->dir_en_entry_len)),
           j = 0; j < lu_count; j++,
           lu_ov_ptr = (LUN_OVERLAY far *)
              ((UCHAR far *)lu_ov_ptr + lu_ov_ptr->lun_entry_len)) {

         DISP_A_LU (lu_ov_ptr, i, j, 0);    /* Display info about the LU    */
                                            /* but don't display the owning */
                                            /* CP name again                */

      } /* endfor (LU overlays) */

   } /* endfor (EN overlays) */
}

/*--------------------------------------------------------------------------*/
/* Subroutine to display an LU overlay                                      */
/*--------------------------------------------------------------------------*/
void DISP_A_LU (LUN_OVERLAY far * lu_ov_ptr, unsigned i, unsigned j,
                int display_cp_name_flag)
{
   int k;

   print_index2 (i+1, j+1);

   print_desc (MSG_LU_NAME);
   ebcdic_to_ascii (lu_ov_ptr->fqlu_name, sizeof(lu_ov_ptr->fqlu_name));
   for (k = sizeof(lu_ov_ptr->fqlu_name) - 1;
        (k >= 0) && (lu_ov_ptr->fqlu_name[k] == ' ');
        k--) {
      lu_ov_ptr->fqlu_name[k] = '\0';  /* Delete trailing blanks            */
   } /* endfor */
   myprintf("%.*s", sizeof(lu_ov_ptr->fqlu_name), lu_ov_ptr->fqlu_name);

   if (lu_ov_ptr->wildcard_entry == AP_YES) myprintf (MSG_WILDCARD);
   myprintf (MSG_CRLF);

   if (display_cp_name_flag) {         // Display the owning CP name
      print_desc_ebcdic (MSG_OWNING_CP_NAME,
                         sizeof(lu_ov_ptr->fq_nncp_name),
                         lu_ov_ptr->fq_nncp_name);
   } /* endif */

   print_desc (MSG_LU_ENTRY_TYPE);
   switch (lu_ov_ptr->lu_entry_type) {
   case AP_HOME:     myprintf (MSG_HOME);     break;
   case AP_REGISTER: myprintf (MSG_REGISTER); break;
   case AP_CACHE:    myprintf (MSG_CACHE);    break;
   default:          myprintf (MSG_ERROR_VALUE, lu_ov_ptr->lu_entry_type);
   } /* endswitch */
}
