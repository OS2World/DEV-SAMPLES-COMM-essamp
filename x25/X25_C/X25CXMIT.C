/*****************************************************************************/
/* Module      : X25CXMIT.C                                                  */
/*                                                                           */
/* Copyright   : (C) Copyright IBM Corp. 1989,1991                           */
/*               Licensed Material - Program Property of IBM                 */
/*               All Rights Reserved                                         */
/*                                                                           */
/* Compiler    : Microsoft C Compiler Version 6.00                           */
/*               Microsoft is a registered trademark of Microsoft Corporation*/
/*                                                                           */
/* Description : This is the transmitter part of the sample program to       */
/*               demonstrate use of the X.25 API supplied in the OS/2        */
/*               Extended Services Communications Manager.                   */
/*                                                                           */
/*               After initialising the X.25 API, the program places         */
/*               a call to the name given as the parameter to the program.   */
/*               It transmits several blocks of data before sending an end   */
/*               of data marker. It then waits for the receiver to           */
/*               acknowledge receipt of the data by sending a clear packet.  */
/*                                                                           */
/*               In the event of an error, a message is printed,             */
/*               and the program tries to tidy up before exiting.            */
/*                                                                           */
/*               The program uses the following X.25 API verbs:              */
/*                                                                           */
/*                   X25APPINIT                                              */
/*                   X25CALL                                                 */
/*                   X25DATASEND                                             */
/*                   X25DATARECEIVE                                          */
/*                   X25APPTERM                                              */
/*                                                                           */
/* Note        : This program should be started after the corresponding      */
/*               receiver program, X25CRCV.                                  */
/*                                                                           */
/*               The name given must exist in the X.25 Directory. It should  */
/*               refer to a non-SNA remote SVC.                              */
/*                                                                           */
/*               D bit acknowledgement can be requested by setting USE_DBIT  */
/*               to 1.                                                       */
/*****************************************************************************/

#include <stdio.h>                         /* Standard include files         */
#include <string.h>
#include <process.h>
#include <dos.h>
#include <doscalls.h>
#include <x25_c.h>                         /* The X.25 API definitions       */


#define RCV_BUFFER_SIZE   310             /* buffer size for call clear data */
#define BUFFER_SIZE       128              /* buffer size for transmit data  */
#define MESSAGE_SIZE      37               /* length of actual transmit data */
#define NUMBER_OF_MESSAGES 5

#define TRUE              1
#define FALSE             0
#define USE_DBIT          0                /* don't use d bit                */
#define NOTIMEOUT         -1L
#define SEGSIZE           32767        /* size for DOSALLOCSEG and DUSSUBSET */
#define SHAREABLE         (0x03)            /* flags parm for DOSALLOCSEG    */
#define INITIALISE        1                 /* flags parameter for DOSSUBSET */

/*****************************************************************************/
/* Prototypes for functions defined in this program                          */
/*****************************************************************************/
static void make_xvrb_init (X25VERB far*);
static void make_xvrb_call(X25VERB far*, X25CALL_DATA far*, char*);
static void make_xvrb_send(X25VERB far*, X25DATA_DATA far*, long, char*);
static void make_xvrb_rcv(X25VERB far*, X25DATA_DATA far*, long);
static void sys_err (X25VERB far*, char*);

