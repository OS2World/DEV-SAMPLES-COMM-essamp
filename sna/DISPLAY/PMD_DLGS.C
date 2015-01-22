/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : PMD_DLGS.C                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991,                        */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   File dialog procedure and subroutines for PMDSPLAY          */
/*              program.                                                    */
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
#define  INCL_WINBUTTONS
#define  INCL_WINENTRYFIELDS
#define  INCL_WININPUT
#define  INCL_WINLISTBOXES
#define  INCL_WINSHELLDATA
#define  INCL_WINSYS
#define  INCL_WINTIMER
 
/*---------------------------------------------------------------------*\
 | Includes                                                            |
\*---------------------------------------------------------------------*/
#include "pmdsplay.h"
#include "display.h"
 
/*---------------------------------------------------------------------*\
 | External Variables - PMDSPLAY.C                                     |
\*---------------------------------------------------------------------*/
extern HAB       hab;
extern HWND      hwndClient;
extern CHAR      achString[];
extern HMODULE   RCHandle;              /* Handle to RC DLL            */
 
/*---------------------------------------------------------------------*\
 | Global Variables - PMD_MAIN.C                                       |
\*---------------------------------------------------------------------*/
extern WINPARAM wp;
 
/*---------------------------------------------------------------------*\
 | Global Variables - PMD_UTIL.C                                       |
\*---------------------------------------------------------------------*/
extern VOID far *data_buffer_ptr;      /* Pointer to seg DISPLAY uses  */
                                       /* for returned information.    */
extern BOOL fLocalAPPN;    /* Indicates if local machine is latest APPC*/
 
/*---------------------------------------------------------------------*\
 | External Variables - DISPUTIL.C                                     |
\*---------------------------------------------------------------------*/
extern BOOL appn;       /* Indicates whether or not I'm running on the */
                        /* latest version of APPC.                     */
 
/*---------------------------------------------------------------------*\
 | Global Variables                                                    |
\*---------------------------------------------------------------------*/
CHAR szFilename[80];
CHAR * szListFont = "10.System Monospaced";
BOOL fAppend = FALSE;
 
/*---------------------------------------------------------------------*\
 |                          AboutDlgProc                               |
 +---------------------------------------------------------------------+
 | This function displays the About Box.                               |
\*---------------------------------------------------------------------*/
MRESULT EXPENTRY AboutDlgProc (HWND   hwnd, /* Handle of Dialog Window */
                               USHORT msg,  /* Message to be handled   */
                               MPARAM mp1,  /* Parameters for message  */
                               MPARAM mp2) {
   switch (msg) {
 
      /* The about window is being called.  Do what the user told us   */
      /* to do (the Control Panel stores this information in OS2.INI). */
      case WM_INITDLG : {
         SHORT LogoTime;
         PBOOL pfInit;
 
         pfInit = MPFROMP (mp2);
         if (TRUE == (*pfInit)) {
      /* If you want the About box to appear at load time according to */
      /* the user's wishes as set in the Control Panel, then uncomment */
      /* the following statement.                                      */
/*
            LogoTime = PrfQueryProfileInt (HINI_USERPROFILE,
                                           "PM_ControlPanel",
                                           "LogoDisplayTime", -1);
*/
            LogoTime = 0;
            *pfInit = FALSE;
         } else {
            LogoTime = -1;
            } /* endif */
 
         switch (LogoTime) {
            case -1:
               break;
            case 0 :
               return ((MRESULT) (WinDismissDlg (hwnd, TRUE)));
               break;
            default:
               WinStartTimer (hab, hwnd, TID_USERMAX - 1, LogoTime);
            } /* endswitch */
 
         } break;
 
      /* User pressed the space bar, escape key, enter key, or clicked */
      /* the default "OK" push button                                  */
      case WM_COMMAND :
         switch (COMMANDMSG(&msg)->cmd) {
            case DID_OK :
            case DID_CANCEL :
               return ((MRESULT) (WinDismissDlg (hwnd, TRUE)));
               break;
            default: ;
            } /* endswitch */
            break;
 
      /* User specified a time out period for the Logo panel           */
      case WM_TIMER :
         WinStopTimer (hab, hwnd, TID_USERMAX - 1);
         return ((MRESULT) (WinDismissDlg (hwnd, TRUE)));
         break;
 
      /* Default: do nothing, pass it on to the Default Dialog Proc    */
      default: ;
      } /* endswitch */
 
   /* Let the default Dialog Procedure do what it's gotta do           */
   return WinDefDlgProc (hwnd, msg, mp1, mp2);
   }
 
