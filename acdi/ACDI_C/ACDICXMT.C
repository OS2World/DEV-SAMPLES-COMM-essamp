/************************************************************************/
/*                                                                      */
/*   MODULE NAME: ACDICXMT.C                                            */
/*                                                                      */
/*   DESCRIPTIVE NAME: ACDI C SAMPLE TRANSMIT PROGRAM                   */
/*                     OS/2 EXTENDED SERVICES                           */
/*                                                                      */
/*   COPYRIGHT:  (C) COPYRIGHT IBM CORP. 1988, 1991                     */
/*               LICENSED MATERIAL - PROGRAM PROPERTY OF IBM            */
/*               ALL RIGHTS RESERVED                                    */
/*                                                                      */
/*   STATUS:   LPP Release 1.0 Modification 0                           */
/*                                                                      */
/*   FUNCTION: The sample program will use the ACDI interface to echo   */
/*             line by line screen image from the first Personal        */
/*             Computer on the second Personal Computer connected       */
/*             through asynchronous line.                               */
/*             The TRANSMIT PROGRAM, is executed in the first Personal  */
/*             Computer.                                                */
/*                                                                      */
/*             Uses the following ACDI Verbs:                           */
/*                                                                      */
/*                COMOPEN                                               */
/*                COMDEFOUTPUTBUFF                                      */
/*                COMDEFINPUT                                           */
/*                COMSETBITRATE                                         */
/*                COMSETLINECTRL                                        */
/*                COMCONNECT                                            */
/*                COMSETTIMEOUTS                                        */
/*                COMWRITECHARSTRING                                    */
/*                COMFLUSHOUTPUT                                        */
/*                COMSENDBREAK                                          */
/*                COMDISCONNECT                                         */
/*                COMCLOSE                                              */
/*                                                                      */
/*   NOTE:     The asynchronous device name to be used in this          */
/*             program is COM1. If this is to be changed it must        */
/*             be changed in the main header.                           */
/*                                                                      */
/*             This program is designed to run with connect type 4      */
/*             only.                                                    */
/*                                                                      */
/*   MODULE TYPE = Micosoft C Compiler Version 6.00A                    */
/*                 (Compiler options - /ALw /G2 /Zp)                    */
/*                                                                      */
/*   PREREQS = Requires Message file "ACX.MSG".                         */
/*                                                                      */
/************************************************************************/

#include <ACDI_C.H>
#include <STDIO.H>
#include <CONIO.H>
#include <STDDEF.H>
#include <STRING.H>
#include <DOS.H>
#include <DOSCALLS.H>
#include <SUBCALLS.H>

#define TRUE 1
#define AA_OK 0

#define LINT_ARGS

#define clear_vcb()   memset(&vcb,(int)'\0',sizeof(vcb))
                                            /* macro to clear control block */
char sys_err       =0;                      /* system error flag            */

char dev_name[6] ="COM1\0";                 /* communication device name    */

unsigned short handle;                      /* communication device handle  */

unsigned short ret_code;                    /* save area for ACDI return    */
unsigned char func_code;                    /* and function codes           */

unsigned short dos_rc;                      /* save area for dos ret code   */
unsigned short dos_vio_rc;                  /* save area for vio ret code   */

char far *i_anbptr;                         /* pointer to input             */
                                            /* AlphaNumeric buffer          */
char far *o_anbptr;                         /* pointer to output            */
                                            /* AlphaNumeric buffer          */
unsigned short o_selector;                  /* selector value returned by   */
                                            /* DOSALLOCSEG                  */

unsigned short concttimeout1 = 0;           /* connection timeout parameters*/
unsigned short concttimeout2 = 30;          /* for comconnect verb          */

unsigned short inf_rdblocktimeout = 0;      /* infinite timeout             */
unsigned short inf_rdchartimeout = 0;       /* parameters for the verb      */
unsigned short inf_wrttimeout = 0;          /* comsettimeouts               */

unsigned short minbrksgnldurn = 20;         /* break signal length duration */

char msg_buff[100];
char msg_filename[] = "ACX.MSG";

