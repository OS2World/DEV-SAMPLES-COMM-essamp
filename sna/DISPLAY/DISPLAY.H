/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : DISPLAY.H                                                 */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991                         */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Global definitions and declaractions for the DISPLAY        */
/*              and PMDSPLAY sample programs.                               */
/*                                                                          */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
/*              (compiles with large memory model)                          */
/*                                                                          */
/*  ASSOCIATED FILES:                                                       */
/*                                                                          */
/*      DISPLAY.MAK  - MAKE file                                            */
/*      DISPLAY.DEF  - Module definition file (for LINK)                    */
/*      DISPLAY.H    - Global typedefs, prototypes, and #includes           */
/*      DISPLAY.C    - Main function and unique utility functions           */
/*      DISPUTIL.C   - Common utility functions                             */
/*      EXECDISP.C   - Utility function to execute DISPLAY verb             */
/*      APPCUTIL.C   - Common utilities for remote DISPLAY                  */
/*      SNA.C        - Formats global information                           */
/*      LU62.C       - Formats LU 6.2 information                           */
/*      AM.C         - Formats attach manager information                   */
/*      TP.C         - Formats transaction program information              */
/*      SESS.C       - Formats session information                          */
/*      LINKS.C      - Formats link information                             */
/*      LU03.C       - Formats LU 0, 1, 2, and 3 information                */
/*      GW.C         - Formats gateway information                          */
/*      X25.C        - Formats X.25 logical link information                */
/*      DEFAULTS.C   - Formats system defaults information                  */
/*      ADAPTER.C    - Formats adapter information                          */
/*      LU_DEF.C     - Formats LU definition information                    */
/*      PLU_DEF.C    - Formats partner LU definition information            */
/*      MODE_DEF.C   - Formats mode definition information                  */
/*      LINK_DEF.C   - Formats link definition information                  */
/*      MS.C         - Formats management services information              */
/*      NODE.C       - Formats APPN node information                        */
/*      DIR.C        - Formats APPN directory information                   */
/*      TOP.C        - Formats APPN topology information                    */
/*      ISR.C        - Formats APPN intermediate session information        */
/*      COS.C        - Formats APPN class of service information            */
/*      CN.C         - Formats APPN connection network information          */
/*      APD.TXT      - MESSAGES (ENGLISH)                                   */
/*      MSGID.H      - #defines for messages                                */
/*                                                                          */
/****************************************************************************/
#ifndef OS2_INCLUDED
   #define INCL_NOPM      /* Not using OS/2 PM functions                    */
   #include <OS2.H>       /* OS/2 definitions and declarations              */
   #endif
 
/* MSC 6.0 handles multi-thread includes differently than IBM C/2           */
#if _MSC_VER >= 600
   #ifndef _MT
      #define _MT
      #endif
   #include <STRING.H>    /* Standard C library string and memory functions */
#else
   #include <MT\STRING.H> /* Standard C library string and memory functions */
   #endif
#include <ACSMGTC.H>      /* DISPLAY verb definitions and declarations      */
#include "MSGID.H"        /* Message array definitions                      */
 
/* Indicator of type of DISPLAY information, in the same order as the info  */
/* flags and pointers in the DISPLAY and DISPLAY_APPN verbs.                */
 
typedef enum {
   DISPLAY_INFO_NONE = -1  ,
   DISPLAY_INFO_GLOBAL     ,
   DISPLAY_INFO_LU62       ,
   DISPLAY_INFO_AM         ,
   DISPLAY_INFO_TP         ,
   DISPLAY_INFO_SESSIONS   ,
   DISPLAY_INFO_LINKS      ,
   DISPLAY_INFO_LU03       ,
   DISPLAY_INFO_GW         ,
   DISPLAY_INFO_X25        ,
   DISPLAY_INFO_SYSDEF     ,
   DISPLAY_INFO_ADAPTER    ,
   DISPLAY_INFO_LUDEF      ,
   DISPLAY_INFO_PLUDEF     ,
   DISPLAY_INFO_MODES      ,
   DISPLAY_INFO_LINKDEF    ,
   DISPLAY_INFO_MS         ,
   DISPLAY_INFO_NODE       ,
   DISPLAY_INFO_DIR        ,
   DISPLAY_INFO_TOP        ,
   DISPLAY_INFO_ISR        ,
   DISPLAY_INFO_COS        ,
   DISPLAY_INFO_CN         ,
   DISPLAY_INFO_LAST
} DISPLAY_INFO_TYPE;
 
typedef struct {
   USHORT primary_rc;
   ULONG  secondary_rc;
   ULONG  info_offset;
} DISP_RETURN_INFO;
 
#define INFO_BUFFER_SIZE  (USHORT)(65530)        /* max returned data = 64K */
 
/*--------------------------------------------------------------------------*/
/*                      Global Function Prototypes                          */
/*--------------------------------------------------------------------------*/
/* APPC verb execution functions (APPCUTIL.C)                               */
 
UCHAR  far * alloc_shared_buffer (USHORT size);
void   free_shared_buffer (void far * buffer_ptr);
void   convert (UCHAR * name, UINT size, UCHAR dir, UCHAR char_set);
void   ascii_name (UCHAR * dest, UCHAR * src, UINT dest_size);
void   ebcdic_name (UCHAR * dest, UCHAR * src,
                    UINT dest_size, UCHAR char_set);
