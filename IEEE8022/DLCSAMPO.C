/******************************************************************************
*                                                                             *
*   Module name :     DLCSAMPL                                                *
*                                                                             *
*   Function :        Demo program for 802.2 interface of the LAN             *
*                                                                             *
*   Usage :           DLCSAMPL   {SEND       Netaddress=123456789abc          *
*                                            [Adapter={0³1} ]     }           *
*                                                                             *
*                                {RECV       [Adapter={0³1} ]     }           *
*                                                                             *
*                                {DISP       [Adapter={0³1} ]     }           *
*                                                                             *
*                                defaults: Adapter=0                          *
*                                                                             *
*                                note that the remote address MUST            *
*                                be supplied with the SEND option.            *
*                                                                             *
*                     DLCSAMPL   ?                                            *
*                                                                             *
*   Return codes :    0    if successful                                      *
*                     1    if a parameter was bad and/or Help was given       *
*                     x    if an NCB fails the ncb.retcode is returned        *
*                                                                             *
*                                                                             *
*                                                                             *
*                                                                             *
*******************************************************************************/
#define INCL_DOSSEMAPHORES
#define INCL_16

#include <stdio.h>
#ifndef E32TO16
#include <dos.h>
#endif
#include <ctype.h>

#include <OS2.H>

/* Include programming language support declarations    */
#include "lan_7_c.h"   /* general typedefs */
#include "lan_1_c.h"   /* DLC defines      */
#include "lan_2_c.h"   /* misc definitions */
#include "lan_3_c.h"   /* CCB structures   */
#include "lan_6_c.h"   /* ACSLAN external definitions */

/* Include sample program declarations   */
#include "dlcerror.h"
#include "dlcgblv.h"
#include "dlcintf.h"
#include "dirintf.h"

#ifdef E32TO16
dword ReceiveSem, WorkSem;
#else
HSYSSEM ReceiveSem, WorkSem;
#endif

