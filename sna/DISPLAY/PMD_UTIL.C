/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : PMD_UTIL.C                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Miscellaneous utilities                                     */
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
#define  INCL_GPIBITMAPS
#define  INCL_WINMENUS
#define  INCL_WINSCROLLBARS
#define  INCL_WINPOINTERS
#define  INCL_WINSWITCHLIST
#define  INCL_WINSYS
#define  INCL_VIO
 
/*---------------------------------------------------------------------*\
 | Includes                                                            |
\*---------------------------------------------------------------------*/
#include "pmdsplay.h"
#include "display.h"
 
/* MSC 6.0 handles multi-thread includes differently than IBM C/2      */
#if _MSC_VER >= 600
   #include <process.h>
   #include <stdarg.h>
   #include <stdlib.h>
#else
   #include <mt\process.h>
   #include <mt\stdarg.h>
   #include <mt\stdlib.h>
   #endif
 
/*---------------------------------------------------------------------*\
 | Defines for Remote DISPLAY                                          |
\*---------------------------------------------------------------------*/
#define  MODE_NAME "#INTER"
#define  REQUESTOR "RDISPLAY"
#define  SERVER    "RDSPSRVR"
 
/*---------------------------------------------------------------------*\
 | External Variables - PMDSPLAY.C                                     |
\*---------------------------------------------------------------------*/
extern HAB      hab;
extern HWND     hwndMainFrame,
                hwndMainClient;
extern CHAR     achString[];
extern HMODULE  RCHandle;              /* Handle to RC DLL             */
extern BOOL     file_ok;               /* APD.MSG file flag            */
 
/*---------------------------------------------------------------------*\
 | External Variables - PMD_DLGS.C                                     |
\*---------------------------------------------------------------------*/
extern CHAR     szFilename[80];
extern BOOL     fAppend;
 
/*---------------------------------------------------------------------*\
 | External Variables - DISPUTIL.C                                     |
\*---------------------------------------------------------------------*/
extern BOOL appn;       /* Indicates whether or not I'm running on the */
                        /* latest version of APPC.                     */
 
/*---------------------------------------------------------------------*\
 | Global Variables - PMD_MAIN.C                                       |
\*---------------------------------------------------------------------*/
extern WINPARAM wp;
 
/*---------------------------------------------------------------------*\
 | Global Variables - PMD_UTIL.C                                       |
\*---------------------------------------------------------------------*/
VOID far *data_buffer_ptr;             /* Pointer to seg DISPLAY uses  */
                                       /* for returned information.    */
INT  work_buf_index;       /* Index into buffer for myprintf           */
FILE * stream = NULL;      /* File stream for writing info             */
BOOL fLocalAPPN;           /* Indicates if local machine is latest APPC*/
BYTE bBlankCell [4];                   /* Blank character cell - allows*/
                                       /* for extended attributes      */
static LONG VioToClr[] = {
   CLR_BLACK,
   CLR_DARKBLUE,
   CLR_DARKGREEN,
   CLR_DARKCYAN,
   CLR_DARKRED,
   CLR_DARKPINK,
   CLR_BROWN,
   CLR_PALEGRAY,
   CLR_DARKGRAY,
   CLR_BLUE,
   CLR_GREEN,
   CLR_CYAN,
   CLR_RED,
   CLR_PINK,
   CLR_YELLOW,
   CLR_WHITE
};
 
/*---------------------------------------------------------------------*\
 | Functions                                                           |
\*---------------------------------------------------------------------*/
 
/*---------------------------------------------------------------------*\
 |                            DoErrorExit                              |
 +---------------------------------------------------------------------+
 | This function does an error exit.                                   |
\*---------------------------------------------------------------------*/
VOID DoErrorExit (VOID)
{
   WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
                  "Error Exit", NULL, 0,
                  MB_OK | MB_ICONEXCLAMATION);
   exit(-1);
}
 
