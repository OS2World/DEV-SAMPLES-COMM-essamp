/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : DISPUTIL.C                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Utilities for the DISPLAY and PMDSPLAY sample programs.     */
/*                                                                          */
/*                                                                          */
/*              Uses the following ACSSVC (Common Services) Verbs:          */
/*                                                                          */
/*                 CONVERT                                                  */
/*                                                                          */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
/*              (compiles with large memory model)                          */
/*                                                                          */
/*   ASSOCIATED FILES:  See also DISPLAY.MAK, PMDSPLAY.MAK                  */
/*                                                                          */
/*      APD.TXT      - MESSAGES (ENGLISH)                                   */
/*      MSGID.H      - #defines for messages                                */
/*                                                                          */
/****************************************************************************/
#include "DISPLAY.H"      /* Program definitions and declarations           */
 
/* MSC 6.0 handles multi-thread includes differently than IBM C/2           */
#if _MSC_VER >= 600
   #include <STDLIB.H>    /* Standard C library miscellaneous functions     */
#else
   #include <MT\STDLIB.H> /* Standard C library miscellaneous functions     */
   #endif
#include <ACSSVCC.H>      /* Common Services definitions and declarations   */
 
#define VALUE_COLUMN 45   /* Column where parameter values are printed      */
/*--------------------------------------------------------------------------*/
/* Prototype for ldexp standard library function - This is normally         */
/* specified in MATH.H, but MATH.H doesn't work with /MT and /Ox on MSC 6.0.*/
/*--------------------------------------------------------------------------*/
#if _MSC_VER >= 600
   double _FAR_ _pascal ldexp(double, int);
#else
   double far pascal ldexp(double, int);
#endif
 
/*--------------------------------------------------------------------------*/
/*                           Global Variables                               */
/*--------------------------------------------------------------------------*/
BOOL appn = TRUE;         /* Indicates that I'm running on the latest       */
                          /* version of APPC.                               */
USHORT indent = 0;        /* Number of spaces of indentation                */
BOOL indented = FALSE;    /* Indicates already indented                     */
 
/* Array of pointers to the DISPLAY information formatting functions.       */
/* Indexed by DISPLAY_INFO_TYPE.                                            */
 
void (*print_info[])(void far *) = {
   DISP_SNA,
   DISP_LU62,
   DISP_AM,
   DISP_TP,
   DISP_SESSIONS,
   DISP_LINKS,
   DISP_LU_0_3,
   DISP_GW,
   DISP_X25,
   DISP_SYS_DFLT,
   DISP_ADAPTER,
   DISP_LU_DEF,
   DISP_PLU_DEF,
   DISP_MODE_DEF,
   DISP_LINK_DEF,
   DISP_MS,
   DISP_NODE,
   DISP_DIR,
   DISP_TOP,
   DISP_ISR,
   DISP_COS,
   DISP_CN
} ;
 
/*--------------------------------------------------------------------------*/
/*                      Local Function Prototypes                           */
/*--------------------------------------------------------------------------*/
void print_info_header (DISPLAY_INFO_TYPE info_type);
float decode_3_5 (UCHAR encoded_value, double unit, int * decoded_exp_ptr);
 
/****************************************************************************/
/* set_version:  Determine the version of APPC by trying to execute a       */
/*               DISPLAY_APPN verb.  If the primary return code is AP_OK,   */
/*               I'm running on the latest version of APPC; if the primary  */
/*               return code is AP_INVALID_VERB, I'm running on an older    */
/*               version of APPC; and if the primary return code is         */
/*               anything else, something is wrong.                         */
/****************************************************************************/
int set_version (void far * info_buffer_ptr, UINT info_buffer_size,
                 UCHAR tp_id[], ULONG conv_id, BOOL remote)
{
   void far * info_ptr;             /* Pointer to returned DISPLAY info */
   DISP_RETURN_INFO dri;           /* Structure returned from remote DISPLAY*/
   BOOL fSuccess = TRUE;           /* Used to convey success of remote disp */
   ULONG display_length;           /* Lenght of info returned in buffer     */
 
/* Request APPN node info because it is of fixed and small size, and is     */
/* therefore unlikely to overflow the info buffer.                          */
   if (!remote) {
      info_ptr = exec_display(DISPLAY_INFO_NODE,
                              info_buffer_ptr, info_buffer_size,
                              &dri.primary_rc, &dri.secondary_rc,
                              &display_length);
   } else {
      info_ptr = get_remote_data (DISPLAY_INFO_NODE, info_buffer_ptr,
                                  tp_id, conv_id, &dri, &fSuccess);
      } /* endif */
 
   if ((info_ptr != 0L) && (dri.primary_rc == AP_OK)) {
      appn = TRUE;                     /* Set global variable = APPN */
   } else
   if (dri.primary_rc == AP_INVALID_VERB) {
      appn = FALSE;                    /* Set global variable = not APPN */
   } else {
      switch (dri.primary_rc) {
      case AP_OK:
         return (-1);
         break;
      case AP_COMM_SUBSYSTEM_NOT_LOADED:
         myprintf(MSG_APPC_NOT_LOADED);
         break;
      default:
         print_error(dri.primary_rc, dri.secondary_rc);
         return (-1);
      } /* endswitch */
      return(-1);                      /* Return failure indication */
   } /* endif */
 
   return(0);                          /* Return ok indication */
}
 
