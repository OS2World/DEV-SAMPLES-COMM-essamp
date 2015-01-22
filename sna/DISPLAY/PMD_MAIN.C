/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : PMD_MAIN.C                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Window procedures for the PMDSPLAY client windows.          */
/*                                                                          */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
/*              (compiles with large memory model)                          */
/*                                                                          */
/*  ASSOCIATED FILES:  See PMDSPLAY.MAK                                     */
/*                                                                          */
/****************************************************************************/

/*---------------------------------------------------------------------*\
 | Defines for the include of OS2.H in PMDSPLAY.H                      |
\*---------------------------------------------------------------------*/
#define  INCL_DOS
#define  INCL_DOSERRORS
#define  INCL_GPICONTROL
#define  INCL_GPILCIDS
#define  INCL_GPIPRIMITIVES
#define  INCL_WININPUT
#define  INCL_WINMENUS
#define  INCL_WINPOINTERS
#define  INCL_WINSCROLLBARS
#define  INCL_WINSYS
#define  INCL_VIO

/*---------------------------------------------------------------------*\
 | Includes                                                            |
\*---------------------------------------------------------------------*/
#include "pmdsplay.h"

#include <malloc.h>
#include <process.h>
#include <stdlib.h>
#include <stdio.h>

#define STACKSIZE 8192
/*---------------------------------------------------------------------*\
 | Function Prototypes - PMD_DLGS.C                                    |
\*---------------------------------------------------------------------*/
MRESULT EXPENTRY FileDlgProc   (HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY TargetDlgProc (HWND, USHORT, MPARAM, MPARAM);
MRESULT EXPENTRY AboutDlgProc  (HWND, USHORT, MPARAM, MPARAM);

/*---------------------------------------------------------------------*\
 | Function Prototypes - PMD_UTIL.C                                    |
\*---------------------------------------------------------------------*/
VOID             InitDisplay   (VOID);
VOID             FreeBuffers   (VOID);
LONG             SetColors     (VOID);
VOID _cdecl      DisplayToVio  (VOID far * unused);
VOID             AdjustVio     (SHORT);
VOID             CreateVio     (VOID);
VOID             FreeLines     (VOID);
VOID         AdjustInfoWindow  (VOID);
VOID         EnableThreadUsers (HWND hwndMenu, BOOL fEnable);
VOID         EnableAPPNOptions (HWND hwndMenu);
VOID _cdecl  AllocConversation (VOID far * unused);
VOID       DeallocConversation (VOID);

/*---------------------------------------------------------------------*\
 | Global Variables - PMDSPLAY.C                                       |
\*---------------------------------------------------------------------*/
extern HAB     hab;                    /* Anchor Block handle          */
extern HWND    hwndMainHelp,           /* Help Instance handle         */
               hwndMainClient;         /* Handle to Main Client window */
extern CHAR    achString[];            /* Load string resources here   */
extern HMODULE RCHandle;               /* Handle to RC DLL             */
extern BOOL    file_ok;                /* APD.MSG operations flag      */

/*---------------------------------------------------------------------*\
 | Global Variables - PMD_UTIL.C                                       |
\*---------------------------------------------------------------------*/
extern BYTE *  bBlankCell;             /* Blank VIO cell               */

/*---------------------------------------------------------------------*\
 | Global Variables - PMD_MAIN.C                                       |
\*---------------------------------------------------------------------*/
WINPARAM wp;                           /* Structure of window parms    */
COLOR clrBackgnd;                      /* Window background color to   */
                                       /* match VIO background         */
BOOL fThreadAvail = TRUE;              /* Is thread available for use? */
BOOL fInfoSizeChanged = FALSE;         /* Did info window size change? */

/*---------------------------------------------------------------------*\
 | Functions                                                           |
\*---------------------------------------------------------------------*/
MRESULT EXPENTRY InfoWndProc (HWND, USHORT, MPARAM, MPARAM);

/*---------------------------------------------------------------------*\
 |                        MainWndProc                                  |
 +---------------------------------------------------------------------+
 | This function handles all the messages received in the queue for    |
 | the main window.                                                    |
\*---------------------------------------------------------------------*/
MRESULT EXPENTRY MainWndProc (HWND   hwnd,  /* Handle of client window */
                              USHORT msg,   /* Message to be handled   */
                              MPARAM mp1,   /* Parameters for message  */
                              MPARAM mp2)
{
   static HWND  hwndInfoFrame,         /* Handle to info frame window  */
                hwndInfoClient,        /* Handle to info client window */
                hwndMenu;              /* Handle to the Action Bar     */

   static CHAR szClientClass [] = "PMDisplay.info";
   static ULONG flFrameFlags =         /* Frame Creation Flags to      */
      FCF_HORZSCROLL    |              /* define the info window's     */
      FCF_VERTSCROLL    ;              /* appearance                   */

   static SHORT cxChar, cyChar;        /* Size of main client chars    */
   static RECTL rclHeader;             /* Rectangular coordinates of   */
                                       /* info header                  */
   static CHAR  szHeader[STRINGSIZE];  /* Header text                  */
   static LONG  cyHeader;              /* Height of header text box    */

   static BOOL  fInit = TRUE;          /* Initialization flag saying   */
                                       /* whether to show the About Box*/
   static TID   tid;                   /* Thread ID                    */
   static VOID  far *pThreadStack;     /* Pointer to stack for thread  */

   FONTMETRICS *pfm;                   /* Pointer to font metrics      */
   HPS          hps;                   /* Handle to presentation space */
   POINTL       ptl;                   /* Point coordinates            */
   RECTL        rcl;                   /* Rectangular coordinates      */
   USHORT       cmd;                   /* WM_COMMAND command code      */
   SHORT        s;
   char        *filename = "APD.MSG";

   switch (msg) {

   /*------------------------------------------------------------------*\
    | The window is being created                                      |
   \*------------------------------------------------------------------*/
   case WM_CREATE:
   /* Show the About Box to be CUA                                     */
      WinDlgBox (HWND_DESKTOP, hwnd, AboutDlgProc, RCHandle,
                 IDD_ABOUT, &fInit);

   /* Set the HourGlass pointer                                        */
      WinSetPointer (HWND_DESKTOP,
                     WinQuerySysPointer (HWND_DESKTOP, SPTR_WAIT, FALSE));

   /* Get the handle to the Action Bar                                 */
      hwndMenu = WinWindowFromID (WinQueryWindow (hwnd, QW_PARENT, FALSE),
                                  FID_MENU);

   /* Set up the DISPLAY verb                                          */
      InitDisplay ();
      EnableAPPNOptions (hwndMenu);

   /* Set up VIO colors, get window background color                   */
      clrBackgnd = SetColors();

   /* Get size of main window characters (query font metrics).         */
      hps = WinGetPS (hwnd);           /* Needed for Gpi calls         */
      pfm = malloc (sizeof (FONTMETRICS));
      GpiQueryFontMetrics (hps, (LONG) sizeof (FONTMETRICS), pfm);
      cxChar = (SHORT) pfm -> lAveCharWidth;
      cyChar = (SHORT) pfm -> lMaxBaselineExt;
      free (pfm);

   /* Get height and text color for info header, and set initial text  */
      WinLoadString (hab, RCHandle, LOCAL_MACHINE, STRINGSIZE, szHeader);

      cyHeader = WinQuerySysValue (HWND_DESKTOP, SV_CYMENU);
                                       /* Set height of header text box*/
                                       /* = height of action-bar menu  */
      WinReleasePS (hps);

   /* Create the info window inside the main client window             */
   /* (note window is invisible until user selects info to display)    */
      WinRegisterClass (hab,
                        szClientClass,
                        InfoWndProc,
                        CS_SIZEREDRAW,
                        0);

      hwndInfoFrame = WinCreateStdWindow (hwnd,
                                          0L,
                                          &flFrameFlags,
                                          szClientClass,
                                          NULL,
                                          0L,
                                          RCHandle,
                                          ID_INFO_RESOURCE,
                                          &hwndInfoClient);

   /* Set the existence and target flags for this window               */
      wp.fWinExists = TRUE;
      wp.fTargetFile = FALSE;
      wp.fRemote = FALSE;
      WinLoadString (hab, RCHandle, LOCAL_MACHINE, 18, wp.PLU_name);

   /* Set the focus to the main client window                          */
      WinSetFocus (HWND_DESKTOP, hwnd);

   /* Allocate stack for thread on far heap                            */
      if (NULL == (pThreadStack = _fmalloc (STACKSIZE))) {
         WinLoadString (hab, RCHandle, THREAD_ALLOC_ERR, STRINGSIZE, achString);
         WinMessageBox (HWND_DESKTOP, hwnd, achString, NULL, 0,
                        MB_OK | MB_ICONEXCLAMATION);
         break;
         } /* endif */

   /* Set the arrow pointer                                            */
      WinSetPointer (HWND_DESKTOP,
                  WinQuerySysPointer (HWND_DESKTOP, SPTR_ARROW, FALSE));

      break;

   /*------------------------------------------------------------------*\
    | The window is being sized                                        |
   \*------------------------------------------------------------------*/
   case WM_SIZE :
   /* Set the coordinates of the info header                           */
      rclHeader.xRight  = SHORT1FROMMP (mp2); /* Main window width     */
      rclHeader.yTop    = SHORT2FROMMP (mp2); /* Main window height    */
      rclHeader.xLeft   = 0;

      if (FALSE == WinIsWindowVisible(hwndInfoFrame)) {
                                       /* Before user selects info     */
         rclHeader.yBottom = 0;        /* Header occupies entire window*/
                                       /* (contains program title)     */
      } else {                         /* After user selects info      */
         rclHeader.yBottom = rclHeader.yTop - cyHeader;
         } /* endif */                 /* Header occupies top of window*/

   /* Set position of info window below the "after user selects info"  */
   /* position of the info header                                      */
      WinSetWindowPos (hwndInfoFrame, NULL,
                       0, 0,
                       (SHORT) rclHeader.xRight,
                       (SHORT) (rclHeader.yTop - cyHeader),
                       SWP_MOVE | SWP_SIZE);
      break;

   /*------------------------------------------------------------------*\
    | A menu item has been selected                                    |
   \*------------------------------------------------------------------*/
   case WM_COMMAND :
      cmd = COMMANDMSG(&msg)->cmd;

      if ((cmd >= IDM_INFO_FIRST) && (cmd <= IDM_INFO_LAST) &&
          (fThreadAvail)) {
      /* Set the HourGlass pointer                                     */
         WinSetPointer (HWND_DESKTOP, WinQuerySysPointer (HWND_DESKTOP,
                                                          SPTR_WAIT, FALSE));

         if (FALSE == WinIsWindowVisible (hwndInfoFrame)) {
            rclHeader.yBottom = rclHeader.yTop - cyHeader;
                                          /* Adjust info header size   */
            WinShowWindow (hwndInfoFrame, TRUE);
            } /* endif */
      /* Get and display newly selected information                    */
         WinLoadString (hab, RCHandle, cmd, STRINGSIZE, szHeader);
         WinInvalidateRect (hwnd, &rclHeader, FALSE); /* Draw header   */
         wp.sCurrentInfo = cmd - IDM_INFO_FIRST;

      /* Begin the thread to show the selected info for the desired    */
      /* target system.  DisplayToVio is the function that will        */
      /* handle the thread's actions                                   */
         if (-1 == (tid = _beginthread (DisplayToVio, pThreadStack,
                                        STACKSIZE, NULL))) {
            WinLoadString (hab, RCHandle, BEGINTHREAD_ERR, STRINGSIZE, achString);
            WinMessageBox (HWND_DESKTOP, hwnd, achString, NULL, 0,
                           MB_OK | MB_ICONEXCLAMATION);
         } else {
            fThreadAvail = FALSE;
            EnableThreadUsers (hwndMenu, fThreadAvail);
            if (!file_ok){
              WinMessageBox (HWND_DESKTOP, hwnd,
                             "Error in reading file: APD.MSG",
                             NULL, 0,
                             MB_OK | MB_ICONEXCLAMATION);
              file_ok = TRUE;
             }
            } /* endif */

      /* Set the arrow pointer                                         */
         WinSetPointer (HWND_DESKTOP, WinQuerySysPointer (HWND_DESKTOP,
                                                          SPTR_ARROW, FALSE));

      } else {
         switch (cmd) {

         /* Refresh the currently selected information                 */
         case IDM_REFRESH :
            if (fThreadAvail) {
               WinLoadString (hab, RCHandle, wp.sCurrentInfo + IDM_INFO_FIRST,
                              STRINGSIZE, szHeader);
               WinInvalidateRect (hwnd, &rclHeader, FALSE); /* Draw header*/
            /* Begin the thread to show the selected info for the desired */
            /* target system.  DisplayToVio is the function that will     */
            /* handle the thread's actions                                */
               if (-1 == (tid = _beginthread (DisplayToVio, pThreadStack,
                                              STACKSIZE, NULL))) {
                  WinLoadString (hab, RCHandle, BEGINTHREAD_ERR,
                                 STRINGSIZE, achString);
                  WinMessageBox (HWND_DESKTOP, hwnd, achString, NULL, 0,
                                 MB_OK | MB_ICONEXCLAMATION);
               } else {
                  fThreadAvail = FALSE;
                  EnableThreadUsers (hwndMenu, fThreadAvail);
                  if (!file_ok){
                   WinMessageBox (HWND_DESKTOP, hwnd,
                                  "Error in reading file: APD.MSG",
                                  NULL, 0,
                                  MB_OK | MB_ICONEXCLAMATION);
                   file_ok = TRUE;
                   }
                  } /* endif */
               } /* endif */

            break;

         /* Screen:  Write further output to screen                    */
         case IDM_SCREEN :
            wp.fTargetFile = FALSE;
            WinSendMsg (hwndMenu, MM_SETITEMATTR,  /* Screen = checked */
                        MPFROM2SHORT (IDM_SCREEN, TRUE),
                        MPFROM2SHORT (MIA_CHECKED, MIA_CHECKED));
            WinSendMsg (hwndMenu, MM_SETITEMATTR,  /* File = unchecked */
                        MPFROM2SHORT (IDM_FILE, TRUE),
                        MPFROM2SHORT (MIA_CHECKED, 0));
            break;

         /* File:  Get file name, and if successful, write further     */
         /*        output to file as well as screen                    */
         case IDM_FILE :
            if (WinDlgBox (HWND_DESKTOP, hwnd, FileDlgProc,
                           RCHandle, IDD_FILE, &hwndMainHelp)) {
               wp.fTargetFile = TRUE;
               WinSendMsg (hwndMenu, MM_SETITEMATTR, /* Screen = unchecked */
                           MPFROM2SHORT (IDM_SCREEN, TRUE),
                           MPFROM2SHORT (MIA_CHECKED, 0));
               WinSendMsg (hwndMenu, MM_SETITEMATTR,     /* File = checked */
                           MPFROM2SHORT (IDM_FILE, TRUE),
                           MPFROM2SHORT (MIA_CHECKED, MIA_CHECKED));
               } /* endif */
            break;

         /* Target:  Get PLU name of target machine                    */
         case IDM_TARGET :
            if (fThreadAvail) {
               if (WinDlgBox (HWND_DESKTOP, hwnd, TargetDlgProc, RCHandle,
                              IDD_TARGET, &hwndMainHelp)) {
               /* Set the HourGlass pointer                               */
                  WinSetPointer (HWND_DESKTOP,
                                 WinQuerySysPointer (HWND_DESKTOP,
                                                     SPTR_WAIT, FALSE));

               /* Start a conversation with the desired partner           */
                  if (-1 == (tid = _beginthread (AllocConversation,
                                                 pThreadStack,
                                                 STACKSIZE, NULL))) {
                     WinLoadString (hab, RCHandle, BEGINTHREAD_ERR,
                                    STRINGSIZE, achString);
                     WinMessageBox (HWND_DESKTOP, hwnd, achString, NULL, 0,
                                    MB_OK | MB_ICONEXCLAMATION);
                  } else {
                     fThreadAvail = FALSE;
                     EnableThreadUsers (hwndMenu, fThreadAvail);
                     } /* endif */

                  WinLoadString (hab, RCHandle, ATTEMPTING_ALLOC,
                                 STRINGSIZE, szHeader);
                  WinInvalidateRect (hwnd, &rclHeader, FALSE);

               /* Set the arrow pointer                                   */
                  WinSetPointer (HWND_DESKTOP,
                                 WinQuerySysPointer (HWND_DESKTOP,
                                                     SPTR_ARROW, FALSE));
                  } /* endif */
               } /* endif */
            break;

         /* About:  Show the About box                                 */
         case IDM_ABOUT :
            WinDlgBox (HWND_DESKTOP, hwnd, AboutDlgProc,
                       RCHandle, IDD_ABOUT, &fInit);
            break;

         /* Exit:  Send a WM_CLOSE message                             */
         case IDM_EXIT :
            WinSendMsg (hwnd, WM_CLOSE, 0L, 0L);
            break;

         /* Help for Help has been Requested.  Tell Help Manager to    */
         /* display the system default panel                           */
         case IDM_HELP_FOR_HELP :
            WinSendMsg (hwndMainHelp, HM_DISPLAY_HELP, 0L, 0L);
            break;

         default: ;
         } /* endswitch */
      } /* endif */

      break;

   /*------------------------------------------------------------------*\
    | Character input received                                         |
   \*------------------------------------------------------------------*/
   case WM_CHAR :
      /* Send characters to the info client window                     */
      return (WinSendMsg (hwndInfoClient, msg, mp1, mp2));
      break;

   /*------------------------------------------------------------------*\
    | Keys Help request received                                       |
   \*------------------------------------------------------------------*/
   case HM_QUERY_KEYS_HELP :
      /* Tell Help Manager which panel contains my Keys Help           */
      return ((MRESULT) IDH_KEYS_HELP);
      break;

   /*------------------------------------------------------------------*\
    | Time to paint the client window                                  |
   \*------------------------------------------------------------------*/
   case WM_PAINT :
      hps = WinBeginPaint (hwnd, NULL, NULL);

   /* Erase the info header                                            */
      WinFillRect (hps, &rclHeader, clrBackgnd);

   /* Draw the info header                                             */
      if (FALSE == WinIsWindowVisible (hwndInfoFrame)) {
                                       /* Before user selects info, or */
                                       /* if DISPLAY to file, then     */
                                       /* Display program title        */
         rcl.xLeft   = rclHeader.xLeft;
         rcl.xRight  = rclHeader.xRight;
         rcl.yBottom = (rclHeader.yTop / 2) +
                          ((PROG_TITLE_LAST - PROG_TITLE_FIRST) / 2 * cyChar);
         rcl.yTop    = rcl.yBottom + cyChar;
         for (s = PROG_TITLE_FIRST; s <= PROG_TITLE_LAST; ++s) {
            WinLoadString (hab, RCHandle, s, STRINGSIZE, achString);
            WinDrawText (hps,
                         -1,
                         achString,
                         &rcl,
                         CLR_NEUTRAL, clrBackgnd,
                         DT_CENTER | DT_VCENTER);
            rcl.yTop    -= cyChar;
            rcl.yBottom -= cyChar;
         } /* endfor */
      } else {                         /* After user selects info      */
                                       /* Display info header text     */
      /* Draw line between info header and info window                 */
         ptl.x = rclHeader.xLeft;
         ptl.y = rclHeader.yBottom;
         GpiMove (hps, &ptl);
         ptl.x = rclHeader.xRight;
         GpiSetColor (hps, CLR_PALEGRAY);
         GpiLine (hps, &ptl);
         GpiSetColor (hps, CLR_DEFAULT);
      /* Write info header text                                        */
         WinDrawText (hps,
                      -1,
                      szHeader,
                      &rclHeader,
                      CLR_NEUTRAL, clrBackgnd,
                      DT_CENTER | DT_VCENTER);
      } /* endif */

      WinEndPaint (hps);
      break;

   /*------------------------------------------------------------------*\
    | Clean up before the window is destroyed                          |
   \*------------------------------------------------------------------*/
   case WM_DESTROY :
   /* Release the Buffers and the presentation spaces                  */
      if (!fThreadAvail) {
         DosSuspendThread (tid);
         } /* endif */
      DeallocConversation ();
      wp.fWinExists = FALSE;
   /* Don't free any memory here, it will cause thread 1 to become     */
   /* blocked, leaving the process in memory (but the window WILL be   */
   /* destroyed!)                                                      */
      break;

   /*------------------------------------------------------------------*\
    | Inform the user of the success of the display to file            |
   \*------------------------------------------------------------------*/
   case WM_FILE_OPEN :
      if (mp1) {
      /* The display to the file was presumably successful             */
      /* The user could be notified at this point, but since we echo   */
      /* the display to the screen, why bother?                        */
      } else {
      /* The file could not be opened                                  */
      /* The user needs to be notified because we echo the information */
      /* to the screen                                                 */
         WinLoadString (hab, RCHandle, FILE_OPEN_FAILURE, STRINGSIZE, achString);
         WinMessageBox (HWND_DESKTOP, hwnd, achString, NULL, 0,
                        MB_OK | MB_ICONEXCLAMATION);
         } /* endif */
      break;

   /*------------------------------------------------------------------*\
    | Inform the user of the success of the allocation attempt         |
    |   - mp1 indicates whether or not a conversation exists with a    |
    |     partner or not                                               |
    |   - mp2 indicates whether no coversation exists because the local|
    |     machine was selected or because the attempt was unsuccessfull|
   \*------------------------------------------------------------------*/
   case WM_ALLOCATE :
      if (mp1) {
         strcpy (szHeader, wp.PLU_name);
         WinLoadString (hab, RCHandle, CONV_ALLOC_SUCC, STRINGSIZE, achString);
         WinMessageBox (HWND_DESKTOP, hwnd, achString, szHeader, 0,
                        MB_OK | MB_INFORMATION);
      } else {
         if (!mp2) {
            WinLoadString (hab, RCHandle, LOCAL_MACHINE, 18, wp.PLU_name);
            if (FALSE == WinIsWindowVisible (hwndInfoFrame)) {
               rclHeader.yBottom = rclHeader.yTop - cyHeader;
                                          /* Adjust info header size   */
               WinShowWindow (hwndInfoFrame, TRUE);
               } /* endif */
            WinLoadString (hab, RCHandle, CONV_ALLOC_UNSUCC, STRINGSIZE, achString);
            WinMessageBox (HWND_DESKTOP, hwnd, achString, NULL, 0,
                           MB_OK | MB_ICONEXCLAMATION);
            } /* endif */
         WinLoadString (hab, RCHandle, LOCAL_MACHINE, STRINGSIZE, szHeader);
         } /* endif */
      WinInvalidateRect (hwnd, &rclHeader, FALSE);
      break;

   /*------------------------------------------------------------------*\
    | Set the flag that says the thread is available again             |
   \*------------------------------------------------------------------*/
   case WM_THREAD_AVAIL :
      while (DosResumeThread(tid) == NO_ERROR) {
         DosSleep(0L);                 /* Wait for thread to exit      */
      } /* endwhile */

      fThreadAvail = TRUE;

      if (fInfoSizeChanged) {
      /* If the size of the info window changed while the thread was   */
      /* busy, no adjustments were made to the info window at that     */
      /* time to avoid conflicts with the thread.  Make the            */
      /* adjustments now.                                              */
         AdjustInfoWindow ();
         fInfoSizeChanged = FALSE;
      } /* endif */

      EnableThreadUsers (hwndMenu, fThreadAvail);
      EnableAPPNOptions (hwndMenu);
      break;

   /*------------------------------------------------------------------*\
    | We never want the InfoWindow to get the focus                    |
   \*------------------------------------------------------------------*/
   case WM_GRAB_FOCUS :
      WinFocusChange (HWND_DESKTOP, hwnd, FC_NOLOSEFOCUS);
      break;

   /*------------------------------------------------------------------*\
    | Let the default window procedure handle all other messages       |
   \*------------------------------------------------------------------*/
   default:
      return WinDefWindowProc (hwnd, msg, mp1, mp2);
   } /* endswitch */
   return 0;
}

/*---------------------------------------------------------------------*\
 |                        InfoWndProc                                  |
 +---------------------------------------------------------------------+
 | This function handles all the messages received in the queue for    |
 | the info window.                                                    |
\*---------------------------------------------------------------------*/
MRESULT EXPENTRY InfoWndProc (HWND   hwnd,  /* Handle of client window */
                              USHORT msg,   /* Message to be handled   */
                              MPARAM mp1,   /* Parameters for message  */
                              MPARAM mp2)
{
   SIZEL         sizl;
   RECTL         rcl;                  /* Rectangular coordinates      */
   VIOCURSORINFO vioci;                /* Needed to hide VIO cursor    */
   SHORT         sScrollInc;           /* Scroll increment             */

   switch (msg) {

   /*------------------------------------------------------------------*\
    | The window is being created                                      |
   \*------------------------------------------------------------------*/
   case WM_CREATE:
   /* Get a device context for the client info window                  */
      wp.hdc = WinOpenWindowDC (hwnd);

   /* Create a cached micro-presentation space for the device context  */
      sizl.cx = sizl.cy = 0;
      wp.hps = GpiCreatePS (hab, wp.hdc, &sizl, PU_PELS |
                            GPIF_DEFAULT | GPIT_MICRO | GPIA_ASSOC);

   /* Get the default VIO device cell size                             */
      VioCreatePS (&(wp.hvps), 1, 1, 0, VIO_NUM_ATTRIB, 0);
      VioAssociate (wp.hdc, wp.hvps);
      VioGetDeviceCellSize (&(wp.cyChar), &(wp.cxChar), wp.hvps);

   /* Initialize window display parameters                             */
      wp.pTopLineQe = wp.pBotLineQe = NULL;
      wp.sNumLines = wp.sNumCols = 0;
      wp.sVscrollPos = wp.sVscrollMax = 0;

   /* Get the handles to the scroll bars                               */
      wp.hwndHscroll = WinWindowFromID (WinQueryWindow (hwnd,
                                                        QW_PARENT,
                                                        FALSE),
                                        FID_HORZSCROLL);
      wp.sHscrollPos = 0;

      wp.hwndVscroll = WinWindowFromID (WinQueryWindow (hwnd,
                                                        QW_PARENT,
                                                        FALSE),
                                        FID_VERTSCROLL);
      wp.sVscrollPos = 0;

      break;

   /*------------------------------------------------------------------*\
    | The window is being sized                                        |
   \*------------------------------------------------------------------*/
   case WM_SIZE :
   /* Save new height and width of info window                         */
      wp.cxWindow = SHORT1FROMMP (mp2);
      wp.cyWindow = SHORT2FROMMP (mp2);

   /* Make adjustments for the new window size                         */
      if (fThreadAvail) {              /* Child thread is not active   */
         AdjustInfoWindow ();          /* Do adjustments now           */
      } else {                         /* Child thread is active       */
         fInfoSizeChanged = TRUE;      /* Put off adjustments until    */
                                       /*  thread is finished to avoid */
                                       /*  conflicts                   */
      } /* endif */

   /* Let the default AVIO window procedure do its magic               */
      WinDefAVioWindowProc (hwnd, msg, mp1, mp2);
      break;

   /*------------------------------------------------------------------*\
    | The window is being scrolled horizontally                        |
   \*------------------------------------------------------------------*/
   case WM_HSCROLL :
      switch (SHORT2FROMMP (mp2)) {
      case SB_LINELEFT :
         sScrollInc = -1;
         break;
      case SB_LINERIGHT :
         sScrollInc = 1;
         break;
      case SB_PAGELEFT :
         sScrollInc = -8;
         break;
      case SB_PAGERIGHT :
         sScrollInc = 8;
         break;
      case SB_SLIDERTRACK :
         sScrollInc = SHORT1FROMMP (mp2) - wp.sHscrollPos;
         break;
      default:
         sScrollInc = 0;
      } /* endswitch */
      sScrollInc = max (-(wp.sHscrollPos),
                         min (sScrollInc, wp.sHscrollMax -
                                          wp.sHscrollPos));
      if (sScrollInc != 0) {
         wp.sHscrollPos += sScrollInc;
         VioSetOrg (0, wp.sHscrollPos, wp.hvps);
         WinSendMsg (wp.hwndHscroll, SBM_SETPOS,
                     MPFROMSHORT (wp.sHscrollPos), NULL);
         WinUpdateWindow (hwnd);
      } /* endif */
      break;

   /*------------------------------------------------------------------*\
    | The window is being scrolled vertically                          |
   \*------------------------------------------------------------------*/
   case WM_VSCROLL :
      switch (SHORT2FROMMP (mp2)) {
      case SB_LINEUP :
         sScrollInc = -1;
         break;
      case SB_LINEDOWN :
         sScrollInc = 1;
         break;
      case SB_PAGEUP :
         sScrollInc = min (-1, -wp.sWinRows);
         break;
      case SB_PAGEDOWN :
         sScrollInc = max (1, wp.sWinRows);
         break;
      case SB_SLIDERTRACK :
         sScrollInc = SHORT1FROMMP (mp2) - wp.sVscrollPos;
         break;
      default:
         sScrollInc = 0;
      } /* endswitch */
      sScrollInc = max (-(wp.sVscrollPos),
                         min (sScrollInc, wp.sVscrollMax -
                                          wp.sVscrollPos));
      if (sScrollInc != 0) {
         wp.sVscrollPos += sScrollInc; /* Set new vertical position     */
         AdjustVio (sScrollInc);       /* Adjust contents of VIO buffer */
         WinSendMsg (wp.hwndVscroll, SBM_SETPOS,
                     MPFROMSHORT (wp.sVscrollPos), NULL);
         WinUpdateWindow (hwnd);
      } /* endif */
      break;

   /*------------------------------------------------------------------*\
    | Character input received                                         |
   \*------------------------------------------------------------------*/
   case WM_CHAR :
      /* All keyboard input is piped to the scroll bars execpt the  */
      /* keys defined in the accelerator table, which generate      */
      /* WM_COMMAND messages                                        */
      switch (CHARMSG (&msg)->vkey) {
      case VK_LEFT :
      case VK_RIGHT :
         return WinSendMsg (wp.hwndHscroll, msg, mp1, mp2);
         break;
      case VK_UP :
      case VK_DOWN :
      case VK_PAGEUP :
      case VK_PAGEDOWN :
         return WinSendMsg (wp.hwndVscroll, msg, mp1, mp2);
         break;
      case VK_HOME:
      case VK_END:
         if (CHARMSG (&msg)->vkey == VK_HOME) {
            sScrollInc = -wp.sVscrollPos; /* Go to first page          */
         } else {
            sScrollInc = wp.sVscrollMax - wp.sVscrollPos;
         } /* endif */                    /* Go to last page           */
         if (sScrollInc != 0) {
            wp.sVscrollPos += sScrollInc; /* Set new vertical position */
            AdjustVio (sScrollInc);       /* Load new page in VIO buf  */
            WinSendMsg (wp.hwndVscroll, SBM_SETPOS,
                        MPFROMSHORT (wp.sVscrollPos), NULL);
            WinUpdateWindow (hwnd);
         } /* endif */
         break;
      default: ;
      } /* endswitch */
      break;

   /*------------------------------------------------------------------*\
    | We never want to get the focus!                                  |
   \*------------------------------------------------------------------*/
   case WM_SETFOCUS:
      if (SHORT1FROMMP (mp2)) {
         WinPostMsg (hwndMainClient, WM_GRAB_FOCUS, NULL, NULL);
         } /* endif */
      break;

   /*------------------------------------------------------------------*\
    | Time to paint the client window                                  |
   \*------------------------------------------------------------------*/
   case WM_PAINT :
      /* Paint the micro-presentation space                            */
      WinBeginPaint (hwnd, wp.hps, NULL);

      /* Paint the window background to match the VIO background       */
      WinQueryWindowRect (hwnd, &rcl);
      WinFillRect (wp.hps, &rcl, clrBackgnd);

      /* Show the contents of the VIO buffer in the window             */
      VioShowBuf (0, VIO_CELL_SIZE * wp.sVioRows * wp.sVioCols, wp.hvps);

      /* Hide the VIO cursor.  AVIO is supposed to display a cursor    */
      /* only when VioWrtTTY is called.  However, the cursor is        */
      /* shown when the window is sized or scrolled, so hide it here.  */
      VioGetCurType (&vioci, wp.hvps);
      vioci.attr = -1;
      VioSetCurType (&vioci, wp.hvps);

      /* Done painting the micro-presentation space                    */
      WinEndPaint (wp.hps);

      break;

   /*------------------------------------------------------------------*\
    | Clean up before the window is destroyed                          |
   \*------------------------------------------------------------------*/
   case WM_DESTROY :
   /* Release the Buffers and the presentation spaces                  */
      FreeBuffers();
      FreeLines();
      VioAssociate (NULL, wp.hvps);
      VioDestroyPS (wp.hvps);
      GpiDestroyPS (wp.hps);
      wp.fWinExists = FALSE;
      break;

   /*------------------------------------------------------------------*\
    | Let the default window procedure handle all other messages       |
   \*------------------------------------------------------------------*/
   default:
      return WinDefWindowProc (hwnd, msg, mp1, mp2);
   } /* endswitch */
   return 0;
}