unsigned long SLEEPTIME = 3000;             /* parameter for DOSSLEEP       */

extern void pascal far ACDI(unsigned far *);

/****************************************************************************/
/*  ACDI verb control block (vcb)                                           */
/****************************************************************************/

union {
   struct comopen_cb open_cb;
   struct comdefoutputbuff_cb defoutputbuff_cb;
   struct comdefinput_cb definputbuff_cb;
   struct comsetbitrate_cb setbitrate_cb;
   struct comsetlinectrl_cb setlinectrl_cb;
   struct comsetflowmode_cb setflowmode_cb;
   struct comconnect_cb connect_cb;
   struct comsettimeouts_cb settimeouts_cb;
   struct comwritecharstring_cb writecharstring_cb;
   struct comflushoutput_cb flushoutput_cb;
   struct comsendbreak_cb sendbreak_cb;
   struct comdisconnect_cb disconnect_cb;
   struct comclose_cb close_cb;
}vcb;

unsigned far *vcbptr;                       /* pointer to vcb               */

#ifdef LINT_ARGS
/****************************************************************************/
/*  Function prototypes                                                     */
/****************************************************************************/
void main(void);
void ACDISENDLINES(void);

void SHOW_ERR(void);
void SHOW_DOS_ERR(void);
void SHOW_VIO_ERR(void);
void SHOW_MSG(unsigned short);

void clear_screen(void);
void set_crsr_pos(int, int);

void comopen(void);
void comdefoutputbuff(unsigned short, unsigned char far *, unsigned short);
void comdefinput(unsigned short, unsigned char far *, unsigned short,
                 unsigned char);
void comsetbitrate(unsigned short, unsigned short, unsigned short);
void comsetlinectrl(unsigned short, unsigned short, unsigned short,
                    unsigned short);
void comconnect(unsigned short, unsigned short, unsigned short,
                unsigned short);
void comsettimeouts(unsigned short, unsigned short, unsigned short,
                    unsigned short);
void comwritecharstring(unsigned short, unsigned short, unsigned short);
void comflushoutput(unsigned short);
void comsendbreak(unsigned short, unsigned short);
void comdisconnect(unsigned short);
void comclose(unsigned short);

#endif
/****************************************************************************/
/*  The main program.                                                       */
/****************************************************************************/

void
main()
{
  SHOW_MSG(1);                              /* Msg. #1 - ACDI sample trans- */
                                            /* mitter program               */
  vcbptr = (unsigned far *) &vcb;           /* initialize ptr to vcb        */

  if (! sys_err)
    comopen();                              /* call sub. to issue com_open  */
  if (! sys_err)  {
    o_anbptr = 0;                           /* zero out the pointer         */
    dos_rc = DOSALLOCSEG(82, (unsigned far *)&FP_SEG(o_anbptr), 0);
                                            /* doscall to get output buffer */
    if (dos_rc != 0)  {
      SHOW_DOS_ERR();
      sys_err = TRUE;
    }                                       /* handle any error             */
  }

  if (! sys_err)                            /* issue com_def_output_buff    */
    comdefoutputbuff(handle, o_anbptr, 82);

  if (! sys_err)  {
    i_anbptr = 0;                           /* initialize pointer to zero   */
    dos_rc = DOSALLOCSEG(82, (unsigned far *)&FP_SEG(i_anbptr), 0);
                                            /* doscall to get shared buffer */
    if (dos_rc != 0)  {
      SHOW_DOS_ERR();
      sys_err = TRUE;
    }                                       /* handle any error             */
  }

  if (! sys_err)                            /* issue com_def_input          */
    comdefinput(handle, i_anbptr, 82, AA_CHAR_MODE);

  if (! sys_err)                            /* issue com_set_bit_rate       */
    comsetbitrate(handle, AA_300_BPS, AA_300_BPS);

  if (! sys_err)                            /* issue com_set_line_ctrl      */
     comsetlinectrl(handle, AA_1_STOP_BIT, AA_EVEN_PARITY, AA_7_DATA_BITS);

  if (! sys_err)                            /* issue com_connect            */
     comconnect(handle,AA_CONNECT_TYPE_4, concttimeout1, concttimeout2);

  if (! sys_err)                            /* issue com_set_timeouts       */
     comsettimeouts(handle, inf_rdblocktimeout, inf_rdchartimeout,
                    inf_wrttimeout);

  if (! sys_err)                            /* call subroutine-send message */
     ACDISENDLINES();

  if (! sys_err)                            /* issue com_send_break to      */
                                            /* indicate end of session      */
     comsendbreak(handle, minbrksgnldurn);

  if (! sys_err) {                          /* wait for some time before    */
    dos_rc = DOSSLEEP(SLEEPTIME);           /* disconnect                   */
    if (dos_rc != 0) {
      SHOW_DOS_ERR();
      sys_err = TRUE;
    }                                       /* handle any error             */
  }

  if (! sys_err)                            /* issue com_disconnect         */
     comdisconnect(handle);

  if (! sys_err)                            /* issue com_close              */
     comclose(handle);

}   /* end of main() */