BOOL   invalid_PLU_name (char * PLU_name);
BOOL   tp_started (UCHAR * tp_name, UCHAR tp_id[]);
BOOL   mc_allocate (UCHAR * plu_name, UCHAR * tp_name, UCHAR tp_id[],
                    UCHAR * mode_name, UCHAR synch_level, ULONG * conv_id);
BOOL   receive_allocate (CHAR * tp_name, UCHAR tp_id[], ULONG * conv_id);
BOOL   mc_send_data (UCHAR  far * buffer_ptr, UCHAR tp_id[],
                   ULONG  conv_id, USHORT length, UCHAR  type);
USHORT mc_receive_and_wait (UCHAR  far * buffer_ptr, UCHAR tp_id[],
                            ULONG  conv_id, USHORT max_length);
void   mc_confirmed (UCHAR tp_id[], ULONG conv_id);
void   mc_deallocate (UCHAR tp_id[], ULONG conv_id, UCHAR type);
void   tp_ended (UCHAR tp_id[]);
void   show_verb_retcode(USHORT opcode, USHORT primary_rc,
                         ULONG  secondary_rc, ULONG  sense_data);
 
/* DISPLAY and DISPLAY_APPN verb execution function (EXECDISP.C)            */
 
void far * exec_display (DISPLAY_INFO_TYPE info_type,
                         UCHAR far * info_buffer_ptr,
                         UINT info_buffer_size,
                         USHORT * primary_rc, ULONG * secondary_rc,
                         ULONG * display_length);
 
/* Common utilities (DISPUTIL.C) */
 
void far * get_remote_data (DISPLAY_INFO_TYPE info_type,
                            UCHAR far * info_buffer_ptr,
                            UCHAR tp_id[], ULONG conv_id,
                            DISP_RETURN_INFO  far * dri,
                            BOOL far * fSuccess);
int  set_version (void far * info_buffer_ptr, UINT info_buffer_size,
                  UCHAR tp_id[], ULONG conv_id, BOOL remote);
BOOL get_and_format_info (DISPLAY_INFO_TYPE info_type,
                          UCHAR far * info_buffer_ptr,
                          UCHAR tp_id[], ULONG conv_id, BOOL remote);
void print_info_header (DISPLAY_INFO_TYPE info_type);
void print_error (USHORT primary_rc, ULONG secondary_rc);
void ebcdic_to_ascii (UCHAR far * name, USHORT length);
void print_index1 (int i);
void print_index2 (int i, int j);
void print_index3 (int i, int j, int k);
void print_desc (char * desc_string);
void print_u (USHORT u);
void print_02x (UCHAR x);
void print_ebcdic_name (UCHAR far * name, USHORT length);
void print_hex_string (UCHAR far * value, unsigned size);
void print_string (UCHAR far * string, unsigned stringsize);
void print_total (char * string, USHORT total_count, USHORT display_count);
void print_tg_capacity (char * desc_string, UCHAR encoded_value);
void print_tg_prop_delay (char * desc_string, UCHAR encoded_value);
void print_security (char * desc_string, UCHAR security);
void print_desc_u (char * desc_string, USHORT u);
void print_desc_s (char * desc_string, char * string);
void print_desc_ls (char * desc_string, USHORT stringsize, char * string);
void print_desc_ebcdic (char * desc_string, USHORT stringsize, char * string);
void print_desc_xs (char * desc_string, USHORT stringsize, char * string);
void print_desc_02x (char * desc_string, UCHAR x);
void print_desc_lx (char * desc_string, ULONG lx);
 
/* DISPLAY information formatting functions */
 
void DISP_SNA      (SNA_GLOBAL_INFO_SECT far *);
void DISP_LU62     (LU62_INFO_SECT far *);
void DISP_AM       (AM_INFO_SECT far *);
void DISP_TP       (TP_INFO_SECT far *);
void DISP_SESSIONS (SESS_INFO_SECT far *);
void DISP_LINKS    (LINK_INFO_SECT far *);
void DISP_LU_0_3   (LU_0_3_INFO_SECT far *);
void DISP_GW       (GW_INFO_SECT far *);
void DISP_X25      (X25_PHYSICAL_LINK_INFO_SECT far *);
void DISP_SYS_DFLT (SYS_DEF_INFO_SECT far *);
void DISP_ADAPTER  (ADAPTER_INFO_SECT far *);
void DISP_LU_DEF   (LU_DEF_INFO_SECT far *);
void DISP_PLU_DEF  (PLU_DEF_INFO_SECT far *);
void DISP_MODE_DEF (MODE_DEF_INFO_SECT far *);
void DISP_LINK_DEF (LINK_DEF_INFO_SECT far *);
void DISP_MS       (MS_INFO_SECT far *);
void DISP_NODE     (NODE_INFO_SECT far *);
void DISP_DIR      (DIRECTORY_INFO_SECT far *);
void DISP_TOP      (TOPOLOGY_INFO_SECT far *);
void DISP_ISR      (ISR_INFO_SECT far *);
void DISP_COS      (COS_INFO_SECT far *);
void DISP_CN       (CN_INFO_SECT far *);
 
/* Output formatting function (DISPLAY.C and PMDSPLAY.C) */
 
int cdecl myprintf(char * string, ...);
 