/****************************************************************************/
/* get_and_format_info:  Execute DISPLAY to get the requested info, and     */
/*                       format the returned information.                   */
/****************************************************************************/
BOOL get_and_format_info (DISPLAY_INFO_TYPE info_type,
                          UCHAR far * info_buffer_ptr,
                          UCHAR tp_id[], ULONG conv_id,
                          BOOL  remote)
{
   void  far * info_ptr;           /* Pointer to information returned by    */
                                   /* the DISPLAY verb                      */
   DISP_RETURN_INFO dri;           /* Structure returned from remote DISPLAY*/
   BOOL fSuccess = TRUE;           /* Used to convey success of remote disp */
   ULONG display_length;           /* Lenght of info returned in buffer     */
 
      print_info_header(info_type);              /* print DISPLAY info type */
      if (!remote) {
         info_ptr = exec_display(info_type,
                                 info_buffer_ptr, INFO_BUFFER_SIZE,
                                 &dri.primary_rc, &dri.secondary_rc,
                                 &display_length);
      } else {
         info_ptr = get_remote_data (info_type, info_buffer_ptr,
                                     tp_id, conv_id, &dri, &fSuccess);
         } /* endif */
 
      switch (dri.primary_rc) {      /* check DISPLAY return codes */
      case AP_COMM_SUBSYSTEM_NOT_LOADED:
      /*--------------------------------------------------------------------*/
      /* Either Communications Manager is not started, or the required      */
      /* communications subsystem (either APPC or X.25) is not installed or */
      /* loaded.                                                            */
      /*--------------------------------------------------------------------*/
         if (info_type == DISPLAY_INFO_X25) {
            myprintf(MSG_X25_NOT_LOADED);
         } else {
            myprintf(MSG_APPC_NOT_LOADED);
            } /* endif */
         break;
      case AP_STATE_CHECK:
         if (dri.secondary_rc != AP_DISPLAY_INFO_EXCEEDS_LEN) {
            print_error(dri.primary_rc, dri.secondary_rc);
            break;
         } else {
          /*----------------------------------------------------------------*/
          /* If return code is AP_DISPLAY_INFO_EXCEED_LEN, print message    */
          /* and let code fall through to the case AP_OK.                   */
          /*----------------------------------------------------------------*/
            myprintf(MSG_INFO_EXCEEDS_LENGTH);
            } /* endif */
      case AP_OK:
         if (info_ptr) (*print_info[info_type])(info_ptr);
         break;
      default:
         print_error(dri.primary_rc, dri.secondary_rc);
         break;
         } /* endswitch */
 
      indent = 0;                    /* Reset indentation */
      return (fSuccess);
}
 