/*---------------------------------------------------------------------*\
 |                             CreateHelp                              |
 +---------------------------------------------------------------------+
 | This function creates the help instance and returns the handle to   |
 | the help window.                                                    |
\*---------------------------------------------------------------------*/
HWND CreateHelp (VOID) {
   HELPINIT hiMainData;               /* Help initialization structure */
   HWND     hwnd;                     /* Handle for help instance      */
 
   hiMainData.cb                        = sizeof (HELPINIT);
   hiMainData.ulReturnCode              = 0L;
   hiMainData.pszTutorialName           = NULL;
   hiMainData.phtHelpTable              = (PVOID) (0xffff0000 |
                                                   ID_MAIN_RESOURCE);
   hiMainData.hmodAccelActionBarModule  = 0;
   hiMainData.idAccelTable              = 0;
   hiMainData.idActionBar               = 0;
   WinLoadString (hab, RCHandle, HELP_WIN_TITLE, STRINGSIZE, achString);
   hiMainData.hmodHelpTableModule      = RCHandle;
   hiMainData.pszHelpWindowTitle        = achString;
   hiMainData.usShowPanelId             = 0;
   hiMainData.pszHelpLibraryName        = "PMDSPLAY.HLP";
 
   hwnd = WinCreateHelpInstance (hab, &hiMainData);
   if (!hwnd) {
      WinLoadString (hab, RCHandle, NO_HELP_AVAILABLE, STRINGSIZE, achString);
      WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
                     achString, NULL, 1,
                     MB_OK | MB_APPLMODAL | MB_MOVEABLE);
   } else {
      if (hiMainData.ulReturnCode) {
         WinLoadString (hab, RCHandle, HELP_ERROR, STRINGSIZE, achString);
         WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
                        achString, NULL, 1,
                        MB_OK | MB_APPLMODAL | MB_MOVEABLE);
         WinDestroyHelpInstance (hwnd);
         hwnd = 0;
      } /* endif */
   } /* endif */
 
   return (hwnd);
}
 
/*---------------------------------------------------------------------*\
 |                             SetTitle                                |
 +---------------------------------------------------------------------+
 | This function sets the title-bar text and the task-list switch      |
 | entry title.                                                        |
\*---------------------------------------------------------------------*/
VOID SetTitle (VOID) {
   HSWITCH hSwitch;               /* Handle for task list switch entry */
   SWCNTRL SwitchData;            /* Structure for switch entry data   */
 
/* Get the title text */
   WinLoadString (hab, RCHandle, TITLEBAR_TEXT, STRINGSIZE, achString);
 
/* Set the title-bar text */
   WinSetWindowText (hwndMainFrame, achString);
 
/* Get the current task-list switch entry and change the title */
   hSwitch = WinQuerySwitchHandle (hwndMainFrame, 0);
   WinQuerySwitchEntry (hSwitch, (PSWCNTRL)& SwitchData);
   strcpy (SwitchData.szSwtitle, achString);
   WinChangeSwitchEntry (hSwitch, (PSWCNTRL)& SwitchData);
}
 
/*---------------------------------------------------------------------*\
 |                            DisplayToFile                            |
 +---------------------------------------------------------------------+
 | This function executes the DISPLAY verb and puts the formatted      |
 | output in a file.                                                   |
\*---------------------------------------------------------------------*/
VOID DisplayToFile (VOID) {
   CHAR *AccessType;                   /* Access Mode of file          */
   LINE_QE * pLineQe;                  /* Pointer to line queue element*/
 
   if (fAppend) {
      AccessType = "a";
   } else {
      AccessType = "w";
      } /* endif */
 
   if (NULL != (stream = fopen (szFilename, AccessType))) {
                                       /* If file opens ok             */
      print_info_header (wp.sCurrentInfo);
                                       /* Write info header to file    */
      pLineQe = wp.pTopLineQe;         /* Get top-of-screen line in list */
      if (pLineQe) {                   /* If any lines in list         */
         while (pLineQe -> pPrev) pLineQe = pLineQe -> pPrev;
                                       /* Find first line in list      */
         while (NULL != (pLineQe)) {   /* Write lines to file          */
            fprintf (stream, "%s\n", pLineQe -> pLine);
            pLineQe = pLineQe -> pNext;
            } /* endwhile */
         } /* endif */
      fclose (stream);
      stream = NULL;                   /* Set stream to NULL for myprintf */
      WinPostMsg (hwndMainClient, WM_FILE_OPEN, (MPARAM) TRUE, NULL);
   } else {
   /* Perform error processing, if desired.  Here I just post a message   */
   /* I POST the message because this procedure is called from within a   */
   /* thread that has no message queue                                    */
      WinPostMsg (hwndMainClient, WM_FILE_OPEN, (MPARAM) FALSE, NULL);
      } /* endif */
   return;
}
 
