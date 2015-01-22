/************************************************************************/
/*                                                                      */
/*   MODULE NAME: ACDICRCV.C                                            */
/*                                                                      */
/*   DESCRIPTIVE NAME: ACDI C SAMPLE RECEIVE PROGRAM                    */
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
/*             The RECEIVE PROGRAM, is executed in the second Personal  */
/*             Computer.                                                */
/*                                                                      */
/*              Uses the following ACDI Verbs:                          */
/*                                                                      */
/*                COMOPEN                                               */
/*                COMDEFOUTPUTBUFF                                      */
/*                COMDEFINPUT                                           */
/*                COMSETBITRATE                                         */
/*                COMSETLINECTRL                                        */
/*                COMCONNECT                                            */
/*                COMSETTIMEOUTS                                        */
/*                COMREADEVENT                                          */
/*                COMREADCHARSTRING                                     */
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
#define STACK_LGTH 4096                     /* len of stack for thread proc */

#define session_active 0
#define session_ended 1

char sys_err       = 0;                     /* system error flag            */
char dev_name[6]   ="COM1\0";               /* communication device name    */

unsigned short handle;                      /* communication device handle  */

unsigned short ret_code;                    /* save area for ACDI return    */
unsigned char func_code;                    /* and function codes           */

unsigned short dos_rc;                      /* save area for dos and        */
unsigned short dos_vio_rc;                  /* vio return codes             */

char far *i_anbptr;                         /* pointer to input             */
                                            /* AlphaNumeric buffer          */

char far *o_anbptr;                         /* pointer to output            */
                                            /* AlphaNumeric buffer          */

char far *s_anbptr;                         /* pointer to stack for the     */
                                            /* thread process-ACDIEVENT     */

unsigned short thread_id;                   /* returned by DosCreateThread  */

unsigned long event_sem;                    /* ram semaphore for signaling  */
                                            /* occurance of an event        */

unsigned short concttimeout1 = 0;           /* connection timeout parameters*/
unsigned short concttimeout2 = 30;          /* for comconnect verb          */

unsigned short inf_rdblocktimeout = 0;      /* infinite timeout             */
unsigned short rdchartimeout = 1;           /* parameters for the verb      */
unsigned short inf_wrttimeout = 0;          /* comsettimeouts               */

char msg_buff[100];
char msg_filename[] = "ACX.MSG";

char ACDIEVENTFLAG;                         /* event occurance flag         */

extern void pascal far ACDI(unsigned far *);
void far ACDIEVENT() ;

/****************************************************************************/
/*  ACDI communications control block                                       */
/****************************************************************************/

union {
   struct comopen_cb open_cb;
   struct comdefinput_cb definputbuff_cb;
   struct comdefoutputbuff_cb defoutputbuff_cb;
   struct comsetbitrate_cb setbitrate_cb;
   struct comsetlinectrl_cb setlinectrl_cb;
   struct comconnect_cb connect_cb;
   struct comsettimeouts_cb settimeouts_cb;
   struct comreadcharstring_cb readcharstring_cb;
   struct comdisconnect_cb disconnect_cb;
   struct comclose_cb close_cb;
}vcb;

unsigned far *vcbptr;                       /* pointer to vcb               */

struct event_struct event_types;            /* structures to be used in the */
struct masks event_masks;                   /* thread process-ACDIEVENT()   */
struct comreadevent_cb readevent_cb;

#ifdef LINT_ARGS
/****************************************************************************/
/*  Function prototypes                                                     */
/****************************************************************************/
void main(void);
void ACDIREADLINES(void);
void ACDIEVENT(void);

void SHOW_ERR(void);
void SHOW_DOS_ERR(void);
void SHOW_VIO_ERR(void);
void SHOW_MSG(unsigned short);

void set_sem(unsigned long);
void sem_wait(unsigned long);
void clear_screen(void);
void set_crsr_pos(int, int);

void comopen(void);
void comdefinput(unsigned short, unsigned char far *, unsigned short,
                 unsigned char);
void comdefoutputbuff(unsigned short, unsigned char far *, unsigned short);
void comsetbitrate(unsigned short, unsigned short, unsigned short);
void comsetlinectrl(unsigned short, unsigned short, unsigned short,
                    unsigned short);
void comconnect(unsigned short, unsigned short, unsigned short,
                unsigned short);
