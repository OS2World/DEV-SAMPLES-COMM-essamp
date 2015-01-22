#****************************************************************************/
#*                                                                          */
#*  MODULE NAME : DISPLAY.MAK                                               */
#*                                                                          */
#*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
#*                                                                          */
#*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991                         */
#*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
#*                     ALL RIGHTS RESERVED                                  */
#*                                                                          */
#*  FUNCTION:   MAKE file for the DISPLAY sample program.                   */
#*                                                                          */
#*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
#*              (compiles with large memory model)                          */
#*                                                                          */
#*  ASSOCIATED FILES:                                                       */
#*                                                                          */
#*      DISPLAY.MAK  - MAKE file                                            */
#*      DISPLAY.DEF  - Module definition file (for LINK)                    */
#*      DISPLAY.H    - Global typedefs, prototypes, and #includes           */
#*      DISPLAY.C    - Main function and unique utility functions           */
#*      DISPUTIL.C   - Common utility functions                             */
#*      APPCUTIL.C   - Common utility functions for remote DISPLAY          */
#*      EXECDISP.C   - Utility function to execute DISPLAY verb             */
#*      SNA.C        - Formats global information                           */
#*      LU62.C       - Formats LU 6.2 information                           */
#*      AM.C         - Formats attach manager information                   */
#*      TP.C         - Formats transaction program information              */
#*      SESS.C       - Formats session information                          */
#*      LINKS.C      - Formats link information                             */
#*      LU03.C       - Formats LU 0, 1, 2, and 3 information                */
#*      GW.C         - Formats gateway information                          */
#*      X25.C        - Formats X.25 logical link information                */
#*      DEFAULTS.C   - Formats system defaults information                  */
#*      ADAPTER.C    - Formats adapter information                          */
#*      LU_DEF.C     - Formats LU definition information                    */
#*      PLU_DEF.C    - Formats partner LU definition information            */
#*      MODE_DEF.C   - Formats mode definition information                  */
#*      LINK_DEF.C   - Formats link definition information                  */
#*      MS.C         - Formats management services information              */
#*      NODE.C       - Formats APPN node information                        */
#*      DIR.C        - Formats APPN directory information                   */
#*      TOP.C        - Formats APPN topology information                    */
#*      ISR.C        - Formats APPN intermediate session information        */
#*      COS.C        - Formats APPN class of service information            */
#*      CN.C         - Formats APPN connection network information          */
#*      APD.MSG      - Messages (English)                                   */
#*      MSGID.H      - #defines for messages                                */
#*                                                                          */
#****************************************************************************/

C_OPTIONS    = -c -nologo -Alfw -G2   -Gt64 -Ox -W3 $(other_c_opt)
DEBUG_OPT    = -c -nologo -Alfw -G2   -Gt64 -Od -W3 -Zi $(other_c_opt)
other_c_opt  =
other_l_opt  =
DISPLAY.EXE:  DISPLAY.DEF  \
              DISPLAY.OBJ  \
              APPCUTIL.OBJ \
              DISPUTIL.OBJ \
              EXECDISP.OBJ \
              SNA.OBJ      \
              LU62.OBJ     \
              AM.OBJ       \
              TP.OBJ       \
              SESS.OBJ     \
              LINKS.OBJ    \
              LU03.OBJ     \
              GW.OBJ       \
              X25.OBJ      \
              DEFAULTS.OBJ \
              ADAPTER.OBJ  \
              LU_DEF.OBJ   \
              PLU_DEF.OBJ  \
              MODE_DEF.OBJ \
              LINK_DEF.OBJ \
              MS.OBJ       \
              NODE.OBJ     \
              DIR.OBJ      \
              TOP.OBJ      \
              ISR.OBJ      \
              COS.OBJ      \
              CN.OBJ
   LINK @<<DISPLAY.LRF
