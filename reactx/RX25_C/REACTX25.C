/*************************************************************************/
/*                                                                       */
/*  MODULE NAME : REACTX25.C                                             */
/*                                                                       */
/*  DESCRIPTIVE : REACTIVATE THE X.25 DLC "C" SAMPLE PROGRAM             */
/*                                                                       */
/*  FUNCTION:   This program issues the SET_USER_LOG_QUEUE verb          */
/*              to wait for an error of type 0017.  If this is           */
/*              a subtype 2 then either a link failure or an             */
/*              adapter failure occured.  This module assumes an         */
/*              adapter failed.  The DISCONNECT_PHYSICAL_LINK            */
/*              and CONNECT_PHYSICAL_LINK verbs are then issued          */
/*              until the CONNECT_PHYSICAL_LINK verb returns with        */
/*              a non zero return code.  The Communications              */
/*              Manager periodically reactivates the DLC so that         */
/*              when this X.25 is reconnected the DLC should be          */
/*              automatically reactivated.  It is still up to the        */
/*              users application to reactivate the link either          */
/*              explicitly or by attempting to use it again.             */
/*                                                                       */
/*  NOTES:      1. If the adapter is in a condition such that it can     */
/*                 not be reactivated successfully then this program     */
/*                 may attempt many times.  Each attempt will cause      */
/*                 errors to be logged.                                  */
/*                                                                       */
/*  Licensed Material - Program Property of IBM - All Rights Reserved    */
/*                                                                       */
/*************************************************************************/

#include <STDIO.H>
#include <STDDEF.H>
#include <STDLIB.H>
#include <STRING.H>
#include <ACSMGTC.H>
#include <ACSSVCC.H>
#include <io.h>

/* Macro clear_vcb sets the APPC verb control block to zeros */

#define clear_vcb()     memset(&vcb,(int)'\0',sizeof(vcb))

/* Extract selector or offset from far pointer */
#define SELECTOROF(p)       (((unsigned short *)&(p))[1])
#define OFFSETOF(p)         (((unsigned short *)&(p))[0])

/***********************************************************/
/*        General Declares                                 */
/***********************************************************/

unsigned far *vcbptr;                       /* Pointer to the vcb           */
unsigned short error_code;                  /* Dos Call return code         */
unsigned short qhandle;                     /* Queue handle                 */
unsigned long qrequest;                     /* Queue request                */
unsigned char qpriority;                    /* Queue priority               */
void far *qsemhandle;                       /* Queue semephore handle       */
unsigned short qlength;                     /* Queue element length         */
unsigned char qname[17];                    /* Queue name \QUEUES\ERRLOG17  */
unsigned long thirty_seconds = 30000;       /* Wait time for CM to start    */
unsigned long forty_five_seconds = 45000;   /* Wait time betweeen attempts  */
unsigned short msel;                        /* Selector of CM shared mem    */
unsigned short cont;                        /* Continue if adap fail        */
unsigned short i;
unsigned short j;
unsigned char mname[24];                    /* Memory \SHAREMEM\ACSLGMEM    */
unsigned char save_link_name[8] = "";       /* X.25 link name               */
unsigned char save_link_mode = ' ';         /* X.25 link mode               */
struct x25_overlay far *x25_info_ptr;       /* Pointer to X.25 display info */

struct allocmem
    {
    unsigned short allocoff;
    unsigned short allocsel;
    };

union
    {
    struct allocmem am;
    unsigned char far *buff_ptr;
    } alloc_ptr;

/***********************************************************/
/*        General Constants                                */
/***********************************************************/

#define ON                  0x01
#define OFF                 0x00
#define YES                 0x01
#define NO                  0x00
#define LOGS                0x00
#define ALL                 0x01
#define SOME                0x00
#define ZERO                0x00
#define ONE                 0x01
#define TWO                 0x02
#define FIFO                0x0000
#define ALLOC_FLAGS         0x0003
#define ERROR_TYPE          0x1700
#define NUM_BYTES           0x0400
#define WAIT                0x00
#define NOWAIT              0x01
#define LEN_INIT_SECT       44;
#define NUM_SECTIONS        9;

