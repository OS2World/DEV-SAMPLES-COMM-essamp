/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : NODE.C                                                    */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format APPN Node information returned by the     */
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

void DISP_NODE (NODE_INFO_SECT far * node_ptr)
{
   print_desc_u (MSG_ROUTE_RESIST, node_ptr->route_resist);

   print_desc (MSG_MAX_CACHE);
   if (node_ptr->max_cache == 0) {
      myprintf(MSG_NO_MAXIMUM);
   } else {
      print_u (node_ptr->max_cache);
   } /* endif */

   print_desc_u (MSG_CURRENT_CACHE, node_ptr->current_cache);

   print_desc (MSG_DIR_DUMP_INTERVAL);
   if (node_ptr->dir_dump_interval == 0) {
      myprintf (MSG_NEVER_SAVED);
   } else {
      print_u (node_ptr->dir_dump_interval);
   } /* endif */
}
