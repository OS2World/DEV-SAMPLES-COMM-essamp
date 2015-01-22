/*
*   Module name :     NETSAMPL
*
*   Function :        Demo program for NETBIOS
*
*   Usage :           NETSAMPL   {SEND|RECV} [Adapter={0|1}]
*                                            [Lclname=aaaaaaaaaaaaaaa]
*                                            [Netname=bbbbbbbbbbbbbbb]
*
*                                where the two names can have up to
*                                      fifteen characters.
*
*                                defaults: SEND
*                                          Adapter=0
*                                          Lclname=NETSAMPLSEND
*                                          Netname=NETSAMPLRECV
*                     NETSAMPL   ?
*
*   External modules: none
*
*   Return codes :    0    if successful
*                     1    if a parameter was bad and/or Help was given
*                     x    if an NCB fails the ncb.retcode is returned
*
*
*
*/

#include <stdio.h>
#include <ctype.h>
/* if you are 32 bit applications, then add the following statement
#define  E32TO16                    */


#include "lan_7_c.h"       /* general typedefs */
#include "netb_1_c.h"      /* NCB defines      */
#include "netb_2_c.h"      /* NCB structures   */
#include "netb_4_c.h"      /* NETBIOS external definition */

#include "netgblv.h"
#include "neterror.h"

/**********************************************************************/
/*
**  main        Accepts the arguments and provides help if requested
**              RESETs the specified PC Network Adapter
**              ADDs the localname to the network
**              Initiates a CALL of the remotename over the network
**              When the CALL is answered, a message is SENT to the
**                answering network station over the new session
**              HANGs UP the session with the remote station
**              RESETs the specified adapter
**
**              Error conditions:
**                      First parameter not SEND or RECV
**                      Incorrect Lana number specified
**                      NAME already exists on the network
**                      Fatal NETBIOS error 0x..
**
*/
main ( argc, argv )
int argc;
char *argv[];
{
        /*------------------------------------------------------------+
        |  Set the default parameters and initial values              |
        +------------------------------------------------------------*/

        Mode = SEND;
        Lana = 0;
        memset( LclName, 0, 16);
        memset( NetName, 0, 16);

        if ( GrabArgs( argc, argv ) )  {
                Help();
                exit(1);
        }

        /*------------------------------------------------------------+
        |  If a name wasn't given, use the defaults                   |
        +------------------------------------------------------------*/

        if ( ! LclName[0] )
                strncpy(LclName,(Mode==SEND ?"NETSAMPLSEND":"NETSAMPLRECV"),15);
        if ( ! NetName[0] )
                strncpy(NetName,(Mode==SEND ?"NETSAMPLRECV":"NETSAMPLSEND"),15);

        /*------------------------------------------------------------+
        |  To make sure we start with a clean adapter, we reset it.   |
        +------------------------------------------------------------*/

        retcode = NetReset( Lana, NET_LSN, NET_NUM );
        if (retcode)
                Terminate( "Reset", retcode );

        /*------------------------------------------------------------+
        |  Add the name by which we wish to be known to the network   |
        +------------------------------------------------------------*/

        retcode = NetAddName( Lana, LclName );
        if (retcode)
                Terminate( "AddName", retcode );

        /*------------------------------------------------------------+
        |  If we are acting as the SENDing station, we next do:       |
        |     a CALL to the other station, and if it is answered,     |
        |     a SEND of our message to the other station.             |
        |  Otherwise, we are the RECEIVing station and we next do:    |
        |     a LISTEN for a CALL from the other station, and when    |
        |       it occurs,                                            |
        |     a RECEIVE to accept one message from the other station. |
        +------------------------------------------------------------*/

        if ( Mode == SEND )  {
                retcode = NetCall( Lana, LclName, NetName );
                if (retcode)
                        Terminate( "Call", retcode );

                strcpy( Message, "Hello there!" );

                retcode = NetSend( Lana, Ncb.basic_ncb.ncb_lsn, Message, strlen(Message) );
                if (retcode)
                        Terminate( "Send", retcode );
        }
        else  {
                retcode = NetListen( Lana, LclName, NetName );
                if (retcode)
                        Terminate( "Listen", retcode );

                retcode = NetReceive( Lana, Ncb.basic_ncb.ncb_lsn, Message, sizeof Message );
                if (retcode)
                        Terminate( "Receive", retcode );

                printf( "Received Message:\n" );
                printf( "  \"%s\"\n", Message);
        }

        /*------------------------------------------------------------+
        |  We terminate the session made above with the CALL/LISTEN   |
        |  by doing a HANGUP of the session.                          |
        +------------------------------------------------------------*/

        retcode = NetHangup( Lana, Ncb.basic_ncb.ncb_lsn );
        if (retcode && retcode != NB_SESSION_CLOSED)
                Terminate( "Hangup", retcode );

        /*------------------------------------------------------------+
        |  Finally, we reset the adapter to remove all traces of our  |
        |  ever using the adapter.                                    |
        +------------------------------------------------------------*/

        retcode = NetReset( Lana, NET_LSN, NET_NUM );
        if (retcode)
                Terminate( "Reset", retcode );

        exit(0);
}


