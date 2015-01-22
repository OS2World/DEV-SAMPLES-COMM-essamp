/*  ACDI API over an Asychronous Connection to a Remote Computer */
/**********************************************************************/
/*                                                                    */
/*  The features and records that must be configured to establish     */
/*  this connection are:                                              */
/*                                                                    */
/*  - MACHINE WKST                                                    */
/*  - ACDI STRING                                                     */
/*  - ACDI PORT                                                       */
/*                                                                    */
/*  Setup:                                                            */
/*        Hardware        :  PC                                       */
/*                           Remote computer                          */
/*        PC Software     :  OS/2 1.30.1 Extended Edition or higher   */
/*                           Extended Services                        */
/*        Files           :  Model configuration file ACSCFGUS        */
/*                                                                    */
/*                                                                    */
/* NOTES/ASSUMPTIONS:                                                 */
/*                                                                    */
/*  -  It is assumed that the modem connected to the workstation for  */
/*     this sample is one for which an IBM model exists as follows:   */
/*                                                                    */
/*     1)  M1 - for Hayes Smartmodem 1200,IBM PC 2400, ROLM 244PC     */
/*     2)  M2 - for Hayes Smartmodem 2400                             */
/*     3)  M3 - for Hayes V-Series Modem Enhancer 1200                */
/*     4)  M4 - for Hayes V-Series Modem Enhancer 2400                */
/*     5)  M5 - for MicroCom AX/2400C                                 */
/*     6)  M6 - for V.25bis Command Set Modems                        */
/*     7)  M7 - for IBM PC 1200                                       */
/*                                                                    */
/*  NOTE:  Replace names, addresses and any other record or           */
/*         field information to what is appropriate for your          */
/*         environment.                                               */
/*                                                                    */
/**********************************************************************/

/* Begin the job                                                       */

rc = JOB(BEGIN,ACDIPC)

/* Create a new configuration                                         */

rc = FILE(ADD,ACDI,MODEL EQ acscfgus)

/*  Setup the MACHINE record                                          */

rc = MACHINE(PUT,WKST,MACHINE_TYPE    EQ 8570,,
                      DISP_MSG_POPUP  EQ Y,,
                      START_3270_DFT  EQ N,,
                      START_3270_APPC EQ N,,
                      START_ASYNC_TRM EQ N,,
                      START_5250_WSF  EQ N,,
                      FIRST_SCREEN    EQ 1,,
                      LOAD_SERVICES   EQ 1)

/* Configure the STRING record                                        */

rc = ACDI(ADD,STRING           EQ AN002DEV,,
              MODEL            EQ M1,,
              NTF_CC           EQ 011,,
              COMMENT          EQ String profile for sample ACDI,,
              AUTO_DELAY       EQ 1,,
              ANSWER_DELAY     EQ 000,,
              CARRIER_TIMEOUT  EQ 060,,
              'DEV_INIT        EQ ATZ|ATE1Q0V1X1S7=60S0=01',,
              'PREFIX          EQ ATDT',,
              'SUFFIX          EQ |',,
              'AUTO_ANS        EQ ATS0=1|',,
              'HANGUP          EQ &tilde.&tilde.+++&tilde.&tilde.ATH0|',,
              'VOICE_TO_ORG    EQ ATD|',,
              'VOICE_TO_ANS    EQ ATA|',,
              'SUC_CMD_RESP    EQ OK|^J',,
              'UNSUC_CMD_RESP  EQ ERROR|^J',,
              'INCOMING_RESP   EQ RING|^J',,
              VALID_RESP       EQ CONNECT)

/* Due to a REXX limitation that only allows 20 arguments             */
/* to be passed to a called routine, the following PUT                */
/* is to assign the last 4 fields.                                    */

rc = ACDI(PUT,STRING        EQ AN002DEV,,
              CALL_TYPE1    EQ 1,,
              'RESP_STRING1 EQ BUSY|^J',,
              CALL_TYPE2    EQ 2,,
              'RESP_STRING2 EQ NO CARRIER|^J')

/*  Add PORT record named COM1 with specified values.                 */
/*  If the modem is not connected to the COM1 port the record name    */
/*  must be changed to COM2 or COM3 accordingly.                      */

rc = ACDI(ADD,PORT                EQ com1,,
              COMMENT             EQ Port profile for Sample ACDI,,
              NTF_CC              EQ 011,,
              DCE                 EQ 10,,
              STRING_PROFILE_NAME EQ AN002DEV)

/*  Verify the configuration file                                     */

rc = FILE(VERIFY,ACDI)

/*  End the job                                                       */

rc = JOB(END)

say 'ACDI PROCESSING COMPLETE'