/*---------------------------------------------------------------------*\
 |                              AddLine                                |
 +---------------------------------------------------------------------+
 | Allocates queue element and buffer for a new line, copies line to   |
 | buffer, attaches buffer to queue element, and appends queue element |
 | to list.  NOTE:  This function skips the first lines containing     |
 | the info header (which is displayed in a static text window in      |
 | PMDSPLAY), so the number of lines in the info header                |
 | (NUM_HEADER_LINES) must be subtracted from wp.sNumLines when done.  |
\*---------------------------------------------------------------------*/
#define NUM_HEADER_LINES 4
void AddLine (char * pLine, SHORT sLen)
{
   LINE_QE * pNewLineQe;
 
   if (wp.sNumLines >= NUM_HEADER_LINES) {
      pNewLineQe = malloc(sizeof(LINE_QE));
      if (pNewLineQe == NULL) {
         DoErrorExit();
      } /* endif */
      pNewLineQe -> pLine = strdup(pLine);
      if (pNewLineQe -> pLine == NULL) {
         DoErrorExit();
      } /* endif */
      pNewLineQe -> sLen = sLen;
      pNewLineQe -> pNext = NULL;
      pNewLineQe -> pPrev = wp.pBotLineQe;
      if (!wp.pTopLineQe) wp.pTopLineQe = pNewLineQe;
      if (wp.pBotLineQe) wp.pBotLineQe -> pNext = pNewLineQe;
      wp.pBotLineQe = pNewLineQe;
      if (wp.sNumCols < sLen) wp.sNumCols = sLen;
                                     /* Remember longest line length   */
   } /* endif */
   ++wp.sNumLines;                   /* Increment line count           */
}
 
/*---------------------------------------------------------------------*\
 |                             FreeLines                               |
 +---------------------------------------------------------------------+
 | Frees line queue elements and line buffers allocated in AddLine     |
\*---------------------------------------------------------------------*/
void FreeLines (void)
{
   LINE_QE * pLineQe;
 
   if (wp.pTopLineQe) {             /* If any lines in list            */
      while (wp.pTopLineQe -> pPrev) wp.pTopLineQe = wp.pTopLineQe -> pPrev;
                                    /* Find first line in list         */
      while (NULL != (pLineQe = wp.pTopLineQe)) {
         wp.pTopLineQe = wp.pTopLineQe -> pNext;
         free(pLineQe -> pLine);    /* Free the line                   */
         free(pLineQe);             /* Free the queue element          */
      } /* endwhile */
   } /* endif */
   wp.pBotLineQe = NULL;
}
 
/*---------------------------------------------------------------------*\
 |                           InitDisplay                               |
 +---------------------------------------------------------------------+
 | This function allocates the buffer for the DISPLAY verb returned    |
 | information and sets the DISPLAY verb parameters for the version of |
 | APPC/APPN on which I'm running.                                     |
\*---------------------------------------------------------------------*/
VOID InitDisplay (VOID)
{
/* Allocate buffer for Display verb calls                              */
   data_buffer_ptr = alloc_shared_buffer(INFO_BUFFER_SIZE);
   if (!data_buffer_ptr) {
      WinLoadString (hab, RCHandle, BUF_ALLOC_ERROR, STRINGSIZE, achString);
      WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
                     achString, NULL, 0,
                     MB_OK | MB_ICONEXCLAMATION);
      DoErrorExit();
   } /* endif */
 
/* Determine version of APPC/APPN & set DISPLAY parameters accordingly */
   set_version (data_buffer_ptr, INFO_BUFFER_SIZE, NULL, 0L, FALSE);
 
/* Save local APPC/APPN version for remote purposes                    */
   fLocalAPPN = appn;
}
 
/*---------------------------------------------------------------------*\
 |                           FreeBuffers                               |
 +---------------------------------------------------------------------+
 | This function frees the buffers allocated in InitDisplay and        |
 | AddLine.                                                            |
\*---------------------------------------------------------------------*/
VOID FreeBuffers (VOID)
{
   free_shared_buffer (data_buffer_ptr);
   FreeLines();
   return;
}
 
/*---------------------------------------------------------------------*\
 |                          RgbToVioColor                              |
 +---------------------------------------------------------------------+
 | This function returns the approximate VIO representation of the     |
 | supplied RGB color.                                                 |
\*---------------------------------------------------------------------*/
BYTE RgbToVioColor (COLOR clrRgb) {
   BYTE bIrgb = 0;                     /* Assume Black to start        */
   RGB  rgb;
 
   rgb = MAKETYPE (clrRgb, RGB);
 
   /* Low intensity colors except Black                                */
   if (rgb.bBlue  >= 0x80) bIrgb |= '\x01';
   if (rgb.bGreen >= 0x80) bIrgb |= '\x02';
   if (rgb.bRed   >= 0x80) bIrgb |= '\x04';
 
   /* High intensity colors except Dark Gray                           */
   if ((rgb.bBlue >= 0xC0) || (rgb.bGreen >= 0xC0) || (rgb.bRed >= 0xC0))
      bIrgb |= 8;
 
   /* Dark Gray                                                        */
   if ((bIrgb == 0) && (rgb.bBlue >= 0x40) &&
       (rgb.bGreen >= 0x40) && (rgb.bRed >= 0x40)) bIrgb = 8;
 
   return bIrgb;
   }
 
