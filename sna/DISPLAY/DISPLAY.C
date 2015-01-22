/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : DISPLAY.C                                                 */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Executes the DISPLAY and DISPLAY_APPN verbs requesting      */
/*              the information requested in the command arguments, and     */
/*              displays the returned information on the console in         */
/*              human-readable form.                                        */
/*                                                                          */
/*              Uses the following APPC verbs:                              */
/*                                                                          */
/*                 DISPLAY                                                  */
/*                 DISPLAY_APPN                                             */
/*                 TP_STARTED                                               */
/*                 RECEIVE_ALLOCATE                                         */
/*                 MC_ALLOCATE                                              */
/*                 MC_SEND_DATA                                             */
/*                 MC_RECEIVE_AND_WAIT                                      */
/*                 MC_DEALLOCATE                                            */
/*                 MC_CONFIRMED                                             */
/*                 TP_ENDED                                                 */
/*                                                                          */
/*              Uses the following ACSSVC (Common Services) Verbs:          */
/*                                                                          */
/*                 CONVERT                                                  */
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
/*      RDSPSRVR.C   - Main function and unique utility functions           */
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
/*      APD.TXT      - Messages (English)                                   */
/*      MSGID.H      - #defines for messages                                */
/*                                                                          */
/****************************************************************************/

#define LINT_ARGS

#include "DISPLAY.H"      /* Program definitions and declarations           */

#include <STDIO.H>     /* Standard C library I/O functions               */
#include <STDARG.H>    /* Standard C library variable argument functions */

#define  MODE_NAME "#INTER"
#define  REQUESTOR "RDISPLAY"
#define  SERVER    "RDSPSRVR"

/*--------------------------------------------------------------------------*/
/*                       Local Function Prototypes                          */
/*--------------------------------------------------------------------------*/
int cdecl main (int argc, char * argv[]);
void print_options (void);
void print_header (void);
DISPLAY_INFO_TYPE get_option (int argc, char * argv[]);
void print_invalid_option (char * argv);
void print_non_appc_option (char * argv);
BOOL get_partner (int * argc, char * argv[], char * plu_name);

/*--------------------------------------------------------------------------*/
/*                          External References                             */
/*--------------------------------------------------------------------------*/
extern BOOL appn;         /* Indicates whether or not I'm running on the    */
                          /* latest version of APPC.                        */
extern  BOOL file_ok;     /* Indicates file APN.MSG file oOK                */

/****************************************************************************/
/*                       Main Program Section                               */
/****************************************************************************/
int cdecl main(int argc, char *argv[])
{
   BOOL  remote;                   /* Flag indicating locality of DISPLAY   */
   UCHAR tp_id[8];                 /* Transaction Program ID                */
   ULONG conv_id;                  /* Conversation ID                       */
   UCHAR plu_name[18];             /* PLU name (alias or fully qualified)   */
   UCHAR far * info_buffer_ptr;    /* Pointer to segment to be used by the  */
                                   /* DISPLAY verb for returned information */
   DISPLAY_INFO_TYPE option;       /* DISPLAY information type code         */

   file_ok = TRUE;

/* Initialize the text tables from APD.MSG                             */
   InitializeText();


/* If help is requested, print the options available and exit program.      */
   if ((argc > 1) && (argv[1][0] == '?')) {
      print_options();
      return(0);
   } /* endif */

/* Get a shared memory buffer for info returned by DISPLAY verb.            */
   info_buffer_ptr = alloc_shared_buffer (INFO_BUFFER_SIZE);
   if (NULL == info_buffer_ptr) return (-1); /* exit if error */

   remote = get_partner(&argc, argv, plu_name);

   if (remote) {
      if (tp_started (REQUESTOR, tp_id)) {
         if (!(mc_allocate (plu_name, SERVER, tp_id, MODE_NAME,
                            AP_CONFIRM_SYNC_LEVEL, &conv_id))) {
            return (-1);
            }
      } else {
         return (-1);
         } /* endif */
      } /* endif */

/* Determine version of APPC/APPN                                           */
   if (set_version (info_buffer_ptr, INFO_BUFFER_SIZE,
                    tp_id, conv_id, remote)) return (-1);
                                            /* exit if error */

   print_header();                 /* print the program header              */

/*--------------------------------------------------------------------------*/
/* For each type of DISPLAY info specified in the command line arguments,   */
/* get the info and format it to stdout.                                    */
/*--------------------------------------------------------------------------*/
   while (DISPLAY_INFO_NONE != (option = get_option(argc, argv))) {
      if (!(get_and_format_info (option, info_buffer_ptr,
                                 tp_id, conv_id, remote))) {
         return (-1);
         } /* endif */
      } /* endwhile */
      if (!file_ok){
         printf("Error occurred with file: APD.MSG");
         return (-1);
       }
   if (remote) {
      mc_deallocate (tp_id, conv_id, AP_SYNC_LEVEL);
      tp_ended (tp_id);
      } /* endif */

/* Free the DISPLAY info buffer.                                            */
   free_shared_buffer (info_buffer_ptr);

   return (0);
}