/****************************************************************************/
/* get_remote_data:  Performs the APPC verbs SEND_DATA and RECEIVE_AND_WAIT */
/*                   to get the information returned by the DISPLAY verb    */
/*                   on a remote machine.                                   */
/****************************************************************************/
void far * get_remote_data (DISPLAY_INFO_TYPE info_type,
                            UCHAR far * info_buffer_ptr,
                            UCHAR tp_id[], ULONG conv_id,
                            DISP_RETURN_INFO far * dri,
                            BOOL far * fSuccess)
{
   void   far * info_ptr;          /* Pointer to information returned by    */
                                   /* the DISPLAY verb                      */
 
   memcpy (info_buffer_ptr, &info_type, sizeof (DISPLAY_INFO_TYPE));
   if (mc_send_data (info_buffer_ptr, tp_id, conv_id,
                     sizeof (DISPLAY_INFO_TYPE), AP_NONE)) {
      if (AP_DATA_COMPLETE == mc_receive_and_wait (info_buffer_ptr,
                                                   tp_id, conv_id,
                                                sizeof (DISP_RETURN_INFO))) {
         memcpy (dri, info_buffer_ptr, sizeof (DISP_RETURN_INFO));
         info_ptr = (UCHAR far *) (dri->info_offset +
                                   (ULONG) info_buffer_ptr);
         if (!(AP_DATA_COMPLETE_SEND == mc_receive_and_wait (info_buffer_ptr,
                                                             tp_id, conv_id,
                                                        INFO_BUFFER_SIZE))) {
         /* Second MC_RECEIVE_AND_WAIT failed to get DATA_COMPLETE_SEND     */
         /* Return codes are worthless if buffer not received               */
            dri->primary_rc = AP_OK;
            info_ptr = NULL;
            *fSuccess = FALSE;
            } /* endif */
      } else {
      /* First MC_RECEIVE_AND_WAIT failed to get DATA_COMPLETE              */
      /* Return codes not received, set to 0                                */
         dri->primary_rc = AP_OK;
         info_ptr = NULL;
         *fSuccess = FALSE;
         }
   } else {
   /* MC_SEND_DATA failed                                                   */
   /* Everything is worthless if you can't send anything                    */
      dri->primary_rc = AP_OK;
      info_ptr = NULL;
      *fSuccess = FALSE;
      } /* endif */
   return (info_ptr);
}
 
/****************************************************************************/
/* print_info_header:  Prints a message indicating the type of info being   */
/*                     requested in the DISPLAY or DISPLAY_APPN verb.       */
/****************************************************************************/
void print_info_header (DISPLAY_INFO_TYPE info_type)
{
   char * info_header;
 
   myprintf(MSG_INFO_HEADER_TOP);
   switch (info_type) {
   case DISPLAY_INFO_GLOBAL:   info_header = MSG_SNA;      break;
   case DISPLAY_INFO_LU62:     info_header = MSG_LU62;     break;
   case DISPLAY_INFO_AM:       info_header = MSG_AM;       break;
   case DISPLAY_INFO_TP:       info_header = MSG_TP;       break;
   case DISPLAY_INFO_SESSIONS: info_header = MSG_SESSIONS; break;
   case DISPLAY_INFO_LINKS:    info_header = MSG_LINKS;    break;
   case DISPLAY_INFO_LU03:     info_header = MSG_LU_0_3;   break;
   case DISPLAY_INFO_GW:       info_header = MSG_GW;       break;
   case DISPLAY_INFO_X25:      info_header = MSG_X25;      break;
   case DISPLAY_INFO_SYSDEF:   info_header = MSG_SYS_DFLT; break;
   case DISPLAY_INFO_ADAPTER:  info_header = MSG_ADAPTER;  break;
   case DISPLAY_INFO_LUDEF:    info_header = MSG_LU_DEF;   break;
   case DISPLAY_INFO_PLUDEF:   info_header = MSG_PLU_DEF;  break;
   case DISPLAY_INFO_MODES:    info_header = MSG_MODE_DEF; break;
   case DISPLAY_INFO_LINKDEF:  info_header = MSG_LINK_DEF; break;
   case DISPLAY_INFO_MS:       info_header = MSG_MS;       break;
   case DISPLAY_INFO_NODE:     info_header = MSG_NODE;     break;
   case DISPLAY_INFO_DIR:      info_header = MSG_DIR;      break;
   case DISPLAY_INFO_TOP:      info_header = MSG_TOP;      break;
   case DISPLAY_INFO_ISR:      info_header = MSG_ISR;      break;
   case DISPLAY_INFO_COS:      info_header = MSG_COS;      break;
   case DISPLAY_INFO_CN:       info_header = MSG_CN;       break;
   } /* endswitch */
   myprintf(info_header);
   myprintf(MSG_INFO_HEADER_BOTTOM);
}
 
