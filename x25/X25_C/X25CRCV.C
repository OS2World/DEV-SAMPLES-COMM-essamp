/*****************************************************************************/
/* Module      : X25CRCV.C                                                   */
/*                                                                           */
/* Copyright   : (C) Copyright IBM Corp. 1989,1991                           */
/*               Licensed Material - Program Property of IBM                 */
/*               All Rights Reserved                                         */
/*                                                                           */
/* Compiler    : Microsoft C Compiler Version 6.00                           */
/*               Microsoft is a registered trademark of Microsoft Corporation*/
/*                                                                           */
/* Description : This is the receiver part of the sample program to          */
/*               demonstrate use of the X.25 API supplied with the OS/2      */
/*               Extended Services Communications Manager.                   */
/*                                                                           */
/*               After initialising the X.25 API, the program starts         */
/*               listening for an incoming call corresponding to a name      */
/*               in the routing table. The name is given as the single       */
/*               parameter to this program.                                  */
/*                                                                           */
/*               When the incoming call is received, it is accepted and      */
/*               the program then starts to receive data transmitted         */
/*               from the remote application. Acknowlegdements are sent      */
/*               when required.                                              */
/*                                                                           */
/*               The program loops until a an end of data marker is          */
/*               received. It then sends a clear packet to acknowledge       */
/*               that all data has been received before tidying up and       */
/*               exiting.                                                    */
/*                                                                           */
/*               In the event of an error, a message is printed,             */
/*               and the program tries to tidy up before exiting.            */
/*                                                                           */
/*               The program uses the following X.25 API verbs:              */
/*                                                                           */
/*                   X25APPINIT                                              */
/*                   X25LISTEN                                               */
/*                   X25CALLRECEIVE                                          */
/*                   X25CALLACCEPT                                           */
/*                   X25CALLCLEAR                                            */
/*                   X25DATARECEIVE                                          */
/*                   X25ACK                                                  */
/*                   X25DEAFEN                                               */
/*                   X25APPTERM                                              */
/*                                                                           */
/* Note        : The name supplied as a parameter must already exist in      */
/*               the routing table. It must allow reception of any           */
/*               incoming calls. An entry created from model profile         */
/*               M7 is suitable for this purpose.                            */
/*                                                                           */
/*               This program should be started before the transmitter       */
/*               program.                                                    */
/*****************************************************************************/

#include <stdio.h>                         /* Standard include files         */
#include <string.h>
#include <process.h>
#include <dos.h>
#include <doscalls.h>

#include <x25_c.h>                         /* The X.25 API definitions       */

#define BUFFER_SIZE       4096             /* constants used in this program */

#define TRUE              1
#define FALSE             0
#define NOTIMEOUT         -1L              /* used by DOSSEMWAIT             */
#define SEGSIZE           32767        /* size for DOSALLOCSEG and DOSSUBSET */
#define SHAREABLE         (0x03)           /* flags parm for DOSALLOCSEG     */
#define INITIALISE        1                /* flags parm for DOSSUBSET       */


/*****************************************************************************/
/* prototypes for functions defined in this program                          */
/*****************************************************************************/
static void make_xvrb_init(X25VERB far*);
static void make_xvrb_listen(X25VERB far*, X25LISTEN_DATA far*, char*);
static void make_xvrb_deafen(X25VERB far*, X25LISTEN_DATA far*, char* );
static void make_xvrb_callrcv(X25VERB far*, X25CALL_DATA far*);
static void make_xvrb_callacc(X25VERB far*, long);
static void make_xvrb_rcv(X25VERB far*, X25DATA_DATA far*, long);
static void make_xvrb_clear(X25VERB far*, long);
static void sys_err(X25VERB far*, char*);

