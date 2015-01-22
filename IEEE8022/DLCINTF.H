/*******************************************************************************
*   Module name :    DLCINTF                                                   *
*                                                                              *
*   Function :       Lan adapter 802.2 interface functions                     *
*                                                                              *
*   Routines in                                                                *
*   this module:     dlc_open_sap        (ccb_parm,adapternum,sap)             *
*                    dlc_open_station    (ccb_parm,adapternum,sapid,destaddr)  *
*                    dlc_reset           (ccb_parm,adapternum,stationid)       *
*                    dlc_transmit_I_fram (ccb_parm,adapternum,buffer1,bufflen1,*
*                                         buffer2,bufflen2,stationid)          *
*                    dlc_connect_station (ccb_parm,adapternum,stationid,       *
*                                         route_info)                          *
*                    dlc_status_read     (ccb_parm,adapternum,stationid)       *
*                                                                              *
* Parameters:                                                                  *
*                                                                              *
*         Input:     The adapter number. Other parms are supplied as necessary.*
*                                                                              *
*        Output:     Return code is based upon the kind of CCB executed.       *
*                    If the return code is 0xFF then the CCB is one that       *
*                    does not make the program wait while it runs, so the      *
*                    CCB command is valid, but incomplete when it returns.     *
*                    The return code for a successful "no-wait" CCB is zero.   *
*                    Otherwise the return code from the CCB is returned,       *
*                    indicating that an error was detected.                    *
*                                                                              *
*   OS/2 Required:    Extended Edition 1.1 or higher.                          *
*                                                                              *
*******************************************************************************/


/*-------------------------------------------------------------------+
³ dlc_open_sap                                                       ³
³                 Activate a SAP and reserve a number of link        ³
³                 stations for the SAP.                              ³
³                                                                    ³
+-------------------------------------------------------------------*/

byte dlc_open_sap (ccb_parm,adapternum,sap,sem,wait)
struct ccb_pt *ccb_parm;
byte adapternum;
byte  sap;
#ifdef E32TO16
HSYSSEM _Seg16 sem;
#else
HSYSSEM sem;
#endif
BOOL wait;
{
        struct command_control_block_dlr *ccbptr;
        struct dlc_open_sap_parms *pt;
        static byte sap_bpool[4096];

        ccbptr = &ccb_parm->ccb;
        pt     = &ccb_parm->pt.open_sap;

        memset(ccbptr,0,sizeof(struct command_control_block_dlr));
        memset(pt,0,sizeof(struct dlc_open_sap_parms));

        ccbptr->ccb_command     = LLC_DLC_OPEN_SAP;
        ccbptr->ccb_adapter     = adapternum;
#ifdef E32TO16
        ccbptr->parm_1.ccb_parm_offset = (address)pt;
#else
        ccbptr->parm_1.parm.off_12.ccb_parm_offset = FP_OFF (pt);
#endif
        ccbptr->ccb_pointer     = ZEROADDRESS;
        ccbptr->ccb_retcode     = 0xff;
        ccbptr->ccb_appl_key    = ApplKey;
        ccbptr->ccb_appl_id     = ApplId;
#ifdef E32TO16
        ccbptr->ccb_semaphore   = sem;
#else
        ccbptr->ccb_semaphore   = (dword)sem;
#endif

        pt->sap_value         = sap;
        pt->sap_station_count = 2;
        pt->options_priority  = 0x0C;
        pt->dlc_pool_address  = (address) sap_bpool;
        pt->dlc_pool_len      = 256;
        pt->status.dlc_status_flag   = (address)(0x0001);

        if(sem)
          DosSemSet(sem);
        pass_ccb (ccbptr);

        if(wait)
          {
          if(sem)
            DosSemWait(sem,-1L);
          else
            while (ccbptr->ccb_retcode == 0xff);
          }
        return ccbptr->ccb_retcode;
}


/*-------------------------------------------------------------------+
³ dlc_open_station                                                   ³
³                 Allocate resources for a link station.  The adapter³
³                 support interface performs functions to set up the ³
³                 the link station in the adapter, but no ring       ³
³                 communications takes place.  A dlc_connect_station ³
³                 command must be issued either to the local or      ³
³                 remote link station by its application program to  ³
³                 initiate ring communications. Thereafter, another  ³
³                 dlc_connect_station must be issued at the other    ³
³                 station to complete the establishment of the link. ³
³                                                                    ³
+-------------------------------------------------------------------*/

