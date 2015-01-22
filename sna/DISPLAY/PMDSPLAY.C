/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : PMDSPLAY.C                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1989, 1990, 1991             */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Executes the DISPLAY and DISPLAY_APPN verbs requesting      */
/*              the information requested, and displays the returned        */
/*              information in an OS/2 Presentation Manager window in       */
/*              human-readable form.                                        */
/*                                                                          */
/*              Uses the following APPC verbs:                              */
/*                                                                          */
/*                 DISPLAY                                                  */
/*                 DISPLAY_APPN                                             */
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
/*      PMDSPLAY.MAK - MAKE file                                            */
/*      PMDSPLAY.DEF - Module definition file (for LINK)                    */
/*      PMDSPLAY.RC  - Resource definitions                                 */
/*      PMDSPLAY.IPF - Help source                                          */
/*      PMDSPLAY.H   - Global typedefs, prototypes, and #includes           */
/*      PMDSPLAY.C   - Main function                                        */
/*      PMD_MAIN.C   - Client window procedure                              */
/*      PMD_UTIL.C   - Utilities                                            */
/*      PMD_DLGS.C   - Dialog functions                                     */
/*      DISPLAY.H    - Global typedefs, prototypes, and #includes           */
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

/*---------------------------------------------------------------------*\
 | Includes                                                            |
\*---------------------------------------------------------------------*/
#include "pmdsplay.h"
#include "msgid.h"

/*---------------------------------------------------------------------*\
 | Function Prototypes - PMD_MAIN.C                                    |
\*---------------------------------------------------------------------*/
MRESULT EXPENTRY MainWndProc (HWND, USHORT, MPARAM, MPARAM);

/*---------------------------------------------------------------------*\
 | Function Prototypes - PMD_UTIL.C                                    |
\*---------------------------------------------------------------------*/
HWND             CreateHelp  (VOID);
VOID             SetTitle    (VOID);

/*---------------------------------------------------------------------*\
 | Global Variables - PMDSPLAY.C                                       |
\*---------------------------------------------------------------------*/
HAB         hab;                      /* Handle to the Anchor Block    */
HWND        hwndMainHelp;             /* Handle to Help window         */
HWND        hwndMainFrame,            /* Handle to main Frame window   */
            hwndMainClient;           /* Handle to main Client window  */
CHAR        achString[STRINGSIZE];    /* Load String Resources here    */
HMODULE     RCHandle;                 /* Handle to RC DLL              */
int         rc;                       /* Return code                   */
/*---------------------------------------------------------------------*\
 | Functions                                                           |
\*---------------------------------------------------------------------*/

/*---------------------------------------------------------------------*\
 |                             Main                                    |
 +---------------------------------------------------------------------+
 | This function creates the main window, passes messages on to the    |
 | appropriate control window, and destroys itself when a WM_QUIT      |
 | message is received                                                 |
\*---------------------------------------------------------------------*/
int cdecl main (argc, argv)
int  argc;                         /* Number of command line arguments */
char *argv[];                      /* List of command line arguments   */
{
   static CHAR  szClientClass [] = "PMDisplay.main";
   static ULONG flFrameFlags =         /* Frame Creation Flags to      */
      FCF_TITLEBAR      |              /* define the main window's     */
      FCF_SYSMENU       |              /* appearance                   */
      FCF_MENU          |
      FCF_SIZEBORDER    |
      FCF_MINMAX        |
      FCF_SHELLPOSITION |
      FCF_TASKLIST      |
      FCF_ICON          |
      FCF_ACCELTABLE    ;

   HMQ   hmq;                          /* Handle to message queue      */
   QMSG  qmsg;                         /* Message queue element        */

   USHORT rc = 0;

   ULONG WindowStyle;                 /* Style to display window       */
   BOOL  CalledFromCM = FALSE;        /* Is PMDSPLAY called from       */
                                      /* Communications Manager?       */

extern  BOOL   file_ok;               /* Init flag for APD.MSG testing */
        file_ok = TRUE;



/* Initialize the text tables from APD.MSG                             */
   InitializeText();

/* Initialize the anchor block and message queue handles               */

   hab = WinInitialize (0);
   hmq = WinCreateMsgQueue (hab, 0);

/* Load the .RC DLL                                                    */
  rc = DosLoadModule(NULL, 0 , "PMDSPLAY", &RCHandle);
  if (rc){
              WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
                             "Error loading PMDSPLAY.DLL",
                             NULL, 0,
                             MB_OK | MB_ICONEXCLAMATION);
              WinDestroyMsgQueue (hmq);           /* Message queue      */
              WinTerminate (hab);                 /* Anchor block       */
    return(rc);
    }
/* Register the client window's class                                  */

   WinRegisterClass (hab,
                     szClientClass,
                     MainWndProc,
                     CS_SIZEREDRAW,
                     0);

/* Create the help instance                                            */

   hwndMainHelp = CreateHelp ();

/* Determine if PMDSPLAY is called from Communications Manager or      */
/* from a command line.  When PMDSPLAY is called from Communications   */
/* Manager, the command line argument /M will be used and the PMDSPLAY */
/* window will appear maximized.  When PMDSPLAY is called from a       */
/* command line, the PMDSPLAY window will appear normally.             */

   if ( (argc > 1) && (argv[1][0] == '/') &&
        ( (argv[1][1] == 'M') || (argv[1][1] == 'm') ) ) {
      CalledFromCM = TRUE ;
      WindowStyle = WS_MAXIMIZED | WS_VISIBLE ;
   }
   else WindowStyle = WS_VISIBLE;

/* Create the main window                                              */

   hwndMainFrame = WinCreateStdWindow (HWND_DESKTOP,
                                       WindowStyle,
                                       &flFrameFlags,
                                       szClientClass,
                                       NULL,
                                       0L,
                                       RCHandle,
                                       ID_MAIN_RESOURCE,
                                       &hwndMainClient);

/* If PMDSPLAY was called from Communications Manager, maximize the    */
/* window.                                                             */

   if (CalledFromCM)
      WinSetWindowPos(hwndMainFrame, HWND_TOP, 0, 0, 0, 0, SWP_MAXIMIZE);

/* Associate the main frame with the help instance                     */

   if (hwndMainHelp) {
      WinAssociateHelpInstance (hwndMainHelp, hwndMainFrame);
   } /* endif */

/* Set the title-bar text and the task-list switch entry title         */

   SetTitle ();

/* Get and dispatch messages until WM_QUIT is received                 */

   while (WinGetMsg (hab, &qmsg, NULL, 0, 0)) {
      WinDispatchMsg (hab, &qmsg);
   } /* endwhile */

/* Clean up                                                            */

   if (hwndMainHelp) {                 /* Help instance                */
      WinDestroyHelpInstance (hwndMainHelp);
   } /* endif */
   WinDestroyWindow (hwndMainFrame);   /* Main window frame            */
   WinDestroyMsgQueue (hmq);           /* Message queue                */
   WinTerminate (hab);                 /* Anchor block                 */
   return 0;
}

