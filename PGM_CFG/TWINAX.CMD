/*  APPN API over a Twinaxial connection*/
/**********************************************************************/
/*    The features and records that must be configured to establish   */
/*    this connection are:                                            */
/*                                                                    */
/*    - MACHINE WKST                                                  */
/*    - SNA TDLC                                                      */
/*    - APPN DEFINE_LOCAL_CP                                          */
/*    - APPN DEFINE_LOGICAL_LINK                                      */
/*    - APPN DEFINE_LOCAL_LU                                          */
/*    - APPN DEFINE_PARTNER_LU                                        */
/*    - APPN DEFINE_PARTNER_LU_LOCATION                               */
/*    - APPN DEFINE_MODE                                              */
/*    - APPN DEFINE_DEFAULTS is automatically added                   */
/*    - APPN DEFINE_TP                                                */
/*    - APPN START_ATTACH_MANAGER is automatically added              */
/*    - APPN CNOS_FQ_PLU_NAME                                         */
/*    - APPN DEFINE_USER_ID_PASSWORD                                  */
/*                                                                    */
/*  DEPENDENCIES/LIMITATIONS:                                         */
/*                                                                    */
/*          There must be an application program installed on the     */
/*          workstation that interfaces with the APPC API.            */
/*                                                                    */
/* SETUP:                                                             */
/*   -  Hardware:       PC                                            */
/*                      IBM AS/400 or S/370 Host                      */
/*   -  PC Software:    OS/2 Standard Edition 1.30.1 or higher        */
/*                      Extended Services                             */
/*   -  Host Software:  AS/400 or S/370 operating system              */
/*   -  Files:          Model configuration files ACSCFGUS            */
/*                                                                    */
/* NOTE:    You will need to replace names and address to what        */
/*          is appropriate for your environment                       */
/*                                                                    */
/**********************************************************************/

/* Begin the job                                                      */

rc = JOB(BEGIN,TWINAX)

/* Create new local configuration                                     */

rc = FILE(ADD,TWINAX)

/* Modify the workstation profile.                                    */

rc = MACHINE(PUT,WKST,  COMMENT           EQ   'Workstation profile',,
                        MACHINE_TYPE      EQ   '8570',,
                        DISP_MSG_POPUP    EQ   'Y',,
                        START_3270_DFT    EQ   'N',,
                        START_3270_APPC   EQ   'N',,
                        START_ASYNC_TRM   EQ   'N',,
                        START_5250_WSF    EQ   'N',,
                        FIRST_SCREEN      EQ   '1',,
                        LOAD_SERVICES     EQ   '1')

/* Setup the SNA TDLC record                                          */

rc = SNA(ADD,TDLC, LOAD            EQ 'Y',,
                   MAX_RU_SIZE     EQ '256',,
                   CONTROL_ADDR    EQ '0',,
                   ADAPT_TYPE      EQ '1',,
                   5250_IO_PORT    EQ '71',,
                   5250_INT_LEVEL  EQ '5',,
                   5250_RAM_LOCA   EQ '1')

/* Setup all APPN records                                             */

rc = APPN(ADD,DEFINE_LOCAL_CP               EQ 'NY.THF825',,
              CP_ALIAS                      EQ 'THF285',,
              NAU_ADDRESS                   EQ 0,,
              NODE_TYPE                     EQ 2,,
              NODE_ID                       EQ F8232,,
              HOST_FP_SUPPORT               EQ 'N')

rc = APPN(ADD,DEFINE_LOGICAL_LINK           EQ LINK0001,,
              FQ_ADJACENT_CP_NAME           EQ 'NY.CP000002',,
              ADJACENT_NODE_TYPE            EQ 1,,
              DLC_NAME                      EQ 'TWINAX',,
              ADAPTER_NUMBER                EQ 0,,
              CP_CP_SESSION_SUPPORT         EQ 'N',,
              ACTIVATE_AT_STARTUP           EQ 'N',,
              LIMITED_RESOURCE              EQ '-1',,
              LINK_STATION_ROLE             EQ '-1',,
              SOLICIT_SSCP_SESSION          EQ 'Y',,
              EFFECTIVE_CAPACITY            EQ '-1',,
              COST_PER_CONNECT_TIME         EQ '-1',,
              COST_PER_BYTE                 EQ '-1',,
              SECURITY                      EQ '-1',,
              PROPAGATION_DELAY             EQ '-1',,
              USER_DEFINED_1                EQ '-1',,
              USER_DEFINED_2                EQ '-1',,
              USER_DEFINED_3                EQ '-1')

rc = APPN(ADD,DEFINE_LOCAL_LU               EQ THF82501,,
              LU_ALIAS                      EQ THF82501,,
              NAU_ADDRESS                   EQ 1)

rc = APPN(ADD,DEFINE_PARTNER_LU             EQ 'NY.TH2CICSA',,
              PARTNER_LU_ALIAS              EQ 'TH2CICSA',,
              PARTNER_LU_UNINTERPRETED_NAME EQ 'TH2CICSA',,
              MAX_MC_LL_SEND_SIZE           EQ 32767,,
              CONV_SECURITY_VERIFICATION    EQ 'Y',,
              PARALLEL_SESSION_SUPPORT      EQ 'N')

rc = APPN(ADD,DEFINE_PARTNER_LU_LOCATION    EQ 'NY.TH2CICSA',,
              WILDCARD_ENTRY                EQ 0,,
              FQ_OWNING_CP_NAME             EQ 'NY.CP000002',,
              LOCAL_NODE_NN_SERVER          EQ 'Y')

rc = APPN(ADD,DEFINE_MODE                   EQ LU62,,
              COS_NAME                      EQ '#CONNECT',,
              DEFAULT_RU_SIZE               EQ 'N',,
              MAX_RU_SIZE_UPPER_BOUND       EQ 256,,
              RECEIVE_PACING_WINDOW         EQ 8,,
              MAX_NEGOTIABLE_SESSION_LIMIT  EQ 32767,,
              PLU_MODE_SESSION_LIMIT        EQ 1,,
              MIN_CONWINNERS_SOURCE         EQ 0)

rc = APPN(ADD,DEFINE_TP                     EQ APPNPCTP,,
              FILESPEC                      EQ '\RJPROG\EDF.EXE',,
              CONVERSATION_TYPE             EQ 2,,
              CONV_SECURITY_RQD             EQ N,,
              SYNC_LEVEL                    EQ 2,,
              TP_OPERATION                  EQ 0,,
              PROGRAM_TYPE                  EQ 0,,
              INCOMING_ALLOCATE_QUEUE_DEPTH EQ 5,,
              INCOMING_ALLOCATE_TIMEOUT     EQ 28800,,
              RECEIVE_ALLOCATE_TIMEOUT      EQ 28800)

rc = APPN(ADD,CNOS_FQ_PLU_NAME              EQ 'THF82501,NY.TH2CICS,LU62',,
              SET_NEGOTIABLE                EQ 'N',,
              PLU_MODE_SESSION_LIMIT        EQ 1,,
              MIN_CONWINNERS_SOURCE         EQ 0,,
              MIN_CONWINNERS_TARGET         EQ 0,,
              AUTO_ACTIVATE                 EQ 0)

rc = APPN(ADD,DEFINE_USER_ID_PASSWORD       EQ U7337,,
              PASSWORD                      EQ 7337)

rc = FILE(VERIFY,TWINAX.CFG)

/* End exec and close log file. */

rc = JOB(END)

say 'TWINAX  PROCESSING COMPLETE'