/*---------------------------------------------------------------------*\
 |                            SetColors                                |
 +---------------------------------------------------------------------+
 | Sets the VIO foreground and background colors to match the system   |
 | defaults for the window foreground and background colors as closely |
 | as possible, and returns a window background color to match the VIO |
 | background color.                                                   |
\*---------------------------------------------------------------------*/
LONG SetColors (VOID)
{
/* Set attributes of blank VIO cell for info window                    */
   bBlankCell[0] = ' ';
   bBlankCell[1] =
      (BYTE)(RgbToVioColor (WinQuerySysColor (HWND_DESKTOP,
                                              SYSCLR_WINDOW, 0L)) << 4) |
      RgbToVioColor (WinQuerySysColor (HWND_DESKTOP,
                                       SYSCLR_WINDOWTEXT, 0L));
   bBlankCell[2] = VIO_EXTENDED_ATTRIB;/* Extended attribute byte      */
   bBlankCell[3] = 0x00;               /* Reserved for DBCS            */
 
/* Return window background color to match VIO background color        */
   return (VioToClr[bBlankCell[1] >> 4]);
}
 
/*---------------------------------------------------------------------*\
 |                        EnableThreadUsers                            |
 +---------------------------------------------------------------------+
 | This function enables or disables the menuitems that need to use    |
 | thread, or the shared segment that DISPLAY returns info in          |
\*---------------------------------------------------------------------*/
VOID EnableThreadUsers (HWND hwndMenu, BOOL fEnable)
{
   WinSendMsg (hwndMenu, MM_SETITEMATTR,
               MPFROM2SHORT (IDM_APPC, TRUE),
               MPFROM2SHORT (MIA_DISABLED, fEnable ? 0 : MIA_DISABLED));
   WinSendMsg (hwndMenu, MM_SETITEMATTR,
               MPFROM2SHORT (IDM_APPN, TRUE),
               MPFROM2SHORT (MIA_DISABLED, fEnable ? 0 : MIA_DISABLED));
   WinSendMsg (hwndMenu, MM_SETITEMATTR,
               MPFROM2SHORT (IDM_TARGET, TRUE),
               MPFROM2SHORT (MIA_DISABLED, fEnable ? 0 : MIA_DISABLED));
   WinSendMsg (hwndMenu, MM_SETITEMATTR,
               MPFROM2SHORT (IDM_REFRESH, TRUE),
               MPFROM2SHORT (MIA_DISABLED, fEnable ? 0 : MIA_DISABLED));
   return;
}
 
/*---------------------------------------------------------------------*\
 |                        EnableAPPNOptions                            |
 +---------------------------------------------------------------------+
 | This function enables or disables the APPN menuitem options based   |
 | on the value of the variable appn                                   |
\*---------------------------------------------------------------------*/
VOID EnableAPPNOptions (HWND hwndMenu)
{
   INT  i;
 
   for (i = IDM_INFO_SYSDEF; i <= IDM_INFO_MS; i++) {
      WinSendMsg (hwndMenu, MM_SETITEMATTR, MPFROM2SHORT (i, TRUE),
                  MPFROM2SHORT (MIA_DISABLED, appn ? 0 : MIA_DISABLED));
      } /* endfor */
 
   WinSendMsg (hwndMenu, MM_SETITEMATTR, MPFROM2SHORT (IDM_APPN, TRUE),
               MPFROM2SHORT (MIA_DISABLED, appn ? 0 : MIA_DISABLED));
   return;
}
 
/*---------------------------------------------------------------------*\
 |                             LoadVio                                 |
 +---------------------------------------------------------------------+
 | Loads formatted text into the VIO presentation space starting with  |
 | the current top line (wp.pTopLineQe).  Note that no text is written |
 | into the last row of the VIO presentation space; the last row keeps |
 | trash from collecting at the bottom of the window during sizing and |
 | is left blank.                                                      |
\*---------------------------------------------------------------------*/
void LoadVio (void)
{
   SHORT sRow;                         /* Row counter                  */
   LINE_QE * pLineQe;                  /* Pointer to line queue element*/
 
   VioScrollDn (0, 0, -1, -1, -1, bBlankCell, wp.hvps);
                                       /* Erase the VIO buffer         */
   for (sRow = 0, pLineQe = wp.pTopLineQe;
        sRow < wp.sVioRows - 1;
        ++sRow, pLineQe = pLineQe -> pNext) {
      VioWrtCharStr(pLineQe -> pLine,
                    pLineQe -> sLen,
                    sRow, 1, wp.hvps);
      wp.pBotLineQe = pLineQe;
      } /* endfor */
   return;
}
 
