/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : LU62.C                                                    */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format LU 6.2 information returned by the        */
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
 
extern BOOL appn;                       /*  Flag for APPN vs. APPC code     */
 
/*--------------------------------------------------------------------------*/
/*                      Local Function Prototypes                           */
/*--------------------------------------------------------------------------*/
void DISP_LU62 (LU62_INFO_SECT far *lu62_ptr);
void DISP_AN_LU (LU62_OVERLAY far * lu62_ov_ptr, unsigned i);
void DISP_A_PLU (PLU62_OVERLAY far * plu62_ov_ptr, unsigned i, unsigned j);
void DISP_A_MODE (MODE_OVERLAY far * mode_ov_ptr,
                  unsigned i, unsigned j, unsigned k);
 
void DISP_LU62 (LU62_INFO_SECT far * lu62_ptr)
{
   unsigned i,j,k;
   unsigned lu_count;                      /* Counter for LUs   */
   unsigned plu_count;                     /* Counter for partner LUs */
   unsigned mode_count;                    /* Counter for modes */
 
   LU62_OVERLAY  far * lu62_ov_ptr;        /* Pointer to current LU         */
   PLU62_OVERLAY far * plu62_ov_ptr;       /* Pointer to current partner LU */
   MODE_OVERLAY  far * mode_ov_ptr;        /* Pointer to current mode       */
 
   /*--------------------------------------------------------*/
   /* Display each LU 6.2 overlay                            */
   /*--------------------------------------------------------*/
   if (appn) {
      print_total (MSG_NUMBER_LUS,
                   lu62_ptr->total_lu62s, lu62_ptr->num_lu62s);
   } else {
      print_total (MSG_NUMBER_LUS, lu62_ptr->num_lu62s, lu62_ptr->num_lu62s);
      } /* endif */
   lu_count = lu62_ptr->num_lu62s;         /* Get the LU count */
   for (lu62_ov_ptr = (LU62_OVERLAY far *)
           ((UCHAR far *)lu62_ptr + lu62_ptr->lu62_init_sect_len),
        i = 0; i < lu_count; i++,
        lu62_ov_ptr = (LU62_OVERLAY far *)
           ((UCHAR far *)lu62_ov_ptr + lu62_ov_ptr->lu62_entry_len)) {
 
      DISP_AN_LU (lu62_ov_ptr, i);         /* Display info about LU */
 
      /*--------------------------------------------------------*/
      /* Display each partner LU overlay in this LU 6.2 overlay */
      /*--------------------------------------------------------*/
      plu_count = lu62_ov_ptr->num_plus;   /* Get partner LU count */
      print_desc_u (MSG_NUMBER_PLUS, plu_count);
      for (plu62_ov_ptr = (PLU62_OVERLAY far *)
              ((UCHAR far *)lu62_ov_ptr +
               lu62_ov_ptr->lu62_overlay_len +
               sizeof(lu62_ov_ptr->lu62_entry_len)),
           j = 0; j < plu_count; j++,
           plu62_ov_ptr = (PLU62_OVERLAY far *)
              ((UCHAR far *)plu62_ov_ptr + plu62_ov_ptr->plu62_entry_len)) {
 
         DISP_A_PLU (plu62_ov_ptr, i, j);  /* Display info about the PLU */
 
         /*--------------------------------------------------------*/
         /* Display each mode overlay in this partner LU overlay   */
         /*--------------------------------------------------------*/
         mode_count = plu62_ov_ptr->num_modes; /* Get mode count*/
         print_desc_u (MSG_NUMBER_MODES, mode_count);
         for (mode_ov_ptr = (MODE_OVERLAY far *)
                 ((UCHAR far *)plu62_ov_ptr +
                  plu62_ov_ptr->plu62_overlay_len +
                  sizeof(plu62_ov_ptr->plu62_entry_len)),
              k = 0; k < mode_count; k++,
              mode_ov_ptr = (MODE_OVERLAY far *)
                 ((UCHAR far *)mode_ov_ptr + mode_ov_ptr->mode_entry_len)) {
 
            DISP_A_MODE(mode_ov_ptr, i, j, k); /* Display mode info */
         } /* endfor (mode overlays) */
      } /* endfor (partner LU overlays) */
   } /* endfor (LU overlays) */
}
 