main(argc,argv)
int argc;
char *argv[];
{
        /*------------------------------------------------------------+
        ³  Set the default parameters and initial values              ³
        +------------------------------------------------------------*/
        dword semrc;

        Mode = SEND;
        Lana = 0;
        memset( RmtAddr, 0, 6);

        if ( GrabArgs( argc, argv ) )  {
                Help();
                exit(1);
        }
        else if (Mode == SEND && RmtAddr[0] == 0) {
                puts( ">> Remote network adapter address required for SEND" );
                Terminate( " ",0 );
        }

        printf( "\nDLC Sample Program Started in %s mode.",
                (Mode == SEND ? "SEND" : (Mode == RECV ? "RECEIVE" : "DISP")));

        /*------------------------------------------------------------+
        ³  Create System Semaphores for use by the DLC code           ³
        ³  System Semaphores are required because they are posted at  ³
        ³  the device driver level.                                   ³
        +------------------------------------------------------------*/

#ifdef E32TO16
        DosCreateSem(CSEM_PUBLIC,(void *)&WorkSem,"\\SEM\\DLCSAMPO\\WORKSEM");
        if(Mode==RECV)
          {
          DosCreateSem(CSEM_PUBLIC,(void *)&ReceiveSem,"\\SEM\\DLCSAMPO\\RECEIVE");
          } /* end else */
#else
        DosCreateSem(CSEM_PUBLIC, &WorkSem,"\\SEM\\DLCSAMPO\\WORKSEM");
        if(Mode==RECV)
          {
          DosCreateSem(CSEM_PUBLIC, &ReceiveSem,"\\SEM\\DLCSAMPO\\RECEIVE");
          } /* end else */
#endif

        /*------------------------------------------------------------+
        ³  See that the adapter is initialized and open and make sure ³
        ³  the user isn't trying to get us to talk to ourselves.      ³
        +------------------------------------------------------------*/

        LanaInit();
        if (Mode == DISP)  {

                printf( "\nThe lan adapter %u address is  ", Lana);
                for( i=0; i<6; ++i)
                        printf( "%02X", LclAddr[i] );
                printf( ".\n" );
                exit(0);
        }

        if (memcmp(LclAddr,RmtAddr,6) == 0) {
                printf( "\n>> This program cannot talk to itself" );
                Terminate( " ",0);
        }

        /*------------------------------------------------------------+
        ³  Open our special SAP                                       ³
        +------------------------------------------------------------*/
        printf( "Opening our SAP..." );
        if(dlc_open_sap( &workccb, Lana, Mode==SEND?SendSap:RecvSap,WorkSem,Yes))
           Terminate( "DLC_OPEN_SAP", workccb.ccb.ccb_retcode);
        else {
           Sapid = workccb.pt.open_sap.station_id;
           puts( "ok" );
           }


        if (Mode == SEND) {
                /*----------------------------------------------------+
                ³  If we're the SENDer, we now open a link station    ³
                ³  and do a dlc_connect.                              ³
                +----------------------------------------------------*/

                printf( "Opening the link station ..." );
                if(dlc_open_station( &workccb, Lana, Sapid, RmtAddr ,RecvSap,WorkSem,Yes))
                    Terminate( "DLC_OPEN_STATION", workccb.ccb.ccb_retcode);
                else
                    puts( "ok" );

                Stationid = workccb.pt.open_station.link_station_id;

                printf( "Connecting the link station ..." );

                if(dlc_connect_station( &workccb, Lana, Stationid, RoutingInfo,WorkSem,Yes ))
                   Terminate( "DLC_CONNECT_STATION", workccb.ccb.ccb_retcode);
                else
                   puts( "ok" );

                /*----------------------------------------------------+
                ³  Next, we send our test data to the other station   ³
                +----------------------------------------------------*/

                msglen = strlen(message);

                printf( "\nSending the message: <%s> ...", message );
                if(dlc_transmit_I_frame( &workccb, Lana, message, msglen,
                                      ZEROADDRESS, (word) 0, Stationid,RecvSap,
                                      WorkSem,Yes))
                  Terminate( "DLC_TRANSMIT_I_FRAME", workccb.ccb.ccb_retcode);
                else
                  puts( "ok" );

        }

        else {  /* Mode is RECV */

                /*----------------------------------------------------+
                ³  If we're the RECeiVer, we must wait for a DLC      ³
                ³  status change to tell us a link station has been   ³
                ³  opened.                                            ³
                +----------------------------------------------------*/
                printf( "\nWaiting on DLC status change..." );
                dlc_status_read ( &workccb, Lana, Sapid,WorkSem,Yes);

                dsp = &workccb.pt.read.status_tbl;
                if (dsp->reg_ax == LLC_SABME_RCV_STN_OPENED )
                     {
                     Stationid = dsp->stationid;
                     printf( "\n=> DLC_STAT_CHANGE: 0x%04x", dsp->reg_ax);
                     printf( "\nLink Station Opened.");
                     }
                else
                     {
                     printf ("\nDLC_STAT_CHANGE code not as expected");
                     printf ("\nEvent code = %x expected %x",dsp->reg_ax,
                                                LLC_SABME_RCV_STN_OPENED);
                     exit(0);
                     }

                /*----------------------------------------------------+
                ³  Next we start a receive for messages coming        ³
                ³  to this new link station.
                +----------------------------------------------------*/
                printf( "\nStarting a receive on the new link station ..." );
                receive( &rcvccb, Lana, Stationid,ReceiveSem,No);
                retcode = rcvccb.ccb.ccb_retcode;
                if (retcode != 0xFF)
                        Terminate( "RECEIVE", retcode);
                else
                        puts( "ok" );


                /*----------------------------------------------------+
                ³  Finally We must complete the connection by issuing ³
                ³  a DLC_CONNECT_STATION CCB ourselves.               ³
                +----------------------------------------------------*/
                printf( "Connecting the link station ..." );
                if(dlc_connect_station( &workccb, Lana, Stationid, RoutingInfo,WorkSem,Yes))
                  Terminate( "DLC_CONNECT_STATION", workccb.ccb.ccb_retcode);
                else
                   puts( "ok" );

                /*----------------------------------------------------+
                ³  Now the link has been established, so we wait for  ³
                ³  a received message.
                +----------------------------------------------------*/
#ifdef E32TO16
                semrc = passemwait(ReceiveSem);
#else
                DosSemWait(ReceiveSem,-1L);
#endif
                if ((retcode=rcvccb.ccb.ccb_retcode) != 0x00)
                      Terminate( "RECEIVE", retcode);

                else  {
                        printf( "\n=> Message received: ");
                        rbptr = (struct receive_not_contiguous *) rcvccb.pt.receive.first_buffer;
#ifdef E32TO16
                        data_add.address = (char *)rbptr;
#else
                        data_add.address = (dword) rbptr;
#endif
                        data_add.part.offset = rbptr->user_offset;
#ifdef E32TO16
                        dataptr = data_add.address;
#else
                        dataptr = (char *) data_add.address;
#endif
                        for( i=0; i<rbptr->length_in_buffer; ++i)
                                printf( "%c", *dataptr++ );
                        printf( "\n");
                }
        }

        /*------------------------------------------------------------+
        ³  That's all, so we reset the whole SAP, thereby closing all ³
        ³  its open link stations and freeing its resources.          ³
        +------------------------------------------------------------*/
        printf( "\nDoing DLC_RESET ..." );
        dlc_reset( &workccb, Lana, Sapid ,WorkSem,No);

        if (workccb.ccb.ccb_retcode)
                Terminate( "DLC_RESET", workccb.ccb.ccb_retcode);
        else
                puts( "ok" );

        printf( "DLC Sample Program Completed.%c\n", BELL_CHAR );

        exit(0);
}

