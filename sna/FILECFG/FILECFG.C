/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : FILECFG.C                                                 */
/*                                                                          */
/*  DESCRIPTIVE : APPC/APPN DEFINE SAMPLE PROGRAM FOR THE FILE TRANSFER     */
/*  NAME          SAMPLE PROGRAMS FOR IBM Communications Manager            */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991                         */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*                                                                          */
/*  FUNCTION:   Configures the FILE Sample program using the                */
/*              dynamic configuration verbs.                                */
/*                                                                          */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
/*                                                                          */
/****************************************************************************/

#define  LINT_ARGS

#include <appn.h>
#include <acssvcc.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


#define  REQUESTER_NAME "FILECREQ"
#define  SERVER_NAME    "FILECSVR"
#define  TP_NAME        "FileServer"
#define  PLU_ALIAS      "FILESVR"
#define  FILESPEC       "C:\\CMLIB\\SNA\\FILE\\FILECSVR.EXE"
#define  MAX_RU         15360
#define  PACING_WINDOW  63

/* Macro BLANK_STRING sets string to all blanks              */
#define BLANK_STRING(str)  memset(str,(int)' ',sizeof(str))

#define TOUPPER_STRING(str,length,counter) \
                           {for (counter=0;counter < length;counter ++) \
                           {str[counter] = (UCHAR)toupper(str[counter]);}}


UCHAR key[8];                               /* KEY for the Keylock          */

#ifdef LINT_ARGS
/*--------------------------------------------------------------------------*/
/*                       Function Prototypes                                */
/*--------------------------------------------------------------------------*/
void ascii2ebcdic(UCHAR * string, UINT length, UCHAR type);
void print_appc_retcode(USHORT opcode, USHORT primary_rc, ULONG secondary_rc);
int  define_partner_lu(UCHAR * plu_alias, UCHAR * fqplu_name);
int  define_local_lu(UCHAR * lu_alias, UCHAR * lu_name);
int  define_tp(UCHAR * tp_name, UCHAR * filespec);
void convert_fqplu_name(UCHAR * fqplu_name);
void print_info(void);
#endif


void cdecl
main(int argc, char * argv[])
{
   int i;
   int rc = 0;

   if (argc > 1) {

      for (i=1 ; i<argc ; i++ ) {
         switch (argv[i][0]) {
         case '-':
         case '/':
            switch (argv[i][1]) {
            case 'k':
            case 'K':
               if (i == 1) {
                  if (++i < argc) {
                     BLANK_STRING(key);
                     memcpy(key,argv[i],min(strlen(argv[i]),sizeof(key)));
                  } else {
                     fprintf(stderr,"Error: keylock not present\n");
                  } /* endif */
               } else {
                  fprintf(stderr,
                  "Error: keylock parameter must be specified first!\n");
               } /* endif */
               break;
            case 'r':
            case 'R':
               /* configure requester */
               if (++i < argc) {
                  rc |= define_partner_lu(PLU_ALIAS, argv[i]);
                  if (rc == 0) {
                     printf("%s defined successfully.\n",REQUESTER_NAME);
                  } else {
                     fprintf(stderr,"%s definition Failed.\n",REQUESTER_NAME);
                  } /* endif */
               } else {
                  fprintf(stderr,"Error: fqplu_name not present\n");
               } /* endif */
               break;
            case 's':
            case 'S':
               /* configure server */
               rc |= define_tp(TP_NAME, FILESPEC);
               if (rc == 0) {
                  printf("%s defined successfully.\n",SERVER_NAME);
               } else {
                  fprintf(stderr,"%s definition Failed.\n",SERVER_NAME);
               } /* endif */
               break;
            case 'b':
            case 'B':
               /* configure for LU=own */
               if (++i < argc) {
                  rc |= define_tp(TP_NAME, FILESPEC);
                  rc |= define_local_lu(PLU_ALIAS,argv[i]);
                  if (rc == 0) {
                     printf("%s and %s defined successfully.\n",
                             REQUESTER_NAME,SERVER_NAME);
                  } else {
                     fprintf(stderr,"%s and %s definition Failed.\n",
                             REQUESTER_NAME,SERVER_NAME);
                  } /* endif */
               } else {
                  fprintf(stderr,"Error: lu_name not present\n");
               } /* endif */
               break;
            default:
               ;
            } /* endswitch */
            break;
         default:
            print_info();
            exit(EXIT_FAILURE);
            break;
         } /* endswitch */
      } /* endfor */
   } else {
      print_info();
      exit(EXIT_FAILURE);
   } /* endif */

   exit(EXIT_SUCCESS);

}