/****************************************************************************/
/* print_options:  Show the command line options.                           */
/****************************************************************************/
void print_options(void)
{
   printf(MSG_USAGE);                  /* Usage: DISPLAY [-sn] etc */

   printf(MSG_OPTION_HEADER);          /* Help table header */

   if (appn) {                         /* Print all options */
      printf(MSG_OPTION_AD );  printf(MSG_OPTION_MD );  printf("\n");
      printf(MSG_OPTION_CN );  printf(MSG_OPTION_MS );  printf("\n");
      printf(MSG_OPTION_CO );  printf(MSG_OPTION_N  );  printf("\n");
      printf(MSG_OPTION_D  );  printf(MSG_OPTION_P  );  printf("\n");
      printf(MSG_OPTION_G  );  printf(MSG_OPTION_SE );  printf("\n");
      printf(MSG_OPTION_I  );  printf(MSG_OPTION_SN );  printf("\n");
      printf(MSG_OPTION_LD );  printf(MSG_OPTION_SY );  printf("\n");
      printf(MSG_OPTION_LI );  printf(MSG_OPTION_TO );  printf("\n");
      printf(MSG_OPTION_LUD);  printf(MSG_OPTION_TP );  printf("\n");
      printf(MSG_OPTION_LU0);  printf(MSG_OPTION_AM );  printf("\n");
      printf(MSG_OPTION_LU6);  printf(MSG_OPTION_X  );  printf("\n");
   } else {                            /* Print only APPC options */
      printf(MSG_OPTION_G  );  printf(MSG_OPTION_SN );  printf("\n");
      printf(MSG_OPTION_LI );  printf(MSG_OPTION_TP );  printf("\n");
      printf(MSG_OPTION_LU0);  printf(MSG_OPTION_AM );  printf("\n");
      printf(MSG_OPTION_LU6);  printf(MSG_OPTION_X  );  printf("\n");
      printf(MSG_OPTION_SE );                           printf("\n");
   } /* endif */

   printf(MSG_OPTION_NOTES);           /* Print notes about options */
}

/****************************************************************************/
/* print_header:  Print the program header.                                 */
/****************************************************************************/
void print_header(void)
{
   printf(MSG_HEADER_TOP);
   printf(MSG_DISPLAY_VERSION);
   printf(MSG_HEADER_BOTTOM);
   if (appn) {
      printf(MSG_RUNNING_APPN);        /* Running on newest APPC */
   } else {
      printf(MSG_RUNNING_APPC);        /* Running on older APPC */
   } /* endif */
}

