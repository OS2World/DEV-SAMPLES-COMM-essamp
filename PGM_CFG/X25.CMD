/* APPN over X.25 connection using 5250 workstation emulation                */
/*****************************************************************************/
/*                                                                           */
/*   The Profiles that must be configured to establish                       */
/*   this connection are:                                                    */
/*                                                                           */
/*      - Workstation Profile                                                */
/*      - Auto-start Options Profile                                         */
/*      - X.25 DLC Adapter Profile                                           */
/*      - APPN Local CP Profile                                              */
/*      - APPN Logical Link Profile                                          */
/*      - APPN Partner LU Profile                                            */
/*      - APPN Partner LU Location Profile                                   */
/*      - APPN Mode Profile                                                  */
/*      - APPN Defaults Profile                                              */
/*      - APPN CNOS PLU Alias Profile                                        */
/*      - X.25 Adapter Profile                                               */
/*      - X.25 PVC Profile                                                   */
/*      - X.25 Link Profile                                                  */
/*      - X.25 Remote Directory  Profile                                     */
/*      - X.25 Routing Table Profile                                         */
/*      - 5250 WSF Terminal Configuration Profile                            */
/*      - 5250 WSF Session Selection Profile                                 */
/*      - 5250 APPC LU Alias Profile                                         */
/*                                                                           */
/*                                                                           */
/* SETUP:                                                                    */
/*   o  Hardware:       PC                                                   */
/*                      IBM AS/400 or S/370 Host Computer                    */
/*   o  PC Software:    OS/2 1.3 Extended Edition or higher                  */
/*                      Extended Services                                    */
/*   o  Host Software:  OS/400 or S/370  depending on Host                   */
/*   o  Connectivity:   Lab Patch area must be configured for Host to        */
/*                      PC X.25 Connection                                   */
/*   o  Files:          Model Configuration Files  (ACSCFGUS.CFG)            */
/*                                                                           */
/*****************************************************************************/

rc = JOB(BEGIN, X25PC)

rc = FILE(ADD, X25)

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

rc = SNA(ADD,XDLC EQ XLINK,,
         COMMENT           EQ 'X25 SNA Link',,
         LOAD              EQ Y,,
         FREE_CONNECTS     EQ Y,,
         MAX_RU_SIZE       EQ 1024,,
         IN_CONNECTS       EQ 0,,
         CALL_ADDR_REQ     EQ Y,,
         NEGOT_PACK_SIZE   EQ N,,
         NEGOT_WIND_SIZE   EQ N,,
         DEF_LINK_STA_ROLE EQ 1,,
         DEF_RETRY_COUNT   EQ 5,,
         DEF_TIMEOUT       EQ 200,,
         REVERSE_CHARGES   EQ N)

rc = APPN(ADD,DEFINE_LOCAL_CP              EQ 'USIBMTH.X25ST02',,
              CP_ALIAS                     EQ 'X25ST02',,
              NAU_ADDRESS                  EQ 0,,
              NODE_TYPE                    EQ 3,,
              NODE_ID                      EQ '00000',,
              HOST_FP_LINK_NAME            EQ 'LINK0001',,
              HOST_FP_SUPPORT              EQ 'Y')

rc = APPN(ADD,DEFINE_LOGICAL_LINK          EQ LINK0001,,
              FQ_ADJACENT_CP_NAME          EQ 'USIBMTH.AUSHATT',,
              ADJACENT_NODE_TYPE           EQ 1,,
              DLC_NAME                     EQ 'X25DLC',,
              X25_DIRECTORY_ENTRY          EQ 'XDIR',,
              CP_CP_SESSION_SUPPORT        EQ 'N',,
              ACTIVATE_AT_STARTUP          EQ 'Y',,
              LIMITED_RESOURCE             EQ '-1',,
              LINK_STATION_ROLE            EQ '-1',,
              SOLICIT_SSCP_SESSION         EQ 'Y',,
              EFFECTIVE_CAPACITY           EQ '-1',,
              COST_PER_CONNECT_TIME        EQ '-1',,
              COST_PER_BYTE                EQ '-1',,
              SECURITY                     EQ '-1',,
              PROPAGATION_DELAY            EQ '-1',,
              USER_DEFINED_1               EQ '-1',,
              USER_DEFINED_2               EQ '-1',,
              USER_DEFINED_3               EQ '-1')