/****************************************************************************/
/* print_error:  Displays unexpected APPC return code.                      */
/****************************************************************************/
void print_error (USHORT primary_rc, ULONG secondary_rc)
{
   myprintf(MSG_UNEXPECTED_RETURN_CODE);
   myprintf(MSG_RETURN_CODE1, SWAP4(secondary_rc));
   myprintf(" X'%04X'\n", SWAP2(primary_rc));
   myprintf(MSG_RETURN_CODE2);
   myprintf(" X'%08lX'\n", SWAP4(secondary_rc));
}
 
/****************************************************************************/
/* print_index1:  Prints a 1-number index and sets indentation.             */
/****************************************************************************/
void print_index1 (int i)
{
   indent = myprintf ("\n%u>", i);     /* Print blank line, index */
   --indent;                           /* Don't count newline in indent */
   indented = TRUE;                    /* Already indented for first line */
}
 
/****************************************************************************/
/* print_index2:  Prints a 2-number index and sets indentation.             */
/****************************************************************************/
void print_index2 (int i, int j)
{
   indent = myprintf ("\n%u.%u>", i, j);
                                       /* Print blank line, index */
   --indent;                           /* Don't count newline in indent */
   indented = TRUE;                    /* Already indented for first line */
}
 
/****************************************************************************/
/* print_index3:  Prints a 3-number index and sets indentation.             */
/****************************************************************************/
void print_index3 (int i, int j, int k)
{
   indent = myprintf ("\n%u.%u.%u>", i, j, k);
                                       /* Print blank line, index */
   --indent;                           /* Don't count newline in indent */
   indented = TRUE;                    /* Already indented for first line */
}
 
/****************************************************************************/
/* print_desc:  Prints the left column (description) of a parameter.        */
/*              Handles indentation, and tabs to the position for the right */
/*              column (value).                                             */
/*                                                                          */
/*              To allow more space for translating the descriptor,         */
/*              a max. of 2 lines are supported, with a '\n' denoting a     */
/*              new line; a maximum of 78 characters are supported over the */
/*              two lines. If the first line (including indentation)        */
/*              exceeds 44 characters, the line is truncated.               */
/*              When a '\n' is encountered, a second descriptor line is     */
/*              generated, with data indented 2 characters from the first   */
/*              line, and the 'value' printed on the second line.           */
/*                                                                          */
/****************************************************************************/
void print_desc (char * desc_string)
{
   int  i;
   char svchar;
   char * adr1;
   BOOL oneline = TRUE;  /* Indicates number of lines in descriptor field */
 
   for (adr1 = desc_string; (*adr1 != '\0') && (*adr1 != 0x0a); adr1++);
   if (*adr1 == 0x0a){
      svchar = * ++adr1;                /* save next char            */
      *adr1 = '\0';                      /* terminate string after NL */
      oneline = FALSE;                   /* must handle 2 lines       */
   } /* endif */
 
   if((strlen(desc_string)+ indent) > 44){ /* truncate 1st line if too long */
      * (desc_string + 44 - indent) = '\0';
      oneline = TRUE;
   } /* endif */
 
   if (!indented) {                    /* Indent and print 1st description */
      i = myprintf ("%.*s%s ", indent, MSG_BLANKS, desc_string);
   } else {                            /* If already indented */
      i = indent + myprintf (desc_string); /* Just print description */
      indented = FALSE;                /* Enable indentation again */
   } /* endif */
 
   if (oneline == FALSE) {             /* Process 2nd line    */
    *adr1 = svchar;                    /* restore saved char  */
    i = myprintf ("%.*s%s ", indent+1, MSG_BLANKS, adr1);
    ++i;
    } /* endif */
 
   myprintf ("%.*s", VALUE_COLUMN - i, MSG_BLANKS); /* Tab to value column */
}
 
/****************************************************************************/
/* print_u:  Prints an unsigned short value and a newline.                  */
/****************************************************************************/
void print_u (USHORT u)
{
   myprintf ("%u\n", u);
}
 
/****************************************************************************/
/* print_02x:  Prints a one-byte hex value and a newline.                   */
/****************************************************************************/
void print_02x (UCHAR x)
{
   myprintf ("X'%02X'\n", x);
}
 
