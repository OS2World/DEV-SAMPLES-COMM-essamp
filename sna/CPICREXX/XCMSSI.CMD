/*-------------------------------------------------------*/
/* REXX sample program to set CPI-C side information     */
/*                                                       */
/* Notes: CPICREXX.EXE must be run at some point prior   */
/*        to running this program in order to register   */
/*        the CPICOMM environment to REXX.               */
/*        Also, before running this program, the         */
/*        Communications Manager keylock feature         */
/*        must either be secured with a service key of   */
/*        "svckey" or not be secured at all.             */
/*-------------------------------------------------------*/

/*------------------------*/
/* set defined constants. */
/*------------------------*/
CM_OK = 0
XC_APPLICATION_TP = 0
XC_SECURITY_PROGRAM  = 2

say 'CPI-C set side information sample program'

/*-------------------------------------------*/
/* set up parameters and sideinfo structure. */
/*-------------------------------------------*/
sideinfo_len = 124
key = "svckey  "
sideinfo.sym_dest_name = "SYMDEST1"
sideinfo.partner_LU_name = "ALIASLU2"
sideinfo.TP_name =  "MYTPN"
sideinfo.mode_name = "        "
sideinfo.TP_name_type = XC_APPLICATION_TP
sideinfo.conversation_security_type = XC_SECURITY_PROGRAM
sideinfo.security_user_id = "myuserid"
sideinfo.security_password = "xxxxxxxxx"

/*--------------------------------------------------*/
/* issue the CPI-C call to the CPICOMM environment. */
/*--------------------------------------------------*/
address CPICOMM 'xcmssi key sideinfo sideinfo_len retc'

if rc = 0 & retc = CM_OK then
   do
   say '***'
   say 'CPI-C side information successfully set.'
   say '***'
   end
else
   do
   say 'Failure to set CPI-C side information.'
   if rc = 0 then
      say 'CPI-C return code =' retc
   else if rc = 30 then
      say 'CPICREXX has not been executed.'
   else
      say 'REXX return code =' rc
   end
'pause'
'exit'           /* exit window */