/****************************************************************************/
/*  Subroutine to send message lines                                        */
/****************************************************************************/

void
ACDISENDLINES()
/* This subroutine will clear the screen, wait for user to type in the      */
/* message, echo it back on the screen, issue comwritecharstring for        */
/* every line of message to send the message across line by line. Escape    */
/* key is end of message indicator                                          */

{

#define CAR_RET 0x0D                        /* the Enter key                */
#define ESCAPE 0x1B                         /* the Escape key               */

unsigned short btswrtn = 0;                 /* initialize parameters to be  */
unsigned short writebytesneeded = 81;       /* passed in ComWriteCharString */

int row = 0;                                /* variables to keep track of   */
int col = 0;                                /* and alter cursor position    */

int index = 0;                              /* counter                      */

char char_to_be_sent = 0;                   /* character to be sent         */

int ch_atr = 0x720;                         /* parameter for vioscrollup    */

  clear_screen();                           /* blank out the screen         */

  if (! sys_err)                            /* set cursor position to row - */
    set_crsr_pos(row, col);                 /* zero, column zero            */

  if (! sys_err) {
                                            /* loop to get char's of the    */
                                            /* message and issue com_write_ */
                                            /* char_string for every line   */

     while ((! sys_err) && (char_to_be_sent != ESCAPE))  {

                                            /* loop to get 80 char's of the */
                                            /* message one by one and put   */
                                            /* them in output buffer        */
        while (char_to_be_sent != CAR_RET)  {
           char_to_be_sent = getch();       /* get the next character       */
           if (char_to_be_sent == ESCAPE )
             break;                         /* quit if it is escape         */
           else  {
             if ( char_to_be_sent == CAR_RET)  {
               char_to_be_sent = 0;
               break;                       /* exit loop if it is carriage  */
             }
             else
               printf("%c",char_to_be_sent);
           }                                /* wrt char to be sent on screen*/
           *o_anbptr = char_to_be_sent;     /* write character to be sent   */
                                            /* in output buffer             */
           ++o_anbptr;                      /* inc. ptr to point to next pos*/
           ++index;                         /* increment the counter        */
           if (index == 80)
             break;
           ++col;                           /* increment the column to next */
           set_crsr_pos(row, col);          /* set cursor to next position  */
        }                                   /* end of inner while loop      */
        *o_anbptr = CAR_RET;                /* append char str. with car_ret*/
        btswrtn = index + 1;                /* bytes written is number of   */
                                            /* char's(index) + one(car_ret) */
        col = 0;                            /* set column = 0               */
        ++row;                              /* increment current row to next*/
        if (row > 23) {                     /* if current row is 24, scroll */
                                            /* up screen by one row and.... */
          dos_rc = VIOSCROLLUP (0, 0, 24, 79, 1, (char *)&ch_atr, 0);
          row = 23;                         /* set current row to 23 again  */
        }
        set_crsr_pos(row, col);             /* set cursor to new row        */
        comwritecharstring(handle, btswrtn, writebytesneeded);
                                            /* issue com_write_char_string  */
        comflushoutput(handle);             /* issue com_flush_output to set*/
                                            /* next_avail_ptr to o_anbptr   */
        FP_OFF(o_anbptr) = 0;               /* set anbptr to point to the   */
                                            /* beginning of output buffer   */
        index = 0;                          /* set counter to zero and begin*/
                                            /* all over again for next line */
     }                                      /* end of outer while loop      */
   }                                        /* end of if(!sys_err)....      */
}
/* end of subroutine */

