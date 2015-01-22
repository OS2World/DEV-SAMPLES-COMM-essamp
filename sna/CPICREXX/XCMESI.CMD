/*-------------------------------------------------------*/
/* REXX sample program to extract CPI-C side information */
/*                                                       */
/* Note: CPICREXX.EXE must be run at some point prior    */
/*       to running this program in order to register    */
/*       the CPICOMM environment to REXX.                */
/*-------------------------------------------------------*/
 
/*------------------------*/
/* Set defined constants. */
/*------------------------*/
CM_OK = 0
CM_PROGRAM_PARAMETER_CHECK = 24
XC_SECURITY_PROGRAM = 2
 
say 'CPI-C extract side information sample program'
 
entry_number = 1
sideinfolen = 124
rc = 0
retc = CM_OK
 
do while rc = 0 & retc = CM_OK
address cpicomm 'xcmesi entry_number symdest sideinfo sideinfolen retc'
if rc = 0 & retc = CM_OK then
   do
   say '***'
   say '*** Extracted CPI-C side information for entry number:' entry_number
   say '***'
   say 'entry_number =' entry_number
   say 'sym_dest_name =' sideinfo.sym_dest_name
   say 'TP_name =' sideinfo.TP_name
   say 'TP_name_type =' sideinfo.TP_name_type
   say 'partner_LU_name =' sideinfo.partner_LU_name
   say 'mode_name =' sideinfo.mode_name
   say 'conversation_security_type =' sideinfo.conversation_security_type
   if sideinfo.conversation_security_type = XC_SECURITY_PROGRAM then
      say 'security_user_id =' sideinfo.security_user_id
   say
   end
else
   do
   if rc = 30 then
      say 'CPICREXX has not been executed.'
   else if rc <> 0 then
      do
      say 'Failure extracting CPI-C side information '
      say 'for entry number:' entry_number
      say 'REXX return code =' rc
      end
   else if retc <> CM_PROGRAM_PARAMETER_CHECK then
      do
      say 'Failure extracting CPI-C side information '
      say 'for entry number:' entry_number
      say 'CPI-C return code =' retc
      end
   else if entry_number = 1 then
      say 'No CPI-C side information.'
   else
      say 'End of CPI-C side information entries.'
   end
entry_number = entry_number + 1
end
 
/*-----------------------------------------------------------*/
/* Now extract side information by symbolic destination name */
/*-----------------------------------------------------------*/
use_symdest = 0
entry_number = use_symdest
symdest = "SYMDEST1"
address cpicomm 'xcmesi entry_number symdest sideinfo sideinfolen retc'
 
if rc = 0 & retc = CM_OK then
   do
   say '***'
   say '*** Extracted CPI-C side information for symbolic destination name:' symdest
   say '***'
   say 'sym_dest_name =' sideinfo.sym_dest_name
   say 'TP_name =' sideinfo.TP_name
   say 'TP_name_type =' sideinfo.TP_name_type
   say 'partner_LU_name =' sideinfo.partner_LU_name
   say 'mode_name =' sideinfo.mode_name
   say 'conversation_security_type =' sideinfo.conversation_security_type
   if sideinfo.conversation_security_type = XC_SECURITY_PROGRAM then
      say 'security_user_id =' sideinfo.security_user_id
   end
else if rc = 0 then
   do
   say 'Failure extracting CPI-C side information'
   say 'for symbolic destination name:' symdest
   say 'CPI-C return code =' retc
   end
else if rc = 30 then
   say 'CPICREXX has not been executed.'
else
   say 'REXX return code =' rc
'pause'
'exit'