/**********************************************************************/
/*
**  LanaInit    Initialize and open the adapter.
**
*/

LanaInit()
{
        byte openrc;
        /*--------------------------------------------------------------------+
        ³   dir_initialize the adapter                                        ³
        +--------------------------------------------------------------------*/
        printf( "\nInitializing the adapter ..." );
        dir_initialize( &workccb, Lana ,WorkSem,No);

        if (workccb.ccb.ccb_retcode == LLC_INVALID_SYS_KEYCODE)
                {
                printf( "\nDIR_INITIALIZE failed because your System Key in the LAN config file");
                printf( "\nwas not 'CFB0' which this sample program uses.  Everything should");
                printf( "\nstill work since the initialize is optional on OS/2.\n");
                }
        else if (workccb.ccb.ccb_retcode)
                Terminate( "DIR_INITIALIZE", workccb.ccb.ccb_retcode);
        else
                puts( "ok" );

        /*--------------------------------------------------------------------+
        ³   dir_open the adapter with mostly default parameters               ³
        ³   If we're the receiver, we ring the bell when this CCB completes   ³
        ³   to signal that the sender program should be started.              ³
        +--------------------------------------------------------------------*/

        printf( "Opening the adapter ..." );
        dir_openadapter(&workccb,Lana,ProductId ,WorkSem,Yes);

        openrc = workccb.ccb.ccb_retcode;
        if (openrc)
                Terminate( "DIR_OPEN", openrc);
        else
                printf( "ok%c\n",(Mode==RECV? BELL_CHAR : ' '));
        ApplId = workccb.ccb.ccb_appl_id;

        /* get our adapter's address */
        dir_status( &workccb, Lana ,WorkSem,Yes);
        if (workccb.ccb.ccb_retcode)
                Terminate( "DIR_STATUS", workccb.ccb.ccb_retcode);

        memcpy (LclAddr,workccb.pt.status.node_address,6);
}


/**********************************************************************/
/*
**  GrabArgs    Parses the command line arguments.  The first letters
**              of each possible parameter keyword are sufficient to
**              identify the parameter.
**
**              Accepts the standard argc,argv parameters
**
**              Modifies the global variables Mode, Lana, and NetAddr
**              if successful, or produces an error message and exits.
*/