void comsettimeouts(unsigned short, unsigned short, unsigned short,
                    unsigned short);
void comreadcharstring(unsigned short, unsigned short, unsigned short,
                       unsigned short);
void comreadevent(unsigned short, unsigned long, struct event_struct *,
                  struct masks *, unsigned far *);
void comdisconnect(unsigned short);
void comclose(unsigned short);

#endif
/****************************************************************************/
/*   The main program.                                                      */
/****************************************************************************/

void
main()
{
  SHOW_MSG(7);                              /* msg 7-'ACDI sample recv.prog'*/
  vcbptr = (unsigned far *)&vcb;            /* initialize vcbptr            */

  if (! sys_err)
    comopen();                              /* issue com_open verb          */

  if (! sys_err)  {
    i_anbptr = 0;                           /* zero out the pointer         */
    dos_rc = DOSALLOCSEG(82, (unsigned far *)&FP_SEG(i_anbptr), 0);
                                            /* doscall to get input buffer  */
    if (dos_rc != 0)  {
      SHOW_DOS_ERR();
      sys_err = TRUE;
    }
  }

  if (! sys_err)                            /* issue com_def_input          */
    comdefinput(handle, i_anbptr, 82, AA_CHAR_MODE);

  if (! sys_err)  {
    o_anbptr = 0;                           /* zero out the pointer         */
    dos_rc = DOSALLOCSEG(82, (unsigned far *)&FP_SEG(o_anbptr), 0);
                                            /* doscall to get output buffer */
    if (dos_rc != 0)  {
      SHOW_DOS_ERR();
      sys_err = TRUE;
    }
  }

  if (!sys_err)                             /* issue com_def_output_buff    */
    comdefoutputbuff(handle, o_anbptr, 82);

  if (! sys_err)                            /* issue com_set_bit_rate       */
    comsetbitrate(handle, AA_300_BPS, AA_300_BPS);

  if (! sys_err)                            /* issue com_set_line_ctrl      */
     comsetlinectrl(handle, AA_1_STOP_BIT, AA_EVEN_PARITY, AA_7_DATA_BITS);

  if (! sys_err)                            /* issue com_connect            */
     comconnect(handle,AA_CONNECT_TYPE_4, concttimeout1, concttimeout2);

  if (! sys_err)                            /* issue com_set_timeouts       */
     comsettimeouts(handle, inf_rdblocktimeout, rdchartimeout,
                    inf_wrttimeout);

  if (! sys_err)  {
    s_anbptr = 0;                           /* zero out the pointer         */
    dos_rc = DOSALLOCSEG(STACK_LGTH,(unsigned far *)&FP_SEG(s_anbptr), 3);
                                            /* doscall to get a buffer to be*/
                                            /* used as stack for thread proc*/
    if (dos_rc != 0)  {
      SHOW_DOS_ERR();
      sys_err = TRUE;
    }
    s_anbptr = s_anbptr + (STACK_LGTH - 1);
  }

  if (! sys_err)  {
    dos_rc = DOSCREATETHREAD(ACDIEVENT, (unsigned far *)&thread_id, s_anbptr);
                                            /* doscall to start thread proc */
    if (dos_rc != 0)  {
      SHOW_DOS_ERR();
      sys_err = TRUE;
    }
  }

  if (!sys_err)                             /* call subroutine-read message */
    ACDIREADLINES();

  if (! sys_err)                            /* issue com_disconnect         */
     comdisconnect(handle);

  if (! sys_err)                            /* issue com_close              */
     comclose(handle);

}   /* end of main */

/*****************************************************************************/
/*   Thread process                                                          */
/*****************************************************************************/

void far ACDIEVENT()
/*  This is the thread process. It will set the EVENTFLAG to zero to enable */
/*  receiving characters by the main process, then set the semaphore; issue */
/*  comreadevent verb, and wait for the semaphore to be cleared by async    */
/*  subsystem - which will happen when any one of three events specified -  */
/*  break signal, disconnect or a stop is received. When this occurs it     */
/*  will set the EVENTFLAG to signal end of session.                        */
/*  This process runs asynchronously with the main process so that receiving*/
/*  message, and watching for the break signal can be done simultaneously   */

