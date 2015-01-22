#****************************************************************************/
#*                                                                          */
#*  MODULE NAME : PMDSPLAY.MAK                                              */
#*                                                                          */
#*  DESCRIPTIVE NAME : DISPLAY SAMPLE PROGRAM FOR COMMUNICATIONS MANAGER    */
#*                                                                          */
#*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991                         */
#*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
#*                     ALL RIGHTS RESERVED                                  */
#*                                                                          */
#*  FUNCTION:   MAKE file for the PMDSPLAY sample program.                  */
#*                                                                          */
#*  MODULE TYPE:     Microsoft C Compiler Version 6.0                       */
#*              (compiles with large memory model)                          */
#*                                                                          */
#*  ASSOCIATED FILES:                                                       */
#*                                                                          */
#*      PMDSPLAY.MAK - MAKE file                                            */
#*      PMDSPLAY.DEF - Module definition file (for LINK)                    */
#*      PMDSPLAY.RC  - Resource definitions                                 */
#*      PMDSPLAY.IPF - Help source                                          */
#*      PMDSPLAY.H   - Global typedefs, prototypes, and #includes           */
#*      PMDSPLAY.C   - Main function                                        */
#*      PMD_MAIN.C   - Client window procedure                              */
#*      PMD_UTIL.C   - Utilities                                            */
#*      PMD_DLGS.C   - Dialog functions                                     */
#*      DISPLAY.H    - Global typedefs, prototypes, and #includes           */
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
#*      MSGID.H      - #defines for messages                                */
#*      APD.MSG      - Messages (English)                                   */
#****************************************************************************/

C_OPTIONS    = -c -nologo -Alfw -G2   -Gt64 -Ox -W3 $(other_c_opt)
C_OPTIONS_PM = -c -nologo -Alfw -G2sw       -Ox -W3 $(other_c_opt)
other_c_opt =
other_l_opt =

all: pmdsplay.exe pmdsplay.hlp pmdsplay.dll apd.msg

PMDSPLAY.EXE: PMDSPLAY.DEF \
              PMDSPLAY.OBJ \
              PMD_MAIN.OBJ \
              PMD_UTIL.OBJ \
              PMD_DLGS.OBJ \
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
   link  @<<pmdsplay.lrf
PMDSPLAY.OBJ +
PMD_MAIN.OBJ +
PMD_UTIL.OBJ +
PMD_DLGS.OBJ +
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
PMDSPLAY.EXE /NOD /MAP $(other_l_opt)
PMDSPLAY.MAP
OS2 LLIBCMT ACS
PMDSPLAY.DEF
<<
#/*--- Help Modules ----------------------------------------------------*/

PMDSPLAY.HLP: PMDSPLAY.IPF PMDSPLAY.H
   ipfc PMDSPLAY.IPF /COUNTRY=001 /LANGUAGE=ENU

#/*--- Message Module --------------------------------------------------*/

APD.MSG: APD.TXT
   mkmsgf APD.TXT APD.MSG

#/*--- Resource Module  ------------------------------------------------*/
PMRCSTUB.OBJ: PMRCSTUB.C
   cl /c /Gs PMRCSTUB.C

PMDSPLAY.DLL: PMRCSTUB.OBJ \
              PMDSPLAY.RC  \
              PMDSPLAY.H   \
              PMDSPRC.DEF
   link pmrcstub.obj, PMDSPLAY.DLL, nul, /NOD, PMDSPRC.DEF
   rc PMDSPLAY.RC PMDSPLAY.DLL

#/*--- C and ASM Modules -----------------------------------------------*/

PMDSPLAY.OBJ: PMDSPLAY.C PMDSPLAY.H
   cl $(C_OPTIONS_PM) PMDSPLAY.C

PMD_MAIN.OBJ: PMD_MAIN.C PMDSPLAY.H
   cl $(C_OPTIONS_PM) PMD_MAIN.C

PMD_DLGS.OBJ: PMD_DLGS.C PMDSPLAY.H DISPLAY.H MSGID.H
   cl $(C_OPTIONS_PM) PMD_DLGS.C

PMD_UTIL.OBJ: PMD_UTIL.C PMDSPLAY.H DISPLAY.H
   cl $(C_OPTIONS_PM) PMD_UTIL.C

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