/*---------------------------------------------------------------------*\
 |                           FillDriveListBox                          |
 +---------------------------------------------------------------------+
 | This function fills the Drive List Box                              |
\*---------------------------------------------------------------------*/
VOID FillDriveListBox (HWND hwnd, CHAR *pcCurrentPath) {
   static CHAR szDrive [] = " :";
   SHORT  sDrive;
   USHORT usDriveNum;
   ULONG  ulDriveMap;
 
   DosQCurDisk (&usDriveNum, &ulDriveMap);
   pcCurrentPath [0] = (CHAR)(usDriveNum + '@');
   pcCurrentPath [1] = ':';
   pcCurrentPath [2] = '\\';
   WinSetDlgItemText (hwnd, IDD_PATH, pcCurrentPath);
   WinSendDlgItemMsg (hwnd, IDD_DRIVE_LIST, LM_DELETEALL, NULL, NULL);
   for (sDrive = 0; sDrive < 26; sDrive++) {
      if (ulDriveMap & 1L << sDrive) {
         szDrive [0] = (CHAR)(sDrive + 'A');
         WinSendDlgItemMsg (hwnd, IDD_DRIVE_LIST, LM_INSERTITEM,
                            MPFROM2SHORT (LIT_END, 0), MPFROMP (szDrive));
         } /* endif */
      } /* endfor */
   return;
   }
 
/*---------------------------------------------------------------------*\
 |                           FillDirListBox                            |
 +---------------------------------------------------------------------+
 | This function fills the Directory List Box                          |
\*---------------------------------------------------------------------*/
VOID FillDirListBox (HWND hwnd, CHAR *pcCurrentPath) {
   FILEFINDBUF findbuf;
   HDIR        hDir = 1;
   USHORT      usCurPathLen = 64, usSearchCount = 1;
 
   DosQCurDir (0, pcCurrentPath + 3, &usCurPathLen);
   WinSetDlgItemText (hwnd, IDD_PATH, pcCurrentPath);
   WinSendDlgItemMsg (hwnd, IDD_DIR_LIST, LM_DELETEALL, NULL, NULL);
   DosFindFirst ("*.*", &hDir, 0x0017, &findbuf, sizeof (findbuf),
                 &usSearchCount, 0L);
   while (usSearchCount) {
      if (findbuf.attrFile & 0x0010 &&
          (findbuf.achName [0] != '.' || findbuf.achName [1])) {
         WinSendDlgItemMsg (hwnd, IDD_DIR_LIST, LM_INSERTITEM,
                            MPFROM2SHORT (LIT_SORTASCENDING, 0),
                            MPFROMP (findbuf.achName));
         } /* endif */
      DosFindNext (hDir, &findbuf, sizeof (findbuf), &usSearchCount);
      } /* endwhile */
   return;
   }
 
/*---------------------------------------------------------------------*\
 |                           FillFileListBox                           |
 +---------------------------------------------------------------------+
 | This function fills the File List Box                               |
\*---------------------------------------------------------------------*/
VOID FillFileListBox (HWND hwnd) {
   FILEFINDBUF findbuf;
   HDIR        hDir = 1;
   USHORT      usSearchCount = 1, us, usIndex;
 
   WinSendDlgItemMsg (hwnd, IDD_FILE_LIST, LM_DELETEALL, NULL, NULL);
   DosFindFirst ("*.*", &hDir, 0x0007, &findbuf, sizeof (findbuf),
                 &usSearchCount, 0L);
   while (usSearchCount) {
      WinSendDlgItemMsg (hwnd, IDD_FILE_LIST, LM_INSERTITEM,
                         MPFROM2SHORT (LIT_SORTASCENDING, 0),
                         MPFROMP (findbuf.achName));
      DosFindNext (hDir, &findbuf, sizeof (findbuf), &usSearchCount);
      } /* endwhile */
 
   us = SHORT1FROMMR (WinSendDlgItemMsg (hwnd, IDD_FILE_LIST,
                                         LM_QUERYITEMCOUNT, NULL, NULL));
   for (usIndex = 0; usIndex < us; usIndex++) {
      WinSendDlgItemMsg (hwnd, IDD_FILE_LIST, LM_QUERYITEMTEXT,
                         MPFROM2SHORT (usIndex, STRINGSIZE),
                         MPFROMP (achString));
      if (strstr (szFilename, achString)) {
         WinSendDlgItemMsg (hwnd, IDD_FILE_LIST, LM_SELECTITEM,
                            MPFROMSHORT (usIndex), (MPARAM) TRUE);
         } /* endif */
      } /* endfor */
   return;
   }
 
