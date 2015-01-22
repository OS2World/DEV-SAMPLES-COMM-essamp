/*---------------------------------------------------------------------------+
³ Global variables for the DLCSAMPL package                                  ³
+---------------------------------------------------------------------------*/

/*------------------------------------------------------------------------+
³  DLC status change message structure                                    ³
+------------------------------------------------------------------------*/

typedef struct DLCStatusTable {
        word    stationid;
        word    reg_ax;
        byte    reasoncode[5];
        byte    accessprty;
        byte    remote_node[6];
        byte    remote_sap;
        byte    lana;
        }DLC_STAT_TBL;

struct read_pt_status
   {
   word station_id;
   byte option_idicator;
   byte event_set;
   byte event;
   byte critical_subset;
   dword notification_flag;
   DLC_STAT_TBL status_tbl;
   };

typedef struct {
        byte   *next_buf_ptr;
        byte    data[1];
        } BUFFER;

byte ProductId[18];
byte RoutingInfo[18];

union pt_add
   {
#ifdef E32TO16
   unsigned char * _Seg16 address;
#else
   unsigned long int address;
#endif
   struct {
      word offset;
      word segment;
      } part;
   };

union pt_types
   {
   struct dlc_open_sap_parms open_sap;
   struct dlc_connect_station_parms connect;
   struct dlc_open_station_parms open_station;
   struct dir_initialize_parms init;
   struct dir_status_parms status;
   struct receive_parms receive;
   struct transmit_parms transmit;
   struct read_pt_status read;
   struct dir_open_adapter_parms open;
   };

struct open_pt
   {
   struct dir_open_ad_parms  adpt;
   struct dir_open_dlc_parms dlcpt;
   };

struct ccb_pt
   {
   struct command_control_block_dlr ccb;
   union pt_types  pt;
   struct open_pt  opt;
   };

struct ccb_pt workccb, rcvccb;
union pt_add data_add;
struct receive_not_contiguous * rbptr;
struct lan_header lanheader;    /* get enough to hold a big one       */
DLC_STAT_TBL *dsp;

word Sapid;
word Stationid;
word i, msglen;

byte retcode;
byte ApplId;
byte * dataptr;
byte Mode;                      /* 0 = Send, 1 = Recv                 */
byte Lana;                      /* 0 or 1                             */
byte RmtAddr[6];                /* Address of remote adapter          */
byte LclAddr[6];                /* Address of local adapter           */
byte lanheaderlen;              /* size of the current lan header     */
byte funcaddr[4] = {0x00, 0x01, 0x00, 0x00};
byte message[] = "Gooooooo Longhorns!";

#define SEND    0               /* mode flag for SEND                 */
#define RECV    1               /* mode flag for RECEIVE              */
#define DISP    2               /* mode flag for DISPLAY              */
#define No 0
#define Yes 1
#define SendSap 0xA4
#define RecvSap 0xA6
#define BELL_CHAR       '\007'
#define READ_SAP 0x01           /* READ match options           */
#define ZEROADDRESS     ((unsigned char *)(0L))
#define SystemKey 0xCFB0
#define ApplKey 0x000D