byte dlc_open_station (ccb_parm,adapternum,sapid,destaddr,destsap,sem,wait)
struct ccb_pt *ccb_parm;
byte adapternum;
word sapid,destsap;
byte destaddr[];
#ifdef E32TO16
HSYSSEM _Seg16 sem;
#else
HSYSSEM sem;
#endif
BOOL wait;
{
        struct command_control_block_dlr *ccbptr;
        struct dlc_open_station_parms *pt;

        ccbptr = &ccb_parm->ccb;
        pt     = &ccb_parm->pt.open_station;

        memset(ccbptr,0,sizeof(struct command_control_block_dlr));
        memset(pt,0,sizeof(struct dlc_open_station_parms));

        ccbptr->ccb_command     = LLC_DLC_OPEN_STATION;
        ccbptr->ccb_adapter     = adapternum;
#ifdef E32TO16
        ccbptr->parm_1.ccb_parm_offset = (address)pt;
#else
        ccbptr->parm_1.parm.off_12.ccb_parm_offset = FP_OFF (pt);
#endif
        ccbptr->ccb_pointer     = ZEROADDRESS;
        ccbptr->ccb_retcode     = 0xff;
        ccbptr->ccb_appl_key    = ApplKey;
        ccbptr->ccb_appl_id     = ApplId;
#ifdef E32TO16
        ccbptr->ccb_semaphore   = sem;
#else
        ccbptr->ccb_semaphore   = (dword)sem;
#endif

        pt->station_id         = sapid;
        pt->rsap_value         = destsap;
#ifdef E32TO16
        pt->destination_offset =  (address)destaddr;
#else
        pt->destination_offset = FP_OFF (destaddr);
#endif

        if(sem)
          DosSemSet(sem);

        pass_ccb (ccbptr);

        if(wait)
          {
          if(sem)
            DosSemWait(sem,-1L);
          else
            while (ccbptr->ccb_retcode == 0xff);
          }
        return ccbptr->ccb_retcode;
}


/*-------------------------------------------------------------------+
³ dlc_connect_station                                                ³
³                 Start or complete a SABME-UA exchange to place     ³
³                 both the local and remote link stations in a data  ³
³                 transfer state.  This will be the first time ring  ³
³                 activity takes place between two devices.          ³
³                                                                    ³
+-------------------------------------------------------------------*/

byte dlc_connect_station (ccb_parm,adapternum,stationid,route_info,sem,wait)
struct ccb_pt *ccb_parm;
byte adapternum;
word stationid;
byte *route_info[];
#ifdef E32TO16
HSYSSEM _Seg16 sem;
#else
HSYSSEM sem;
#endif
BOOL wait;
{
        struct command_control_block_dlr *ccbptr;
        struct dlc_connect_station_parms *pt;

        ccbptr = &ccb_parm->ccb;
        pt     = &ccb_parm->pt.connect;

        memset(ccbptr,0,sizeof(struct command_control_block_dlr));
        memset(pt,0,sizeof(struct dlc_connect_station_parms));

        ccbptr->ccb_command     = LLC_DLC_CONNECT_STATION;
        ccbptr->ccb_adapter     = adapternum;
#ifdef E32TO16
        ccbptr->parm_1.ccb_parm_offset = (address)pt;
#else
        ccbptr->parm_1.parm.off_12.ccb_parm_offset = FP_OFF (pt);
#endif
        ccbptr->ccb_pointer     = ZEROADDRESS;
        ccbptr->ccb_retcode     = 0xff;
        ccbptr->ccb_appl_key    = ApplKey;
        ccbptr->ccb_appl_id     = ApplId;
#ifdef E32TO16
        ccbptr->ccb_semaphore   = sem;
#else
        ccbptr->ccb_semaphore   = (dword)sem;
#endif

        pt->station_id = stationid;
#ifdef E32TO16
        pt->routing_offset  = (address)route_info;
#else
        pt->routing_offset  = FP_OFF (route_info);
#endif

        if(sem)
          DosSemSet(sem);
        pass_ccb (ccbptr);
        if(wait)
          {
          if(sem)
            DosSemWait(sem,-1L);
          else
            while (ccbptr->ccb_retcode == 0xff);
          }
        return ccbptr->ccb_retcode;
}


/*-------------------------------------------------------------------+
³ dlc_reset                                                          ³
³                 Resets either one SAP and all associated link      ³
³                 stations or all SAPs and all associated link       ³
³                 stations.  All outstanding commands will be        ³
³                 terminated for the SAP(s) and station(s).  All     ³
³                 communications will cease and all the related      ³
³                 resources will be released.  They must be reopened ³
³                 to be used again.                                  ³
³                                                                    ³
³                 A stationid of 0x0000 defines all SAPs & stations, ³
³                 and a stationid of 0xnn00 defines SAP 'nn' and all ³
³                 its stations.                                      ³
³                                                                    ³
+-------------------------------------------------------------------*/

byte dlc_reset (ccb_parm,adapternum,stationid,sem,wait)
struct ccb_pt *ccb_parm;
byte adapternum;
word  stationid;
#ifdef E32TO16
HSYSSEM _Seg16 sem;
#else
HSYSSEM sem;
#endif
BOOL wait;
{
        struct command_control_block_dlr *ccbptr;

        ccbptr = &ccb_parm->ccb;
        memset(ccbptr,0,sizeof(struct command_control_block_dlr));

        ccbptr->ccb_command   = LLC_DLC_RESET;
        ccbptr->ccb_adapter   = adapternum;
        ccbptr->ccb_pointer   = ZEROADDRESS;
        ccbptr->ccb_retcode   = 0xff;
        ccbptr->parm_1.parm.off_12.station_id    = stationid;
        if (stationid == 0x0000)
           ccbptr->parm_1.parm.off_14.reset_all_saps = 0x01;
        ccbptr->parm_2.system_key    = SystemKey;
        ccbptr->ccb_appl_key  = ApplKey;
        ccbptr->ccb_appl_id   = ApplId;
#ifdef E32TO16
        ccbptr->ccb_semaphore   = sem;
#else
        ccbptr->ccb_semaphore   = (dword)sem;
#endif

        if(sem)
          DosSemSet(sem);
        pass_ccb (ccbptr);
        if(wait)
          {
          if(sem)
          {
            DosSemWait(sem,-1L);
          }
          else
            while (ccbptr->ccb_retcode == 0xff);
          }
        return ccbptr->ccb_retcode;
}

