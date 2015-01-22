struct CodeTable
{
        unsigned char code;
        char *label;
};

union ncb_types
   {
   struct network_control_block basic_ncb;
   struct ncb_chain_send send;
   struct ncb_reset reset;
   };

union ncb_types Ncb;            /* NCB used for all commands          */

byte NetReset();
byte NetAddName();
byte NetCall();
byte NetSend();
byte NetListen();
byte NetReceive();
byte NetHangup();
unsigned TRNcmd();

byte retcode;                   /* NCB retcode                        */
byte Mode;                      /* 0 = Send, 1 = Recv                 */
byte Lana;                      /* 0 or 1                             */
byte LclName[16];               /* local name to add to the network   */
byte NetName[16];               /* remote name to call/listen for     */
byte Message[80];               /* send/recv buffer                   */
byte Message1[300];             /* send/recv buffer for chain send    */
byte Message2[300];

#define  NET_LSN         6      /* maximum number of local sessions   */
#define  NET_NUM        12      /* maximum number of outstanding NCBs */
#define  SEND            0      /* Mode flag for SEND                 */
#define  RECV            1      /* Mode flag for RECEIVE              */