/*****************************************************************************/
/* Function: main                                                            */
/* Start of the transmitter program                                          */
/*****************************************************************************/
main(
     int    argc,
     char **argv)
{
  /***************************************************************************/
  /* Variable definitions                                                    */
  /***************************************************************************/
  X25VERB      far *xvrb;          /* Pointer to the main X.25 control block */
  X25CALL_DATA far *xcall_data;            /* Call Request data block        */
  X25DATA_DATA far *xdata_data;            /* The data to be transmitted     */

  unsigned short seg;           /* identifier for OS/2 shared memory segment */
  unsigned long connid;                    /* Connection identifier          */
  unsigned char data_type;                 /* Data type                      */
  static char mmm[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890"};
  static char eom[] = {"***"};

  char err_text[128];                      /* Message text                   */
  short rc;                                /* Immediate Return code          */
  int i;                                   /* loop counter                   */

  /***************************************************************************/
  /* Check number of parameters the program was called with.                 */
  /* If there is only 1 parameter, it is taken as a name from the            */
  /* X.25 directory so we continue, otherwise exit.                          */
  /***************************************************************************/
  if (argc != 2)
  {
    (void)printf("Syntax is %s <name>\n",argv[0]);
    return(1);
  }

  /***************************************************************************/
  /* Allocate an OS/2 data segment to share with the X.25 API.               */
  /* The segment requested is 32K long, which is at least as big as the      */
  /* data which will be put into the segment later by DosSubAlloc calls.     */
  /***************************************************************************/
  if (DOSALLOCSEG (SEGSIZE, (unsigned far *)&seg, SHAREABLE) != 0)
  {
    (void)printf("Couldn't allocate OS/2 segment\n");
    return(1);
  }

  /***************************************************************************/
  /* Prepare allocated segment for suballocation.                            */
  /***************************************************************************/
  (void)DOSSUBSET (seg, INITIALISE, SEGSIZE);

  /***************************************************************************/
  /* Allocate memory for each of the pointers to point at - assume there     */
  /* are good return values.                                                 */
  /***************************************************************************/
  (void)DOSSUBALLOC (seg, (unsigned far *) &xvrb, sizeof(X25VERB));
  (void)DOSSUBALLOC (seg, (unsigned far *) &xdata_data,
                                           sizeof(X25DATA_DATA)*BUFFER_SIZE);
  (void)DOSSUBALLOC (seg, (unsigned far *) &xcall_data,
                                                       sizeof(X25CALL_DATA));

  FP_SEG(xvrb)       = seg;        /* set our pointers                       */
  FP_SEG(xdata_data) = seg;        /* to the shared segment                  */
  FP_SEG(xcall_data) = seg;
  /***************************************************************************/
  /* We can now initialise the X.25 API.                                     */
  /* First call an internal function to set all the values in the            */
  /* control block and then call the X25 function.                           */
  /***************************************************************************/

  make_xvrb_init(xvrb);

  (void)printf("Issuing X25AppInit verb\n");
  rc = X25(xvrb);                          /* Call X.25 API                  */

  if (rc != X25_OK)                        /* Check immediate return code    */
  {
    (void)sprintf(err_text,
                 "X25AppInit verb failed, immediate return code of %d\n",rc);
    sys_err(xvrb,err_text);
  }

  (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */

  if (xvrb->return_code != X25_OK)         /* Check completion return code   */
  {
    (void)sprintf(err_text,
                 "X25AppInit verb failed, completion return code of %d\n",
                 xvrb->return_code);
    sys_err(xvrb,err_text);
   }
  else
    (void)printf("X25AppInit verb completed successfully\n");

  /***************************************************************************/
  /* Now that the API is initialised, the call can be placed. Again          */
  /* an internal function is called to set up all the control information    */
  /* and then the API function is called. After getting the immediate        */
  /* return code, the program waits for the associated semaphore to be       */
  /* cleared, indicating completion of the function. The call should now     */
  /* have been accepted by the remote application. The immediate and         */
  /* completion return code are checked. A special case is considered of     */
  /* the function failing because the call was cleared.                      */
  /***************************************************************************/

  make_xvrb_call(xvrb,xcall_data,argv[1]);

  (void)printf("Making a call\n");
  rc = X25(xvrb);                          /* Call X.25 API                  */

  if (rc != X25_OK)                        /* Check immediate return code    */
  {
    (void)sprintf(err_text,
                 "Couldn't make call, immediate return code of %d\n",rc);
    sys_err(xvrb,err_text);
  }

  (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */

  if (xvrb->return_code != X25_OK )        /* Check completion return code   */
  {
    if (xvrb->return_code == X25_CALL_CLEARED)
    {
      (void)sprintf(err_text,
            "Call was cleared - Cause %02X (Hex) Diagnostic %02X (Hex)\n",
            xvrb->cause_code,
            xvrb->diagnostic_code);
      sys_err(xvrb,err_text);
    }
    else
    {
      (void)sprintf(err_text,
                    "Couldn't make call, completion return code of %d\n",
                    xvrb->return_code);
      sys_err(xvrb,err_text);
    }
  }
  else
  {
    connid = xvrb->connection_id;          /* save connection identifier     */
    (void)printf("Connection established to remote DTE\n");
  }
  /***************************************************************************/
  /* Now send the required number of data packets. The packets will have the */
  /* D-bit set, so that the wait on the semaphore will be completed only     */
  /* after the remote application has issued an acknowledgement              */
  /***************************************************************************/
  for (i=0;i<NUMBER_OF_MESSAGES;i++)
  {
    make_xvrb_send(xvrb,xdata_data,connid,mmm);

    (void)printf("Sending data\n");
    rc = X25(xvrb);                        /* Call the X.25 API              */

    if (rc != X25_OK)                      /* Check immediate return code    */
    {
      (void)sprintf(err_text, "Send failed, immediate return code of %d\n",rc);
      sys_err(xvrb,err_text);
    }

    (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */

    if (xvrb->return_code != X25_OK )      /* Check completion return code   */
    {
      (void)sprintf(err_text,
                    "Send failed, completion return code of %d\n",
                    xvrb->return_code);
      sys_err(xvrb,err_text);
    }
    else
      (void)printf("Sent data OK ...\n");
  }

  /***************************************************************************/
  /* All data has now been sent so send the end of data marker               */
  /***************************************************************************/

    make_xvrb_send(xvrb,xdata_data,connid,eom);

    (void)printf("Sending End of data marker\n");
    rc = X25(xvrb);                        /* Call the X.25 API              */

    if (rc != X25_OK)                      /* Check immediate return code    */
    {
      (void)sprintf(err_text, "Send failed, immediate return code of %d\n",rc);
      sys_err(xvrb,err_text);
    }

    (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */

    if (xvrb->return_code != X25_OK )      /* Check completion return code   */
    {
      (void)sprintf(err_text,
                    "Send failed, completion return code of %d\n",
                    xvrb->return_code);
      sys_err(xvrb,err_text);
    }
    else
      (void)printf("Sent data OK ...\n");

  /***************************************************************************/
  /* Now wait for clear packet                                               */
  /***************************************************************************/
  do
  {
    make_xvrb_rcv(xvrb,xdata_data,connid); /* Prepare control block          */

    (void)printf("Waiting for clear packet\n");
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
    /* Ensure clear has been received                                        */
    /*************************************************************************/
    if (xvrb->data_event_type == X25DATARCV_CLEAR)
    {
      (void)printf("Clear indication packet received\n");
    }
    else
    {
      (void)printf("Unexpected type of data has been received\n");
    }
  } while (data_type != X25DATARCV_CLEAR); /* until a clear packet received  */

  /***************************************************************************/
  /* Now issue the X25AppTerm verb                                           */
  /***************************************************************************/
  xvrb->verb_code = X25APPTERM;            /* API function to be called      */

  (void)printf("Terminating the X.25 API\n");
  rc = X25(xvrb);                          /* Call X.25 API                  */

  if (rc != X25_OK)                        /* Check immediate return code    */
  {
    (void)sprintf(err_text,
         "Failed to terminate the X.25 API, immediate return code of %d\n",rc);
    sys_err(xvrb,err_text);
  }

  (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);
                                           /* Wait on the semaphore          */

  (void)printf("Program finished successfully\n");

  return(0);                               /* End program                    */
}

/*****************************************************************************/
/* Function: make_xvrb_init                                                  */
/* This function sets up the control blocks needed to initialise the         */
/* X.25 API.                                                                 */
/*****************************************************************************/
static void make_xvrb_init(
             X25VERB far *xvrb)            /* The main control block         */
{
  xvrb->verb_code = X25APPINIT;            /* API function to be called      */
  xvrb->version_id = X25_API_VERSION;
  xvrb->data_buffer_size = 0;              /* The data buffer is not used    */
  return;
}

/*****************************************************************************/
/* Function: make_xvrb_call                                                  */
/* This function sets up the control blocks needed to make a call.           */
/*****************************************************************************/
static void make_xvrb_call(
             X25VERB      far *xvrb,       /* Main control block             */
             X25CALL_DATA far *xcall_data, /* Verb specific control block    */
             char             *name)       /* Name from directory            */
{
  xvrb->verb_code = X25CALL;               /* API function to be called      */
  xvrb->version_id = X25_API_VERSION;
  xvrb->queue_number = 0;                  /* Queues not used by this appln  */
  xvrb->data_buffer_ptr.call_data = xcall_data; /* Ptr to call info          */
  xvrb->data_buffer_size = sizeof(*xcall_data);

  /***************************************************************************/
  /* This circuit will not be using D-bit procedures later on, so we need to */
  /* inform the API before the call is established.                          */
  /***************************************************************************/
  xvrb->d_bit = USE_DBIT;

  /***************************************************************************/
  /* Now show how long the name is, and copy it into the control block.      */
  /* The name is used by the subsystem to arrive at the network addresses    */
  /* and link information needed for the call. It has a maximum length of    */
  /* 8 bytes.                                                                */
  /***************************************************************************/
  xcall_data->called_address_length = strlen(name) > 8 ? 8 : strlen(name);
  (void)strncpy(xcall_data->called_address,name,8);

  xcall_data->calling_address_length = 0;   /* no calling address            */
  xcall_data->facilities_length = 0;        /* no facilities data            */
  xcall_data->cud_length = 0;               /* no call user data             */

  return;
}

/*****************************************************************************/
/* Function: make_xvrb_send                                                  */
/* This function sets up the control blocks needed to send data on a         */
/* previously established circuit.                                           */
/*****************************************************************************/
static void make_xvrb_send(
             X25VERB      far *xvrb,       /* Main control block             */
             X25DATA_DATA far *xdata_data, /* Verb specific control block    */
             long              connid,     /* Connection identifier          */
             char             *m1m)
{
  xvrb->verb_code = X25DATASEND;           /* API function to be called      */
  xvrb->version_id = X25_API_VERSION;
  xvrb->queue_number = 0;                  /* Queues not used by this appln  */
  xvrb->connection_id = connid;            /* Which circuit to send data on  */
  xvrb->data_buffer_ptr.data_data = xdata_data; /* Ptr to data block         */
  xvrb->data_buffer_size = sizeof(X25DATA_DATA)*BUFFER_SIZE;
  xvrb->data_length = MESSAGE_SIZE;        /* Length of actual data          */
  xvrb->d_bit = USE_DBIT;                  /* No delivery confirmation       */
  xvrb->q_bit = FALSE;                     /* Set to indicate user data      */
  xvrb->m_bit = FALSE;                     /* No more data in this sequence  */

  (void)strcpy(xdata_data,m1m);

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
  xvrb->data_buffer_size = sizeof(X25DATA_DATA)*RCV_BUFFER_SIZE;

  return;
}

/*****************************************************************************/
/* Function to print an error message to stderr, tidy up and exit            */
/*****************************************************************************/
static void sys_err(
                    X25VERB far *xvrb,     /* Main control block             */
                    char        *errmsg)   /* Error message                  */
{
  /***************************************************************************/
  /* First try to terminate the X.25 API cleanly, but ignore any errors      */
  /* returned.                                                               */
  /***************************************************************************/
  xvrb->verb_code = X25APPTERM;            /* API function called            */
  xvrb->version_id = X25_API_VERSION;
  (void)printf("Terminating the X.25 API with error...\n");


  if (X25(xvrb) == X25_OK)                 /* Call X.25 API                  */
     (void)DOSSEMWAIT( (unsigned long) &xvrb->ram_semaphore, NOTIMEOUT);

  (void)fprintf(stderr,"%s",errmsg);       /* Print the error message        */
  exit(1);                                 /* End the program                */
}