/**********************************************************************/
/*
** NetReset     Resets the NETBIOS interface status, clears the name
**              and session tables, and aborts all sessions.
**
**              Accepts the adapter number, max sessions, and max
**              pending NCBs.
**
**              Returns the NCB return code.
*/

byte NetReset( lana, sessions, commands )
byte lana, sessions, commands;
{
        printf( "Resetting lan adapter %u ... ", lana);

        memset( &Ncb, 0, sizeof(union ncb_types) );
        Ncb.reset.ncb_command  = NB_RESET_WAIT;
        Ncb.reset.ncb_lana_num = lana;
        Ncb.reset.req_sessions = sessions;
        Ncb.reset.req_commands = commands;

        TRNcmd( &Ncb.reset );

        if ( ! Ncb.reset.ncb_retcode )
                puts( "Ok");

        return (Ncb.reset.ncb_retcode);
}

/**********************************************************************/
/*
** NetAddName   Adds a 16-character name to the table of names.  The
**              name must be unique across the network.  This is a
**              name that this station will be known by.
**
**              Accepts the adapter number and the char array holding
**              the name.
**
**              Returns the NCB return code.
*/

byte NetAddName( lana, name )
byte lana;
byte *name;
{
        printf( "Adding the local name \"%s\" to the network ... ", name);

        memset( &Ncb, 0, sizeof(union ncb_types) );
        Ncb.basic_ncb.ncb_command  = NB_ADD_NAME_WAIT;
        Ncb.basic_ncb.ncb_lana_num = lana;
        strncpy( Ncb.basic_ncb.ncb_name, name, 16 );

        TRNcmd( &Ncb.basic_ncb );

        if ( ! Ncb.basic_ncb.ncb_retcode )
                puts( "Ok");

        return (Ncb.basic_ncb.ncb_retcode);
}

/**********************************************************************/
/*
** NetCall      Opens a session with another name specified in the
**              NCB.CALLNAME field using the local name specified in
**              the NCB.NAME field.
**
**              Accepts the adapter number and the char arrays holding
**              the local and remote names.
**
**              Returns the NCB return code.  If successful, the session
**              number is returned in the NCB.LSN field.
*/

#define RECV_TIMEOUT    10
#define SEND_TIMEOUT    10

byte NetCall( lana, lclname, rmtname)
byte lana;
byte *lclname, *rmtname;
{
        printf( "Calling the remote station \"%s\" ... ", rmtname);

        memset( &Ncb, 0, sizeof(union ncb_types) );
        Ncb.basic_ncb.ncb_command  = NB_CALL_WAIT;
        Ncb.basic_ncb.ncb_lana_num = lana;
        Ncb.basic_ncb.ncb_rto      = RECV_TIMEOUT<<1;  /* times 2 since in   */
        Ncb.basic_ncb.ncb_sto      = SEND_TIMEOUT<<1;  /* steps of 500 msecs */
        strncpy( Ncb.basic_ncb.ncb_name, lclname, 16 );
        strncpy( Ncb.basic_ncb.ncb_callname, rmtname, 16 );

        TRNcmd( &Ncb.basic_ncb );

        if ( ! Ncb.basic_ncb.ncb_retcode )
                puts( "Ok");

        return (Ncb.basic_ncb.ncb_retcode);
}