/*---------------------------------------------------------------------*\
 |                            CreateVio                                |
 +---------------------------------------------------------------------+
 | If there is data to display, this function destroys the current VIO |
 | presentation space (if one exists) and creates a new one to fit     |
 | the amount of formatted text to be displayed and the current        |
 | dimensions of the window.                                           |
\*---------------------------------------------------------------------*/
void CreateVio (void)
{
/* If there is data to display, create a new VIO presentation space    */
   if (wp.sNumCols) {
 
   /* Destroy current VIO presentation space if one exists             */
      if (wp.hvps) {
         VioAssociate (NULL, wp.hvps);
         VioDestroyPS (wp.hvps);
      } /* endif */
 
   /* Calculate the number of columns and rows for the VIO buffer.     */
   /* Considerations:                                                  */
   /* - The VIO buffer must be wide enough to hold the longest         */
   /*   line of text.                                                  */
   /* - Make the VIO buffer one column wider than the longest line of  */
   /*   text and (if possible) one row longer than the window to keep  */
   /*   trash from collecting at the bottom and right side of the      */
   /*   window during sizing.                                          */
   /* - Make the VIO buffer no more than one row longer than the       */
   /*   number of lines of text (the last row is not used).            */
   /* - The max number of bytes that can be in a VIO buffer (64kB)     */
   /* - The max number of rows that can be in a VIO buffer (255)       */
      wp.sVioCols = wp.sNumCols + 1;
      wp.sVioRows = wp.sWinRows + 1;
      if (wp.sVioRows > wp.sNumLines + 1) wp.sVioRows = wp.sNumLines + 1;
      if (wp.sVioRows > (SHORT)(MAX_VIO_CHARS / wp.sVioCols))
         wp.sVioRows = (SHORT)(MAX_VIO_CHARS / wp.sVioCols);
      if (wp.sVioRows > MAX_VIO_HEIGHT) wp.sVioRows = MAX_VIO_HEIGHT;
 
   /* Create a VIO presentation space                                  */
      VioCreatePS (&(wp.hvps), wp.sVioRows, wp.sVioCols,
                   0, VIO_NUM_ATTRIB, 0);
      VioAssociate (wp.hdc, wp.hvps);
 
   /* Set the VIO presentation space's position in the window          */
      VioSetOrg (0, wp.sHscrollPos, wp.hvps);
 
   /* Load formatted text into the VIO presentation space              */
      LoadVio();
 
   } /* endif */
}
 
/*---------------------------------------------------------------------*\
 |                           DisplayToVio                              |
 +---------------------------------------------------------------------+
 | This function executes the DISPLAY verb and displays the formatted  |
 | output in a VIO presentation space.                                 |
\*---------------------------------------------------------------------*/
void _cdecl DisplayToVio (void far * unused)
{
/* Clear current display and disable the scroll bars                   */
 
   WinPostMsg (wp.hwndHscroll, SBM_SETTHUMBSIZE, MPFROM2SHORT (0, 0), NULL);
   WinPostMsg (wp.hwndVscroll, SBM_SETTHUMBSIZE, MPFROM2SHORT (0, 0), NULL);
   if (wp.hvps) {
      VioScrollDn (0, 0, -1, -1, -1, bBlankCell, wp.hvps);
   } /* endif */
 
/* Initialize counters and free earlier formatted lines                */
   wp.sNumLines = wp.sNumCols = work_buf_index = 0;
   FreeLines();
 
/* Get the info.  The boolean return value from this function is only  */
/* FALSE if the REMOTE display failed (perhaps because of a deallocate */
/* abend in the server).                                               */
   if (!(get_and_format_info (wp.sCurrentInfo, data_buffer_ptr,
                              wp.tp_id, wp.conv_id, wp.fRemote))) {
      tp_ended (wp.tp_id);
      wp.fRemote = FALSE;
      } /* endif */
 
/* Adjust wp.sNumLines for the discarded information header            */
   if (wp.sNumLines >= NUM_HEADER_LINES) {
      wp.sNumLines -= NUM_HEADER_LINES;
   } else {
      wp.sNumLines = 0;
   } /* endif */
 
/* Create and load the VIO buffer (presentation space)                 */
   wp.sHscrollPos = 0;        /* Needs to be set before CreateVio(),   */
                              /* so VioSetOrg will be right in the case*/
                              /* where the user has scrolled right     */
   ++wp.sNumCols;             /* Allow for starting text in 2nd column */
   CreateVio();
 
/* If display to file is selected, write info to file                  */
   if (wp.fTargetFile) {
      DisplayToFile ();
   } /* endif */
 
/* Set Scroll Bar Thumbs according to current size of window and       */
/* amount of information in window                                     */
   wp.sHscrollMax = max (0, wp.sNumCols - wp.sWinCols);
   WinPostMsg (wp.hwndHscroll,
               SBM_SETTHUMBSIZE,
               MPFROM2SHORT (wp.sWinCols, wp.sNumCols),
               NULL);
   WinPostMsg (wp.hwndHscroll,
               SBM_SETSCROLLBAR,
               MPFROMSHORT (wp.sHscrollPos),
               MPFROM2SHORT (0, wp.sHscrollMax));
 
   wp.sVscrollMax = max (0, wp.sNumLines - wp.sWinRows);
   wp.sVscrollPos = 0;
   WinPostMsg (wp.hwndVscroll,
               SBM_SETTHUMBSIZE,
               MPFROM2SHORT (wp.sWinRows, wp.sNumLines),
               NULL);
   WinPostMsg (wp.hwndVscroll,
               SBM_SETSCROLLBAR,
               MPFROMSHORT (wp.sVscrollPos),
               MPFROM2SHORT (0, wp.sVscrollMax));
 
   WinPostMsg (hwndMainClient, WM_THREAD_AVAIL, NULL, NULL);
   _endthread ();
}
 