/*---------------------------------------------------------------------*\
 |                           ParseFilename                             |
 +---------------------------------------------------------------------+
 | This function parses the file specification                         |
 | Returns: 0 -- pcOut points to drive , full dir and filename         |
 |          1 -- pcIn had invalid drive                                |
 |          2 -- pcIn had invalid directory                            |
 |          3 -- pcIn had invalid filename                             |
\*---------------------------------------------------------------------*/
SHORT ParseFilename (CHAR *pcOut, CHAR *pcIn) {
   CHAR     *pcLastSlash, *pcFileOnly;
   ULONG    ulDriveMap;
   USHORT   usDriveNum, usDirLen = 64;
 
   strupr(pcIn);
 
   if (pcIn[0] == '\0')
      return 3;
 
   if (pcIn[1] == ':')  {
      if (DosSelectDisk(pcIn[0]  - '@'))
         return 1;
 
   if (((pcIn[0] == 'P') && (pcIn[1] == 'R') &&
        (pcIn[2] == 'N') && (pcIn[3] == '\0')) ||
       ((pcIn[0] == 'L') && (pcIn[1] == 'P') &&
        (pcIn[2] == 'T') && ((pcIn[3] == '1') || (pcIn[3] == '2') ||
                             (pcIn[3] == '3') || (pcIn[3] == '4')) &&
        (pcIn[4] == '\0'))) {
      strcpy(pcOut, pcIn);
      return 0;
      } /* endif */
 
      pcIn +=2;
      } /* endif */
   DosQCurDisk(&usDriveNum, &ulDriveMap);
 
   *pcOut++ = (CHAR)(usDriveNum + '@');
   *pcOut++ = ':';
   *pcOut++ = '\\';
 
   if (pcIn[0] == '\0')
      return 3;
 
   if (NULL == (pcLastSlash = strrchr(pcIn, '\\'))) {
      if (!DosChDir(pcIn, 0L))
         return 3;
 
      DosQCurDir(0, pcOut, &usDirLen);
      if (strlen(pcIn) > 12)
         return 3;
 
      if (*(pcOut + strlen(pcOut)- 1) != '\\')
         strcat(pcOut++, "\\");
 
      strcat(pcOut, pcIn);
      return 0;
      } /* endif */
 
   if (pcIn == pcLastSlash) {
      DosChDir("\\", 0L);
 
      if (pcIn[1] == '\0')
         return 3;
 
      strcpy(pcOut, pcIn + 1);
      return 0;
      } /* endif */
 
   *pcLastSlash = '\0';
 
   if (DosChDir(pcIn, 0L))
      return 2;
 
   DosQCurDir (0, pcOut, &usDirLen);
 
   pcFileOnly = pcLastSlash + 1;
 
   if (*pcFileOnly == '\0')
      return 3;
 
   if (strlen(pcFileOnly) > 12)
      return 3;
 
   if (*(pcOut + strlen(pcOut) - 1) != '\\')
      strcat(pcOut++, "\\");
 
   strcat(pcOut, pcFileOnly);
   return 0;
   }
 