void
print_appc_retcode(USHORT opcode, USHORT primary_rc, ULONG secondary_rc)
{

   printf("verb = %04X   primary = %04X   secondary = %08lX\n",
           SWAP2(opcode),
           SWAP2(primary_rc),
           SWAP4(secondary_rc));

   if (primary_rc == SV_COMM_SUBSYSTEM_NOT_LOADED) {
      printf("\n\tCommunications Manager is not started\n");
      exit(EXIT_FAILURE);
   } else {
      if (primary_rc == AP_COMM_SUBSYSTEM_NOT_LOADED) {
         printf("\n\tEither the Communications Manager is not started\n");
         printf("\tor APPC is not loaded.\n");
         exit(EXIT_FAILURE);
      } else {
         if (primary_rc == AP_COMM_SUBSYSTEM_ABENDED) {
            printf("\n\tAPPC has ABENDED.\n");
            exit(EXIT_FAILURE);
         }
      } /* endif */
   } /* endif */
}

int
define_local_lu(UCHAR * lu_alias, UCHAR * lu_name)
{
   USHORT i;
   DEFINE_LOCAL_LU define_local_lu;

   DEFINE_LOCAL_LU far *ptr_define_local_lu =
                              (DEFINE_LOCAL_LU far *)&define_local_lu;

   TOUPPER_STRING(lu_name,strlen(lu_name),i);
   TOUPPER_STRING(lu_alias,strlen(lu_alias),i);

   CLEAR_VCB(define_local_lu);
   printf("\nIssuing Define Local LU with:\n");
   printf("LU Alias   = %s\n",lu_alias);
   printf("LU Name    = %s\n",lu_name);

   define_local_lu.opcode = AP_DEFINE_LOCAL_LU;

   memcpy(define_local_lu.key, key, sizeof(define_local_lu.key));

   BLANK_STRING(define_local_lu.lu_name);
   memcpy(      define_local_lu.lu_name,
                lu_name,
                min(strlen(lu_name), sizeof(define_local_lu.lu_name)));
   ascii2ebcdic(  define_local_lu.lu_name,
                sizeof(define_local_lu.lu_name),
                SV_A);

   BLANK_STRING(define_local_lu.lu_alias);
   memcpy(      define_local_lu.lu_alias,
                lu_alias,
                min(strlen(lu_alias), sizeof(define_local_lu.lu_alias)));

   define_local_lu.nau_address = AP_INDEPENDENT_LU;

   APPC((ULONG)ptr_define_local_lu);


   if (define_local_lu.primary_rc != AP_OK) {
      print_appc_retcode(define_local_lu.opcode,
                         define_local_lu.primary_rc,
                         define_local_lu.secondary_rc);
      return -1;
   } else {
      return 0;
   } /* endif */

}

int
define_partner_lu(UCHAR * plu_alias, UCHAR * fqplu_name)
{
   USHORT i;
   DEFINE_PARTNER_LU define_partner_lu;

   DEFINE_PARTNER_LU far *ptr_define_partner_lu =
                                  (DEFINE_PARTNER_LU far *)&define_partner_lu;

   TOUPPER_STRING(plu_alias,strlen(plu_alias),i);
   TOUPPER_STRING(fqplu_name,strlen(fqplu_name),i);

   CLEAR_VCB(define_partner_lu);

   printf("\nIssuing Define Partner LU with:\n");
   printf("PLU Alias  = %s\n",plu_alias);
   printf("FQPLU Name = %s\n",fqplu_name);

   define_partner_lu.opcode = AP_DEFINE_PARTNER_LU;

   BLANK_STRING(define_partner_lu.fqplu_name);
   memcpy(      define_partner_lu.fqplu_name,
                fqplu_name,
                min(strlen(fqplu_name), sizeof(define_partner_lu.fqplu_name)));
   convert_fqplu_name(define_partner_lu.fqplu_name);

   BLANK_STRING(define_partner_lu.plu_alias);
   memcpy(      define_partner_lu.plu_alias,
                plu_alias,
                min(strlen(plu_alias), sizeof(define_partner_lu.plu_alias)));

   memcpy(      define_partner_lu.key,
                key,
                sizeof(define_partner_lu.key));

   define_partner_lu.max_mc_ll_send_size = 32767;
   define_partner_lu.conv_security_ver = AP_NO;
   define_partner_lu.parallel_sess_supp = AP_YES;

   APPC((ULONG)ptr_define_partner_lu);

   if (define_partner_lu.primary_rc != AP_OK) {
      print_appc_retcode(define_partner_lu.opcode,
                         define_partner_lu.primary_rc,
                         define_partner_lu.secondary_rc);
      return -1;
   } else {
      return 0;
   } /* endif */

}

