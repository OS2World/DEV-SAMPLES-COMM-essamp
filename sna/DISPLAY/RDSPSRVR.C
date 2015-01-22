/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : RDSPSRVR.C                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME : Remote Display Server Sample Program for             */
/*                      Communications Manager                              */
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
/*      RDSP.MAK     - MAKE file                                            */
/*      RDSPSRVR.DEF - Module definition file (for LINK)                    */
/*      DISPLAY.H    - Global typedefs, prototypes, and #includes           */
/*      RDSPSRVR.C   - Main function and unique utility functions           */
/*      APPCUTIL.C   - Common utility functions                             */
/*      EXECDISP.C   - Utility function to execute DISPLAY verb             */
/*      APD.MSG      - Messages (English)                                   */
/*      MSGID.H      - #defines for messages                                */
/*                                                                          */
/****************************************************************************/

#include "DISPLAY.H"      /* global typedefs, prototypes, and includes      */
#include <STDIO.H>     /* Standard C library I/O functions               */
#include <STRING.H>    /* Standard C library string and memory functions */
#include <STDARG.H>    /* Standard C library variable argument functions */

#define TP_NAME "RDSPSRVR"

/*--------------------------------------------------------------------------*/
/*                       Function Prototypes                                */
/*--------------------------------------------------------------------------*/
int  cdecl main  (void);

/****************************************************************************/
/*                                                                          */
/*                       Main Program Section                               */
/*                                                                          */
/****************************************************************************/
int cdecl main (void) {
   void  far * info_ptr;           /* Pointer to information returned by    */
                                   /* the DISPLAY verb                      */
   UCHAR far * info_buffer_ptr;    /* Pointer to segment to be used by the  */
                                   /* DISPLAY verb for returned information */
   UCHAR far * dri_buffer_ptr;     /* Pointer to DISPLAY status info        */
   DISPLAY_INFO_TYPE option;       /* DISPLAY information type code         */
   DISP_RETURN_INFO  dri;          /* Structure to return from DISPLAY      */
   ULONG  display_length;          /* Length of info returned in buffer     */
   UCHAR  tp_id[8];                /* Transaction Progam ID                 */
   ULONG  conv_id;                 /* Conversation ID                       */
   USHORT what_rcvd;               /* What Received from requestor          */
   BOOL   fDone = FALSE;           /* Flag indicating when finished         */

/* Initialize the text tables from APD.MSG                                  */
   InitializeText();

/* Get a shared memory buffer for info returned by DISPLAY verb.            */
   info_buffer_ptr = alloc_shared_buffer (INFO_BUFFER_SIZE);
   if (NULL == info_buffer_ptr) return (-1); /* exit if error */

/* Get a shared memory buffer for info to return to the requestor.          */
   dri_buffer_ptr = alloc_shared_buffer (sizeof (DISP_RETURN_INFO));
   if (NULL == dri_buffer_ptr) return (-1); /* exit if error */

 /************************************************************************/
 /* Perform Remote DISPLAY.                                              */
 /************************************************************************/

   if (receive_allocate (TP_NAME, tp_id, &conv_id)) {
      while (!fDone) {
      /* Get the option to DISPLAY                                          */
         what_rcvd = mc_receive_and_wait (info_buffer_ptr, tp_id, conv_id,
                                          sizeof (DISPLAY_INFO_TYPE));
         switch (what_rcvd) {
            case AP_DATA_COMPLETE_SEND:
            /* Do the DISPLAY                                               */
               memcpy (&option, info_buffer_ptr,
                       sizeof (DISPLAY_INFO_TYPE));
               info_ptr = exec_display (option, info_buffer_ptr,
                                        INFO_BUFFER_SIZE,
                                        &dri.primary_rc, &dri.secondary_rc,
                                        &display_length);
            /* Calculate the offset of the information section              */
               if (AP_OK == dri.primary_rc) {
                  dri.info_offset = (ULONG) (*(UCHAR far *)info_ptr -
                                             *info_buffer_ptr);
                  } /* endif */
            /* Send the return codes and offset back to the requestor       */
               memcpy (dri_buffer_ptr, &dri, sizeof (DISP_RETURN_INFO));
               if (mc_send_data (dri_buffer_ptr, tp_id, conv_id,
                                 sizeof (DISP_RETURN_INFO),
                                 AP_SEND_DATA_FLUSH)) {
            /* Send the DISPLAY buffer back to the requestor                */
            /* Note: It is alright to cast display_length field to USHORT,  */
            /*       because the actual value is never larger than 0xFFFF.  */
                  if (!(mc_send_data (info_buffer_ptr, tp_id, conv_id,
                                      (USHORT) display_length, AP_NONE))) {
                     mc_deallocate (tp_id, conv_id, AP_ABEND);
                     fDone = TRUE;
                     } /* endif */
               } else { /* SEND_DATA 1 failed */
                  mc_deallocate (tp_id, conv_id, AP_ABEND);
                  fDone = TRUE;
                  } /* endif */
               break;
            case AP_CONFIRM_DEALLOCATE:
               /* MC_CONFIRMED */
               mc_confirmed (tp_id, conv_id);
               fDone = TRUE;
               break;
         /* There are a variety of errors you could check for here, to do   */
         /* error recovery.  Here I perform no error recovery, I just       */
         /* deallocate abend.  During error recovery, you might want to do  */
         /* a SEND_ERROR, followed by a SEND_DATA, to tell the requestor    */
         /* about the error, so it can take whateveraction is necessary to  */
         /* recover.  Note that the same is true if the SEND_DATAs above    */
         /* fail.                                                           */
            default:
               mc_deallocate (tp_id, conv_id, AP_ABEND);
               fDone = TRUE;
            } /* endswitch */
         } /* endwhile */
   /* End the transaction program                                           */
      tp_ended (tp_id);
      } /* endif (RECEIVE_ALLOCATE) */

/* Free the DISPLAY info buffer.                                            */
   free_shared_buffer( info_buffer_ptr );

   return 0;
}

/****************************************************************************/
/* myprintf:  Formatting function called by print_info functions.  Directs  */
/*            output to stdout.                                             */
/****************************************************************************/
int cdecl myprintf(char * string, ...)
{
   va_list arg_ptr;                    /* Pointer to variable argument list */

   va_start(arg_ptr, string);          /* Set pointer to argument list      */
                                       /* following "string"                */
   return(vprintf(string, arg_ptr));   /* Format output to stdout           */
}