{
  ACDIEVENTFLAG = session_active;           /* set event flag to active     */

  set_sem((unsigned long)&event_sem);       /* set event semaphore          */

  if (! sys_err) {
    event_masks.event_mask_1[0] = 0x00;
    event_masks.event_mask_1[1] = 0x00;
    event_masks.event_mask_1[2] = AA_BREAK_RECEIVED | AA_CONNECTION_LOST;
    event_masks.event_mask_1[3] = AA_STOP_ISSUED;
    event_masks.event_mask_2[0] = 0x00;
    event_masks.event_mask_2[1] = 0x00;
    event_masks.event_mask_2[2] = 0x00;
    event_masks.event_mask_2[3] = 0x00;     /* set up masks for COMREADEVENT*/

    comreadevent(handle,(unsigned long) &event_sem, &event_types,
                 &event_masks, (unsigned far *)&readevent_cb );
                                            /* issue comreadevent verb      */
  }
  if (!sys_err)
    sem_wait((unsigned long)&event_sem);    /* wait for sem. to be cleared  */

  if (! sys_err)                            /* semaphore has been cleared so*/
    ACDIEVENTFLAG = session_ended;          /* set flag to session_ended    */
}   /* end of thread process */

/****************************************************************************/
/*  Subroutine to read message.                                             */
/****************************************************************************/

void
ACDIREADLINES()
/* This subroutine will clear the screen in preparation for receiving the   */
/* message, issue com_read_char_string verb as long as ACDIEVENTFLAG is on, */
/* read the message character by character, display it on the screen. It    */
/* will quit when the flag is set to session ended.                         */

{
#define CAR_RET 0x0D                        /* the Enter/Carriage return key*/

unsigned short bytesfreed = 0;              /* parameters for com_read_char_*/
unsigned short readbytesneeded = 1;         /* string verb                  */
unsigned short initialwait = 0;

int row = 0;                                /* variables to keep track of   */
int col = 0;                                /* and alter cursor position    */

int ch_atr = 0x720;                         /* parameter for vioscrollup    */

  clear_screen();                           /* clear the screen             */

  if (!sys_err)
    set_crsr_pos(row, col);                 /* set cursor to row0 column0   */

    while ((ACDIEVENTFLAG == session_active)&&(! sys_err))  {
       comreadcharstring(handle, bytesfreed, readbytesneeded,initialwait);
                                            /* issue com_read_char_string   */

       if (ret_code == 0x54) {
                                            /* if timeout occurred or discon*/
         bytesfreed = 0;                    /* set bytesfreed to zero,      */
         continue;                          /* try to issue verb again      */
       }
       else  {                              /* if no timeout occured and no */
         if (!sys_err)  {                   /* error occured-character has  */
                                            /* been received                */
           i_anbptr = vcb.readcharstring_cb.next_avail_read_ptr;
                                            /* copy ptr to received char.   */
           if (*i_anbptr != CAR_RET)  {     /* if rec'd char is not car_ret */
             dos_vio_rc = VIOWRTNCHAR(i_anbptr, 1, row, col, 0);
                                            /* write character on screen    */
             if (dos_vio_rc != 0)  {        /* check viocall return code &  */
               SHOW_VIO_ERR();              /* handle any error             */
               sys_err = TRUE;
             }
             ++col;                         /* incr col-next pos on screen  */
           }
           else  {                          /* otherwise, car_ret was rec'd */
                                            /* it means-start a new line    */
             dos_vio_rc = VIOGETCURPOS(&row, &col, 0) ;
                                            /* doscall-get current crsr pos */
             col = 0;                       /* set column to 0 of next line */
             ++row;                         /* increment current row to next*/
             if (row == 24)  {              /* if current row is 24, scroll */
                                            /* up screen by one row and...  */
               dos_rc = VIOSCROLLUP(0, 0, 24, 79, 1, (char *)&ch_atr,0);
               row = 23;                    /* set current row to 23 again  */
             }
           }
           if (col == 80)                   /* 79th column is last on screen*/
             col = 0;                       /* so if col 79 is reached,     */
                                            /* set it back to zero          */
           set_crsr_pos(row, col) ;         /* set cursor to new position   */
           bytesfreed = 1;                  /* indicate one byte is read    */
         }   /* end of if (! sys_err) */
       }     /* end of else           */
    }        /* end of while loop     */
}            /* end of subroutine     */
/* end of subroutine */