/*---------------------------------------------------------------------*\
 |                           FileDlgProc                               |
 +---------------------------------------------------------------------+
 | This function displays the Display to File dialog                   |
\*---------------------------------------------------------------------*/
MRESULT EXPENTRY  FileDlgProc (HWND   hwnd, /* Handle of Dialog Window */
                               USHORT msg,  /* Message to be handled   */
                               MPARAM mp1,  /* Parameters for message  */
                               MPARAM mp2) {
   static CHAR szCurrentPath [80], szBuffer [80];
   static HWND hwndFileDlgHelp;
   SHORT  sSelect;
 
   switch (msg) {
      case WM_INITDLG :
         FillDriveListBox (hwnd, szCurrentPath);
         FillDirListBox   (hwnd, szCurrentPath);
         FillFileListBox  (hwnd);
         WinSendDlgItemMsg (hwnd, IDD_DRIVE_LIST, LM_SELECTITEM,
                            MPFROMSHORT (szCurrentPath[0] - 'A'),
                            (MPARAM) TRUE);
         WinSendDlgItemMsg (hwnd, IDD_FILE_EDIT, EM_SETTEXTLIMIT,
                            MPFROM2SHORT (80, 0), NULL);
         WinSendDlgItemMsg (hwnd, IDD_APPEND, BM_SETCHECK,
                            MPFROMSHORT ((fAppend) ? 1 : 0), NULL);
         /* Associate the Help Instance with the Dialog                */
         hwndFileDlgHelp = *((PHWND) mp2);
         if (NULL != hwndFileDlgHelp) {
            if (!WinAssociateHelpInstance (hwndFileDlgHelp, hwnd)) {
               hwndFileDlgHelp = NULL;
               } /* endif */
            } /* endif */
         break;
      case WM_CONTROL :
         if ((IDD_DRIVE_LIST == SHORT1FROMMP (mp1)) ||
             (IDD_DIR_LIST   == SHORT1FROMMP (mp1)) ||
             (IDD_FILE_LIST  == SHORT1FROMMP (mp1))) {
            sSelect = SHORT1FROMMP (WinSendDlgItemMsg (hwnd,
                                                       SHORT1FROMMP (mp1),
                                                       LM_QUERYSELECTION,
                                                       0L, 0L));
            WinSendDlgItemMsg (hwnd, SHORT1FROMMP (mp1), LM_QUERYITEMTEXT,
                               MPFROM2SHORT (sSelect, sizeof (szBuffer)),
                               MPFROMP (szBuffer));
            } /* endif */
 
         switch (SHORT1FROMMP (mp1)) {
            case IDD_DRIVE_LIST :
               switch (SHORT2FROMMP (mp1)) {
                  case LN_ENTER :
                     szCurrentPath[0] = szBuffer[0];
                     DosSelectDisk (szBuffer[0] - '@');
                     FillDirListBox (hwnd, szCurrentPath);
                     FillFileListBox  (hwnd);
                     break;
                  default: ;
                  } /* endswitch */
               break;
            case IDD_DIR_LIST :
               switch (SHORT2FROMMP (mp1)) {
                  case LN_ENTER :
                     DosChDir (szBuffer, 0L);
                     FillDirListBox (hwnd, szCurrentPath);
                     FillFileListBox  (hwnd);
                     WinSetDlgItemText (hwnd, IDD_FILE_EDIT, "");
                     break;
                  default: ;
                  } /* endswitch */
               break;
            case IDD_FILE_LIST :
               switch (SHORT2FROMMP (mp1)) {
                  case LN_SELECT :
                     WinSetDlgItemText (hwnd, IDD_FILE_EDIT, szBuffer);
                     break;
                  case LN_ENTER :
                     ParseFilename (szFilename, szBuffer);
                     fAppend = SHORT1FROMMP (WinSendDlgItemMsg (hwnd,
                                                                IDD_APPEND,
                                                                BM_QUERYCHECK,
                                                                NULL, NULL));
                     return ((MRESULT) (WinDismissDlg (hwnd, TRUE)));
                     break;
                  default: ;
                  } /* endswitch */
               break;
               default: ;
               } /* endswitch */
         break;
      /* User pressed the space bar, escape key, enter key, or clicked */
      /* the default "OK" push button                                  */
      case WM_COMMAND :
         switch (COMMANDMSG(&msg)->cmd) {
            case DID_OK :
               WinQueryDlgItemText (hwnd, IDD_FILE_EDIT,
                                    sizeof (szBuffer), szBuffer);
               switch (ParseFilename (szCurrentPath, szBuffer)) {
                  case 0 :
                     fAppend = SHORT1FROMMP (WinSendDlgItemMsg (hwnd,
                                                                IDD_APPEND,
                                                                BM_QUERYCHECK,
                                                                NULL, NULL));
                     strcpy (szFilename, szCurrentPath);
                     return ((MRESULT) (WinDismissDlg (hwnd, TRUE)));
                     break;
                  case 1 :
                     WinLoadString (hab, RCHandle, INVALID_DRIVE,
                                    STRINGSIZE, achString);
                     WinMessageBox (HWND_DESKTOP, hwnd,
                                    achString, NULL, 0,
                                    MB_OK | MB_ICONEXCLAMATION);
                     FillDirListBox  (hwnd, szCurrentPath);
                     FillFileListBox (hwnd);
                     return 0;
                     break;
                  case 2 :
                     WinLoadString (hab, RCHandle, INVALID_DIRECTORY,
                                    STRINGSIZE, achString);
                     WinMessageBox (HWND_DESKTOP, hwnd,
                                    achString, NULL, 0,
                                    MB_OK | MB_ICONEXCLAMATION);
                     FillDirListBox  (hwnd, szCurrentPath);
                     FillFileListBox (hwnd);
                     return 0;
                     break;
                  case 3 :
                     WinLoadString (hab, RCHandle, INVALID_FILENAME,
                                    STRINGSIZE, achString);
                     WinMessageBox (HWND_DESKTOP, hwnd,
                                    achString, NULL, 0,
                                    MB_OK | MB_ICONEXCLAMATION);
                     FillDirListBox    (hwnd, szCurrentPath);
                     FillFileListBox   (hwnd);
                     WinSetDlgItemText (hwnd, IDD_FILE_EDIT, "");
                     return 0;
                     break;
                  default: ;
                  } /* endswitch */
               break;
            case DID_CANCEL :
               return ((MRESULT) (WinDismissDlg (hwnd, FALSE)));
               break;
            default: ;
            } /* endswitch */
            break;
 
      case WM_CLOSE:
         return ((MRESULT) (WinDismissDlg (hwnd, FALSE)));
         break;
 
      case WM_DESTROY :
         if (NULL != hwndFileDlgHelp) {
            WinAssociateHelpInstance (NULL, hwnd);
            hwndFileDlgHelp = NULL;
         } /* endif */
         break;
 
      /* Default: do nothing, pass it on to the Default Dialog Proc    */
      default: ;
      } /* endswitch */
 
   /* Let the default Dialog Procedure do what it's gotta do           */
   return WinDefDlgProc (hwnd, msg, mp1, mp2);
   }
 