int
define_tp(UCHAR * tp_name, UCHAR * filespec)
{
   USHORT i;
   DEFINE_TP define_tp;

   DEFINE_TP far *ptr_define_tp = (DEFINE_TP far *)&define_tp;

   CLEAR_VCB(define_tp);

   TOUPPER_STRING(filespec,strlen(filespec),i);

   printf("\nIssuing Define TP with:\n");
   printf("TP Name    = %s\n",tp_name);
   printf("Filespec   = %s\n",filespec);

   define_tp.opcode = AP_DEFINE_TP;

   memcpy(define_tp.key, key, sizeof(define_tp.key));

   define_tp.conversation_type = AP_MAPPED;
   define_tp.conv_security_rqd = AP_NO;
   define_tp.sync_level = AP_CONFIRM;
   define_tp.tp_operation = AP_NONQUEUED_AM_STARTED;
   define_tp.program_type = AP_VIO_WINDOWABLE;
   define_tp.incoming_alloc_queue_depth = 20;
   define_tp.incoming_alloc_timeout = 100;
   define_tp.rcv_alloc_timeout = 200;


   BLANK_STRING(define_tp.tp_name);
   memcpy(      define_tp.tp_name,
                tp_name,
                min(strlen(tp_name), sizeof(define_tp.tp_name)));
   ascii2ebcdic(  define_tp.tp_name,
                sizeof(define_tp.tp_name),
                SV_AE);

   BLANK_STRING(define_tp.filespec);
   memcpy(      define_tp.filespec,
                filespec,
                min(strlen(filespec), sizeof(define_tp.filespec)));

   APPC((ULONG)ptr_define_tp);

   if (define_tp.primary_rc != AP_OK) {
      print_appc_retcode(define_tp.opcode,define_tp.primary_rc,
                         define_tp.secondary_rc);
      return -1;
   } else {
      return 0;
   } /* endif */

}


/* This routine handles the fact that the period (.) is not handled by   */
/* either of the supplied conversion tables.                             */

void
convert_fqplu_name(UCHAR * fqplu_name)
{
   unsigned int i;   /* counter */

   for (i=0;i<17 && fqplu_name[i] != '\0' && fqplu_name[i] != '.';i++ ) {
   } /* endfor */

   if (fqplu_name[i] == '.') {        /* avoid trying to convert */
      ascii2ebcdic(fqplu_name,i,SV_A);
      ascii2ebcdic(&fqplu_name[i+1],16-i,SV_A);
      fqplu_name[i] = 0x4B;           /* convert the period by hand */
   } else {
      ascii2ebcdic(fqplu_name, 17, SV_A);
   } /* endif */

}


/* this procedure takes a string of a specified length and converts it to */
/* ebcdic using the specified table (SV_A or SV_AE)                       */

void
ascii2ebcdic(UCHAR * string, UINT length, UCHAR type)
{
   struct convert                   vcb_convert;
   struct convert far *ptr_convert       = (struct convert far *)&vcb_convert;

   if (length == 0) {
      return;
   } else {
   } /* endif */

   CLEAR_VCB(vcb_convert);
   vcb_convert.opcode = SV_CONVERT;
   vcb_convert.direction = SV_ASCII_TO_EBCDIC;
   vcb_convert.char_set = type;
   vcb_convert.len = length;
   vcb_convert.source = vcb_convert.target=
                        (unsigned char far *)string;
   ACSSVC((ULONG)ptr_convert);
   if (vcb_convert.primary_rc != AP_OK) {
      print_appc_retcode(vcb_convert.opcode,
                         vcb_convert.primary_rc,
                         vcb_convert.secondary_rc);
   } else {
   } /* endif */
}

void
print_info()
{
   fprintf(stderr,
   "FILECFG - a sample program to configure the FILE Sample Programs\n\n");
   fprintf(stderr,
   "This program will execute the dynamic configuration verbs necessary\n");
   fprintf(stderr,
   "to run the FILECREQ and FILECSVR sample programs.  This program\n");
   fprintf(stderr,
   "can create definitions for a requester, a server, or both on the\n");
   fprintf(stderr,
   "same machine.\n\n");
   fprintf(stderr,
   "FILECFG usage:\n");
   fprintf(stderr,
   "FILECFG [-k keylock] -r fqplu_name  -- configures requester\n");
   fprintf(stderr,
   "FILECFG [-k keylock] -s             -- configures server\n");
   fprintf(stderr,
   "FILECFG [-k keylock] -b lu_name     -- configures requester and server\n");
   fprintf(stderr,
   "                                       on one machine\n");
   fprintf(stderr,
   "\nWhere:\n");
   fprintf(stderr,
   "   fqplu_name - the Fully Qualified Partner LU Name of the Server.\n");
   fprintf(stderr,
   "   lu_name  - a unique LU name that will be used on your workstation.\n");
   fprintf(stderr,
   "\nNote: If the keylock is specified, it must be the first argument.\n");
}