/****************************************************************************/
/* ebcdic_to_ascii:  Converts a name from EBCDIC to ASCII.                  */
/****************************************************************************/
void ebcdic_to_ascii (UCHAR far * name, USHORT length)
{
   union {
      struct convert      cnvt;             /* Common Services CONVERT verb */
   } vcb;
 
   CLEAR_VCB(vcb);                          /* Zero out the verb            */
   vcb.cnvt.opcode    = SV_CONVERT;         /* Do a CONVERT general service */
   vcb.cnvt.direction = SV_EBCDIC_TO_ASCII; /* Convert EBCDIC to ASCII      */
   vcb.cnvt.char_set  = SV_AE;              /* AE conversion type           */
   vcb.cnvt.len       = length;             /* Convert length of the string */
   vcb.cnvt.source    =                     /* source and target addresses  */
   vcb.cnvt.target    = name;               /* are same (convert in place)  */
 
   ACSSVC_C ((ULONG)((void far *)&vcb));    /* Do the convert to ASCII      */
}
 
/****************************************************************************/
/* print_ebcdic_name:  Usually converts a name from EBCDIC to ASCII and     */
/*                     prints it.  However, if the name does not begin with */
/*                     an EBCDIC character (ie, if it is an architected     */
/*                     name), prints the name in hex.                       */
/****************************************************************************/
void print_ebcdic_name (UCHAR far * name, USHORT length)
{
   if (*name < 0x40) {                 /* First character is not EBCDIC     */
      while ((length != 0) && (name[length-1] == 0x40)) {
         --length;                     /* Delete trailing blanks            */
      } /* endwhile */
      print_hex_string (name, length); /* Print name as hex string          */
   } else {                            /* First character is EBCDIC         */
      ebcdic_to_ascii (name, length);  /* Convert name to ASCII             */
      print_string (name, length);     /* Print name as ASCII string        */
   } /* endif */
}
 
/****************************************************************************/
/* print_hex_string:  Prints the hex value of an array of bytes.            */
/****************************************************************************/
void print_hex_string (UCHAR far * value, unsigned size)
{
  unsigned i;
 
  myprintf("X'");
  for (i = 0; i < size; i++) {
     myprintf("%02X",value[i]);
  } /* endfor */
  myprintf("'\n");
}
 
/****************************************************************************/
/* print_string:  Prints a ASCII string that may or may not be null         */
/*                terminated.                                               */
/****************************************************************************/
void print_string (UCHAR far * string, unsigned stringsize)
{
   while ((stringsize != 0) && (string[stringsize-1] == ' ')) {
      string[stringsize-1] = '\0';     /* Delete trailing blanks */
      --stringsize;
   } /* endwhile */
   myprintf("%.*s\n", stringsize, string);
}
 
/****************************************************************************/
/* print_total:  Prints the total number of elements given the supplied     */
/*               format string, and also an overflow message if the DISPLAY */
/*               buffer overflowed.                                         */
/****************************************************************************/
void print_total (char * string, USHORT total_count, USHORT display_count)
{
   indent = 0;                         /* Reset indentation */
   print_desc_u (string, total_count); /* Print the total */
   if (total_count > display_count)    /* If total exceeds number displayed */
      myprintf (MSG_BUFFER_OVERFLOW, display_count); /* Print error message */
}
 
/****************************************************************************/
/* print_tg_capacity:  Prints a TG's effective capacity given the 3/5-      */
/*                     encoded byte.  Range of decoded effective capacity   */
/*                     values is from 169 bps to 604 Gbps.                  */
/****************************************************************************/
void print_tg_capacity (char * desc_string, UCHAR encoded_value)
{
   float decoded_value;
   int   decoded_exp;
   PCHAR units;
   int units_calculation;
 
   print_desc (desc_string);           /* Print parm description */
 
/* Decode the 3/5-encoded byte.  The unit of the encoded value for
   effective capacity is 300 bps.  The exponent of the result is
   returned in decoded_exp and is divisible by 3; the mantissa is
   returned in decoded_value and is between 1 and 1000. */
 
   decoded_value = decode_3_5 (encoded_value, 300.0e0, &decoded_exp);
 
   myprintf ("%.2f ",decoded_value);   /* Print the value */
   units_calculation = (decoded_exp/3);
   switch(units_calculation){
   case 0:
    units = MSG_BPS; ;
    break;
   case 1:
    units = MSG_KBPS;
    break;
   case 2:
    units = MSG_MBPS;
    break;
   case 3:
    units = MSG_GBPS;
    break;
  }
 
   myprintf (units); /* Print the units */
   myprintf (MSG_CRLF);
}
 
