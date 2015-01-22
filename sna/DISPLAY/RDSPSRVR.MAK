#****************************************************************************/
#*                                                                          */
#*  MODULE NAME : RDSPSRVR.MAK                                              */
#*                                                                          */
#*  DESCRIPTIVE NAME : REMOTE DISPLAY SERVER SAMPLE PROGRAM                 */
#*                      FOR COMMUNICATIONS MANAGER                          */                        */
#*                                                                          */
#*  COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1991                         */
#*                     LICENSED MATERIAL - PROGRAM PROPERTY OF IBM          */
#*                     ALL RIGHTS RESERVED                                  */
#*                                                                          */
#*  FUNCTION:   MAKE file for RDSPSRVR, the remote display server sample    */
#*              program.                                                    */
#*                                                                          */
#*  MODULE TYPE:     MICROSOFT C COMPILER VERSION 6.0                       */
#*              (compiles with large memory model)                          */
#*                                                                          */
#*  ASSOCIATED FILES:                                                       */
#*                                                                          */
#*      RDSPSRVR.MAK - MAKE file                                            */
#*      RDSPSRVR.DEF - Module definition file (for LINK)                    */
#*      DISPLAY.H    - Global typedefs, prototypes, and #includes           */
#*      RDSPSRVR.C   - Main function and unique utility functions           */
#*      APPCUTIL.C   - Common utilities for remote DISPLAY                  */
#*      EXECDISP.C   - Utility function to execute DISPLAY verb             */
#*      APD.MSG      - Messages (English)                                   */
#*      MSGID.H      - #defines for messages                                */
#*                                                                          */
#****************************************************************************/

C_OPTIONS    = -c -nologo -Alfw -G2   -Gt64 -Ox -W3 $(other_c_opt)
DEBUG_OPT    = -c -nologo -Alfw -G2   -Gt64 -Od -W3 -Zi $(other_c_opt)

RDSPSRVR.EXE: RDSPSRVR.DEF \
              RDSPSRVR.OBJ \
              APPCUTIL.OBJ \
              EXECDISP.OBJ
   LINK @<<RDSPSRVR.LRF
RDSPSRVR.OBJ +
APPCUTIL.OBJ +
EXECDISP.OBJ
RDSPSRVR.EXE /NOD /MAP $(other_l_opt)
RDSPSRVR.MAP
OS2 LLIBCMT ACS
RDSPSRVR.DEF
<<

RDSPSRVR.OBJ: RDSPSRVR.C DISPLAY.H MSGID.H
   CL $(C_OPTIONS) RDSPSRVR.C

APPCUTIL.OBJ: APPCUTIL.C DISPLAY.H MSGID.H
   CL $(C_OPTIONS) APPCUTIL.C

EXECDISP.OBJ: EXECDISP.C DISPLAY.H MSGID.H
   CL $(C_OPTIONS) EXECDISP.C