/****************************************************************************/
/*  Utility subroutines                                                     */
/****************************************************************************/

void
SHOW_ERR()
/* This function shows errors relating to ACDI verbs.                       */

{
  SHOW_MSG(2);
  printf ("%04X\n", func_code);
  SHOW_MSG(3);
  printf ("%04X\n", ret_code);
}

void
SHOW_DOS_ERR()
/* This function shows errors relating to DOS function calls.               */

{
  SHOW_MSG(4);
  printf ("%04X\n", dos_rc);
}

void
SHOW_VIO_ERR()
/* This function shows errors relating to Vio function calls.               */

{
  SHOW_MSG(5);
  printf ("%04X\n", dos_vio_rc);
}

void
SHOW_MSG(MSGNO)
/* This function displays error messages using argument MSGNO ( message     */
/* number) from the message file "ACX.MSG"                                  */
unsigned short MSGNO;

{
  unsigned msg_rc;
  unsigned msg_len;

  msg_rc = DOSGETMESSAGE ((char far *)0, 0, (char far *)msg_buff,
                          sizeof(msg_buff), MSGNO, (char far *)msg_filename,
                          (unsigned far *)&msg_len);
                                            /* doscall to get message from  */
                                            /* message file                 */
  if (msg_rc != AA_OK) {
    printf("Unable to process message file -DOSGETMESSAGE function call \n");
    printf("in error. Return code = %04d  Message Number = %04d \n",
            msg_rc,MSGNO);
    return;
  }
  msg_buff[msg_len] = 0x00;
  printf("%s",msg_buff);
}

/****************************************************************************/
/*  ACDI related subroutines                                                */
/****************************************************************************/

/* To issue an acdi verb the program has to build the control block struc.  */
/* with the parameters and pass the pointer to the control block to the     */
/* acdi subsystem. Each of following subroutines, when called, will build   */
/* the control block structure and then call acdi. When the acdi subsystem  */
/* returns the subroutine will check the return code, call SHOW_ERR process */
/* if the return code is bad, otherwise return to the calling process.      */

void
comopen()
/* This subroutine will issue com_open verb to open the specified com.      */
/* device for communication                                                 */

{
  clear_vcb();                              /* zero out the control block   */

  vcb.open_cb.common.function_code = COM_OPEN;
                                            /* verb - com_open              */
  strcpy(vcb.open_cb.com_dev_name,dev_name);
                                            /* copy communication device    */
                                            /* name into the control block  */
  ACDI(vcbptr);                             /* issue the acdi verb          */
  handle = vcb.open_cb.common.com_dev_handle;
                                            /* copy device handle returned  */
                                            /* by acdi to use in other verbs*/
  if (vcb.open_cb.common.return_code != AA_OK) {
                                            /* check if return code is zero */
                                            /* if not, copy ret & func code */
     ret_code = vcb.open_cb.common.return_code;
     func_code = vcb.open_cb.common.function_code;
     SHOW_ERR();                            /* show error  and              */
     sys_err = TRUE;                        /* set the system error         */
  }
}

void
comdefoutputbuff(handle, outputbuff, outbufflength)
/* This subroutine will issue com_def_output_buff to define output buffer   */

unsigned short handle;                      /* device handle                */
unsigned char far *outputbuff;              /* pointer to output buffer     */
unsigned short outbufflength;               /* length of output buffer      */