DISPLAY.OBJ  +
APPCUTIL.OBJ +
DISPUTIL.OBJ +
EXECDISP.OBJ +
SNA.OBJ      +
LU62.OBJ     +
AM.OBJ       +
TP.OBJ       +
SESS.OBJ     +
LINKS.OBJ    +
LU03.OBJ     +
GW.OBJ       +
X25.OBJ      +
DEFAULTS.OBJ +
ADAPTER.OBJ  +
LU_DEF.OBJ   +
PLU_DEF.OBJ  +
MODE_DEF.OBJ +
LINK_DEF.OBJ +
MS.OBJ       +
NODE.OBJ     +
DIR.OBJ      +
TOP.OBJ      +
ISR.OBJ      +
COS.OBJ      +
CN.OBJ
DISPLAY.EXE /NOD /MAP $(other_l_opt)
DISPLAY.MAP
OS2 LLIBCMT ACS
DISPLAY.DEF
<<
#/*--- Message Module --------------------------------------------------*/

APD.MSG: APD.TXT
   mkmsgf APD.TXT APD.MSG

#/*--- C Modules -------------------------------------------------------*/

DISPLAY.OBJ:  DISPLAY.C  DISPLAY.H MSGID.H
   CL $(C_OPTIONS) DISPLAY.C

APPCUTIL.OBJ: APPCUTIL.C DISPLAY.H MSGID.H
   CL $(C_OPTIONS) APPCUTIL.C

DISPUTIL.OBJ: DISPUTIL.C DISPLAY.H MSGID.H
   CL $(C_OPTIONS) DISPUTIL.C

EXECDISP.OBJ: EXECDISP.C DISPLAY.H MSGID.H
   CL $(C_OPTIONS) EXECDISP.C

AM.OBJ      : AM.C       DISPLAY.H MSGID.H
   CL $(C_OPTIONS) AM.C

GW.OBJ      : GW.C       DISPLAY.H MSGID.H
   CL $(C_OPTIONS) GW.C

LINKS.OBJ   : LINKS.C    DISPLAY.H MSGID.H
   CL $(C_OPTIONS) LINKS.C

LU03.OBJ    : LU03.C     DISPLAY.H MSGID.H
   CL $(C_OPTIONS) LU03.C

LU62.OBJ    : LU62.C     DISPLAY.H MSGID.H
   CL $(C_OPTIONS) LU62.C

SESS.OBJ    : SESS.C     DISPLAY.H MSGID.H
   CL $(C_OPTIONS) SESS.C

SNA.OBJ     : SNA.C      DISPLAY.H MSGID.H
   CL $(C_OPTIONS) SNA.C

TP.OBJ      : TP.C       DISPLAY.H MSGID.H
   CL $(C_OPTIONS) TP.C

X25.OBJ     : X25.C      DISPLAY.H MSGID.H
   CL $(C_OPTIONS) X25.C

DEFAULTS.OBJ: DEFAULTS.C DISPLAY.H MSGID.H
   CL $(C_OPTIONS) DEFAULTS.C

ADAPTER.OBJ : ADAPTER.C  DISPLAY.H MSGID.H
   CL $(C_OPTIONS) ADAPTER.C

LU_DEF.OBJ  : LU_DEF.C   DISPLAY.H MSGID.H
   CL $(C_OPTIONS) LU_DEF.C

PLU_DEF.OBJ : PLU_DEF.C  DISPLAY.H MSGID.H
   CL $(C_OPTIONS) PLU_DEF.C

MODE_DEF.OBJ: MODE_DEF.C DISPLAY.H MSGID.H
   CL $(C_OPTIONS) MODE_DEF.C

LINK_DEF.OBJ: LINK_DEF.C DISPLAY.H MSGID.H
   CL $(C_OPTIONS) LINK_DEF.C

MS.OBJ      : MS.C       DISPLAY.H MSGID.H
   CL $(C_OPTIONS) MS.C

NODE.OBJ    : NODE.C     DISPLAY.H MSGID.H
   CL $(C_OPTIONS) NODE.C

DIR.OBJ     : DIR.C      DISPLAY.H MSGID.H
   CL $(C_OPTIONS) DIR.C

TOP.OBJ     : TOP.C      DISPLAY.H MSGID.H
   CL $(C_OPTIONS) TOP.C

ISR.OBJ     : ISR.C      DISPLAY.H MSGID.H
   CL $(C_OPTIONS) ISR.C

COS.OBJ     : COS.C      DISPLAY.H MSGID.H
   CL $(C_OPTIONS) COS.C

CN.OBJ      : CN.C       DISPLAY.H MSGID.H
   CL $(C_OPTIONS) CN.C