/****************************************************************************/
/* print_tg_prop_delay:  Prints a TG's propagation delay given the 3/5-     */
/*                       encoded byte.  Range of decoded propagation delay  */
/*                       values is from 562 nanoseconds to 2013 seconds.    */
/****************************************************************************/
void print_tg_prop_delay (char * desc_string, UCHAR encoded_value)
{
   float decoded_value;
   int   decoded_exp;
   PCHAR units;
   int units_calculation;
 
   print_desc (desc_string);      /* Print parm description */
 
/* Decode the 3/5-encoded byte.  The unit of the encoded value for
   propagation delay is 1 microsecond.  The exponent of the result is
   returned in decoded_exp and is devisible by 3; the mantissa is
   returned in decoded_value and is between 1 and 1000. */
 
   decoded_value = decode_3_5 (encoded_value, 1.0e-6, &decoded_exp);
 
   while (decoded_exp > 0) {      /* Display units no bigger than seconds */
      decoded_value *= 1.0e3;
      decoded_exp -= 3;
   } /* endwhile */
 
   myprintf ("%.2f ",decoded_value); /* Print the value */
   units_calculation = abs(decoded_exp)/3;
   switch(units_calculation){
   case 0:
    units = MSG_SECONDS;
    break;
   case 1:
    units = MSG_MILLISECONDS;
    break;
   case 2:
    units = MSG_MICROSECONDS;
    break;
   case 3:
    units = MSG_NANOSECONDS;
    break;
  }
 
   myprintf (units); /* Print the units */
 
/* If the propagation delay is a default value, print the interpretation. */
 
   switch (encoded_value) {
   case AP_PROP_DELAY_MINIMUM:
      myprintf (MSG_DELAY_MINIMUM);
      break;
   case AP_PROP_DELAY_LAN:
      myprintf (MSG_DELAY_LAN);
      break;
   case AP_PROP_DELAY_TELEPHONE:
      myprintf (MSG_DELAY_TELEPHONE);
      break;
   case AP_PROP_DELAY_PKT_SWITCHED_NET:
      myprintf (MSG_DELAY_PKT_SWITCHED_NET);
      break;
   case AP_PROP_DELAY_SATELLITE:
      myprintf (MSG_DELAY_SATELLITE);
      break;
   case AP_PROP_DELAY_MAXIMUM:
      myprintf (MSG_DELAY_MAXIMUM);
      break;
   } /* endswitch */
 
   myprintf(MSG_CRLF);
}
 
/****************************************************************************/
/* decode_3_5:  Decodes a 3/5-encoded byte for effective capacity or        */
/*              propagation delay.                                          */
/* Input:  encoded_value - A 3/5-encoded byte of bit format eeeeemmm.  When */
/*                         the encoded byte is 0, it represents 0;          */
/*                         otherwise, it represents the binary expression   */
/*                         .1mmm * 10**(eeeee), which has a range of decimal*/
/*                         values from .5625 to 2,013,265,920.              */
/*         unit - Defines the unit of the encoded value.  For effective     */
/*                capacity, unit should be 300.0e0 (for 300 bps), and for   */
/*                propagation delay, 1.0e-6 (for microsecond).  This        */
/*                function multiplies the decoded value by unit before      */
/*                returning it to the caller.                               */
/*         decoded_exp_ptr - Pointer to an int in which the exponent of the */
/*                           decoded value is returned to the caller.  The  */
/*                           value of the exponent is a multiple of 3.      */
/* Output:  A float which is the mantissa of the decoded value.  The value  */
/*          of the mantissa is between 1 and 1000.                          */
/****************************************************************************/
float decode_3_5 (UCHAR encoded_value, double unit, int * decoded_exp_ptr)
{
   double decoded_mantissa;
 
   *decoded_exp_ptr = 0;
 
   if (encoded_value == 0) {      /* Special case check for zero */
      return ((float)0);
   } /* endif */
 
/* Convert the encoded_value to a double, and multiply by unit.
   Subtract 4 from the encoded_value exponent to account for the
   2**(-4) in the encoded_value mantissa. */
 
   decoded_mantissa = unit * ldexp ((double)((encoded_value & 0x07) | 0x08),
                                    (((encoded_value & 0xF8) >> 3) - 4));
 
/* Adjust the decoded_mantissa and *decoded_exp_ptr so that the
   decoded mantissa is between 1 and 1000. */
 
   while ((decoded_mantissa < 1.0e0) || (decoded_mantissa >= 1.0e3)) {
      if (decoded_mantissa < 1.0e0) {
         decoded_mantissa *= 1.0e3;
         *decoded_exp_ptr -= 3;
      } else {
         decoded_mantissa /= 1.0e3;
         *decoded_exp_ptr += 3;
      } /* endif */
   } /* endwhile */
 
   return ((float)decoded_mantissa);
}
 