/*---------------------------------------------------------------------*\
 |                             AdjustVio                               |
 +---------------------------------------------------------------------+
 | Adjust the contents of the VIO buffer to satisfy a vertical scroll  |
 | command.  Note that, for line down, the new line of text is written |
 | to the next-to-last row of the VIO buffer rather than the last row; |
 | the last row keeps trash from collecting at the bottom of the window|
 | during sizing and is left blank.                                    |
\*---------------------------------------------------------------------*/
void AdjustVio (SHORT sVscrollInc)
{
CHAR *TempLine;
 
   if (sVscrollInc == 1) {             /* Line down                    */
   /* Scroll the data in the VIO buffer up one line, and load a new    */
   /* line at the bottom.                                              */
      wp.pTopLineQe = wp.pTopLineQe -> pNext;
      wp.pBotLineQe = wp.pBotLineQe -> pNext;
      TempLine = malloc(wp.sVioCols);
      if (TempLine == NULL) DoErrorExit();
      VioScrollUp (0, 0, 0xFFFF, wp.sVioCols, 1, bBlankCell, wp.hvps);
      memset(TempLine, ' ', wp.sVioCols);
      VioWrtCharStr(TempLine,
                    wp.sVioCols,
                    wp.sVioRows - 2, 1, wp.hvps);
      VioWrtCharStr(wp.pBotLineQe -> pLine,
                    wp.pBotLineQe -> sLen,
                    wp.sVioRows - 2, 1, wp.hvps);
      free(TempLine);
   } else
   if (sVscrollInc == -1) {            /* Line up                      */
   /* Scroll the data in the VIO buffer down one line, and load a new  */
   /* line at the top.                                                 */
      wp.pTopLineQe = wp.pTopLineQe -> pPrev;
      wp.pBotLineQe = wp.pBotLineQe -> pPrev;
      TempLine = malloc(wp.sVioCols);
      if (TempLine == NULL) DoErrorExit();
      VioScrollDn (0, 0, 0xFFFF, wp.sVioCols, 1, bBlankCell, wp.hvps);
      memset(TempLine, ' ', wp.sVioCols);
      VioWrtCharStr(TempLine,
                    wp.sVioCols,
                    0, 1, wp.hvps);
      VioWrtCharStr(wp.pTopLineQe -> pLine,
                    wp.pTopLineQe -> sLen,
                    0, 1, wp.hvps);
      free(TempLine);
   } else {
      if (sVscrollInc > 0) {           /* Page down                    */
         for ( ;sVscrollInc > 0; --sVscrollInc) {
            wp.pTopLineQe = wp.pTopLineQe -> pNext;
         } /* endfor */
      } else {                         /* Page up                      */
         for ( ;sVscrollInc < 0; ++sVscrollInc) {
            wp.pTopLineQe = wp.pTopLineQe -> pPrev;
         } /* endfor */
      } /* endif */
      LoadVio();                       /* Load new data in VIO buffer  */
   } /* endif */
}
 