/*****************************************************************************/
/* Function: main                                                            */
/* Start of the receiver program.                                            */
/*****************************************************************************/
main(
     int    argc,
     char **argv)
{
  /***************************************************************************/
  /* Variable declares and definitions                                       */
  /***************************************************************************/
  X25VERB        far *xvrb;                /* Main X.25 control block        */
  X25CALL_DATA   far *xcall_data;          /* Call Accept data block         */
  X25DATA_DATA   far *xdata_data;          /* The data to be received        */
  X25LISTEN_DATA far *xlisten_data;        /* Listen data block              */

  unsigned short seg;           /* identifier for OS/2 shared memory segment */

  unsigned long connid;                    /* Connection identifier          */
  unsigned char data_type;                 /* Data type                      */

  char err_text[128];                      /* Message text                   */
  short rc;                                /* Immediate Return code          */
  static unsigned char eom[] = {"***"};

  /***************************************************************************/
  /* Check number of parameters the program was called with.                 */
  /* If there is only 1 parameter, we assume that it is in the correct       */
  /* numeric form and continue, otherwise exit.                              */
  /***************************************************************************/
  if (argc != 2)
  {
    (void)printf("Syntax is %s <name>\n",argv[0]);
    return(1);
  }

  /***************************************************************************/
  /* Allocate an OS/2 memory segment to share with the X.25 API.             */
  /* The segment requested is 32K long, which is at least as big as the      */
  /* data which will be put into the segment later by DosSubAlloc calls.     */
  /***************************************************************************/
  if (DOSALLOCSEG(SEGSIZE, (unsigned far *)&seg, SHAREABLE) != 0)
  {
    (void)printf("Couldn't allocate OS/2 segment\n");
    return(1);
  }

  /***************************************************************************/
  /* Prepare allocated segment for suballocation.                            */
  /***************************************************************************/
  (void)DOSSUBSET (seg, INITIALISE, SEGSIZE);

  /***************************************************************************/
  /* Allocate memory for each of the pointers to point at - assume return    */
  /* values are good.                                                        */
  /***************************************************************************/
  (void)DOSSUBALLOC (seg, (unsigned far *) &xvrb, sizeof(X25VERB));
  (void)DOSSUBALLOC (seg, (unsigned far *)&xdata_data,
                                            sizeof(X25DATA_DATA)*BUFFER_SIZE);
  (void)DOSSUBALLOC (seg, (unsigned far *)&xcall_data, sizeof(X25CALL_DATA));
  (void)DOSSUBALLOC (seg, (unsigned far *)&xlisten_data,
                                                      sizeof(X25LISTEN_DATA));
  FP_SEG(xvrb)       = seg;                /* set our pointers               */
  FP_SEG(xdata_data) = seg;                /* to the shared segment          */
  FP_SEG(xcall_data) = seg;
  FP_SEG(xlisten_data) = seg;


  /***************************************************************************/
  /* We can now try to initialise the X.25 API.                              */
  /* First call an internal function to set all the values in the            */
  /* control block and then call the X25 function.                           */
  /***************************************************************************/
  make_xvrb_init(xvrb);

  (void)printf("Initialising the X.25 API\n");
  rc = X25(xvrb);                          /* Call X.25 API                  */

  if (rc != X25_OK)                        /* Check immediate return code    */
  {
    (void)sprintf(err_text,
                 "Couldn't initialise X.25 API, immediate return code of %d\n",
                 rc);
    sys_err(xvrb,err_text);
  }

  (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */

  if (xvrb->return_code != X25_OK)         /* Check completion return code   */
  {
    (void)sprintf(err_text,
               "Couldn't initialise X.25 API, completion return code of %d\n",
               xvrb->return_code);
    sys_err(xvrb,err_text);
   }
  else
    (void)printf("X.25 API Initialised\n");

  /***************************************************************************/
  /* Now we prepare to receive incoming calls by issuing a listen for a      */
  /* routing table entry name.                                               */
  /***************************************************************************/
  make_xvrb_listen(xvrb,xlisten_data,argv[1]);

  (void)printf("Preparing to listen for a call\n");
  rc = X25(xvrb);                          /* Call X.25 API                  */

  if (rc != X25_OK)                        /* Check immediate return code    */
  {
    (void)sprintf(err_text,
                 "Listen failed, immediate return code of %d\n",rc);
    sys_err(xvrb,err_text);
  }

  (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */

  if (xvrb->return_code != X25_OK )        /* Check completion return code   */
  {
    (void)sprintf(err_text,
                  "Listen failed, completion return code of %d\n",
                   xvrb->return_code);
    sys_err(xvrb,err_text);
  }
  else
    (void)printf("Now ready to receive an incoming call\n");

  /***************************************************************************/
  /* We now issue a call receive verb in order to receive any incoming call  */
  /* that may be routed to us                                                */
  /***************************************************************************/
  make_xvrb_callrcv(xvrb,xcall_data);

  (void)printf("Waiting for a call\n");
  rc = X25(xvrb);                          /* Call X.25 API                  */

  if (rc != X25_OK)                        /* Check immediate return code    */
  {
    (void)sprintf(err_text,
                 "Call Receive failed, immediate return code of %d\n",rc);
    sys_err(xvrb,err_text);
  }

  (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */

  if (xvrb->return_code != X25_OK )        /* Check completion return code   */
  {
    (void)sprintf(err_text,
                  "Call Receive failed, completion return code of %d\n",
                   xvrb->return_code);
    sys_err(xvrb,err_text);
  }
  else
  {
    connid = xvrb->connection_id;          /* save connection identifier     */
    (void)printf("Incoming call has arrived\n");
  }

  /***************************************************************************/
  /* We now have an incoming call, so accept it.                             */
  /***************************************************************************/
  (void)make_xvrb_callacc(xvrb,connid);

  (void)printf("Accepting the call\n");
  rc = X25(xvrb);                          /* Call X.25 API                  */

  if (rc != X25_OK)                        /* Check immediate return code    */
  {
    (void)sprintf(err_text,
                 "Call Accept failed, immediate return code of %d\n",rc);
    sys_err(xvrb,err_text);
  }

  (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */

  if (xvrb->return_code != X25_OK )        /* Check completion return code   */
  {
    (void)sprintf(err_text,
                  "Call Accept failed, completion return code of %d\n",
                  xvrb->return_code);
    sys_err(xvrb,err_text);
  }
  else
    (void)printf("Call Accepted OK\n");

  /***************************************************************************/
  /* Now loop round receiving data until a Clear Indication is received.     */
  /***************************************************************************/
  do
  {
    make_xvrb_rcv(xvrb,xdata_data,connid); /* Prepare control block          */

    (void)printf("Waiting for data\n");
    rc = X25(xvrb);                        /* Call X.25 API                  */

    if (rc != X25_OK)                      /* Check immediate return code    */
    {
      (void)sprintf(err_text,
                   "Data Receive failed, immediate return code of %d\n",rc);
      sys_err(xvrb,err_text);
    }

    (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */

    if (xvrb->return_code != X25_OK )      /* Check completion return code   */
    {
      (void)sprintf(err_text,
                    "Data Receive failed, completion return code of %d\n",
                     xvrb->return_code);
      sys_err(xvrb,err_text);
    }
    else
    {
      data_type = xvrb->data_event_type;
      (void)printf("Data Receive completed successfully\n");
    }
    /*************************************************************************/
    /* If data received then display it                                      */
    /*************************************************************************/
    if (xvrb->data_event_type == X25DATARCV_DATA
           & xvrb->data_buffer_ptr.data_data[1] != eom[1])
    {
      printf("%s\n",xvrb->data_buffer_ptr.data_data); /* Show data received  */
    }
    /*************************************************************************/
    /* See if acknowledgement is required.                                   */
    /*************************************************************************/
    if (xvrb->d_bit == TRUE && xvrb->data_event_type == X25DATARCV_DATA )
    {
      xvrb->verb_code = X25ACK;            /* Set up xvrb structure          */
      xvrb->version_id = X25_API_VERSION;
      xvrb->connection_id = connid;        /* Connection identifier          */
      xvrb->queue_number = 0;              /* Queues not used by this appln  */

      (void)printf("Sending an Ack\n");
      rc = X25(xvrb);                      /* Call X.25 API                  */

      if (rc != X25_OK)                    /* Check immediate return code    */
      {
        (void)sprintf(err_text,
                      "Ack failed, immediate return code of %d\n",rc);
        sys_err(xvrb,err_text);
      }
      (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */

      if (xvrb->return_code != X25_OK )    /* Check completion return code   */
      {
        (void)sprintf(err_text,
                      "Ack failed, completion return code of %d\n",
                       xvrb->return_code);
        sys_err(xvrb,err_text);
      }
    }
  } while (data_type == X25DATARCV_DATA
           & xvrb->data_buffer_ptr.data_data[1] != eom[1]);

  if (data_type != X25DATARCV_DATA)      /* if non data packet received      */
     (void)printf("Unexpected type of data has been received\n");
  else
     (void)printf("End of data Indicator received\n");


  /***************************************************************************/
  /* Now that all data has been sent the call is cleared                     */
  /***************************************************************************/
  make_xvrb_clear(xvrb,connid);            /* Set up control block           */

  (void)printf("Clearing the call\n");
  rc = X25(xvrb);                          /* Call X.25 API                  */

  if (rc != X25_OK)                        /* Check immediate return code    */
  {
    (void)sprintf(err_text, "Clear failed, immediate return code of %d\n",rc);
    sys_err(xvrb,err_text);
  }

  (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */

  if (xvrb->return_code != X25_OK )        /* Check completion return code   */
  {
    (void)sprintf(err_text,"Clear failed, completion return code of %d\n",
                  xvrb->return_code);
    sys_err(xvrb,err_text);
  }
  (void)printf("Call Cleared OK\n");

  /***************************************************************************/
  /* Now end the program after issuing the Deafen & Termination verbs.       */
  /***************************************************************************/
  make_xvrb_deafen(xvrb,xlisten_data,argv[1]);

  (void)printf("Issuing X25Deafen\n");
  rc = X25(xvrb);                          /* Call X.25 API                  */

  if (rc != X25_OK)                        /* Check immediate return code    */
  {
    (void)sprintf(err_text,
                  "Deafen failed, immediate return code of %d\n",rc);
    sys_err(xvrb,err_text);
  }
  (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */
  if (xvrb->return_code != X25_OK )        /* Check completion return code   */
  {
    (void)sprintf(err_text,
                  "X25Deafen failed, completion return code of %d\n",
                   xvrb->return_code);
    sys_err(xvrb,err_text);
  }

  xvrb->verb_code = X25APPTERM;            /* API function to be called      */
  xvrb->version_id = X25_API_VERSION;

  (void)printf("Issuing X25AppTerm\n");
  rc = X25(xvrb);                          /* Call X.25 API                  */

  if (rc != X25_OK)                        /* Check immediate return code    */
  {
    (void)sprintf(err_text,
             "Failed to terminate the X.25 API, immediate return code of %d\n",
              rc);
    sys_err(xvrb,err_text);
  }
  (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */

  (void)printf("Program finished\n");

  return(0);                               /* End program                    */
}

/*****************************************************************************/
/* Function: make_xvrb_init                                                  */
/* This function sets up the control blocks needed to initialise the         */
/* X.25 API.                                                                 */
/*****************************************************************************/
static void make_xvrb_init(
             X25VERB far * xvrb)           /* The main control block         */
{
  xvrb->verb_code = X25APPINIT;            /* API function to be called      */
  xvrb->version_id = X25_API_VERSION;
  xvrb->data_buffer_size = 0;              /* The data buffer is not used    */
  return;
}

/*****************************************************************************/
/* Function: make_xvrb_listen                                                */
/* This function sets up the control blocks needed to listen for an          */
/* incoming call. The name should already exist in the routing table.        */
/*****************************************************************************/
static void make_xvrb_listen(
            X25VERB        far *xvrb,         /* The main control block      */
            X25LISTEN_DATA far *xlisten_data, /* Verb specific control block */
            char               *name)         /* Routing table entry name    */
{
  xvrb->verb_code = X25LISTEN ;            /* API function to be called      */
  xvrb->version_id = X25_API_VERSION;
  xvrb->queue_number = 0;                  /* Queues not used by this appln  */
  xvrb->data_buffer_ptr.listen_data = xlisten_data;
  xvrb->data_buffer_size = sizeof(*xlisten_data);

  xlisten_data->num_of_rtes = 1;
  /***************************************************************************/
  /* Fill in the name                                                        */
  /***************************************************************************/
  (void)strncpy(xlisten_data->rte_entry_list[0].rte_name,name,8);
  return;
}

/*****************************************************************************/
/* Function: make_xvrb_deafen                                                */
/* This function sets up the control blocks needed to stop listening for     */
/* incoming calls. The name should already exist in the routing table.       */
/*****************************************************************************/
static void make_xvrb_deafen(
            X25VERB        far *xvrb,         /* The main control block      */
            X25LISTEN_DATA far *xlisten_data, /* Verb specific control block */
            char               *name)         /* Name in routing table       */
{
  xvrb->verb_code = X25DEAFEN;             /* API function to be called      */
  xvrb->version_id = X25_API_VERSION;
  xvrb->queue_number = 0;                  /* Queues not used by this appln  */
  xvrb->data_buffer_ptr.listen_data = xlisten_data;
  xvrb->data_buffer_size = sizeof(*xlisten_data);

  xlisten_data->num_of_rtes = 1;

  /***************************************************************************/
  /* Copy name into the routing table entry list.                            */
  /***************************************************************************/
  (void)strncpy(xlisten_data->rte_entry_list[0].rte_name,name,8);

  return;
}

/*****************************************************************************/
/* Function: make_xvrb_callrcv                                               */
/* This function sets up the control blocks needed to receive incoming       */
/* calls.                                                                    */
/*****************************************************************************/
static void make_xvrb_callrcv(
             X25VERB      far *xvrb,       /* The main control block         */
             X25CALL_DATA far *xcall_data) /* Verb specific control block    */
{
  xvrb->verb_code = X25CALLRECEIVE;        /* API function to be called      */
  xvrb->version_id = X25_API_VERSION;
  xvrb->verb_option = X25CALLRCV_ADDRESS;  /* Do not convert address         */
  xvrb->queue_number = 0;                  /* Queues not used by this appln  */
  xvrb->data_buffer_ptr.call_data = xcall_data;
  xvrb->data_buffer_size = sizeof(*xcall_data);

  return;
}

/*****************************************************************************/
/* Function: make_xvrb_callacc                                               */
/* This function sets up the control block needed to accept an incoming      */
/* call.                                                                     */
/*****************************************************************************/
static void make_xvrb_callacc(
             X25VERB far *xvrb,            /* The main control block         */
             long         connid)          /* Connection identifier          */
{
  xvrb->verb_code = X25CALLACCEPT;         /* API function to be called      */
  xvrb->version_id = X25_API_VERSION;
  xvrb->queue_number = 0;                  /* Queues not used by this appln  */
  xvrb->data_buffer_ptr.call_data = NULL;

/*****************************************************************************/
/* We are not interested in the contents of the incoming call packet so we   */
/* do not supply a data buffer.                                              */
/*****************************************************************************/
  xvrb->data_buffer_size = 0;
  xvrb->connection_id = connid;

  /***************************************************************************/
  /* This circuit will be using D-bit procedures later on, so we need to     */
  /* inform the API before the call is completed.                            */
  /***************************************************************************/
  xvrb->d_bit_management = X25DBIT_MANAGE_APP;   /* Acks to be done by appln */
  return;
}

/*****************************************************************************/
/* Function: make_xvrb_rcv                                                   */
/* This function sets up the control blocks needed to receive incoming       */
/* data.                                                                     */
/*****************************************************************************/
static void make_xvrb_rcv(
             X25VERB      far *xvrb,       /* The main control block         */
             X25DATA_DATA far *xdata_data, /* Verb specific control block    */
             long              connid)     /* Connection identifier          */
{
  xvrb->verb_code = X25DATARECEIVE;        /* API function to be called      */
  xvrb->version_id = X25_API_VERSION;
  xvrb->queue_number = 0;                  /* Queues not used by this appln  */
  xvrb->connection_id = connid;
  xvrb->data_buffer_ptr.data_data = xdata_data;
  xvrb->data_buffer_size = sizeof(X25DATA_DATA)*BUFFER_SIZE;

  return;
}

/*****************************************************************************/
/* Function: make_xvrb_clear                                                 */
/* This function sets up the control block needed to clear a call.           */
/*****************************************************************************/
static void make_xvrb_clear(
             X25VERB far *xvrb,            /* Main control block             */
             long         connid)          /* Connection identifier          */
{
  xvrb->verb_code = X25CALLCLEAR;          /* API function called            */
  xvrb->version_id = X25_API_VERSION;
  xvrb->connection_id = connid;            /* Which call to clear            */
  xvrb->queue_number = 0;                  /* Queues not used by this appln  */
  xvrb->data_buffer_ptr.call_data = NULL;  /* No interest in returned data   */
  xvrb->data_buffer_size = 0;              /* no buffer supplied             */
  xvrb->cause_code = 0;                   /* Cause code in Clear Request pkt */
  xvrb->diagnostic_code = 0;         /* Diagnostic code in Clear Request pkt */

  /***************************************************************************/
  /* The verb may be considered complete before the Clear Confirm            */
  /* packet has been received.                                               */
  /***************************************************************************/
  xvrb->verb_option = X25CALLCLEAR_NOWAIT;

  return;
}


/*****************************************************************************/
/* Function to print an error message to stderr, tidy up and exit            */
/*****************************************************************************/
static void sys_err(
             X25VERB  far *xvrb,           /* Main control block             */
             char     *errmsg)             /* Error message                  */
{
  /***************************************************************************/
  /* Try to terminate the X.25 API cleanly, but ignore any errors returned.  */
  /***************************************************************************/
  xvrb->verb_code = X25APPTERM;            /* API function called            */
  xvrb->version_id = X25_API_VERSION;

  (void)printf("Terminating the X25 API\n");

  if (X25(xvrb) == X25_OK)                 /* Call X.25 API                  */
  (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);

  (void)fprintf(stderr,"%s",errmsg);       /* Print the error message        */
  exit(1);                                 /* End the program                */
}