/****************************************************************************/
/* print_security:  Prints a TG's security parameter.                       */
/****************************************************************************/
void print_security (char * desc_string, UCHAR security)
{
   print_desc (desc_string);      /* Print parm description */
 
   switch (security) {
   case AP_SEC_NONSECURE:
      myprintf (MSG_NONSECURE);
      break;
   case AP_SEC_PUBLIC_SWITCHED_NETWORK:
      myprintf (MSG_PUBLIC_SWITCHED_NETWORK);
      break;
   case AP_SEC_UNDERGROUND_CABLE:
      myprintf (MSG_UNDERGROUND_CABLE);
      break;
   case AP_SEC_SECURE_CONDUIT:
      myprintf (MSG_SECURE_CONDUIT);
      break;
   case AP_SEC_GUARDED_CONDUIT:
      myprintf (MSG_GUARDED_CONDUIT);
      break;
   case AP_SEC_ENCRYPTED:
      myprintf (MSG_ENCRYPTED);
      break;
   case AP_SEC_GUARDED_RADIATION:
      myprintf (MSG_GUARDED_RADIATION);
      break;
   case AP_SEC_MAXIMUM:
      myprintf (MSG_MAXIMUM_SECURITY);
      break;
   default:
      myprintf (MSG_ERROR_VALUE, security);
   } /* endswitch */
}
 
/****************************************************************************/
/* print_desc_u:  Prints the specified parameter description and the        */
/*                unsigned short parameter value.                           */
/****************************************************************************/
void print_desc_u (char * desc_string, USHORT u)
{
   print_desc (desc_string);
   print_u (u);
}
 
/****************************************************************************/
/* print_desc_s:  Prints the specified parameter description and the ASCIIZ */
/*                string parameter value.                                   */
/****************************************************************************/
void print_desc_s (char * desc_string, char * string)
{
   print_desc (desc_string);
   myprintf ("%s\n", string);
}
 
/****************************************************************************/
/* print_desc_ls:  Prints the specified parameter description and the       */
/*                 length-limited ASCIIZ string parameter value.            */
/****************************************************************************/
void print_desc_ls (char * desc_string, USHORT stringsize, char * string)
{
   print_desc (desc_string);
   print_string (string, stringsize);
}
 
/****************************************************************************/
/* print_desc_ebcdic:  Prints the specified parameter description and the   */
/*                     EBCDIC string parameter value.                       */
/****************************************************************************/
void print_desc_ebcdic (char * desc_string, USHORT stringsize, char * string)
{
   print_desc (desc_string);
   print_ebcdic_name (string, stringsize);
}
 
/****************************************************************************/
/* print_desc_xs:  Prints the specified parameter description and the       */
/*                 length-limited hex string parameter value.               */
/****************************************************************************/
void print_desc_xs (char * desc_string, USHORT stringsize, char * string)
{
   print_desc (desc_string);
   print_hex_string (string, stringsize);
}
 
/****************************************************************************/
/* print_desc_02x:  Prints the specified parameter description and the      */
/*                  one-byte hex parameter value.                           */
/****************************************************************************/
void print_desc_02x (char * desc_string, UCHAR x)
{
   print_desc (desc_string);
   print_02x (x);
}
 
/****************************************************************************/
/* print_desc_lx:  Prints the specified parameter description and the       */
/*                 long hex integer parameter value.                        */
/****************************************************************************/
void print_desc_lx (char * desc_string, ULONG lx)
{
   print_desc (desc_string);
   myprintf ("X'%08lX'\n", lx);
}