/*---------------------------------------------------------------------*\
 |                         AdjustInfoWindow                            |
 +---------------------------------------------------------------------+
 | Makes adjustments for a change in the size of the info window.      |
\*---------------------------------------------------------------------*/
void AdjustInfoWindow (void)
{
/* Set the new number of rows and columns in the info window           */
   wp.sWinCols = wp.cxWindow / wp.cxChar;
   wp.sWinRows = wp.cyWindow / wp.cyChar;
 
/* Adjust the scroll bar thumbs to suit the new window size and        */
/* the amount of information being displayed                           */
   wp.sHscrollMax = max (0, wp.sNumCols - wp.sWinCols);
   wp.sHscrollPos = min (wp.sHscrollPos, wp.sHscrollMax);
   WinSendMsg (wp.hwndHscroll,
               SBM_SETSCROLLBAR,
               MPFROMSHORT (wp.sHscrollPos),
               MPFROM2SHORT (0, wp.sHscrollMax));
   WinSendMsg (wp.hwndHscroll,
               SBM_SETTHUMBSIZE,
               MPFROM2SHORT (wp.sWinCols, wp.sNumCols),
               NULL);
 
   wp.sVscrollMax = max (0, wp.sNumLines - wp.sWinRows);
   while (wp.sVscrollPos > wp.sVscrollMax) {
   /* If necessary, adjust the vertical position (ie, top line) to     */
   /* suit the new maximum value.                                      */
      wp.pTopLineQe = wp.pTopLineQe -> pPrev;
      --wp.sVscrollPos;
   } /* endwhile */
   WinSendMsg (wp.hwndVscroll,
               SBM_SETSCROLLBAR,
               MPFROMSHORT (wp.sVscrollPos),
               MPFROM2SHORT (0, wp.sVscrollMax));
   WinSendMsg (wp.hwndVscroll,
               SBM_SETTHUMBSIZE,
               MPFROM2SHORT (wp.sWinRows, wp.sNumLines),
               NULL);
 
/* Create a new VIO presentation space to suit the new window size.    */
   CreateVio();
}
 
/*---------------------------------------------------------------------*\
 |                      DeallocateConversation                         |
 +---------------------------------------------------------------------+
 | This function attempts to deallocate a conversation with a partner. |
\*---------------------------------------------------------------------*/
VOID DeallocConversation (VOID)
{
   if (wp.fRemote) {
      mc_deallocate (wp.tp_id, wp.conv_id, AP_SYNC_LEVEL);
      tp_ended (wp.tp_id);
      } /* endif */
   return;
}
 