/*--------------------------------------------------------------------------*/
/* Subroutine to display an LU 6.2 overlay                                  */
/*--------------------------------------------------------------------------*/
void DISP_AN_LU (LU62_OVERLAY far * lu62_ov_ptr, unsigned i)
{
   print_index1 (i+1);
 
   print_desc_ebcdic (MSG_LU_NAME,
                      sizeof(lu62_ov_ptr->lu_name),
                      lu62_ov_ptr->lu_name);
   print_desc_ls (MSG_LU_ALIAS,
                  sizeof(lu62_ov_ptr->lu_alias),
                  lu62_ov_ptr->lu_alias);
   print_desc_ebcdic (MSG_FQLU_NAME,
                      sizeof(lu62_ov_ptr->fqlu_name),
                      lu62_ov_ptr->fqlu_name);
 
   if(lu62_ov_ptr->default_lu == AP_NO)
     print_desc_s (MSG_DEFAULT_LU, MSG_NO);
   else
     print_desc_s (MSG_DEFAULT_LU, MSG_YES);
 
   print_desc (MSG_LOCAL_ADDR);
   if (lu62_ov_ptr->lu_local_addr == 0)
       myprintf (MSG_INDEPENDENT);
   else
       print_u ((USHORT)lu62_ov_ptr->lu_local_addr);
 
   print_desc_u (MSG_SESS_LIM, lu62_ov_ptr->lu_sess_lim);
 
   print_desc (MSG_MAX_TPS);
   if (lu62_ov_ptr->max_tps == 0)
       myprintf (MSG_NO_LIMIT);
   else
       print_u ((USHORT)lu62_ov_ptr->max_tps);
 
   /* The LU type should always be LU6.2, represented as 0x06 */
   print_desc (MSG_LU_TYPE);
   if (lu62_ov_ptr->lu_type == 0x06) {
      myprintf (MSG_LU_TYPE_62);
   } else {
      myprintf (MSG_ERROR_VALUE, lu62_ov_ptr->lu_type);
   } /* endif */
}
 
/*--------------------------------------------------------------------------*/
/* Subroutine to display a partner LU overlay                               */
/*--------------------------------------------------------------------------*/
void DISP_A_PLU (PLU62_OVERLAY far * plu62_ov_ptr, unsigned i, unsigned j)
{  PCHAR msgadr;
 
   print_index2 (i+1, j+1);
 
   print_desc_ls (MSG_PLU_ALIAS,
                  sizeof(plu62_ov_ptr->plu_alias), plu62_ov_ptr->plu_alias);
 
   print_desc_ebcdic (MSG_PLU_UN_NAME,
                      sizeof(plu62_ov_ptr->plu_un_name),
                      plu62_ov_ptr->plu_un_name);
   print_desc_ebcdic (MSG_PLU_NAME,
                      sizeof(plu62_ov_ptr->fqplu_name),
                      plu62_ov_ptr->fqplu_name);
   print_desc_u (MSG_PLU_SESS_LIM, plu62_ov_ptr->plu_sess_lim);
 
   if(plu62_ov_ptr->par_sess_supp)
     msgadr =  MSG_SUPPORTED;
   else
     msgadr = MSG_NOT_SUPPORTED;
   print_desc_s (MSG_PARA_SESS, msgadr);
 
   if(plu62_ov_ptr->def_sess_sec)
     msgadr =  MSG_CONFIGURED;
   else
     msgadr = MSG_NOT_CONFIGURED;
   print_desc_s (MSG_SESS_SEC, msgadr);
 
   print_desc (MSG_CONV_SEC);
   if(plu62_ov_ptr->def_conv_sec)
     msgadr =  MSG_CONFIGURED;
   else
     msgadr = MSG_NOT_CONFIGURED;
   myprintf ("%s", msgadr);
   if (plu62_ov_ptr->def_conv_sec) {   /* If configured, print (in)act */
     if(plu62_ov_ptr->act_conv_sec)
       msgadr =  MSG_ACTIVE;
     else
       msgadr = MSG_NOT_ACTIVE;
     myprintf (", %s", msgadr);
   }
   myprintf (MSG_CRLF);
 
   print_desc (MSG_VERIFIED);
   if(plu62_ov_ptr->def_already_ver)
     msgadr =  MSG_CONFIGURED;
   else
     msgadr = MSG_NOT_CONFIGURED;
   myprintf ("%s", msgadr);
   if (plu62_ov_ptr->def_already_ver) { /* If configured, print (in)act */
     if(plu62_ov_ptr->act_already_ver)
       msgadr =  MSG_ACTIVE;
     else
       msgadr = MSG_NOT_ACTIVE;
     myprintf (", %s", msgadr);
   }
   myprintf (MSG_CRLF);
 
   if(plu62_ov_ptr->implicit_part == AP_NO)
     msgadr =  MSG_NO;
   else
     msgadr = MSG_YES;
   print_desc_s (MSG_IMPL_PARTNER, msgadr);
}
 
