/****************************************************************************/
/*                                                                          */
/*  MODULE NAME : PMDSPLAY.H                                                */
/*                                                                          */
/*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
/*                                                                          */
/*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991                         */
/*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
/*                     ALL RIGHTS RESERVED                                  */
/*                                                                          */
/*  FUNCTION:   Global definitions and declaractions for the PMDSPLAY       */
/*              sample program.                                             */
/*                                                                          */
/*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
/*              (compiles with large memory model)                          */
/*                                                                          */
/*  ASSOCIATED FILES:  See PMDSPLAY.MAK                                     */
/*                                                                          */
/****************************************************************************/

/*---------------------------------------------------------------------*\
 | Common Defines for Includes                                         |
\*---------------------------------------------------------------------*/
#define  INCL_AVIO
#define  INCL_WINDIALOGS
#define  INCL_WINFRAMEMGR      /* Define only what we need in the OS/2 */
#define  INCL_WINHELP          /* header file, so the compiler won't   */
#define  INCL_WINMESSAGEMGR    /* run out of space                     */
#define  INCL_WINWINDOWMGR
#define  INCL_DOSMODULEMGR

/*---------------------------------------------------------------------*\
 | Common Includes                                                     |
\*---------------------------------------------------------------------*/
#include <os2.h>               /* Header file for OS/2 Toolkit         */

/* MSC 6.0 handles multi-thread includes differently than IBM C/2      */
#if _MSC_VER >= 600
   #define  _MT                /* Define multithread var. for MSC 6.0  */
   #include <stdio.h>
   #include <string.h>
#else
   #include <mt\stdio.h>
   #include <mt\string.h>
   #endif

/*---------------------------------------------------------------------*\
 | Resource Defines                                                    |
\*---------------------------------------------------------------------*/
#define ID_MAIN_RESOURCE    1  /* Resource IDs                         */
#define ID_INFO_RESOURCE    2
#define ID_LABEL            3

/*---------------------------------------------------------------------*\
 | SubMenu and MenuItem ID Defines                                     |
\*---------------------------------------------------------------------*/
#define IDM_OPTIONS       100  /* ID for "Options"                     */
#define IDM_SCREEN        101  /* ID for "Display to Screen"           */
#define IDM_FILE          102  /* ID for "Display to File"             */
#define IDM_TARGET        103  /* ID for "Select Target"               */
#define IDM_REFRESH       104  /* ID for "Refresh"                     */
#define IDM_HELP          105  /* ID for "Help"                        */
#define IDM_HELP_FOR_HELP 106  /* ID for "Help for help"               */
#define IDM_ABOUT         107  /* ID for "About"                       */

#define IDM_SELECT        120  /* ID for "Display"                     */
#define IDM_APPC          121  /* ID for "APPC"                        */
#define IDM_APPN          122  /* ID for "APPN"                        */
#define IDM_EXIT          123  /* ID for "Exit"                        */
#define IDM_INFO_FIRST    130
#define IDM_INFO_GLOBAL   130  /* ID for Global SNA info                  */
#define IDM_INFO_LU62     131  /* ID for LU 6.2 info                      */
#define IDM_INFO_AM       132  /* ID for Attach Manager info              */
#define IDM_INFO_TP       133  /* ID for Transaction Program info         */
#define IDM_INFO_SESSIONS 134  /* ID for Active Sessions info             */
#define IDM_INFO_LINKS    135  /* ID for Active Links info                */
#define IDM_INFO_LU03     136  /* ID for LU 0-3 info                      */
#define IDM_INFO_GW       137  /* ID for Gateway info                     */
#define IDM_INFO_X25      138  /* ID for X.25 info                        */
#define IDM_INFO_SYSDEF   139  /* ID for System Defaults info             */
#define IDM_INFO_ADAPTER  140  /* ID for Adapter info                     */
#define IDM_INFO_LUDEF    141  /* ID for LU Definitions info              */
#define IDM_INFO_PLUDEF   142  /* ID for Partner LU Definitions info      */
#define IDM_INFO_MODES    143  /* ID for Mode Definitions info            */
#define IDM_INFO_LINKDEF  144  /* ID for Link Definitions info            */
#define IDM_INFO_MS       145  /* ID for Management Services info         */
#define IDM_INFO_NODE     146  /* ID for APPN Node info                   */
#define IDM_INFO_DIR      147  /* ID for APPN Directory info              */
#define IDM_INFO_TOP      148  /* ID for APPN Topology info               */
#define IDM_INFO_ISR      149  /* ID for APPN Intermediate Sessions info  */
#define IDM_INFO_COS      150  /* ID for APPN Class of Service info       */
#define IDM_INFO_CN       151  /* ID for APPN Connection Networks info    */
#define IDM_INFO_LAST     151

