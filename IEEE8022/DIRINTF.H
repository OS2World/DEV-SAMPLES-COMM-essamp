/******************************************************************************
*   Module name :    DIRINTF                                                  *
*                                                                             *
*   Function :       Lan adapter direct interface functions                   *
*                                                                             *
*   Routines in                                                               *
*   this module:      dir_status      (adapternum)                            *
*                     dir_initialize  (adapternum)                            *
*                     dir_openadapter (adapternum)                            *
*                     receive         (adapternum,station)                    *
*                                                                             *
* Parameters:                                                                 *
*                                                                             *
*         Input:     The adapter number. Other parms are supplied as necessary*
*                                                                             *
*        Output:     Return code is based upon the kind of CCB executed.      *
*                    If the return code is 0xFF then the CCB is one that      *
*                    does not make the program wait while it runs, so the     *
*                    CCB command is valid, but incomplete when it returns.    *
*                    The return code for a successful "no-wait" CCB is zero.  *
*                    Otherwise the return code from the CCB is returned,      *
*                    indicating that an error was detected.                   *
*                                                                             *
*   OS/2 Required:   Extended Edition 1.1 or higher.                          *
*                                                                             *
*******************************************************************************/

/* DIR.STATUS routine */

byte dir_status (ccb_parm,adapternum,sem,wait)
struct ccb_pt *ccb_parm;
byte adapternum;
#ifdef E32TO16
HSYSSEM _Seg16 sem;
#else
HSYSSEM sem;
#endif
BOOL wait;
{
        struct command_control_block_dlr *status_ccb;
        struct dir_status_parms *pt;

        status_ccb = &ccb_parm->ccb;
        pt = &ccb_parm->pt.status;
        memset (status_ccb,0,sizeof(struct command_control_block_dlr));

        status_ccb->ccb_command     = LLC_DIR_STATUS;
        status_ccb->ccb_adapter     = adapternum;
        status_ccb->ccb_retcode     = 0xff;
#ifdef E32TO16
        status_ccb->parm_1.ccb_parm_offset = (address)pt;
#else
        status_ccb->parm_1.parm.off_12.ccb_parm_offset = FP_OFF (pt);
#endif
        status_ccb->ccb_pointer     = ZEROADDRESS;
        status_ccb->ccb_appl_key    = ApplKey;
        status_ccb->ccb_appl_id     = ApplId;
#ifdef E32TO16
        status_ccb->ccb_semaphore   = sem;
#else
        status_ccb->ccb_semaphore   = (dword)sem;
#endif

        memset(pt,0,sizeof(struct dir_status_parms));

        if(sem)
          DosSemSet(sem);
        pass_ccb (status_ccb);
        if(wait)
          {
          if(sem)
            DosSemWait(sem,-1L);
          else
            while (status_ccb->ccb_retcode == 0xff);
          }
        return status_ccb->ccb_retcode;
}

/*-------------------------------------------------------------------+
³ dir_initialize: Initializes the adapter support interface areas    ³
³                 in the PC, resets all adapter tables and buffers,  ³
³                 and directs the adapter to run the bring-up tests. ³
³                                                                    ³
³                 Note: This command completely resets the adapter,  ³
³                       so it should not be used if other programs   ³
³                       could have pending commands in the adapter.  ³
³                                                                    ³
+--------------------------------------------------------------------*/

byte dir_initialize (ccb_parm,adapternum,sem,wait)
struct ccb_pt *ccb_parm;
byte adapternum;
#ifdef E32TO16
HSYSSEM _Seg16 sem;
#else
HSYSSEM sem;
#endif
BOOL wait;
{
char c;
        struct command_control_block_dlr *ccb;
        struct dir_initialize_parms *pt;

        ccb = &ccb_parm->ccb;
        pt = &ccb_parm->pt.init;
        memset(ccb,0,sizeof(struct command_control_block_dlr));

        ccb->ccb_command     = LLC_DIR_INITIALIZE;
        ccb->ccb_adapter     = adapternum;
        ccb->ccb_retcode     = 0xFF;
#ifdef E32TO16
        ccb->parm_1.ccb_parm_offset = (address)pt;
#else
        ccb->parm_1.parm.off_12.ccb_parm_offset = FP_OFF (pt);
#endif
        ccb->ccb_pointer     = ZEROADDRESS;
        ccb->ccb_appl_key    = ApplKey;
        ccb->parm_2.system_key      = SystemKey;
#ifdef E32TO16
        ccb->ccb_semaphore   = sem;
#else
        ccb->ccb_semaphore   = (dword)sem;
#endif

        memset(pt,0,sizeof(struct dir_initialize_parms));
        if(sem)
          DosSemSet(sem);
        pass_ccb (ccb);
        if(wait)
          {
          if(sem)
            DosSemWait(sem,-1L);
          else
            while (ccb->ccb_retcode == 0xff);
          }
        return ccb->ccb_retcode;
}