/****************************************************************************/
/*  Utility Functions                                                       */
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
comreadevent(handle, semhandle, bufferaddress, masksaddress, vcbaddress)
/* This subroutine will issue comreadevent verb so that the calling process */
/* can wait on the semaphore to be cleared.                                 */

unsigned short handle;                      /* device handle                */
unsigned long semhandle;                    /* sem handle-identifier or addr*/
struct event_struct *bufferaddress;         /* event_struct buffer address  */
struct masks *masksaddress;                 /* masks structure address      */
unsigned far *vcbaddress;                   /* control block address        */

{
  readevent_cb.common.com_dev_handle = handle;
  readevent_cb.common.function_code = COM_READ_EVENT;
  readevent_cb.sem_handle = semhandle;
  readevent_cb.buffer_address = bufferaddress;
  readevent_cb.event_masks = masksaddress;
                                            /* fill in control block with   */
                                            /* function code and parameters */
  ACDI(vcbaddress);                         /* issue the verb               */

  if (readevent_cb.common.return_code != AA_OK) {
    ret_code = readevent_cb.common.return_code;
    func_code = readevent_cb.common.function_code;
    SHOW_ERR();
    sys_err = TRUE;                         /* check return code and handle */
                                            /* any errors                   */
  }
}

void
comreadcharstring(handle, bytesfreed, readbytesneeded, initialwait)
/* This subroutine will issue com_read_char_string verb to enable program   */
/* to read data that has been received over the line and is put in the      */
/* input buffer.                                                            */

unsigned short handle;                      /* device handle                */
unsigned short bytesfreed;                  /* num. of bytes read last time */
unsigned short readbytesneeded;             /* return when these many read  */
                                            /* bytes are available          */
unsigned short initialwait;                 /* initial delay for timeout    */

{
  clear_vcb();                              /* zero out the control block   */

  vcb.readcharstring_cb.common.com_dev_handle = handle;
  vcb.readcharstring_cb.common.function_code = COM_READ_CHAR_STRING;
  vcb.readcharstring_cb.bytes_freed = bytesfreed;
  vcb.readcharstring_cb.read_bytes_needed = readbytesneeded;
  vcb.readcharstring_cb.initial_wait = initialwait;
                                            /* fill in control block with   */
                                            /* function code and parameters */
  ACDI(vcbptr);                             /* issue the verb               */


  if (vcb.readcharstring_cb.common.return_code != AA_OK) {
                                            /* if return is not good....    */
    ret_code = vcb.readcharstring_cb.common.return_code;
    func_code = vcb.readcharstring_cb.common.function_code;
                                            /* copy return and func. codes  */
    if ( ret_code == 0x54)                  /* if ret_code is 54H - timeout */
      return;                               /* occured - just return        */
    else {                                  /* otherwise an error occured - */
      SHOW_ERR();                           /* so handle the error          */
      sys_err = TRUE;
      return;
    }
  }
  else                                      /* if return is good - data is  */
    ret_code = AA_OK;                       /* received; set ret_code to ok */
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

/*****************************************************************************/
/*  OS/2 Related Functions                                                   */
/*****************************************************************************/

void
set_sem(sem_handle)
/* This function sets semaphore.                                             */

unsigned long sem_handle;

{
  dos_rc = DOSSEMSET(sem_handle);
  if (dos_rc != AA_OK) {
    SHOW_DOS_ERR();
    sys_err = TRUE;
  }
}

void
sem_wait(sem_handle)
/* This function waits till the semaphore is cleared.                       */

unsigned long sem_handle;

{
unsigned long inf_wait_timeout = -1;        /* cause thread to be blocked   */
                                            /* indef. till sem. is cleared  */
  dos_rc = DOSSEMWAIT(sem_handle,inf_wait_timeout);
  if (dos_rc != AA_OK) {
    SHOW_DOS_ERR();
    sys_err = TRUE;
  }
}

void
clear_screen()
/* This function clears the screen to prepare to display message received.   */
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
/* This function sets the cursor position at the specified row and column.  */
int row_no, col_no;

{
  dos_vio_rc = VIOSETCURPOS(row_no, col_no,0);
                                            /* vio call to set cursor pos   */
  if (dos_vio_rc != 0)  {
    SHOW_VIO_ERR();
    sys_err = TRUE;                         /* handle any error             */
  }
}