/****************************************************************************/
/* get_option:  Interprets command line arguments, returning a series of    */
/*              indicators of the type of DISPLAY information requested.    */
/*              Designed to be called repeatedly, returning an indicator    */
/*              for only one type of DISPLAY information at a time, until   */
/*              all arguments are processed.                                */
/****************************************************************************/
DISPLAY_INFO_TYPE get_option(int argc, char *argv[])
{
   static int index = 0;
   static DISPLAY_INFO_TYPE last_option = DISPLAY_INFO_NONE;
   DISPLAY_INFO_TYPE option;

   option = DISPLAY_INFO_NONE;
   if (argc < 2) {
      ++last_option;
      if ((last_option < DISPLAY_INFO_SYSDEF) ||
          (appn && (last_option < DISPLAY_INFO_LAST))) {
         option = last_option;
      } /* endif */
   } else {
      while (++index < argc) {
         strupr(argv[index]);
         switch (argv[index][0]) {
         case '-':
         case '/':
            switch (argv[index][1]) {
            case 'A':
               switch (argv[index][2]) {
               case 'D':
                  option = DISPLAY_INFO_ADAPTER;
                  break;
               case 'M':               /* -am == -tpd */
                  option = DISPLAY_INFO_AM;
                  break;
               } /* endswitch */
               break;
            case 'C':
               switch (argv[index][2]) {
               case 'N':
                  option = DISPLAY_INFO_CN;
                  break;
               case 'O':
                  option = DISPLAY_INFO_COS;
                  break;
               } /* endswitch */
               break;
            case 'D':
               option = DISPLAY_INFO_DIR;
               break;
            case 'G':
               option = DISPLAY_INFO_GW;
               break;
            case 'I':
               option = DISPLAY_INFO_ISR;
               break;
            case 'L':
               switch (argv[index][2]) {
               case 'D':
                  option = DISPLAY_INFO_LINKDEF;
                  break;
               case 'I':
                  option = DISPLAY_INFO_LINKS;
                  break;
               case 'U':
                  switch (argv[index][3]) {
                  case 'D':
                     option = DISPLAY_INFO_LUDEF;
                     break;
                  case '0':
                     option = DISPLAY_INFO_LU03;
                     break;
                  case '6':
                     option = DISPLAY_INFO_LU62;
                     break;
                  } /* endswitch */
                  break;
               } /* endswitch */
               break;
            case 'M':
               switch (argv[index][2]) {
               case 'D':
                  option = DISPLAY_INFO_MODES;
                  break;
               case 'S':
                  option = DISPLAY_INFO_MS;
                  break;
               } /* endswitch */
               break;
            case 'N':
               option = DISPLAY_INFO_NODE;
               break;
            case 'P':
               option = DISPLAY_INFO_PLUDEF;
               break;
            case 'S':
               switch (argv[index][2]) {
               case 'E':
                  option = DISPLAY_INFO_SESSIONS;
                  break;
               case 'N':
                  option = DISPLAY_INFO_GLOBAL;
                  break;
               case 'Y':
                  option = DISPLAY_INFO_SYSDEF;
                  break;
               } /* endswitch */
               break;
            case 'T':
               switch (argv[index][2]) {
               case 'O':
                  option = DISPLAY_INFO_TOP;
                  break;
               case 'P':
                  switch (argv[index][3]) {
                  case 'D':
                     option = DISPLAY_INFO_AM;
                     break;
                  default:
                     option = DISPLAY_INFO_TP;
                     break;
                  } /* endswitch */
               } /* endswitch */
               break;
            case 'X':
               option = DISPLAY_INFO_X25;
               break;
            } /* endswitch */
         } /* endswitch */
         if (option == DISPLAY_INFO_NONE) {
            print_invalid_option(argv[index]);
         } else
         if (!appn && (option > DISPLAY_INFO_X25)) {
            print_non_appc_option(argv[index]);
            option = DISPLAY_INFO_NONE;
         } else {
            break;                     /* Got valid arg, exit while loop */
         } /* endif */
      } /* endwhile */
   } /* endif */

   return(option);                     /* Return option code to caller */
}

/****************************************************************************/
/* print_invalid_option:  Displays error message for an unrecognized        */
/*                        command line argument.                            */
/****************************************************************************/
void print_invalid_option (char * argv)
{
   myprintf(MSG_STAR_LINE);
   myprintf(MSG_INVALID_OPTION, argv);
   myprintf(MSG_HELP);
   myprintf(MSG_STAR_LINE);
}

/****************************************************************************/
/* print_non_appc_option:  Displays error message when user requests new    */
/*                         DISPLAY info on an older version of APPC.        */
/****************************************************************************/
void print_non_appc_option (char * argv)
{
   myprintf(MSG_STAR_LINE);
   myprintf(MSG_NOT_APPC_OPTION, argv);
   myprintf(MSG_HELP);
   myprintf(MSG_STAR_LINE);
}

/****************************************************************************/
/* get_partner:  If a PLU Alias or fully qualified PLU name was specified,  */
/*               then remove it from the argument list.                     */
/****************************************************************************/
BOOL get_partner (int * argc, char * argv[], char * plu_name)
{
   int i;

   if (*argc < 2) {
      return (FALSE);
   } else {
      if (('-' == argv[1][0]) || ('/' == argv[1][0])) {
         return (FALSE);
      } else {
         if (!(invalid_PLU_name (argv[1]))) {
            strcpy (plu_name, argv[1]);
            for (i = 1; i < *argc; i++) {
               argv[i] = argv[i + 1];
               } /* endfor */
            (*argc)--;
            return (TRUE);
         } else {
            myprintf (MSG_INVALID_PLU_NAME, argv[1]);
            return (TRUE);
            } /* endif */
         } /* endif */
      } /* endif */
}

/****************************************************************************/
/* myprintf:  Formatting function called by print_info functions.  Directs  */
/*            output to stdout.                                             */
/****************************************************************************/
int cdecl myprintf(char * string, ...)
{
if(file_ok){
   va_list arg_ptr;                    /* Pointer to variable argument list */

   va_start(arg_ptr, string);          /* Set pointer to argument list      */
                                       /* following "string"                */
   return(vprintf(string, arg_ptr));   /* Format output to stdout           */
 }
 return(-1);
}