/*-------------------------------------------------------------------+
³ dir_openadapter: This command makes the adapter ready for normal   ³
³                  ring communication.  All buffers & tables will be ³
³                  re-initialized.                                   ³
³                                                                    ³
+-------------------------------------------------------------------*/

byte dir_openadapter (ccb_parm,adapternum,product_code,sem,wait)
struct ccb_pt *ccb_parm;
byte adapternum;
byte product_code[];
#ifdef E32TO16
HSYSSEM _Seg16 sem;
#else
HSYSSEM sem;
#endif
BOOL wait;
{
        struct command_control_block_dlr *open_ccb;
        struct dir_open_adapter_parms *pt;
        struct dir_open_ad_parms      *pt1;
        struct dir_open_dlc_parms     *pt2;
        int i;

        open_ccb = &ccb_parm->ccb;
        pt  = &ccb_parm->pt.open;
        pt1 = &ccb_parm->opt.adpt;
        pt2 = &ccb_parm->opt.dlcpt;
        memset(open_ccb,0,sizeof(struct command_control_block_dlr));
        memset(pt,0,sizeof(struct dir_open_adapter_parms));
        memset(pt1,0,sizeof(struct dir_open_ad_parms));
        memset(pt2,0,sizeof(struct dir_open_dlc_parms));

        open_ccb->ccb_command     = LLC_DIR_OPEN_ADAPTER;
        open_ccb->ccb_adapter     = adapternum;
        open_ccb->ccb_retcode     = 0xff;
#ifdef E32TO16
        open_ccb->parm_1.ccb_parm_offset = (address)pt;
#else
        open_ccb->parm_1.parm.off_12.ccb_parm_offset = FP_OFF (pt);
#endif
        open_ccb->ccb_pointer     = ZEROADDRESS;
        open_ccb->ccb_appl_key    = ApplKey;
#ifdef E32TO16
        open_ccb->ccb_semaphore   = sem;
#else
        open_ccb->ccb_semaphore   = (dword)sem;
#endif

#ifdef E32TO16
        ccb_parm->pt.open.adapter_parms_offset = (address)pt1;
        ccb_parm->pt.open.dlc_parms_offset     = (address)pt2;
#else
        ccb_parm->pt.open.adapter_parms_offset = FP_OFF (pt1);
        ccb_parm->pt.open.dlc_parms_offset     = FP_OFF (pt2);
#endif

        ccb_parm->opt.adpt.open_options      = 0;
        ccb_parm->opt.adpt.node_address[0]   = 0;
        ccb_parm->opt.adpt.group_address     = 0;
        ccb_parm->opt.adpt.functional_address[0]= 0;

#ifdef E32TO16
        ccb_parm->opt.adpt.product_id_offset = product_code;
#else
        ccb_parm->opt.adpt.product_id_offset = FP_OFF (product_code);
#endif

        if(sem)
          DosSemSet(sem);
        pass_ccb (open_ccb);
        if(wait)
          {
          if(sem)
            DosSemWait(sem,-1L);
          else
            while (open_ccb->ccb_retcode == 0xff);
          }
        return open_ccb->ccb_retcode;
}


/*-------------------------------------------------------------------+
³ receive      :  Enables the adapter to receive data for the        ³
³                 specified station.                                 ³
³                                                                    ³
+-------------------------------------------------------------------*/

receive (ccb_parm,adapternum,stationid,sem,wait)
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
        struct command_control_block_dlr *ccb;
        struct receive_parms *pt;

        ccb = &ccb_parm->ccb;
        pt  = &ccb_parm->pt.receive;
        memset (ccb,0,sizeof(struct command_control_block_dlr));
        memset (pt,0,sizeof(struct receive_parms) );

        ccb->ccb_command     = LLC_RECEIVE;
        ccb->ccb_adapter     = adapternum;
        ccb->ccb_retcode     = 0xff;
        ccb->ccb_pointer     = ZEROADDRESS;
#ifdef E32TO16
        ccb->parm_1.ccb_parm_offset = (address)pt;
#else
        ccb->parm_1.parm.off_12.ccb_parm_offset = FP_OFF (pt);
#endif
        ccb->ccb_appl_key    = ApplKey;
        ccb->ccb_appl_id     = ApplId;
#ifdef E32TO16
        ccb->ccb_semaphore   = sem;
#else
        ccb->ccb_semaphore   = (dword)sem;
#endif

        pt->station_id = stationid;

        if(sem)
          DosSemSet(sem);
        pass_ccb (ccb);
        if(wait)
          {
          if(sem)
            DosSemWait(sem,-1L);
          else
            while (ccb->ccb_retcode == 0xff);
          }
        return ccb->ccb_retcode;
}