/*---------------------------------------------------------------------*\
 |                        AllocateConversation                         |
 +---------------------------------------------------------------------+
 | This function attempts to allocate a conversation with a partner.   |
\*---------------------------------------------------------------------*/
VOID _cdecl AllocConversation (VOID far * unused)
{
   BOOL fDispLocal;
 
/* Clear current display and disable the scroll bars                   */
   WinPostMsg (wp.hwndHscroll, SBM_SETTHUMBSIZE, MPFROM2SHORT (0, 0), NULL);
   WinPostMsg (wp.hwndVscroll, SBM_SETTHUMBSIZE, MPFROM2SHORT (0, 0), NULL);
   if (wp.hvps) {
      VioScrollDn (0, 0, -1, -1, -1, bBlankCell, wp.hvps);
   } /* endif */
 
/* Deallocate the current conversation, if it exists                   */
   if (wp.fRemote) {
      DeallocConversation ();
      } /* endif */
 
/* Local machine selected?  If so, leave                               */
   WinLoadString (hab, RCHandle, LOCAL_MACHINE, STRINGSIZE, achString);
   if (!strcmp (achString, wp.PLU_name)) {
      wp.fRemote = FALSE;
      appn = fLocalAPPN;
      fDispLocal = TRUE;
   } else {
      wp.fRemote = TRUE;
      fDispLocal = FALSE;
      } /* endif */
 
/* Initialize counters and free earlier formatted lines                */
   wp.sNumLines = wp.sNumCols = work_buf_index = 0;
   FreeLines();
 
/* Print 3 blank lines to simulate a header                            */
   myprintf ("\n\n\n");
 
   if (wp.fRemote) {
      if (tp_started (REQUESTOR, wp.tp_id)) {
         if (mc_allocate (wp.PLU_name, SERVER, wp.tp_id, MODE_NAME,
                            AP_CONFIRM_SYNC_LEVEL, &wp.conv_id)) {
/* Determine version of APPC/APPN & set DISPLAY parameters accordingly */
            if (set_version (data_buffer_ptr, INFO_BUFFER_SIZE,
                             wp.tp_id, wp.conv_id, wp.fRemote)) {
               mc_deallocate (wp.tp_id, wp.conv_id, AP_SYNC_LEVEL);
               tp_ended (wp.tp_id);
               wp.fRemote = FALSE;
               } /* endif */
         } else {
            tp_ended (wp.tp_id);
            wp.fRemote = FALSE;
            } /* endif */
      } else {
         wp.fRemote = FALSE;
         } /* endif */
      } /* endif */
 
/* Adjust wp.sNumLines for the discarded information header            */
   if (wp.sNumLines >= NUM_HEADER_LINES) {
      wp.sNumLines -= NUM_HEADER_LINES;
   } else {
      wp.sNumLines = 0;
   } /* endif */
 
/* Create and load the VIO buffer (presentation space)                 */
   wp.sHscrollPos = 0;        /* Needs to be set before CreateVio(),   */
                              /* so VioSetOrg will be right in the case*/
                              /* where the user has scrolled right     */
   ++wp.sNumCols;             /* Allow for starting text in 2nd column */
   CreateVio();
 
/* Set Scroll Bar Thumbs according to current size of window and       */
/* amount of information in window                                     */
   wp.sHscrollMax = max (0, wp.sNumCols - wp.sWinCols);
   WinPostMsg (wp.hwndHscroll, SBM_SETSCROLLBAR,
               MPFROMSHORT (wp.sHscrollPos),
               MPFROM2SHORT (0, wp.sHscrollMax));
   WinPostMsg (wp.hwndHscroll, SBM_SETTHUMBSIZE,
               MPFROM2SHORT (wp.sWinCols, wp.sNumCols),
               NULL);
 
   wp.sVscrollMax = max (0, wp.sNumLines - wp.sWinRows);
   wp.sVscrollPos = 0;
   WinPostMsg (wp.hwndVscroll, SBM_SETSCROLLBAR,
               MPFROMSHORT (wp.sVscrollPos),
               MPFROM2SHORT (0, wp.sVscrollMax));
   WinPostMsg (wp.hwndVscroll, SBM_SETTHUMBSIZE,
               MPFROM2SHORT (wp.sWinRows, wp.sNumLines),
               NULL);
 
/* mp1 = wp.fRemote will be equal to the success of this function      */
/* mp2 = local because of allocation failure or by choice              */
   WinPostMsg (hwndMainClient, WM_ALLOCATE,
               (MPARAM) wp.fRemote, (MPARAM) fDispLocal);
   WinPostMsg (hwndMainClient, WM_THREAD_AVAIL, NULL, NULL);
   _endthread ();
}
 
/*---------------------------------------------------------------------*\
 |                             myprintf                                |
 +---------------------------------------------------------------------+
 | Formatting function called by the DISPLAY information formatting    |
 | functions.  Directs output to stream or buffer.  When directed to   |
 | buffer, separates output into lines (separated by carriage-return/  |
 | line-feed) and calls AddLine to put lines in a linked list.         |
\*---------------------------------------------------------------------*/
int cdecl myprintf(char * string, ...)
{
 if (file_ok){
   va_list arg_ptr;               /* Pointer to variable argument list */
   int length;                    /* Length of formatted string        */
   static char work_buf[256];     /* Working buffer for formatting     */
   char * end_line_ptr;           /* Pointer to end of line            */
   SHORT line_len;                /* Length of line                    */
 
   va_start(arg_ptr, string);     /* Set pointer to argument list      */
                                  /* following "string"                */
   if (stream) {                  /* Format string to file             */
      length = vfprintf(stream, string, arg_ptr);
   } else {                       /* Format string to buffer           */
      length = vsprintf(work_buf + work_buf_index, string, arg_ptr);
      work_buf_index += length;   /* Increment buffer index            */
      while (NULL != (end_line_ptr = strchr(work_buf,'\n'))) {
         *end_line_ptr = '\0';    /* Make line into a string           */
         line_len = (SHORT)(end_line_ptr - work_buf);  /* Get line len */
         AddLine(work_buf, line_len);        /* Add line to list       */
         strcpy(work_buf, end_line_ptr + 1); /* Shift buffer left      */
         work_buf_index -= (line_len + 1);   /* Decrement buffer index */
         } /* endwhile */
      } /* endif */
   return(length);                /* Return length to caller           */
 }
 else
   return(-1);                    /* Rtn w/o processing if APD problem */
}
