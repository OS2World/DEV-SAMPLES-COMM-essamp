/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : SNA.C                                                     */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utility to format SNA Global information returned by the    */
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
#include <DISPLAY.H>         /* global typedefs, prototypes, and #includes  */

extern BOOL appn;                       /*  Flag for APPN vs. APPC code     */

void DISP_SNA (SNA_GLOBAL_INFO_SECT far * sna_ptr)
{
  print_desc_ebcdic (MSG_NET_NAME,
                     sizeof(sna_ptr->net_name),
                     sna_ptr->net_name);
  print_desc_ebcdic (MSG_CP_NAME,
                     sizeof(sna_ptr->pu_name),
                     sna_ptr->pu_name);
  print_desc_ls (MSG_PU_NAME,
                 sizeof(sna_ptr->pu_name),
                 sna_ptr->pu_name);
  print_desc (MSG_XID);
  print_hex_string (sna_ptr->node_id, sizeof(sna_ptr->node_id));

  if (appn) {
     print_desc_ls (MSG_ALIAS_CP_NAME,
                    sizeof(sna_ptr->alias_cp_name), sna_ptr->alias_cp_name);

     print_desc (MSG_NODE_TYPE);
     switch (sna_ptr->node_type) {
     case AP_EN: myprintf (MSG_EN); break;
     case AP_NN: myprintf (MSG_NN); break;
     default:    myprintf (MSG_ERROR_VALUE, sna_ptr->node_type);
     } /* endswitch */

     print_desc (MSG_CP_NAU_ADDRESS);
     if (sna_ptr->cp_nau_addr == 0) {
        myprintf (MSG_NOT_USED_INDEPENDENT);
     } else {
        myprintf ("%u %s", sna_ptr->cp_nau_addr, MSG_DEPENDENT);
     } /* endif */
  } /* endif */

  print_desc (MSG_MACHINE_SERIAL_NUM);
  ebcdic_to_ascii (sna_ptr->product_set_id.plant_of_mfg,
                   sizeof(sna_ptr->product_set_id.plant_of_mfg));
  ebcdic_to_ascii (sna_ptr->product_set_id.machine_seq_num,
                   sizeof(sna_ptr->product_set_id.machine_seq_num));
  myprintf ("%.*s-", sizeof(sna_ptr->product_set_id.plant_of_mfg ),
                     sna_ptr->product_set_id.plant_of_mfg);
  print_string (sna_ptr->product_set_id.machine_seq_num,
                sizeof(sna_ptr->product_set_id.machine_seq_num));

  print_desc_ebcdic (MSG_MACHINE_TYPE,
                     sizeof(sna_ptr->product_set_id.machine_type),
                     sna_ptr->product_set_id.machine_type);
  print_desc_ebcdic (MSG_MODEL_NUMBER,
                     sizeof(sna_ptr->product_set_id.machine_mod_num ),
                     sna_ptr->product_set_id.machine_mod_num);

  print_desc (MSG_CM_VERSION);
  myprintf ("%u.%u\n", sna_ptr->version, sna_ptr->release);

}

