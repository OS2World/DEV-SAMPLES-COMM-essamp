/*  APPN API Connection using 5250 WorkStation Emulation */
/**********************************************************************/
/*                                                                    */
/*      The features and records that must be configured to           */
/*      establish this connection are:                                */
/*                                                                    */
/*      - MACHINE WSKT                                                */
/*      - SNA TDLC                                                    */
/*      - APPN DEFINE_LOGICAL_LINKS                                   */
/*      - APPN DEFINE_LOCAL_CP                                        */
/*      - APPN DEFINE_PARTNER_LU                                      */
/*      - APPN DEFINE_PARTNER_LU_LOCATION                             */
/*      - APPN DEFINE_MODE                                            */
/*      - APPN DEFINE_DEFAULTS is automatically added                 */
/*      - APPN CNOS_FQ_PLU_NAME                                       */
/*      - 5250 TERM                                                   */
/*      - 5250 PRT                                                    */
/*      - 5250 SESSION                                                */
/*                                                                    */
/*  DEPENDENCIES/LIMITATIONS:                                         */
/*                                                                    */
/*          There must be an application program installed on the     */
/*          workstation that interfaces with the APPN API.            */
/*                                                                    */
/* SETUP:                                                             */
/*   -  Hardware:       PC                                            */
/*                      IBM AS/400                                    */
/*   -  PC Software:    OS/2 1.30.1 Extended Edition or higher        */
/*                      Extended Services                             */
/*   -  Host Software:  OS/400                                        */
/*   -  Connectivity:   Twinaxial interface card to AS/400            */
/*   -  Files:          Model configuration file ACSCFGUS             */
/*                                                                    */
/*  NOTE:  Replace names, addresses and any other record or           */
/*         field information to what is appropriate for your          */
/*         environment.                                               */
/*                                                                    */
/**********************************************************************/

/*  Begin the job                                                     */

rc = JOB(BEGIN,5250EM)

/* Create a new configuration                                         */

rc = FILE(ADD,5250EM)

/* Setup the MACHINE record                                           */

rc = MACHINE(PUT,WKST,,
                 COMMENT          EQ  'Workstation profile',,
                 MACHINE_TYPE     EQ  8570,,
                 DISP_MSG_POPUP   EQ  Y,,
                 START_3270_DFT   EQ  N,,
                 START_3270_APPC  EQ  N,,
                 START_ASYNC_TRM  EQ  N,,
                 START_5250_WSF   EQ  Y,,
                 FIRST_SCREEN     EQ  1,,
                 LOAD_SERVICES    EQ  1)

/* Configure the SNA Twinaxial DCL                                    */

rc = SNA(ADD,TDLC,,
             LOAD            EQ Y,,
             MAX_RU_SIZE     EQ 256,,
             CONTROL_ADDR    EQ 0,,
             ADAPT_TYPE      EQ 2,,
             5250_IO_PORT    EQ 71,,
             5250_INT_LEVEL  EQ 5,,
             5250_RAM_LOCA   EQ 1)

/* Configure APPN                                                     */

rc = APPN(ADD,DEFINE_LOCAL_CP              EQ 'APPN.LUSEND1',,
              CP_ALIAS                     EQ LUSEND1,,
              NAU_ADDRESS                  EQ 0,,
              NODE_TYPE                    EQ 2,,
              NODE_ID                      EQ 00001,,
              HOST_FP_SUPPORT              EQ N)

rc = APPN(ADD,DEFINE_LOGICAL_LINK          EQ LINK0001,,
              FQ_ADJACENT_CP_NAME          EQ 'APPN.CP000002',,
              ADJACENT_NODE_TYPE           EQ 1,,
              DLC_NAME                     EQ TWINAX,,
              CP_CP_SESSION_SUPPORT        EQ N,,
              ACTIVATE_AT_STARTUP          EQ N,,
              LIMITED_RESOURCE             EQ N,,
              LINK_STATION_ROLE            EQ '-1',,
              SOLICIT_SSCP_SESSION         EQ N,,
              EFFECTIVE_CAPACITY           EQ '-1',,
              COST_PER_CONNECT_TIME        EQ '-1',,
              COST_PER_BYTE                EQ '-1',,
              SECURITY                     EQ '-1',,
              PROPAGATION_DELAY            EQ '-1',,
              USER_DEFINED_1               EQ '-1',,
              USER_DEFINED_2               EQ '-1',,
              USER_DEFINED_3               EQ '-1')