/***********************************************************/
/*        External Entry Points                            */
/***********************************************************/

                                            /* DosAllocSeg                  */
     extern unsigned short pascal far DosAllocSeg (
            unsigned short,
            unsigned short far *,
            unsigned short);
                                            /* DosCreateQueue               */
     extern unsigned short pascal far DosCreateQueue (
            unsigned short far *,
            unsigned short,
            unsigned char far *);
                                            /* DosReadQueue                 */
     extern unsigned short pascal far DosReadQueue (
            unsigned short,
            unsigned long far *,
            unsigned short far *,
            unsigned long far *,
            unsigned char,
            unsigned char,
            unsigned char far *,
            void far *);
                                            /* DosGetShrSeg                 */
     extern unsigned short pascal far DosGetShrSeg (
            unsigned char far *,
            unsigned short far *);
                                            /* DosSubFree                   */
     extern unsigned short pascal far DosSubFree (
            unsigned short,
            unsigned short,
            unsigned short);

                                            /* DosSleep                     */
     extern unsigned short pascal far DosSleep (
            unsigned long);

   /**********************************************************/
   /*            Application Control Interface               */
   /**********************************************************/

 struct return_code
     {
     unsigned short opcode;
     unsigned short opext;
     unsigned char prim_rc1;
     unsigned char prim_rc2;
     };

 union verbs
     {
     struct connect_physical_link cl;
     struct disconnect_physical_link dl;
     struct display dsp;
     struct set_user_log_queue sulq;
     struct return_code rc;
     } vcb;

 struct error_log_queue_element
     {
     unsigned short elqqueue_id;  /* Verb operation code      */
     unsigned char reserv2[8];    /* Reserved                 */
     unsigned char elqhour;       /* Hour                     */
     unsigned char elqminute;     /* Minute                   */
     unsigned char elqsecond;     /* Second                   */
     unsigned char elqhundred;    /* Hundreth of second       */
     unsigned char elqday;        /* Day                      */
     unsigned char elqmonth;      /* Month                    */
     unsigned short elqyear;      /* Year                     */
     unsigned short elqtype;      /* Error Type               */
     unsigned long elqsubtype;    /* Error Subtype            */
     unsigned char elqconv_id[4]; /* Conversation ID          */
     unsigned char elqorig_id[8]; /* Originator ID            */
     unsigned char reserv3[4];    /* Reserved                 */
     unsigned short elqpid;       /* Process ID               */
     unsigned short elqlength;    /* Element Length           */
     unsigned char elqdata[20];   /* Element Data             */
     } far *error_log_ptr;

 union
     {
     unsigned long log_ptr;
     struct error_log_queue_element far *error_log_ptr;
     } lp;
/*--------------------------------------------------------------------------*/
/*                       Function Prototypes                                */
/*--------------------------------------------------------------------------*/

void main(void);
void GET_INFO(void);                        /* Copy Error Log               */
void SET_LOG_QUEUE(void);                   /* Set User Log Queue           */
void CONNECT_X25(void);                     /* Activate DLC                 */

/****************************************************************************/
/*                                                                          */
/*                       Main Program Section                               */
/*                                                                          */
/****************************************************************************/

void
main()

{
  qsemhandle = 0;                          /* Queue semephore not used     */

  SET_LOG_QUEUE();
  while (1 == 1) {
     printf("\nWaiting for Adapter Failure");
                                            /* Read Queue with WAIT option  */
     error_code = DosReadQueue(qhandle,
                               &qrequest,
                               &qlength,
                               &lp.log_ptr,
                               FIFO,
                               WAIT,
                               &qpriority,
                               qsemhandle);
     printf("\nRead Queue Wait error code:  %u", error_code);
     printf("\nError Log Subtype:  %lu", lp.error_log_ptr->elqsubtype);
                                            /* If error subtype 00000002    */
                                            /* then it is either a link     */
                                            /* or an adapter failure.       */
                                            /* Treat as an adapter failure  */
     if (lp.error_log_ptr->elqsubtype == 2) {
                                            /* Free the memory for the      */
                                            /* error log                    */
        error_code = DosSubFree (SELECTOROF(lp.error_log_ptr),
                                 OFFSETOF(lp.error_log_ptr),
                                 44 + lp.error_log_ptr->elqlength);
        printf("\nDosSubFree error code:  %u", error_code);
                                            /* Get X.25 information with    */
                                            /* the DISPLAY verb.            */
        GET_INFO();
                                            /* Reactivate the DLC           */
        if ((vcb.dsp.primary_rc == 0) && (cont == YES)) {
           do {
               CONNECT_X25();
               if (vcb.dsp.primary_rc > 0) {
                   printf("\nSleeping 45 seconds");
                   DosSleep(forty_five_seconds);
               }
           } while (vcb.cl.primary_rc > 0);
        }
     }
  }
}

/****************************************************************************/
/*                                                                          */
/*                            FUNCTIONS                                     */
/*                                                                          */
/****************************************************************************/