{
  clear_vcb();                              /* zero out the control block   */

  vcb.defoutputbuff_cb.common.com_dev_handle = handle;
  vcb.defoutputbuff_cb.common.function_code = COM_DEF_OUTPUT_BUFF;
  vcb.defoutputbuff_cb.output_buff = outputbuff;
  vcb.defoutputbuff_cb.out_buff_length = outbufflength;
                                            /* fill in control block with   */
                                            /* function code and parameters */
  ACDI(vcbptr);                             /* issue the verb               */

  if (vcb.defoutputbuff_cb.common.return_code != AA_OK)  {
    ret_code = vcb.defoutputbuff_cb.common.return_code;
    func_code = vcb.defoutputbuff_cb.common.function_code;
    SHOW_ERR();
    sys_err = TRUE;                         /* check return code and handle */
                                            /* any errors                   */
  }
}

void
comdefinput(handle, inputbuff, inbufflength, inputmode)
/* This subroutine will issue com_def_input verb to define input buffer     */

unsigned short handle;                      /* device handle                */
unsigned char far *inputbuff;               /* pointer to input buffer      */
unsigned short inbufflength;                /* length of input buffer       */
unsigned char inputmode;                    /* mode of input                */

{
  clear_vcb();                              /* zero out the control block   */

  vcb.definputbuff_cb.common.com_dev_handle = handle;
  vcb.definputbuff_cb.common.function_code = COM_DEF_INPUT;
  vcb.definputbuff_cb.input_buff = inputbuff;
  vcb.definputbuff_cb.in_buff_length = inbufflength;
  vcb.definputbuff_cb.input_mode = inputmode;
                                            /* fill in control block with   */
                                            /* function code and parameters */
  ACDI(vcbptr);                             /* issue the verb               */

  if (vcb.definputbuff_cb.common.return_code != AA_OK) {
    ret_code = vcb.definputbuff_cb.common.return_code;
    func_code = vcb.definputbuff_cb.common.function_code;
    SHOW_ERR();
    sys_err = TRUE;                         /* check return code and handle */
                                            /* any errors                   */
  }
}

void
comsetbitrate(handle, rcv_bps, trans_bps)
/* This subroutine will issue com_set_bit_rate verb to set up the line      */
/* data rates (bps).                                                        */

unsigned short handle;                      /* device handle                */
unsigned short rcv_bps, trans_bps;          /* recv. and trans. data rates  */

{
  clear_vcb();                              /* zero out the control block   */

  vcb.setbitrate_cb.common.com_dev_handle = handle;
  vcb.setbitrate_cb.common.function_code = COM_SET_BIT_RATE;
  vcb.setbitrate_cb.bit_rate_rcv = rcv_bps;
  vcb.setbitrate_cb.bit_rate_send = trans_bps;
                                            /* fill in control block with   */
                                            /* function code and parameters */
  ACDI(vcbptr);                             /* issue the verb               */

  if (vcb.setbitrate_cb.common.return_code != AA_OK)  {
    ret_code = vcb.setbitrate_cb.common.return_code;
    func_code = vcb.setbitrate_cb.common.function_code;
    SHOW_ERR();
    sys_err = TRUE;                         /* check return code and handle */
                                            /* any errors                   */
  }
}

void
comsetlinectrl(handle, stopbits, par, databits)
/* This subroutine will issue com_set_line_ctrl to set up line control      */
/* values.                                                                  */

unsigned short handle;                      /* device handle                */
unsigned short stopbits, par, databits;     /* line control parameters      */

{
  clear_vcb();                              /* zero out the control block   */

  vcb.setlinectrl_cb.common.com_dev_handle = handle;
  vcb.setlinectrl_cb.common.function_code = COM_SET_LINE_CTRL;
  vcb.setlinectrl_cb.stop_bits = stopbits;
  vcb.setlinectrl_cb.parity = par;
  vcb.setlinectrl_cb.data_bits = databits;
                                            /* fill in control block with   */
                                            /* function code and parameters */
  ACDI(vcbptr);                             /* issue the verb               */

  if (vcb.setlinectrl_cb.common.return_code != AA_OK)  {
    ret_code = vcb.setlinectrl_cb.common.return_code;
    func_code = vcb.setlinectrl_cb.common.function_code;
    SHOW_ERR();
    sys_err = TRUE;                         /* check return code and handle */
                                            /* any errors                   */
  }
}

void
comconnect(handle, connecttype, connecttimeout1, connecttimeout2)
/* This subroutine will issue com_connect to establish connection           */