/*---------------------------------------------------------------------*\
 |                           FillPLUListBox                            |
 +---------------------------------------------------------------------+
 | This function fills the PLU List Box                                |
\*---------------------------------------------------------------------*/
BOOL FillPLUListBox (HWND hwnd)
{
   unsigned i,j,k,l;                       /* Loop Counters */
   unsigned lu_count;                      /* Counter for LUs   */
   unsigned plu_count;                     /* Counter for partner LUs */
   USHORT   us,                            /* Return val of LM_SEARCHSTRING */
            usIndex;                       /* Loop Ctr for LM_QUERYITEMTEXT */
   CHAR RC_message[STRINGSIZE];            /* Storage for two RC messages   */
 
   LU62_INFO_SECT far * lu62_ptr;          /* Pointer to LU 6.2 info        */
   LU62_OVERLAY   far * lu62_ov_ptr;       /* Pointer to current LU         */
   PLU62_OVERLAY  far * plu62_ov_ptr;      /* Pointer to current partner LU */
   DISP_RETURN_INFO dri;                   /* Return codes from DISPLAY     */
   ULONG display_length;                   /* Length of info returned in buf*/
 
   lu62_ptr = exec_display (DISPLAY_INFO_LU62, data_buffer_ptr,
                            INFO_BUFFER_SIZE,
                            &dri.primary_rc, &dri.secondary_rc,
                            &display_length);
   if (AP_OK != dri.primary_rc) {
      if (AP_COMM_SUBSYSTEM_NOT_LOADED == dri.primary_rc) {
         WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
                        MSG_APPC_NOT_LOADED, NULL, 0,
                        MB_OK | MB_ICONEXCLAMATION);
      } else {
         RC_message[0] = '\0';
         strcat (RC_message, MSG_RETURN_CODE1);
         strcat (RC_message, " X'%04X'\n");
         strcat (RC_message, MSG_RETURN_CODE2);
         strcat (RC_message, " X'%08lX'\n");
         sprintf (achString, RC_message,
                  SWAP2 (dri.primary_rc), SWAP4 (dri.secondary_rc));
         WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
                        achString, MSG_UNEXPECTED_RETURN_CODE, 0,
                        MB_OK | MB_ICONEXCLAMATION);
         } /* endif */
      return (FALSE);
      } /* endif */
 
   /*--------------------------------------------------------*/
   /* For each LU 6.2 overlay                                */
   /*--------------------------------------------------------*/
   lu_count = lu62_ptr->num_lu62s;         /* Get the LU count */
   for (lu62_ov_ptr = (LU62_OVERLAY far *)
           ((UCHAR far *)lu62_ptr + lu62_ptr->lu62_init_sect_len),
        i = 0; i < lu_count; i++,
        lu62_ov_ptr = (LU62_OVERLAY far *)
           ((UCHAR far *)lu62_ov_ptr + lu62_ov_ptr->lu62_entry_len)) {
 
      /*--------------------------------------------------------*/
      /* For each partner LU overlay in this LU 6.2 overlay     */
      /*--------------------------------------------------------*/
      plu_count = lu62_ov_ptr->num_plus;   /* Get partner LU count */
      for (plu62_ov_ptr = (PLU62_OVERLAY far *)
              ((UCHAR far *)lu62_ov_ptr +
               lu62_ov_ptr->lu62_overlay_len +
               sizeof(lu62_ov_ptr->lu62_entry_len)),
           j = 0; j < plu_count; j++,
           plu62_ov_ptr = (PLU62_OVERLAY far *)
              ((UCHAR far *)plu62_ov_ptr + plu62_ov_ptr->plu62_entry_len)) {
 
         ebcdic_to_ascii (plu62_ov_ptr->fqplu_name,
                          sizeof(plu62_ov_ptr->fqplu_name));
         k = sprintf (achString, "%.*s", sizeof (plu62_ov_ptr->plu_alias),
                                         plu62_ov_ptr->plu_alias);
         for (l = 8 - k; l > 0; l--) {
            k += sprintf (achString + k, " ");
            } /* endfor */
         sprintf (achString + k, "     %.*s",
                  sizeof (plu62_ov_ptr->fqplu_name),
                  plu62_ov_ptr->fqplu_name);
 
         us = SHORT1FROMMR (WinSendDlgItemMsg (hwnd,
                            IDD_PLU_LIST, LM_SEARCHSTRING,
                            MPFROM2SHORT (LSS_CASESENSITIVE, LIT_FIRST),
                            MPFROMP (achString)));
         if (LIT_NONE == us) {
            WinSendDlgItemMsg (hwnd, IDD_PLU_LIST, LM_INSERTITEM,
                               MPFROM2SHORT (LIT_SORTASCENDING, 0),
                               MPFROMP (achString));
            } /* endif */
         } /* endfor (partner LU overlays) */
      } /* endfor (LU overlays) */
 
   if (fLocalAPPN) {
      PLU_DEF_INFO_SECT far * plu_def_ptr; /* Pointer to partner LU info    */
      PLU_DEF_OVERLAY far * plu_def_ov_ptr;/* Pointer to partner LU overlay */
 
      plu_def_ptr = exec_display (DISPLAY_INFO_PLUDEF, data_buffer_ptr,
                                  INFO_BUFFER_SIZE,
                                  &dri.primary_rc, &dri.secondary_rc,
                                  &display_length);
      if (AP_OK != dri.primary_rc) {
         if (AP_COMM_SUBSYSTEM_NOT_LOADED == dri.primary_rc) {
            WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
                           MSG_APPC_NOT_LOADED, NULL, 0,
                           MB_OK | MB_ICONEXCLAMATION);
         } else {
         RC_message[0] = '\0';
         strcat (RC_message, MSG_RETURN_CODE1);
         strcat (RC_message, " X'%04X'\n");
         strcat (RC_message, MSG_RETURN_CODE2);
         strcat (RC_message, " X'%08lX'\n");
            sprintf (achString, RC_message,
                     SWAP2 (dri.primary_rc), SWAP4 (dri.secondary_rc));
            WinMessageBox (HWND_DESKTOP, HWND_DESKTOP,
                           achString, MSG_UNEXPECTED_RETURN_CODE, 0,
                           MB_OK | MB_ICONEXCLAMATION);
            } /* endif */
         return (FALSE);
         } /* endif */
      plu_count = plu_def_ptr->num_plu_def;
      for (plu_def_ov_ptr = (PLU_DEF_OVERLAY far *)
           ((UCHAR far *)plu_def_ptr + plu_def_ptr->plu_def_init_sect_len),
           i = 0; i < plu_count; i++,
           plu_def_ov_ptr = (PLU_DEF_OVERLAY far *)
           ((UCHAR far *)plu_def_ov_ptr + plu_def_ov_ptr->plu_def_entry_len))
         {
         ebcdic_to_ascii(plu_def_ov_ptr->fqplu_name,
                         sizeof(plu_def_ov_ptr->fqplu_name));
         k = sprintf (achString, "%.*s", sizeof (plu_def_ov_ptr->plu_alias),
                                         plu_def_ov_ptr->plu_alias);
         for (l = 8 - k; l > 0; l--) {
            k += sprintf (achString + k, " ");
            } /* endfor */
         sprintf (achString + k, "     %.*s",
                  sizeof (plu_def_ov_ptr->fqplu_name),
                  plu_def_ov_ptr->fqplu_name);
 
         us = SHORT1FROMMR (WinSendDlgItemMsg (hwnd,
                            IDD_PLU_LIST, LM_SEARCHSTRING,
                            MPFROM2SHORT (LSS_CASESENSITIVE, LIT_FIRST),
                            MPFROMP (achString)));
         if (LIT_NONE == us) {
            WinSendDlgItemMsg (hwnd, IDD_PLU_LIST, LM_INSERTITEM,
                               MPFROM2SHORT (LIT_SORTASCENDING, 0),
                               MPFROMP (achString));
            } /* endif */
         } /* endfor */
      } /* endif */
 
   WinLoadString (hab, RCHandle, LOCAL_MACHINE, STRINGSIZE, achString);
   WinSendDlgItemMsg (hwnd, IDD_PLU_LIST, LM_INSERTITEM,
                      MPFROMSHORT (0), MPFROMP (achString));
 
   us = SHORT1FROMMR (WinSendDlgItemMsg (hwnd, IDD_PLU_LIST,
                                         LM_QUERYITEMCOUNT, NULL, NULL));
   for (usIndex = 0; usIndex < us; usIndex++) {
      WinSendDlgItemMsg (hwnd, IDD_PLU_LIST, LM_QUERYITEMTEXT,
                         MPFROM2SHORT (usIndex, STRINGSIZE),
                         MPFROMP (achString));
      if (strstr (achString, wp.PLU_name)) {
         WinSendDlgItemMsg (hwnd, IDD_PLU_LIST, LM_SELECTITEM,
                            MPFROMSHORT (usIndex), (MPARAM) TRUE);
         } /* endif */
      } /* endfor */
   return (TRUE);
}
 