/*-------------------------------------------------------------------+
³ dlc_transmit_I_frame                                               ³
³                 Transmits "information" data for a link station.   ³
³                 The adapter provides the LAN and DLC headers. Only ³
³                 actual data is provided by the caller.  The max    ³
³                 length of the data is 2042 bytes.                  ³
³                                                                    ³
+-------------------------------------------------------------------*/

byte dlc_transmit_I_frame (ccb_parm,adapternum,buffer1,bufflen1,buffer2,bufflen2,stationid,destsap,sem,wait)
struct ccb_pt *ccb_parm;
byte adapternum;
BUFFER *buffer1;
word bufflen1;
BUFFER *buffer2;
word bufflen2;
word stationid,destsap;
#ifdef E32TO16
HSYSSEM _Seg16 sem;
#else
HSYSSEM sem;
#endif
BOOL wait;
{
        struct command_control_block_dlr *ccbptr;
        struct transmit_parms *pt;

        ccbptr = &ccb_parm->ccb;
        pt     = &ccb_parm->pt.transmit;

        memset(ccbptr,0,sizeof(struct command_control_block_dlr));
        ccbptr->ccb_command     = LLC_TRANSMIT_I_FRAME;
        ccbptr->ccb_retcode     = 0xff;
        ccbptr->ccb_adapter     = adapternum;
#ifdef E32TO16
        ccbptr->parm_1.ccb_parm_offset = (address)pt;
#else
        ccbptr->parm_1.parm.off_12.ccb_parm_offset = FP_OFF (pt);
#endif
        ccbptr->ccb_pointer     = ZEROADDRESS;
        ccbptr->ccb_appl_key    = ApplKey;
        ccbptr->ccb_appl_id     = ApplId;
#ifdef E32TO16
        ccbptr->ccb_semaphore   = sem;
#else
        ccbptr->ccb_semaphore   = (dword)sem;
#endif

        memset(pt,0,sizeof(struct transmit_parms));

        pt->station_id     = stationid;
        pt->rsap           = destsap;
        pt->buffer_len_one = bufflen1;
        pt->buffer_len_two = bufflen2;
        pt->buffer_one     = (address) buffer1;
        pt->buffer_two     = (address) buffer2;

        if(sem)
          DosSemSet(sem);
        pass_ccb (ccbptr);
        if(wait)
          {
          if(sem)
            DosSemWait(sem,-1L);
          else
            while (ccbptr->ccb_retcode == 0xff);
          }
        return ccbptr->ccb_retcode;
}

/****************************************************************************
* dlc_status_read                                                           *
*    This function issues a dlc.read command to wait for dlc status change. *
*                                                                           *
*****************************************************************************/

byte dlc_status_read ( ccb_parm,adapternum, stationid ,sem,wait)
struct ccb_pt *ccb_parm;
byte adapternum;
word stationid;
#ifdef E32TO16
HSYSSEM _Seg16 sem;
#else
HSYSSEM sem;
#endif
BOOL wait;

{
        struct command_control_block_dlr *ccbptr;
        struct read_pt_status *pt;

        ccbptr = &ccb_parm->ccb;
        pt     = &ccb_parm->pt.read;
        memset(ccbptr,0,sizeof(struct command_control_block_dlr));

        ccbptr->ccb_command     = LLC_READ;
        ccbptr->ccb_retcode     = 0xff;
        ccbptr->ccb_adapter     = adapternum;
#ifdef E32TO16
        ccbptr->parm_1.ccb_parm_offset = (address)pt;
#else
        ccbptr->parm_1.parm.off_12.ccb_parm_offset = FP_OFF (pt);
#endif
        ccbptr->ccb_pointer     = ZEROADDRESS;
        ccbptr->ccb_appl_key    = ApplKey;
        ccbptr->ccb_appl_id     = ApplId;
#ifdef E32TO16
        ccbptr->ccb_semaphore   = sem;
#else
        ccbptr->ccb_semaphore   = (dword)sem;
#endif

        memset(pt,0,sizeof(struct read_pt_status));

        pt->station_id = stationid;
        pt->option_idicator = READ_SAP;
        pt->event_set = LLC_DLC_STATUS_CHANGE;

        if(sem)
          DosSemSet(sem);
        pass_ccb (ccbptr);
        if(wait)
          {
          if(sem)
            DosSemWait(sem,-1L);
          else
            while (ccbptr->ccb_retcode == 0xff);
          }
        return ccbptr->ccb_retcode;
}