unsigned short handle;                      /* device handle                */
unsigned short connecttype;                 /* type of connection           */
unsigned short connecttimeout1;             /* timeout values to set amount */
unsigned short connecttimeout2;             /* of time to wait before
                                            /* "hanging up"                 */

{
  clear_vcb();                              /* zero out the control block   */

  vcb.connect_cb.common.com_dev_handle = handle;
  vcb.connect_cb.common.function_code = COM_CONNECT;
  vcb.connect_cb.connect_type = connecttype;
  vcb.connect_cb.connect_timeout_1 = connecttimeout1;
  vcb.connect_cb.connect_timeout_2 = connecttimeout2;
                                            /* fill in control block with   */
                                            /* function code and parameters */
  ACDI(vcbptr);                             /* issue the verb               */

  if (vcb.connect_cb.common.return_code != AA_OK)  {
    ret_code = vcb.connect_cb.common.return_code;
    func_code = vcb.connect_cb.common.function_code;
    SHOW_ERR();
    sys_err = TRUE;                         /* check return code and handle */
                                            /* any errors                   */
  }
}

void
comsettimeouts(handle, rdtimoutblock, rdtimoutchar, wrttimout)
/* This subroutine will issue com_set_timeouts to set the timeout values    */
/* for read and write.                                                      */

unsigned short handle;                      /* device handle                */
unsigned short rdtimoutblock;               /* block mode read timeout      */
unsigned short rdtimoutchar;                /* char mode read timeout       */
unsigned short wrttimout;                   /* write timeout                */

{
  clear_vcb();                              /* zero out the control block   */

  vcb.settimeouts_cb.common.com_dev_handle = handle;
  vcb.settimeouts_cb.common.function_code = COM_SET_TIMEOUTS;
  vcb.settimeouts_cb.read_timeout_block = rdtimoutblock;
  vcb.settimeouts_cb.read_timeout_char = rdtimoutchar;
  vcb.settimeouts_cb.write_timeout = wrttimout;
                                            /* fill in control block with   */
                                            /* function code and parameters */
  ACDI(vcbptr);                             /* issue the verb               */

  if (vcb.settimeouts_cb.common.return_code != AA_OK)  {
    ret_code = vcb.settimeouts_cb.common.return_code;
    func_code = vcb.settimeouts_cb.common.function_code;
    SHOW_ERR();
    sys_err = TRUE;                         /* check return code and handle */
                                            /* any errors                   */
  }
}

void
comwritecharstring(handle, byteswritten, writebytesneeded)
/* This subroutine will issue com_write_char_string verb to write data      */
/* in output buffer to the communication device.                            */

unsigned short handle;                      /* device handle                */
unsigned short byteswritten;                /* # of bytes currently in buff */
unsigned short writebytesneeded;            /* required # of write bytes    */

{
  clear_vcb();                              /* zero out the control block   */

  vcb.writecharstring_cb.common.com_dev_handle = handle;
  vcb.writecharstring_cb.common.function_code = COM_WRITE_CHAR_STRING;
  vcb.writecharstring_cb.bytes_written = byteswritten;
  vcb.writecharstring_cb.write_bytes_needed = writebytesneeded;
                                            /* fill in control block with   */
                                            /* function code and parameters */
  ACDI(vcbptr);                             /* issue the verb               */

  if (vcb.writecharstring_cb.common.return_code != AA_OK)  {
    ret_code = vcb.writecharstring_cb.common.return_code;
    func_code = vcb.writecharstring_cb.common.function_code;
    SHOW_ERR();
    sys_err = TRUE;                         /* check return code and handle */
                                            /* any errors                   */
  }
}

void
comflushoutput(handle)
/* This subroutine will issue com_flush_output verb to set next available   */
/* write pointer to output buffer pointer and next available write length   */
/* to size of output buffer.                                                */

unsigned short handle;

