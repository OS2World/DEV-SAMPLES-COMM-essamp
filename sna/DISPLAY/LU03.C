/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : LU03.C                                                    */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format LU 0-3 information returned by the        */
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
 
/* The following defines are necessary because the reserved bits of the
   variables "sscp_lu_sess_state" and "lu_lu_sess_state" have unpredictable
   values and should be zeroed out before comparing to expected values    */
#define SSCPLU_MASK (0xF0)  /* Mask out reserved bits in session state    */
#define LULU_MASK   (0xE0)  /* Mask out reserved bits in session state    */
 
void DISP_LU_0_3 (LU_0_3_INFO_SECT far * lu_0_3_ptr)
{
   LU_0_3_OVERLAY far * lu_0_3_ov_ptr;    /* pointer to lu03 overlay        */
   unsigned i;                            /* loop variable                  */
   unsigned short lu03_count;             /* counter for number of sessions */
 
   /*-----------------------------------------------------------------------*/
   /* Display each LU overlay                                               */
   /*-----------------------------------------------------------------------*/
   lu03_count = lu_0_3_ptr->num_lu_0_3s;
   print_desc_u (MSG_NUMBER_CONFIG_LUS, lu03_count);
   for (lu_0_3_ov_ptr = (LU_0_3_OVERLAY far *)
           ((UCHAR far *)lu_0_3_ptr + lu_0_3_ptr->lu_0_3_init_sect_len),
        i = 0; i < lu03_count; i++,
        lu_0_3_ov_ptr = (LU_0_3_OVERLAY far *)
           ((UCHAR far *)lu_0_3_ov_ptr + lu_0_3_ov_ptr->lu_0_3_entry_len)) {
 
      print_index1 (i+1);
 
      print_desc (MSG_ACCESS_TYPE);
      switch (lu_0_3_ov_ptr->access_type) {
      case AP_3270_EMULATION: myprintf (MSG_3270_EMULATION); break;
      case AP_LUA:            myprintf (MSG_LUA);            break;
      default: myprintf (MSG_ERROR_VALUE, lu_0_3_ov_ptr->access_type);
      } /* endswitch */
 
      print_desc (MSG_LU_TYPE);
      switch (lu_0_3_ov_ptr->lu_type) {
      case AP_UNKNOWN:
         myprintf (MSG_UNKNOWN);
         break;
      case AP_LU0:
      case AP_LU1:
      case AP_LU2:
      case AP_LU3:
         print_u (lu_0_3_ov_ptr->lu_type);
         break;
      default:
         myprintf (MSG_ERROR_VALUE, lu_0_3_ov_ptr->lu_type);
      } /* endswitch */
 
      print_desc_02x (MSG_LOCAL_ADDR, lu_0_3_ov_ptr->lu_daf);
      print_desc (MSG_LU_SHORT_NAME);
      myprintf ("%c\n", lu_0_3_ov_ptr->lu_short_name);
      print_desc_ls (MSG_LU_LONG_NAME,
                     sizeof(lu_0_3_ov_ptr->lu_long_name),
                     lu_0_3_ov_ptr->lu_long_name);
      print_desc_xs (MSG_SESSION_ID,
                     sizeof(lu_0_3_ov_ptr->sess_id),
                     lu_0_3_ov_ptr->sess_id);
      print_desc_ls (MSG_DLC_NAME,
                     sizeof(lu_0_3_ov_ptr->dlc_name),
                     lu_0_3_ov_ptr->dlc_name);
      print_desc_u (MSG_ADAPTER_NUMBER, lu_0_3_ov_ptr->adapter_num);
      print_desc_xs (MSG_DEST_ADDR,
                     lu_0_3_ov_ptr->dest_addr_len,
                     lu_0_3_ov_ptr->dest_addr);
 
      print_desc_ebcdic (MSG_LINK_NAME,
                         sizeof(lu_0_3_ov_ptr->link_id),
                         lu_0_3_ov_ptr->link_id);
 
      print_desc (MSG_SSCP_LU_SESS_STATE);
      switch (lu_0_3_ov_ptr->sscp_lu_sess_state & SSCPLU_MASK & ~AP_DETACHING) {
      case AP_DEACTIVATED:  myprintf(MSG_DEACTIVATED);  break;
      case AP_ACTIVATED:    myprintf(MSG_ACTIVATED);    break;
      case AP_ACTIVATING:   myprintf(MSG_ACTIVATING);   break;
      case AP_DEACTIVATING: myprintf(MSG_DEACTIVATING); break;
      default:
         myprintf(MSG_ERROR_VALUE, lu_0_3_ov_ptr->sscp_lu_sess_state);
      } /* endswitch */
 
      print_desc (MSG_LU_LU_SESS_STATE);
      switch (lu_0_3_ov_ptr->lu_lu_sess_state & LULU_MASK) {
                                       /* Mask off reserved bits */
      case AP_DEACTIVATED:  myprintf(MSG_DEACTIVATED);  break;
      case AP_ACTIVATED:    myprintf(MSG_ACTIVATED);    break;
      case AP_ACTIVATING:   myprintf(MSG_ACTIVATING);   break;
      case AP_DEACTIVATING: myprintf(MSG_DEACTIVATING); break;
      default:
         myprintf(MSG_ERROR_VALUE, lu_0_3_ov_ptr->lu_lu_sess_state);
      } /* endswitch */
 
      if (lu_0_3_ov_ptr->sscp_lu_sess_state & AP_DETACHING) {
         print_desc (MSG_DETACHING);
         myprintf (MSG_CRLF);
      } /* endif */
 
   } /* endfor */
}
