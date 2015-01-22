/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : LU_DEF.C                                                  */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format LU Definitions information returned by    */
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

void DISP_LU_DEF (LU_DEF_INFO_SECT far * lu_def_ptr)
{
   unsigned i;
   unsigned lu_count;                         /* Counter for lu's */
   LU_DEF_OVERLAY far * lu_def_ov_ptr;        /* Pointer to lu_def overlay  */

   /*-----------------------------------------------------------------------*/
   /* Display each LU definition overlay                                    */
   /*-----------------------------------------------------------------------*/
   print_total (MSG_NUMBER_LU_TOTAL,
                lu_def_ptr->total_lu_def,
                lu_def_ptr->num_lu_def);
   lu_count = lu_def_ptr->num_lu_def;
   for (lu_def_ov_ptr = (LU_DEF_OVERLAY far *)
           ((UCHAR far *)lu_def_ptr + lu_def_ptr->lu_def_init_sect_len),
        i = 0; i < lu_count; i++,
        lu_def_ov_ptr = (LU_DEF_OVERLAY far *)
           ((UCHAR far *)lu_def_ov_ptr + lu_def_ov_ptr->lu_def_entry_len)) {

      print_index1 (i+1);

      print_desc_ebcdic (MSG_LU_NAME,
                         sizeof(lu_def_ov_ptr->lu_name),
                         lu_def_ov_ptr->lu_name);
      print_desc_ls (MSG_LU_ALIAS,
                     sizeof(lu_def_ov_ptr->lu_alias),
                     lu_def_ov_ptr->lu_alias);
      print_desc_02x (MSG_LOCAL_ADDR, lu_def_ov_ptr->lu_nau_addr);

   } /* endfor */
}