{
  clear_vcb();                              /* zero out the control block   */

  vcb.flushoutput_cb.common.com_dev_handle = handle;
  vcb.flushoutput_cb.common.function_code = COM_FLUSH_OUTPUT;
                                            /* fill in control block with   */
                                            /* function code and parameters */
  ACDI(vcbptr);                             /* issue the verb               */

  if (vcb.flushoutput_cb.common.return_code != AA_OK)  {
    ret_code = vcb.flushoutput_cb.common.return_code;
    func_code = vcb.flushoutput_cb.common.function_code;
    SHOW_ERR();
    sys_err = TRUE;                         /* check return code and handle */
                                            /* any errors                   */
  }
}

void
comsendbreak(handle, minduration)
/* This subroutine will issue com_send_break verb to signal to receiver the */
/* end of session.                                                          */

unsigned short handle;                      /* device handle                */
unsigned short minduration;                 /* break signal duration        */

{
  clear_vcb();                              /* zero out the control block   */

  vcb.sendbreak_cb.common.com_dev_handle = handle;
  vcb.sendbreak_cb.common.function_code = COM_SEND_BREAK;
  vcb.sendbreak_cb.min_duration = minduration;
                                            /* fill in control block with   */
                                            /* function code and parameters */
  ACDI(vcbptr);                             /* issue the verb               */

  if (vcb.sendbreak_cb.common.return_code != AA_OK)  {
    ret_code = vcb.sendbreak_cb.common.return_code;
    func_code = vcb.sendbreak_cb.common.function_code;
    SHOW_ERR();
    sys_err = TRUE;                         /* check return code and handle */
                                            /* any errors                   */
  }
}

void
comdisconnect(handle)
/* This subroutine will issue com_disconnect to break the connection.       */

unsigned short handle;                      /* device handle                */

{
  clear_vcb();                              /* zero out the control block   */

  vcb.disconnect_cb.common.com_dev_handle = handle;
  vcb.disconnect_cb.common.function_code = COM_DISCONNECT;
                                            /* fill in control block with   */
                                            /* function code and parameters */
  ACDI(vcbptr);                             /* issue the verb               */


  if (vcb.disconnect_cb.common.return_code != AA_OK)  {
    ret_code = vcb.disconnect_cb.common.return_code;
    func_code = vcb.disconnect_cb.common.function_code;
    SHOW_ERR();
    sys_err = TRUE;                         /* check return code and handle */
                                            /* any errors                   */
  }
}

void
comclose(handle)
/* This subroutine will issue com_close to close the communication device.  */

unsigned short handle;                      /* device handle                */

{
  clear_vcb();                              /* zero out the control block   */

  vcb.close_cb.common.com_dev_handle = handle;
  vcb.close_cb.common.function_code = COM_CLOSE;
                                            /* fill in control block with   */
                                            /* function code and parameters */
  ACDI(vcbptr);                             /* issue the verb               */

  if (vcb.close_cb.common.return_code != AA_OK)  {
    ret_code = vcb.close_cb.common.return_code;
    func_code = vcb.close_cb.common.function_code;
    SHOW_ERR();
    sys_err = TRUE;                         /* check return code and handle */
                                            /* any errors                   */
  }
}

/****************************************************************************/
/*  OS/2 related functions                                                  */
/****************************************************************************/

void
clear_screen()
/* This function clears the screen to prepare to take the message to be sent.*/
/* Writes null on the whole screen                                           */

{
#define SCREENSIZE 2000

char null = 0x20;

  dos_vio_rc = VIOWRTNCHAR (&null, SCREENSIZE, 0, 0, 0);
                                            /* vio call to write on screen  */
  if (dos_vio_rc != 0)  {
    SHOW_VIO_ERR();
    sys_err = TRUE;
  }                                         /* handle any error             */
}

void
set_crsr_pos(row_no, col_no)
/* This function sets the cursor at specified row and column.               */
int row_no, col_no;                         /* row & column numbers where   */
                                            /* cursor is to be set          */
{
  dos_vio_rc = VIOSETCURPOS(row_no, col_no,0);
                                            /* vio call to set cursor pos   */
  if (dos_vio_rc != 0)  {
    SHOW_VIO_ERR();
    sys_err = TRUE;                         /* handle any error             */
  }
}

/* end of program */