GrabArgs( argc, argv )
int argc;
char *argv[];
{
        char *cp, *strchr();
        byte i;


        while( --argc )  {

                cp = *++argv;

                switch( toupper(*cp) )  {

                        case 'S':       Mode = SEND;
                                        break;

                        case 'R':       Mode = RECV;
                                        break;

                        case 'D':       Mode = DISP;
                                        break;

                        case 'A':       cp = strchr( cp, '=' );
                                        if (! *cp)
                                            Terminate("Bad adapter number", 0);
                                        if (*++cp == '0')
                                            Lana = 0;
                                        else if (*cp == '1')
                                            Lana = 1;
                                        else
                                            Terminate("Bad adapter number", 0);
                                        break;

                        case 'N':       cp = strchr( cp, '=' );
                                        if (! *cp)
                                            Terminate("Bad remote network address", 0);

                                        for (++cp,i=0; isxdigit(*cp) && i<12; ++cp, ++i)
                                            RmtAddr[i/2] = (RmtAddr[i/2] << 4) +
                                                           (isdigit(*cp) ? (*cp-'0')
                                                                         : (toupper(*cp)-'A'+10));

                                        if (i<12)
                                            Terminate("Bad remote network address", 0);

                                        break;

                        case '?':       return 1;

                        default:        printf( ">>Extraneous input on command line: %s\n",
                                                 cp );
                                        Terminate( " ", 0);
                                        break;
                }

        }

        return 0;
}

/**********************************************************************/
/*
**  Help        Displays directions for running this demo program
*/

Help()
{
        puts( " " );
        puts( " DLCSAMPL   {SEND       Netaddress=123456789abc  ");
        puts( "                        [Adapter={0³1} ]     }   ");
        puts( "                                                 ");
        puts( "            {RECV       [Adapter={0³1} ]     }   ");
        puts( "                                                 ");
        puts( "            {DISP       [Adapter={0³1} ]     }   ");
        puts( "                                                 ");
        puts( "            defaults: Adapter=0                  ");
        puts( "                                                 ");
        puts( "            Note that the remote address MUST    ");
        puts( "            be supplied with the SEND option.    ");
        puts( "            This value can be obtained by running");
        puts( "                                                 ");
        puts( "                DLCSAMPL DISP [Adapter={0³1}]    ");
        puts( "                                                 ");
        puts( "            on the remote PC.                    ");
        puts( " ");

}

/**********************************************************************/
/*
**  Terminate    Produce a formatted error message and
**              terminate the program
**
**              Accepts a character string holding the failed action
**              and an byte holding the return code.
**
**              DOES NOT RETURN
*/

#define BELL_CHAR       '\007'

Terminate( message, code )
byte *message;
byte code;
{
        char *TableLookUp();

        if (code)
                printf( "\n\n>> Error during %s: \n>>   %s%c\n",
                        message, TableLookUp(DLCErrorMsgs,code), BELL_CHAR );
        else    {
                printf( ">> %s\n", message);
                Help();
                code=1;
        }

        exit(code);
}

/*****************************************************************************/
/*
**  TableLookUp This function scans a code table for a specified value,
**              and returns a text string which can be used as a message.
*/

char *TableLookUp(table, key)
struct CodeTable *table;
unsigned char key;
{
        while (table->label && table->code!=key) table++;
        if (table->label) return(table->label);
        else return(" <unknown> ");
}

/*---------------------------------------------------------------------*
* passemwait
* This routine passes a seg16 parameter to DosSemWait
*---------------------------------------------------------------------*/
#ifdef E32TO16
int passemwait(sem)
HSYSSEM _Seg16 sem;
{
                DosSemWait(sem,-1L);
                return(0);
}
#endif

/*---------------------------------------------------------------------*
* pass_ccb()
* This routine passes a CCB to the Adapter Handler.
*---------------------------------------------------------------------*/

int pass_ccb(ccb)
char * ccb;
{
address Bad_CCB_Ptr;

return (ACSLAN(ccb,(address)&Bad_CCB_Ptr));
}