/* When information is added to this list, remember that IDM_INFO_LAST */
/* must be equal to the *LAST* type.                                   */

/*---------------------------------------------------------------------*\
 | String table Defines                                                |
\*---------------------------------------------------------------------*/
#define STRINGSIZE        256  /* Size of string for WinLoadString     */

#define TITLEBAR_TEXT     202  /* ID for title bar text                */
#define PROG_TITLE_FIRST  201  /* ID for first line of program title   */
#define PROG_TITLE_LAST   202  /* ID for last line of program title    */
#define HELP_WIN_TITLE    210  /* ID for "Help for PMDSPLAY"           */
#define NOT_AVAILABLE     211  /* IDs for error messages               */
#define NO_HELP_AVAILABLE 212
#define HELP_ERROR        213
#define BUF_ALLOC_ERROR   214
#define FILE_OPEN_FAILURE 215
#define NOT_APPC_OPTION   216
#define INVALID_DRIVE     217
#define INVALID_DIRECTORY 218
#define INVALID_FILENAME  219
#define DISP_TO_FILE_DONE 220
#define ATTEMPTING_ALLOC  221
#define CONV_ALLOC_SUCC   222
#define CONV_ALLOC_UNSUCC 223
#define LOCAL_MACHINE     224
#define INVALID_PLU_NAME  225
#define THREAD_ALLOC_ERR  226
#define BEGINTHREAD_ERR   227

/*---------------------------------------------------------------------*\
 | Dialog Template Defines                                             |
\*---------------------------------------------------------------------*/
#define IDD_ABOUT         600  /* IDs for the About Box                */

#define IDD_FILE          700  /* IDs for the File Dialog              */
#define IDD_DRIVE_LIST    701
#define IDD_DIR_LIST      702
#define IDD_FILE_LIST     703
#define IDD_PATH          704
#define IDD_FILE_EDIT     705
#define IDD_APPEND        706
#define IDD_FILE_DLG_HELP 707

#define IDD_TARGET        800  /* IDs for the Target PLU Dialog        */
#define IDD_PLU_LIST      801
#define IDD_PLU_NAME      802
#define IDD_PLU_EDIT      803
#define IDD_PLU_DLG_HELP  804

/*---------------------------------------------------------------------*\
 | Help ID Defines -- Main Window                                      |
\*---------------------------------------------------------------------*/
#define IDH_MAIN_EXT     1001  /*                                      */

#define IDH_OPTIONS      1100
#define IDH_SCREEN       1110
#define IDH_FILE         1120
#define IDH_TARGET       1130
#define IDH_REFRESH      1140

#define IDH_SELECT       1200
#define IDH_APPC         1210
#define IDH_ADAPTER      1211
#define IDH_GLOBAL       1212
#define IDH_LINKDEF      1213
#define IDH_LINKS        1214
#define IDH_LU03         1215
#define IDH_LU62         1216
#define IDH_LUDEF        1217
#define IDH_MS           1218
#define IDH_MODES        1219
#define IDH_PLUDEF       1220
#define IDH_SESSIONS     1221
#define IDH_GW           1222
#define IDH_SYSDEF       1223
#define IDH_AM           1224
#define IDH_TP           1225
#define IDH_X25          1226
#define IDH_APPN         1230
#define IDH_COS          1231
#define IDH_CN           1232
#define IDH_DIR          1233
#define IDH_ISR          1234
#define IDH_NODE         1235
#define IDH_TOP          1236
#define IDH_EXIT         1240

#define IDH_HELP         1300
#define IDH_HELPHELP     1310
#define IDH_HELPEXTENDED 1320
#define IDH_HELPINDEX    1330
#define IDH_HELPKEYS     1340
#define IDH_KEYS_HELP    1350
#define IDH_ABOUT        1360

/*---------------------------------------------------------------------*\
 | Help ID Defines -- File Dialog                                      |
\*---------------------------------------------------------------------*/
#define IDH_FILE_DLG_EXT 1002
#define IDH_DRIVE_LIST   1400
#define IDH_DIR_LIST     1410
#define IDH_FILE_LIST    1420
#define IDH_FILE_EDIT    1430
#define IDH_APPEND       1440
#define IDH_FILE_OK      1450
#define IDH_FILE_CANCEL  1460
#define IDH_FILE_DLG_HLP 1470