rc = APPN(ADD,DEFINE_PARTNER_LU            EQ LURECV1,,
              PARTNER_LU_ALIAS             EQ LURECV1,,
              MAX_MC_LL_SEND_SIZE          EQ 32767,,
              CONV_SECURITY_VERIFICATION   EQ N,,
              PARALLEL_SESSION_SUPPORT     EQ N)

rc = APPN(ADD,DEFINE_PARTNER_LU_LOCATION   EQ LURECV1,,
              WILDCARD_ENTRY               EQ 0,,
              FQ_OWNING_CP_NAME            EQ 'APPN.CP000002',,
              LOCAL_NODE_NN_SERVER         EQ Y)

rc = APPN(ADD,DEFINE_MODE                  EQ MODE1,,
              COS_NAME                     EQ '#CONNECT',,
              DEFAULT_RU_SIZE              EQ N,,
              MAX_RU_SIZE_UPPER_BOUND      EQ 1024,,
              RECEIVE_PACING_WINDOW        EQ 8,,
              MAX_NEGOTIABLE_SESSION_LIMIT EQ 32767,,
              PLU_MODE_SESSION_LIMIT       EQ 1,,
              MIN_CONWINNERS_SOURCE        EQ 0)

rc = APPN(ADD,CNOS_FQ_PLU_NAME             EQ 'LUSEND1,LURECV1,MODE1',,
              SET_NEGOTIABLE               EQ N,,
              PLU_MODE_SESSION_LIMIT       EQ 1,,
              MIN_CONWINNERS_SOURCE        EQ 0,,
              MIN_CONWINNERS_TARGET        EQ 0,,
              AUTO_ACTIVATE                EQ 0)

/* Configure the 5250 records                                         */

rc = 5250(ADD,TERM           EQ TERM1,,
              MODEL          EQ M1,,
              COMMENT        EQ '5250 WSF Terminal Profile',,
              AT_KB_NAME     EQ ACS5APUS,,
              ENH_KB_NAME    EQ ACS5ENUS,,
              BUFF_KEYS      EQ Y,,
              HOST_TYPE      EQ 2,,
              AS400_WS_ID    EQ A002001 ,,
              DBCS_USER_CHAR EQ 2,,
              AUTO_SIGNON    EQ N,,
              SEP_FOR_BLNK   EQ 1)

rc = 5250(ADD,TERM           EQ TERM2,,
              MODEL          EQ M1,,
              COMMENT        EQ '5250 WSF Terminal Profile',,
              AT_KB_NAME     EQ ACS5APUS,,
              ENH_KB_NAME    EQ ACS5ENUS,,
              BUFF_KEYS      EQ Y,,
              HOST_TYPE      EQ 2,,
              AS400_WS_ID    EQ A002002,,
              DBCS_USER_CHAR EQ 2,,
              AUTO_SIGNON    EQ N,,
              SEP_FOR_BLNK   EQ 1)

rc = 5250(ADD,PRT            EQ M2COPY,,
              MODEL          EQ M2,,
              COMMENT        EQ '5250 WSF 5256 Printer Profile',,
              AT_KB_NAME     EQ ACS5APUS,,
              ENH_KB_NAME    EQ ACS5ENUS,,
              PRINT_TYPE     EQ 1,,
              PFT_FILE_NAME8 EQ PTAB2,,
              HOST_TYPE      EQ 2,,
              AS400_WS_ID    EQ A002002,,
              NET_CONV       EQ 1,,
              EBCDIC_PAGE    EQ 037,,
              ASCII_PAGE     EQ 437,,
              DBCS_USER_CHAR EQ 2,,
              SER_PAR        EQ 2,,
              SER_PORT       EQ 1,,
              BPS            EQ 2,,
              DATA_BITS      EQ 1,,
              STOP_BITS      EQ 1)