/*--------------------------------------------------------------------------*/
/* Subroutine to display a mode overlay                                     */
/*--------------------------------------------------------------------------*/
void DISP_A_MODE (MODE_OVERLAY far * mode_ov_ptr,
                  unsigned i, unsigned j, unsigned k)
{
   PCHAR msgadr;
   print_index3 (i+1, j+1, k+1);
 
 
   print_desc_ebcdic (MSG_MODE_NAME,
                      sizeof(mode_ov_ptr->mode_name),
                      mode_ov_ptr->mode_name);
 
   print_desc_u (MSG_MAX_RU_SIZE_LOWER, mode_ov_ptr->max_ru_size_low);
   print_desc_u (MSG_MAX_RU_SIZE_UPPER, mode_ov_ptr->max_ru_size_upp);
   print_desc_u (MSG_MAX_SESS_LIMIT, mode_ov_ptr->max_neg_sess_lim);
   print_desc_u (MSG_CURR_SESS_LIMIT, mode_ov_ptr->curr_sess_lim);
   print_desc_u (MSG_MIN_WINNER_LIMIT, mode_ov_ptr->min_win_lim);
   print_desc_u (MSG_MIN_LOSER_LIMIT, mode_ov_ptr->min_lose_lim);
   print_desc_u (MSG_ACT_SESS_COUNT, mode_ov_ptr->act_sess_count);
   print_desc_u (MSG_PEND_SESS_COUNT, mode_ov_ptr->pend_sess_count);
   print_desc_u (MSG_AUTO_ACT_SESS_COUNT, mode_ov_ptr->auto_act_sess_count);
   print_desc_u (MSG_ACT_WINNER_COUNT, mode_ov_ptr->act_win_lim);
   print_desc_u (MSG_ACT_LOSER_COUNT, mode_ov_ptr->act_lose_lim);
   print_desc_u (MSG_TERM_COUNT, mode_ov_ptr->term_count);
 
   if(mode_ov_ptr->drain_source == AP_NO)
     msgadr =  MSG_NO;
   else
     msgadr = MSG_YES;
   print_desc_s (MSG_DRAIN_SOURCE, msgadr);
 
   if(mode_ov_ptr->drain_target == AP_NO)
     msgadr =  MSG_NO;
   else
     msgadr = MSG_YES;
   print_desc_s (MSG_DRAIN_TARGET, msgadr);
 
   print_desc (MSG_PACING_SIZE);
   if (mode_ov_ptr->pacing_size == 0)
       myprintf (MSG_NO_PACING);
   else
       print_u ((USHORT)mode_ov_ptr->pacing_size);
 
   if(mode_ov_ptr->implicit_mode == AP_NO)
     msgadr =  MSG_NO;
   else
     msgadr = MSG_YES;
   print_desc_s (MSG_IMPL_MODE, msgadr);
}