/**********************************************************************/
/*
** NetListen    Enables a session to be opened with the name specified
**              in the NCB.CALLNAME field, using the name specified in
**              the NCB.NAME field.
**
**              Accepts the adapter number and the char arrays holding
**              the local and remote names.
**
**              Returns the NCB return code.  If successful, the session
**              number is returned in the NCB.LSN field.
**
**              Note: A Listen command will NOT timeout, but it occupies
**              a session entry and is considered a pending session in the
**              information returned by a NCB.STATUS command.
*/

byte NetListen( lana, lclname, rmtname)
byte lana;
byte *lclname, *rmtname;
{
        printf( "Listening for a call from the remote station \"%s\" ... ", rmtname);

        memset( &Ncb, 0, sizeof(union ncb_types) );
        Ncb.basic_ncb.ncb_command  = NB_LISTEN_WAIT;
        Ncb.basic_ncb.ncb_lana_num = lana;
        Ncb.basic_ncb.ncb_rto      = RECV_TIMEOUT<<1;   /* times 2 since in   */
        Ncb.basic_ncb.ncb_sto      = SEND_TIMEOUT<<1;   /* steps of 500 msecs */
        strncpy( Ncb.basic_ncb.ncb_name, lclname, 16 );
        strncpy( Ncb.basic_ncb.ncb_callname, rmtname, 16 );

        TRNcmd( &Ncb.basic_ncb );

        if ( ! Ncb.basic_ncb.ncb_retcode )
                puts( "Ok");

        return (Ncb.basic_ncb.ncb_retcode);
}

/**********************************************************************/
/*
** NetSend      Sends data to the session partner as defined by the
**              session number in the NCB.LSN field.  The data to send
**              is in the buffer pointed to by the NCB.BUFFER field.
**
**              Accepts the adapter number, the session number,
**              the char array holding the message to be sent, and
**              the length of the message in that array.
**
**              Returns the NCB return code.
*/

byte NetSend( lana, lsn, message, length )
byte lana, lsn;
byte *message;
word length;
{
        printf( "Sending the following message to the answering station:\n" );
        printf( "  \"%s\" ... ", message);

        memset( &Ncb, 0, sizeof(union ncb_types) );
        Ncb.basic_ncb.ncb_command  = NB_SEND_WAIT;
        Ncb.basic_ncb.ncb_lana_num = lana;
        Ncb.basic_ncb.ncb_lsn      = lsn;
        Ncb.basic_ncb.ncb_buffer_address = message;
        Ncb.basic_ncb.ncb_length   = length;

        TRNcmd( &Ncb.basic_ncb );

        if ( ! Ncb.basic_ncb.ncb_retcode )
                puts( "Ok");

        return (Ncb.basic_ncb.ncb_retcode);
}

/**********************************************************************/
/*
** NetReceive   Receives data from the session partner that sends data
**              to this station.
**
**              Accepts the adapter number, the session number,
**              the char array to hold the message received, and
**              the maximum length the message may occupy in that
**              array.
**
**              Returns the NCB return code and, if successful,
**              the received data in the buffer.
*/

byte NetReceive( lana, lsn, buffer, length )
byte lana, lsn;
byte *buffer;
word length;
{
        printf( "Receiving a message ..." );

        memset( &Ncb, 0, sizeof(union ncb_types) );
        Ncb.basic_ncb.ncb_command  = NB_RECEIVE_WAIT;
        Ncb.basic_ncb.ncb_lana_num = lana;
        Ncb.basic_ncb.ncb_lsn      = lsn;
        Ncb.basic_ncb.ncb_buffer_address = buffer;
        Ncb.basic_ncb.ncb_length   = length-1;

        TRNcmd( &Ncb.basic_ncb );

        if ( ! Ncb.basic_ncb.ncb_retcode )
                puts( "Ok");

        buffer[length-1] = '\0';

        return (Ncb.basic_ncb.ncb_retcode);
}

/**********************************************************************/
/*
** NetHangup    Closes the session with another name on the network
**              specified by the session number.
**
**              Accepts the adapter number and session number.
**
**              Returns the NCB return code.
*/

