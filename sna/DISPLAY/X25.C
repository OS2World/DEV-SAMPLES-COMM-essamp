/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : X25.C                                                     */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format X.25 information returned by the          */
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

void DISP_X25(X25_PHYSICAL_LINK_INFO_SECT far * x25_ptr)
{
   X25_OVERLAY far * x25_ov_ptr;        /* X.25 link overlay pointer  */
   unsigned i;                          /* loop variable              */
   unsigned short x25_count;            /* counter for X.25 links     */

   /*-----------------------------------------------------------------------*/
   /* Display each X.25 link overlay                                        */
   /*-----------------------------------------------------------------------*/
   x25_count = x25_ptr->num_x25_links;
   print_desc_u (MSG_NUMBER_X25_LINKS, x25_count);
   for (x25_ov_ptr = (X25_OVERLAY far *)
           ((UCHAR far *)x25_ptr + x25_ptr->x25_init_sect_len),
        i = 0;
        i < x25_count;
        x25_ov_ptr = (X25_OVERLAY far *)
           ((UCHAR far *)x25_ov_ptr + x25_ov_ptr->x25_entry_len),
        i++) {

      print_index1 (i+1);

      print_desc_ls (MSG_XLINK_NAME,
                     sizeof(x25_ov_ptr->link_name),
                     x25_ov_ptr->link_name);
      print_desc_ls (MSG_LINK_COMMENTS,
                     sizeof(x25_ov_ptr->link_comments),
                     x25_ov_ptr->link_comments);
      print_desc_u (MSG_ADAPTER_SLOT_NUM, x25_ov_ptr->adapter_slot_num);

      print_desc (MSG_XLINK_TYPE);
      switch (x25_ov_ptr->link_type) {
      case AP_LEASED_LINE: myprintf (MSG_LEASED_LINE); break;
      case AP_VX32:        myprintf (MSG_VX32);        break;
      default:             myprintf (MSG_ERROR_VALUE, x25_ov_ptr->link_type);
      } /* endswitch */

      print_desc (MSG_XLINK_MODE);
      switch (x25_ov_ptr->link_mode) {
      case AP_DISCONNECT:   myprintf (MSG_DISCONNECT);   break;
      case AP_CONNECT:      myprintf (MSG_CONNECT);      break;
      case AP_AUTO_CONNECT: myprintf (MSG_AUTO_CONNECT); break;
      default:              myprintf (MSG_ERROR_VALUE, x25_ov_ptr->link_mode);
      } /* endswitch */

      print_desc (MSG_XLINK_STATE);
      switch (x25_ov_ptr->link_state) {
      case AP_CONNECTING:           myprintf (MSG_CONNECTING);    break;
      case AP_CONNECTED:            myprintf (MSG_CONNECTED);     break;
      case AP_ERROR_LEVEL_1:        myprintf (MSG_ERROR_LEVEL_1); break;
      case AP_ERROR_LEVEL_2:        myprintf (MSG_ERROR_LEVEL_2); break;
      case AP_DISCONNECTING:        myprintf (MSG_DISCONNECTING); break;
      case AP_DISCONNECTED:         myprintf (MSG_DISCONNECTED);  break;
      case AP_ADAPTER_ERROR:        myprintf (MSG_ADAPTER_ERROR); break;
      case AP_ADAPTER_ACCESS_ERROR: myprintf (MSG_ADAPTER_ACCESS_ERROR); break;
      case AP_INCOMING_WAIT:        myprintf (MSG_INCOMING_WAIT); break;
      default: myprintf (MSG_ERROR_VALUE, x25_ov_ptr->link_state);
      } /* endswitch */

      print_desc (MSG_XLINK_DIRECTION);
      switch (x25_ov_ptr->link_direction) {
      case AP_INCOMING: myprintf(MSG_INCOMING); break;
      case AP_OUTGOING: myprintf(MSG_OUTGOING); break;
      case AP_2_WAY:    myprintf(MSG_2_WAY);    break;
      default: myprintf(MSG_ERROR_VALUE, x25_ov_ptr->link_direction);
      } /* endswitch */

      print_desc_u (MSG_NUM_ACT_PVCS,      x25_ov_ptr->num_act_pvcs);
      print_desc_u (MSG_TOTAL_NUM_PVCS,    x25_ov_ptr->total_num_pvcs);
      print_desc_u (MSG_NUM_ACT_SVCS,      x25_ov_ptr->num_act_svcs);
      print_desc_u (MSG_NUM_INCOMING_SVCS, x25_ov_ptr->num_incoming_svcs);
      print_desc_u (MSG_NUM_2_WAY_SVCS,    x25_ov_ptr->num_2_way_svcs);
      print_desc_u (MSG_NUM_OUTGOING_SVCS, x25_ov_ptr->num_outgoing_svcs);

   } /* endfor */
}