/*---------------------------------------------------------------------*\
 | Help ID Defines -- Target Dialog                                    |
\*---------------------------------------------------------------------*/
#define IDH_TARG_DLG_EXT 1003
#define IDH_PLU_LIST     1500
#define IDH_PLU_EDIT     1510
#define IDH_TARG_OK      1520
#define IDH_TARG_CANCEL  1530
#define IDH_PLU_DLG_HLP  1540

/*---------------------------------------------------------------------*\
 | User defined messages (WM_USER and above)                           |
\*---------------------------------------------------------------------*/
#define WM_FILE_OPEN     (WM_USER + 0) /* Return boolean status in mp1 */
#define WM_ALLOCATE      (WM_USER + 1) /* Return boolean status in mp1 */
#define WM_THREAD_AVAIL  (WM_USER + 2) /* Set Thread Available status  */
#define WM_GRAB_FOCUS    (WM_USER + 3) /* Time to grab the focus       */

/*---------------------------------------------------------------------*\
 | Maximum VIO buffer dimensions                                       |
\*---------------------------------------------------------------------*/
#define VIO_NUM_ATTRIB             (1) /* Number of attributes per cell*/
#define VIO_CELL_SIZE (1 + VIO_NUM_ATTRIB) /* Data byte + attributes   */
#define MAX_VIO_CHARS ((USHORT)(65530 / VIO_CELL_SIZE))
#define MAX_VIO_HEIGHT           (200) /* 255 doesn't work well        */
#define VIO_EXTENDED_ATTRIB     (0x10) /* VIO Transparent background   */

/*---------------------------------------------------------------------*\
 | Definition of the structure with which formatted lines are linked   |
 | in a list (ie, line queue element)                                  |
\*---------------------------------------------------------------------*/
typedef struct LINE_QE {
   struct LINE_QE * pNext;    /* Pointer to next element in list       */
   struct LINE_QE * pPrev;    /* Pointer to previous element in list   */
   SHORT     sLen;            /* Length of line                        */
   char *    pLine;           /* Pointer to line (ASCIIZ string)       */
} LINE_QE;

/*---------------------------------------------------------------------*\
 | Definition of the structure which contains the variables needed on  |
 | a "per window" basis.                                               |
\*---------------------------------------------------------------------*/
typedef struct {
   HPS    hps;                   /* Handle to Presentation Space       */
   HVPS   hvps;                  /* Handle to Video Presentation Space */
   HWND   hwndHscroll;           /* Handle to Horizontal Scroll Bar    */
   HWND   hwndVscroll;           /* Handle to Vertical Scroll Bar      */
   HDC    hdc;                   /* Handle to Device Context           */
   SHORT  sNumLines;             /* Total number of formatted lines    */
   SHORT  sNumCols;              /* Maximum formatted line length      */
   SHORT  sVioRows;              /* Number of rows in VIO buffer       */
   SHORT  sVioCols;              /* Number of columns in VIO buffer    */
   SHORT  sHscrollPos;           /* Horzizontal Scroll Bar Position    */
   SHORT  sVscrollPos;           /* Vertical Scroll Bar Position       */
   SHORT  sHscrollMax;           /* Horizontal Scroll Bar Limit        */
   SHORT  sVscrollMax;           /* Vertical Scroll Bar Limit          */
   SHORT  cxWindow;              /* Width of window                    */
   SHORT  cyWindow;              /* Height of window                   */
   SHORT  cxChar;                /* Width of VIO character cell        */
   SHORT  cyChar;                /* Height of VIO character cell       */
   SHORT  sWinCols;              /* Number of columns in window        */
   SHORT  sWinRows;              /* Number of rows in window           */
   SHORT  sCurrentInfo;          /* Current Information Index          */
   LINE_QE * pTopLineQe;         /* Line at top of VIO buffer          */
   LINE_QE * pBotLineQe;         /* Line at bottom of VIO buffer       */
   BOOL   fRemote;               /* Flag, DISPLAY on remote machine    */
   UCHAR  PLU_name[18];          /* PLU Name of target machine         */
   UCHAR  tp_id[8];              /* Transaction Program ID             */
   ULONG  conv_id;               /* Conversation ID                    */
   BOOL   fWinExists;            /* Flag indicating window exists      */
   BOOL   fTargetFile;           /* Flag, output to file or screen     */
} WINPARAM;