byte NetHangup( lana, lsn )
byte lana, lsn;
{
        printf( "Hanging up the session ... " );

        memset( &Ncb, 0, sizeof(union ncb_types) );
        Ncb.basic_ncb.ncb_command  = NB_HANG_UP_WAIT;
        Ncb.basic_ncb.ncb_lana_num = lana;
        Ncb.basic_ncb.ncb_lsn      = lsn;

        TRNcmd( &Ncb.basic_ncb );

        if ( ! Ncb.basic_ncb.ncb_retcode || Ncb.basic_ncb.ncb_retcode == 0x0A)
                puts( "Ok");

        return (Ncb.basic_ncb.ncb_retcode);
}


/**********************************************************************/
/*
**  GrabArgs    Parses the command line arguments.  The first letters
**              of each possible parameter keyword are sufficient to
**              identify the parameter.
**
**              Accepts the standard argc,argv parameters
**
**              Modifies the global variables Mode, Lana, LclName, and
**              NetName if successful, or produces an error message and exits.
*/

GrabArgs( argc, argv )
int argc;
char *argv[];
{
        char *cp, *strchr();


        while( --argc )  {

                cp = *++argv;           /* ptr to next argument string */

                switch( toupper(*cp) )  {

                        case 'S':       /* Send option */
                                        Mode = SEND;
                                        break;

                        case 'R':       /* Receive option */
                                        Mode = RECV;
                                        break;

                        case 'A':       /* Adapter number - must be followed
                                           by an '=' and either 0 or 1  */

                                        cp = strchr( cp, '=' );
                                        if (! *cp)
                                                Terminate("Bad adapter number", 0);
                                        if (*++cp == '0')
                                                Lana = 0;
                                        else if (*cp == '1')
                                                Lana = 1;
                                        else
                                                Terminate("Bad adapter number", 0);
                                        break;

                        case 'L':       /* Local network name - must be
                                           followed by an '=' and up to
                                           15 characters for the name  */

                                        cp = strchr( cp, '=' );
                                        if (! cp)
                                                Terminate("Bad local name", 0);
                                        strncpy( LclName, ++cp, 15 );
                                        break;

                        case 'N':       /* Remote network name - must be
                                           followed by an '=' and up to
                                           15 characters for the name   */

                                        cp = strchr( cp, '=' );
                                        if (! cp)
                                                Terminate("Bad remote name", 0);
                                        strncpy( NetName, ++cp, 15 );
                                        break;

                        case '?':       /* Request for the help display */
                                        return 1;

                        default:        printf( ">>Unrecognized argument: %s\n",
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
        puts( " ");
        puts( " NETSAMPL   {Send|Recv} [Adapter={0|1}]          ");
        puts( "                        [Lclname=aaaaaaaaaaaaaaa]");
        puts( "                        [Netname=bbbbbbbbbbbbbbb]");
        puts( " ");
        puts( "            where the two names can have up to   ");
        puts( "                  fifteen characters.            ");
        puts( " ");
        puts( "            defaults: Adapter=0                  ");
        puts( "                      Lclname=NETSAMPL{SEND|RECV}");
        puts( "                      Netname=NETSAMPL{RECV|SEND}");
        puts( " ");

        return;
}

/**********************************************************************/
/*
**  Terminate    Produce a formatted error message and
**              terminate the program
**
**              Accepts a character string holding the failed action
**              and an BYTE holding the return code.
**
**              DOES NOT RETURN
*/

#define BELL_CHAR       '\007'

Terminate( message, code )
byte *message;
byte code;
{
        extern char *TableLookUp();

        if (code)
                printf( "\n\n>> Error during %s: \n>>   %s%c\n",
                        message, TableLookUp(NetErrorMsgs,code), BELL_CHAR );
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

/**********************************************************************/
/*
**  TRNcmd      Execute an NCB
**
**              Accepts a pointer to an NCB and passes it to
**              NETBIOS.
**
**              Returns the value in the AX register which is the NCB
**              return code.
*/

unsigned TRNcmd(cmdptr)
char *cmdptr;
{
        return( NETBIOS(cmdptr));
}