rc = 5250(PUT,PRT         EQ M2COPY,,
              PARITY      EQ 1,,
              PAR_PORT    EQ 1,,
              INIT_STR.1  EQ 00,,
              INIT_STR.2  EQ 00,,
              INIT_STR.3  EQ 00,,
              INIT_STR.4  EQ 00,,
              INIT_STR.5  EQ 00,,
              INIT_STR.6  EQ 00,,
              INIT_STR.7  EQ 00,,
              INIT_STR.8  EQ 00,,
              INIT_STR.9  EQ 00,,
              INIT_STR.10 EQ 00,,
              INIT_STR.11 EQ 00,,
              INIT_STR.12 EQ 00,,
              INIT_STR.13 EQ 00,,
              INIT_STR.14 EQ 00,,
              INIT_STR.15 EQ 00,,
              INIT_STR.16 EQ 00)

rc = 5250(PUT,PRT         EQ M2COPY,,
              INIT_STR.17 EQ 00,,
              INIT_STR.18 EQ 00,,
              INIT_STR.19 EQ 00,,
              INIT_STR.20 EQ 00,,
              INIT_STR.21 EQ 00,,
              INIT_STR.22 EQ 00,,
              INIT_STR.23 EQ 00,,
              INIT_STR.24 EQ 00,,
              INIT_STR.25 EQ 00,,
              INIT_STR.30 EQ 00,,
              INIT_STR.31 EQ 00,,
              INIT_STR.32 EQ 00,,
              INIT_STR.33 EQ 00,,
              INIT_STR.34 EQ 00,,
              INIT_STR.35 EQ 00,,
              INIT_STR.36 EQ 00)

rc = 5250(PUT,PRT         EQ M2COPY,,
              INIT_STR.37 EQ 00,,
              INIT_STR.38 EQ 00,,
              INIT_STR.39 EQ 00,,
              INIT_STR.40 EQ 00,,
              INIT_STR.41 EQ 00,,
              INIT_STR.42 EQ 00,,
              INIT_STR.43 EQ 00,,
              INIT_STR.44 EQ 00,,
              INIT_STR.45 EQ 00,,
              INIT_STR.46 EQ 00,,
              INIT_STR.47 EQ 00,,
              INIT_STR.48 EQ 00,,
              INIT_STR.49 EQ 00,,
              INIT_STR.50 EQ 00,,
              INIT_STR.51 EQ 00,,
              INIT_STR.52 EQ 00)

rc = 5250(PUT,PRT         EQ M2COPY,,
              INIT_STR.53 EQ 00,,
              INIT_STR.54 EQ 00,,
              INIT_STR.55 EQ 00,,
              INIT_STR.56 EQ 00,,
              INIT_STR.57 EQ 00,,
              INIT_STR.58 EQ 00,,
              INIT_STR.59 EQ 00,,
              INIT_STR.60 EQ 00,,
              INIT_STR.61 EQ 00,,
              INIT_STR.62 EQ 00,,
              INIT_STR.63 EQ 00,,
              INIT_STR.64 EQ 00,,
              INIT_STR.26 EQ 00,,
              INIT_STR.27 EQ 00,,
              INIT_STR.28 EQ 00,,
              INIT_STR.29 EQ 00)

rc = 5250(PUT,PRT         EQ M2COPY,,
              SUBCON_NUM  EQ 1,,
              SEP_PAGES   EQ 0,,
              NAME        EQ LANPRT1,,
              MSG_QUEUE   EQ QSYSOPR,,
              MSG_Q_LIB   EQ '*LIBL',,
              ALARM       EQ 1,,
              HAND_EQUIP  EQ 2,,
              PAPER_WID   EQ 1,,
              CPI         EQ 1,,
              LPI         EQ 1,,
              NUM_DRAWERS EQ 0,,
              ENV_HOP     EQ N,,
              FONT        EQ 1)

rc = 5250(PUT,SESSION,,
              SESS_LU_ALIAS   EQ LUSEND1,,
              SESS1_PROF_NAME EQ TERM1,,
              SESS1_PLU_ALIAS EQ LURECV1,,
              SESS1_MODE_NAME EQ MODE1,,
              SESS1_SHORT_ID  EQ A,,
              SESS2_PROF_NAME EQ TERM2,,
              SESS2_PLU_ALIAS EQ LURECV1,,
              SESS2_MODE_NAME EQ MODE1,,
              SESS2_SHORT_ID  EQ B)

/* Verify the configuration                                           */

rc = FILE(VERIFY,5250EM)

/* End the job                                                        */

rc = JOB(END,5250EM)

say '5250EM PROCESSING COMPLETE'

