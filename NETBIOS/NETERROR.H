/* <NETERROR.H>
**
** Definition of Netbios error messages
**
*/

static struct CodeTable NetErrorMsgs[] =
        {
         { 0x00, "Normal return" },
         { 0x01, "Illegal buffer length" },
         { 0x03, "Invalid command" },
         { 0x05, "Command timed out" },
         { 0x06, "Message incomplete" },
         { 0x07, "Data for one or more SEND.NO.ACK commands was not received" },
         { 0x08, "Illegal local session number" },
         { 0x09, "No resource available" },
         { 0x0A, "Session closed" },
         { 0x0B, "Command cancelled" },
         { 0x0D, "Duplicate name in local name table" },
         { 0x0E, "Name table full" },
         { 0x0F, "Name has active session & is now deregistered" },
         { 0x11, "Local session table full" },
         { 0x12, "Session open rejected" },
         { 0x13, "Illegal name number" },
         { 0x14, "Cannot find name called" },
         { 0x15, "Name not found or cannot specify \"*\" or null" },
         { 0x16, "Name in use on remote NETBIOS interface" },
         { 0x17, "Name deleted" },
         { 0x18, "Session ended abnormally" },
         { 0x19, "Name conflict detected" },
         { 0x21, "Interface busy" },
         { 0x22, "Too many commands outstanding" },
         { 0x23, "Invalid number in adapter number field" },
         { 0x24, "Command completed while cancel occurring" },
         { 0x26, "Command not valid to cancel" },
         { 0x30, "Name defined by another environment" },
         { 0x34, "Environment not defined, RESET must be issued" },
         { 0x35, "Required operating system resources exhausted, retry later" },
         { 0x36, "Maximum applications exceeded" },
         { 0x37, "No SAPs available for NETBIOS" },
         { 0x38, "Requested resource(s) not available" },
         { 0x39, "Invalid NCB address or length does not fit in segment" },
         { 0x3A, "RESET may not be issued from a NETBIOS adapter appendage" },
         { 0x3B, "Invalid NCB_DD_ID" },
         { 0x3C, "NETBIOS attempted to lock user storage and the lock failed" },
         { 0x3F, "NETBIOS Device Driver open error" },
         { 0x40, "OS/2 error detected" },
         { 0x4F, "Ring status; one or more of bits 8-11 on" },
         { 0xF6, "Unexpected error on CCB completion" },
         { 0xF8, "Error on implicit DIR.OPEN.ADAPTER" },
         { 0xF9, "Adapter support interface internal error" },
         { 0xFA, "Adapter check" },
         { 0xFB, "NETBIOS program not loaded in PC" },
         { 0xFC, "DIR.OPEN.ADAPTER or DLC.OPEN.SAP failed" },
         { 0xFD, "Unexpected adapter close" },
         { 0, 0 }
        };

