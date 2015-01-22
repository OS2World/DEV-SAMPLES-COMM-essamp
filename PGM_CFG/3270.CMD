/* 3270 DFT connection via a 3278/79 or 3270 adapter                    */
/************************************************************************/
/*                                                                      */
/*       The following profiles need to be configured in order          */
/*       to run 3270 emulation over DFT.                                */
/*                                                                      */
/*         Workstation Profile                                          */
/*         Auto-Start Options                                           */
/*         3270 DFT Terminal Emulation Profile                          */
/*                                                                      */
/* CONFIGURATION:                                                       */
/*       ______                                     ------------        */
/*      | OS/2 |                                   |            |       */
/*      |______|                                   |   S/370    |       */
/*    ___|____|___         Coax Cable              |   Host     |       */
/*   |____________|--------------------------------|            |       */
/*                                                 |            |       */
/*                                                  ------------        */
/*                                                                      */
/* SETUP:                                                               */
/*   o  Hardware:       PC (with 3270 Card)                             */
/*                      S/370 Host                                      */
/*   o  PC Software:    OS/2 1.3 Extended Edition                       */
/*                      Extended Services                               */
/*   o  Host Software:  VTAM                                            */
/*   o  Connectivity:   Coaxial Cable                                   */
/*   o  Files:          ACSCFG.CFG Model Configuration File - this      */
/*                        file has the keyboard profiles needed.        */
/*                        The ACSCFGUS.CFG DOES NOT have them.          */
/*                                                                      */
/************************************************************************/

/* Begin exec and create log file. */
rc = JOB(BEGIN, 3270pc)

/* Create configuration file for testcase. */
rc = FILE(ADD, 3270, MODEL EQ acscfg)

/* Add the 3270 Session profile to DEFAULTS. */
rc = 3270DFT(ADD, SESSION EQ 1, SESN_ID EQ A,,
                                TYPE EQ 1,,
                                COMMENT EQ 3270DFT emul.,,
                                SHORT_ID EQ A,,
                                AT_KEYBOARD EQ ACS3ATUS,,
                                ENH_KEYBOARD EQ ACS3ENUS,,
                                SPACE_SIZE EQ 5,,
                                PSS_ROW EQ 25,,
                                PSS_COL EQ 80,,
                                XFER_SIZE EQ 0,,
                                UNSUP_CC EQ 1,,
                                APS_PRINT EQ Y,,
                                DBCS_NUML EQ 1,,
                                DBCS_LANG EQ 1,,
                                PRT_BUFF EQ 1920)

/* Create a connect record. */
rc = 3270DFT(ADD, CONNECT, ADAPT_TYPE EQ 3)

/* Update the foreground colors. */
rc = 3270DFT(PUT, SESSION EQ 1, F_NORMAL_UNPRO EQ 3,,
                                F_INTENSE_UNPRO EQ 5,,
                                F_NORMAL_PRO EQ 4,,
                                F_INTENSE_PRO EQ 8,,
                                F_BLUE EQ 4,,
                                F_GREEN EQ 3,,
                                F_PINK EQ 14,,
                                F_RED EQ 5,,
                                F_TURQUOISE EQ 12,,
                                F_WHITE EQ 8,,
                                F_YELLOW EQ 15,,
                                F_BLINK EQ 1,,
                                F_DEFLT_HILITE EQ 8,,
                                F_DEFLT_NOHILT EQ 3,,
                                F_OIA EQ 1)

/* Update the background colors and alarm info. */
rc = 3270DFT(PUT, SESSION EQ 1, B_NORMAL_UNPRO EQ 1,,
                                B_INTENSE_UNPRO EQ 1,,
                                B_NORMAL_PRO EQ 1,,
                                B_INTENSE_PRO EQ 1,,
                                B_BLUE EQ 1,,
                                B_GREEN EQ 1,,
                                B_PINK EQ 1,,
                                B_RED EQ 1,,
                                B_TURQUOISE EQ 1,,
                                B_WHITE EQ 1,,
                                B_YELLOW EQ 1,,
                                B_BLINK EQ 8,,
                                B_DEFLT_HILITE EQ 1,,
                                B_DEFLT_NOHILT EQ 1,,
                                B_OIA EQ 3,,
                                HOST_ALARM EQ 1,,
                                PF_ALARM EQ 1,,
                                SU_ALARM EQ 1)

/* Add autostart options to the workstation profile. */
rc = MACHINE(PUT, WKST, COMMENT EQ 3270.CFG File,,
                        QUICK_START EQ Y,,
                        START_3270_DFT EQ Y,,
                        START_ALL_DFT_SESS EQ Y,,
                        FIRST_SCREEN EQ 2)


/* Verify the current configuration file. */
rc = FILE(VERIFY)

/* End exec and close log file. */
rc = JOB(END, 3270pc)