void
SET_LOG_QUEUE (void)
{
   strcpy (qname,"\\QUEUES\\ERRLOG17");     /* Create the Queue             */
   error_code = DosCreateQueue(&qhandle,FIFO,qname);
   printf("\nCreate Queue error code:  %u", error_code);
   strcpy (mname,"\\SHAREMEM\\ACSLGMEM");   /* Access the memory where CM   */
   error_code = DosGetShrSeg(mname,&msel);  /* puts the error log           */
   while (error_code == 2) {
      printf("\nWaiting for 30 seconds for Comm Mgr to start");
      DosSleep(thirty_seconds);
      error_code = DosGetShrSeg(mname,&msel);
   } /* endif */
   printf("\nGet Share Memory error code:  %u", error_code);
   clear_vcb();
   vcb.sulq.opcode = SV_SET_USER_LOG_QUEUE;
   strcpy (vcb.sulq.queue_name, qname);     /* Queue name                   */
   vcb.sulq.forward = LOGS;                 /* Receive LOGS only            */
   vcb.sulq.suppress = ALL;                 /* Suppress all onscreen msgs   */
   vcb.sulq.selection = SOME;               /* Receive some error logs      */
   vcb.sulq.numbers[0] = 23;                /* Receive only Type 0017       */
   vcbptr = (unsigned far *)&vcb;
   ACSSVC((long) vcbptr);                   /* Call Common Services         */
   printf("\nSet User Log Queue primary rc:  %2.2X%2.2X",
          vcb.rc.prim_rc1, vcb.rc.prim_rc2);
}

void
GET_INFO (void)
{
   error_code = DosAllocSeg(NUM_BYTES, &alloc_ptr.am.allocsel, ALLOC_FLAGS);
   printf("\nAlloc Seg error code:  %u", error_code);
   clear_vcb();
   vcb.dsp.opcode = AP_DISPLAY;             /* Prepare and call the         */
   vcb.dsp.init_sect_len = LEN_INIT_SECT;   /* DISPLAY verb                 */
   vcb.dsp.buffer_len = NUM_BYTES;
   vcb.dsp.buffer_ptr = alloc_ptr.buff_ptr;
   vcb.dsp.num_sections = NUM_SECTIONS;
   vcb.dsp.x25_physical_link_info = AP_YES;
   vcbptr = (unsigned far *)&vcb;
   ACSMGT((long) vcbptr);                   /* Call Common Services         */
   printf("\nDisplay primary rc:  %2.2X%2.2X",
          vcb.rc.prim_rc1, vcb.rc.prim_rc2);
                                            /* Save the link name and the   */
                                            /* link mode.                   */
                                            /* This code assumes only one   */
                                            /* failure.                     */
   if (vcb.dsp.primary_rc == 0) {
        cont = NO;
        x25_info_ptr =
            (struct x25_overlay far *)
            ((char far *) vcb.dsp.x25_physical_link_info_ptr +
            vcb.dsp.x25_physical_link_info_ptr -> x25_init_sect_len);
        for (i=0;
             i<=vcb.dsp.x25_physical_link_info_ptr->num_x25_link_entries;
             i++) {
            if (x25_info_ptr->link_state == AP_ERROR_LEVEL_1 |
                x25_info_ptr->link_state == AP_ERROR_LEVEL_2 ) {
               for (j=0;j<8;j++) {
               save_link_name[j] = x25_info_ptr->link_name[j];
               }
               save_link_mode = x25_info_ptr->link_mode;
               cont = YES;
            }
        } /* endfor */
   }
}

void
CONNECT_X25 ()
{
  clear_vcb();                                /* Zero the vcb               */
  vcb.dl.opcode = AP_DISCONNECT_PHYSICAL_LINK;/* MGMT verb - ACTIVATE_DLC   */
  memcpy (vcb.dl.physical_link_name, save_link_name, 8);

  vcbptr = (unsigned far *)&vcb;              /* Get pointer to the vcb     */

  ACSMGT((long) vcbptr);                      /* Call MGMT Services         */
  printf("\nDisconnect Physical Link primary rc:  %2.2X%2.2X",
          vcb.rc.prim_rc1, vcb.rc.prim_rc2);

  clear_vcb();                                /* Zero the vcb               */
  vcb.cl.opcode = AP_CONNECT_PHYSICAL_LINK;   /* MGMT verb - ACTIVATE_DLC   */
  memcpy (vcb.cl.physical_link_name, save_link_name, 8);
  if (save_link_mode == TWO) {
     vcb.cl.connection_type = ONE;
  } else {
     vcb.cl.connection_type = ZERO;
  }

  vcbptr = (unsigned far *)&vcb;              /* Get pointer to the vcb     */

  ACSMGT((long) vcbptr);                      /* Call MGMT Services         */
  printf("\nConnect Physical Link primary rc:  %2.2X%2.2X",
          vcb.rc.prim_rc1, vcb.rc.prim_rc2);

}


/* EOF - REACTX25.C */
   