/*---------------------------------------------------------------------*\
 |                         TargetDlgProc                               |
 +---------------------------------------------------------------------+
 | This function displays the PLU Name Selection dialog                |
\*---------------------------------------------------------------------*/
MRESULT EXPENTRY TargetDlgProc (HWND   hwnd,/* Handle of Dialog Window */
                                USHORT msg, /* Message to be handled   */
                                MPARAM mp1, /* Parameters for message  */
                                MPARAM mp2) {
   static CHAR szBuffer [18];
   static HWND hwndPLUDlgHelp;
   CHAR   *pcFirstBlank;
   SHORT  sSelect;
 
   switch (msg) {
      case WM_INITDLG :
         WinSetPresParam (WinWindowFromID (hwnd, IDD_PLU_LIST),
                          PP_FONTNAMESIZE, (ULONG) strlen (szListFont) + 1,
                          szListFont);
         if (!FillPLUListBox (hwnd))
            return ((MRESULT) (WinDismissDlg (hwnd, FALSE)));
         WinSendDlgItemMsg (hwnd, IDD_PLU_EDIT, EM_SETTEXTLIMIT,
                            MPFROM2SHORT (18, 0), NULL);
         /* Associate the Help Instance with the Dialog                */
         hwndPLUDlgHelp = *((PHWND) mp2);
         if (NULL != hwndPLUDlgHelp) {
            if (!WinAssociateHelpInstance (hwndPLUDlgHelp, hwnd)) {
               hwndPLUDlgHelp = NULL;
               } /* endif */
            } /* endif */
         break;
      case WM_CONTROL :
         if (IDD_PLU_LIST == SHORT1FROMMP (mp1)) {
            sSelect = SHORT1FROMMP (WinSendDlgItemMsg (hwnd,
                                                       SHORT1FROMMP (mp1),
                                                       LM_QUERYSELECTION,
                                                       0L, 0L));
            WinSendDlgItemMsg (hwnd, SHORT1FROMMP (mp1), LM_QUERYITEMTEXT,
                               MPFROM2SHORT (sSelect, sizeof (szBuffer)),
                               MPFROMP (szBuffer));
            if (NULL != (pcFirstBlank = strchr (szBuffer, ' ')))
               *pcFirstBlank = '\0';
            } /* endif */
 
         switch (SHORT1FROMMP (mp1)) {
            case IDD_PLU_LIST :
               switch (SHORT2FROMMP (mp1)) {
                  case LN_SELECT :
                     WinSetDlgItemText (hwnd, IDD_PLU_EDIT, szBuffer);
                     break;
                  case LN_ENTER :
                     if (invalid_PLU_name (szBuffer)) {
                        WinLoadString (hab, RCHandle, INVALID_PLU_NAME,
                                       STRINGSIZE, achString);
                        WinMessageBox (HWND_DESKTOP, hwnd,
                                       achString, NULL, 0,
                                       MB_OK | MB_ICONEXCLAMATION);
                        return 0;
                     } else {
                        strcpy (wp.PLU_name, szBuffer);
                        return ((MRESULT) (WinDismissDlg (hwnd, TRUE)));
                        } /* endif */
                     break;
                  default: ;
                  } /* endswitch */
               break;
            default: ;
            } /* endswitch */
         break;
      /* User pressed the space bar, escape key, enter key, or clicked */
      /* the default "OK" push button                                  */
      case WM_COMMAND :
         switch (COMMANDMSG(&msg)->cmd) {
            case DID_OK :
               WinQueryDlgItemText (hwnd, IDD_PLU_EDIT,
                                    sizeof (szBuffer), szBuffer);
               if (invalid_PLU_name (szBuffer)) {
                  WinLoadString (hab, RCHandle, INVALID_PLU_NAME,
                                 STRINGSIZE, achString);
                  WinMessageBox (HWND_DESKTOP, hwnd,
                                 achString, NULL, 0,
                                 MB_OK | MB_ICONEXCLAMATION);
                  return 0;
               } else {
                  strcpy (wp.PLU_name, szBuffer);
                  return ((MRESULT) (WinDismissDlg (hwnd, TRUE)));
                  } /* endif */
               break;
            case DID_CANCEL :
               return ((MRESULT) (WinDismissDlg (hwnd, FALSE)));
               break;
            default: ;
            } /* endswitch */
            break;
 
      case WM_CLOSE:
         return ((MRESULT) (WinDismissDlg (hwnd, FALSE)));
         break;
 
      case WM_DESTROY :
         if (NULL != hwndPLUDlgHelp) {
            WinAssociateHelpInstance (NULL, hwnd);
            hwndPLUDlgHelp = NULL;
         } /* endif */
         break;
 
      /* Default: do nothing, pass it on to the Default Dialog Proc    */
      default: ;
      } /* endswitch */
 
   /* Let the default Dialog Procedure do what it's gotta do           */
   return WinDefDlgProc (hwnd, msg, mp1, mp2);
   }