rc = APPN(ADD,DEFINE_PARTNER_LU EQ 'USIBMTH.AUSHATT',,
              PARTNER_LU_ALIAS             EQ 'AUSHATT',,
              MAX_MC_LL_SEND_SIZE          EQ 32767,,
              CONV_SECURITY_VERIFICATION   EQ 'N',,
              PARALLEL_SESSION_SUPPORT     EQ 'Y')

rc = APPN(ADD,DEFINE_PARTNER_LU_LOCATION EQ 'USIBMTH.AUSHATT',,
              WILDCARD_ENTRY               EQ 0,,
              FQ_OWNING_CP_NAME            EQ 'USIBMTH.AUSHATT',,
              LOCAL_NODE_NN_SERVER         EQ 'N')

rc = APPN(ADD,DEFINE_MODE EQ QPCSUPP,,
              COS_NAME                     EQ '#CONNECT',,
              DEFAULT_RU_SIZE              EQ 'Y',,
              RECEIVE_PACING_WINDOW        EQ 7,,
              MAX_NEGOTIABLE_SESSION_LIMIT EQ 32767,,
              PLU_MODE_SESSION_LIMIT       EQ 32,,
              MIN_CONWINNERS_SOURCE        EQ 0)

rc = APPN(PUT,DEFINE_DEFAULTS,,
              IMPLICIT_INBOUND_PLU_SUPPORT EQ 'Y',,
              DIRECTORY_FOR_INBOUND_ATTACHES EQ '*',,
              DEFAULT_MODE_NAME            EQ 'BLANK',,
              MAX_MC_LL_SEND_SIZE          EQ 32767,,
              DEFAULT_TP_OPERATION         EQ 2,,
              DEFAULT_TP_PROGRAM_TYPE      EQ 0,,
              DEFAULT_TP_CONV_SECURITY_RQD EQ 'N',,
              MAX_HELD_ALERTS              EQ 10)

rc =  5250(ADD, TERM EQ T1,MODEL EQ M1,,
          COMMENT EQ '5250 WSF Terminal Default Profile  (Copy of M1)',,
          AT_KB_NAME EQ ACS5APUS,,
          ENH_KB_NAME EQ ACS5ENUS,,
          BUFF_KEYS EQ Y,,
          HOST_TYPE EQ 2,,
          AS400_WS_ID EQ ' ',,
          SYS36_WS_ID EQ ' ',,
          DBCS_USER_CHAR EQ 1,,
          DBCS_LANG_SEL EQ 1,,
          AUTO_SIGNON EQ N,,
          SEP_FOR_BLNK EQ 1)

rc = 5250(PUT, SESSION, SESS_LU_ALIAS   EQ X25ST02,,
          SESS1_PROF_NAME EQ T1,,
          SESS1_PLU_ALIAS EQ AUSHATT,,
          SESS1_MODE_NAME EQ QPCSUPP,,
          SESS1_SHORT_ID  EQ 'A')
          /*SESS2_PROF_NAME EQ TERM2,,*/
          /*SESS2_PLU_ALIAS EQ LURECV1,,*/
          /*SESS2_MODE_NAME EQ MODE1,,*/
          /*SESS2_SHORT_ID  EQ 'B')*/
          /*SESS3_PROF_NAME EQ '',,*/  /*Only Two Sessions are Configured*/
          /*SESS3_PLU_ALIAS EQ '',,*/  /*Should More Sessions Be Needed, */
          /*SESS3_MODE_NAME EQ '',,*/  /*Configure The List To The Left  */
          /*SESS4_PROF_NAME EQ '',,*/
          /*SESS4_PLU_ALIAS EQ '',,*/
          /*SESS4_MODE_NAME EQ '',,*/
          /*SESS5_PROF_NAME EQ '',,*/
          /*SESS5_PLU_ALIAS EQ '',,*/
          /*SESS5_MODE_NAME EQ '')*/

rc = X25(ADD, ADAPTER EQ XADAPT,,
         COMMENT EQ 'X.25 adapter slot number',,
         ADPT_SLOT_NUM EQ 6)

rc = X25(ADD, PVC EQ XPVC,,
         COMMENT EQ 'X.25 PVC information',,
         LINK_NAME EQ XLINK,,
         LOGICAL_CHAN_NUM EQ 7,,
         SNA_CIRCUIT EQ Y,,
         INCOM_PK_SIZE EQ 128,,
         OUTGO_PK_SIZE EQ 128,,
         INCOM_WND_SIZE EQ 7,,
         OUTGO_WND_SIZE EQ 7,,
         SNA_RMT_DTE_TYPE EQ 1)

rc = X25(ADD, LINK EQ XLINK,,
         COMMENT EQ 'X.25 link information',,
         ADAPTER_NAME EQ XADAPT,,
         NETWORK_TYPE EQ 1,,
         DTE_ADDR EQ 8000,,
         OPERATION EQ 1,,
         CCITT_COMP EQ 1,,
         INIT_LK_MODE EQ 2,,
         DISCON_DELAY_TOUT EQ 0,,
         LK_SET_UP_MODE EQ 2,,
         AUTO_CALL_UNIT EQ N,,
         CON_TIMEOUT EQ 120,,
         LOW_PVC_NUM EQ 7,,
         NUM_PVC EQ 1,,
         LOW_INONLY_SVC_NUM EQ 25,,
         NUM_INONLY_SVC EQ 0,,
         LOW_2WAY_SVC_NUM EQ 9,,
         NUM_2WAY_SVC EQ 12,,
         LOW_OUTONLY_SVC_NUM EQ 25)

rc = X25(PUT,LINK EQ XLINK,NUM_OUTONLY_SVC EQ 0,,
         FRAME_SEQ_MOD EQ 1)

rc = X25(PUT,LINK EQ XLINK,WINDOW_SIZE EQ 7,,
         RETRY_TM_OUT EQ 3000,,
         FRAME_RETRY_CT EQ 20,,
         INOP_TM_OUT EQ 30,,
         RSTRT_REQ_TOUT EQ 180,,
         CALL_REQ_TOUT EQ 200,,
         RESET_REQ_TOUT EQ 180,,
         CLEAR_REQ_TOUT EQ 180,,
         ACK_TOUT EQ 180,,
         DATA_PK_TOUT EQ 180,,
         INTRUPT_PK_TOUT EQ 180,,
         RESET_REQ_RETRY EQ 5,,
         CLEAR_REQ_RETRY EQ 5,,
         DFALT_INCOM_PKT EQ 128,,
         MAX_INCOM_PKT EQ 128,,
         DFALT_OUTGO_PKT EQ 128,,
         MAX_OUTGO_PKT EQ 128,,
         DFALT_INCOM_WND EQ 7)

rc = X25(PUT,LINK EQ XLINK,MAX_OUTGO_WND EQ 7,,
         MAX_INCOM_WND EQ 7,,
         DFALT_OUTGO_WND EQ 7,,
         PK_SEQ_NUM_MOD EQ 1,,
         PVC_DFALT_IN_PKT EQ 128,,
         PVC_DFALT_OUT_PKT EQ 128,,
         PVC_DFALT_IN_WND EQ 7,,
         PVC_DFALT_OUT_WND EQ 7)

rc = X25(ADD, RDIR EQ XDIR, MODEL EQ M3,,
         COMMENT EQ 'X.25 Remote Directory Entry (PVC) - SNA',,
         LINK_NAME EQ XLINK,,
         PVC_NAME EQ XPVC,,
         RETRY_CT EQ 5,,
         TM_OUT EQ 200,,
         LINK_STN_ROLE EQ 1)
       /*REQ_REV_CHRG EQ '',,*/       /* Move the relevent lines from this */
       /*ACPT_REV_CHRG EQ '',,*/      /* list into the above profile and   */
       /*RETRY_CALL_COL EQ '',,*/     /* assign values to configure another*/
       /*SVC_REMOTE_ADDR EQ '',,*/    /* type of RDIR.                     */
       /*SVC_REMOTE_ADDR_EXT EQ ''*/
       /*RMT_DTE_TYP EQ '',,*/
       /*REMOTE_ADDR EQ '',,*/

rc = X25(ADD, ROUTE EQ M8COPY, MODEL EQ M8,,
         COMMENT EQ 'Copy of Routing Table Profile M8 (SNA)',,
         LINK_NAME EQ '*',,
         RTBL_ENTRY_TYPE EQ 1,,
         CALL_USER_DATA EQ 'C3',,
         CALLED_ADDR EQ '*',,
         CALLING_ADDR EQ '*',,
         CALLED_ADDR_EXT EQ '*',,
         CALLING_ADDR_EXT EQ '*')

rc = FILE(VERIFY,X25)

say 'X25PC PROCESSING COMPLETE'

rc = JOB(END, X25PC)
