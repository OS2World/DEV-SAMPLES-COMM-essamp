/****************************************************************************/
/*                                                                          */
/*   MODULE NAME      : LUASAMP.C                                           */
/*                                                                          */
/*   DESCRIPTIVE NAME : LUA C SAMPLE PROGRAM FOR IBM EXTENDED SERVICES      */
/*                      FOR OS/2                                            */
/*                                                                          */
/*   COPYRIGHT        : (C) COPYRIGHT IBM CORP. 1989, 1990, 1991            */
/*                      LICENSED MATERIAL - PROGRAM PROPERTY OF IBM         */
/*                      ALL RIGHTS RESERVED                                 */
/*                                                                          */
/*   FUNCTION         : This program                                        */
/*                      - Issues an SLI_OPEN to establish an LU_LU session. */
/*                      - Issues an SLI_SEND to transmit data to the host.  */
/*                      - Issues an SLI_RECEIVE to get data from the host.  */
/*                      - Issues an SLI_SEND to transmit a response.        */
/*                      - Issues an SLI_CLOSE to end the LU_LU session.     */
/*                                                                          */
/*   GENERAL SERVICE                                                        */
/*     VERBS USED     : CONVERT - Translates data between ASCII and EBCDIC. */
/*                                                                          */
/*   MODULE TYPE      : C (Compiles with large memory model)                */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/*          Standard include files from 'C'                                 */
/****************************************************************************/
#include <STDIO.H>   /* declarations and definitions for stream I/O routines*/
#include <STDDEF.H>  /* definitions of common pointers, variables, & types  */
#include <STRING.H>  /* string manipulation function declarations           */
#include <DOS.H>     /* DOS interface declarations and definitions          */

/****************************************************************************/
/*          Include file from OS/2 Dev. Tkt.                                */
/****************************************************************************/
#include <DOSCALLS.H>           /* DOS API functions                        */

/****************************************************************************/
/*               Include files from OS/2 Comms Mgr                          */
/****************************************************************************/
#include "LUA_C.H"              /* LUA defines and struct include file      */
#include "ACSSVCC.H"            /* Common Srvs defines and struct incl. file*/

/****************************************************************************/
/*                       Definitions                                        */
/****************************************************************************/

/* ClrLuaVerb() is used to set the lua verb record to 0.                     */
#define ClrLuaVerb() memset(&LuaVerb,(unsigned char)'\0',sizeof(LUA_VERB_RECORD))

#define byte  unsigned char      /* used to allow ease of declarations of   */
#define word  unsigned int       /*        other variables.                 */

/****************************************************************************/
/*                  Externals    Declarations                               */
/****************************************************************************/
extern far pascal SLI (LUA_VERB_RECORD far *);

/****************************************************************************/
/*                Function Calls definitions                                */
/****************************************************************************/
unsigned  pascal  Sli_Open();                 /* Sends open session request */
unsigned  pascal  Sli_Send_Data();            /* Sends data to server       */
unsigned  pascal  Sli_Receive();              /* Receives response from host*/
unsigned  pascal  Sli_Send_Response();        /* Sends acknowledgement      */
unsigned  pascal  Sli_Close(unsigned char);   /* Sends close session request*/

/* Error routines                                                           */
void      pascal  error1(unsigned short,unsigned long);
void      pascal  error2(unsigned short,unsigned long,unsigned int);

/* Conversion routines                                                      */
unsigned  pascal  ascii_ebcdic(unsigned char far *,unsigned);
unsigned  pascal  ebcdic_ascii(unsigned char far *,unsigned);

/*****************************************************************************/
/* Data structures and pointers for LUA SLI interface and Common Svcs Inter.*/
/*****************************************************************************/
LUA_VERB_RECORD      LuaVerb;                       /* SLI API Verb Record   */
LUA_VERB_RECORD far *LuaVerbPtr = &LuaVerb;

struct convert    ConvertVerb;                      /* Conversion structure  */

struct ru_sli_open {                                /* INITSELF Command      */
                     byte iself_rq_01_hdr[3];       /* Structure             */
                     byte iself_rq_01_f0;
                     byte iself_rq_01_mode[8];
                     byte iself_rq_01_ty;
                     byte iself_rq_01_n_l;
                     byte iself_rq_01_plu_n[8];
                     byte iself_rq_01_rid_l;
                     byte iself_rq_01_pw_l;
                     byte iself_rq_01_ud_l;

                   } InitSelfRU = {
                                 { 0x01,0x06,0x81 },  /* Initializes INITSELF*/
                                 0x00,
                                 {'L','U','A','7','6','8','R','U'},
                                 0xF3,
                                 0x08,
                                 {'V','T','A','M','P','G','M',' '},
                                 0x00,
                                 0x00,
                                 0x00
                               };

unsigned char *TestData    = "TEST#SENDING#DATA#TO#HOST";
unsigned char DataBuffer[265];                /* Receive Data Buffer         */
unsigned long UserRamSem   = 0x00000000;      /* Posting semaphore           */
unsigned long LU_SessionID = 0x00000000;      /* LU_LU Session ID            */
unsigned char *LU_Name     = "LUA1    ";      /* it must be up to 8 byte     */
char     SavedSeqNum[2]    = { 0x00, 0x00};   /* Used to save sequence number*/


/****************************************************************************/
/****************************************************************************/
/* Main function      LUA Sample conversation code                          */
/****************************************************************************/

main()
{
   unsigned rc = 0;                /* Return code */

   printf("\nOpening communication with SLI interface....");
   rc = Sli_Open();
   if (rc == LUA_OK)
     {
      printf("\nSLI interface opened and Init_self sent to the host.");
      rc = Sli_Send_Data();
      if (rc == LUA_OK)
        {
         printf("\nTest data sent to host.  Waiting for host data.");
         rc = Sli_Receive();
         if (rc == LUA_OK)
           {
            printf("\nHost data received. Responding to host.");
            rc = Sli_Send_Response();
            if (rc == LUA_OK)
              {
               printf("\nResponse sent. Preparing to close sesssion.");
               rc = Sli_Close(0);
              }                     /* End if (Sli_Send_Response rc==LUA_OK) */
           }                        /* End if (Sli_Receive  rc==LUA_OK)      */
        }                           /* End if (Sli_Send_Data rc==LUA_OK)     */
      if ((rc != LUA_OK ) && (rc != LUA_SESSION_FAILURE))
        {
         rc = Sli_Close(1);         /* Close communication in ab_end         */
         printf("\nQuit from LUA conversation due to an error. Ab-ended.\n");
        }
      else if (rc != LUA_SESSION_FAILURE)
             {
              printf("\nSLI interface closed with no errors.");
             }
           else
             {
              printf("\nThe LU_LU session has failed due to an error.");
             }
     }                              /* End if (Sli_Open rc==LUA_OK)          */
     else
       {
         printf("\nQuit from LUA conversation due to an error. Ab-ended\n");
       }
}  /* end main() */


/****************************************************************************/
/****************************************************************************/
/* Function : SLI_OPEN                                                      */
/* Purpose  : Open a Session with the host using the SLI_OPEN verb and an   */
/*            INITSELF command.                                             */
/****************************************************************************/
unsigned pascal  Sli_Open()
{
 unsigned rc;                                       /* return code          */
 ClrLuaVerb();                                      /* set record to 0      */

 /******************************************************************/
 /* Set the required fields for SLI_OPEN: verb, verb length,       */
 /*     opcode, luname, posthandle, and init type.                 */
 /* Set the data pointer and data length to reflect the INITSELF   */
 /*     command to be sent as part of the SLI_OPEN.                */
 /******************************************************************/
 LuaVerb.common.lua_verb = LUA_VERB_SLI;
 LuaVerb.common.lua_verb_length = sizeof(struct LUA_COMMON) + 4;
 LuaVerb.common.lua_opcode = LUA_OPCODE_SLI_OPEN;
 memcpy(LuaVerb.common.lua_luname,LU_Name,strlen(LU_Name));
 LuaVerb.common.lua_data_length = sizeof(InitSelfRU);
 LuaVerb.common.lua_data_ptr = (char far *) &InitSelfRU;
 LuaVerb.common.lua_post_handle = (unsigned long) &UserRamSem;
 LuaVerb.specific.open.lua_init_type = LUA_INIT_TYPE_SEC_IS;

 /* Convert the InitSelfRU PluName to EBCDIC */
 rc = ascii_ebcdic(InitSelfRU.iself_rq_01_plu_n,
                   sizeof(InitSelfRU.iself_rq_01_plu_n) );
 /* If conversion ok */
 if (rc == SV_OK)
  {    /* then convert the InitSelfRU ModeName */
    rc = ascii_ebcdic(InitSelfRU.iself_rq_01_mode,
                      sizeof(InitSelfRU.iself_rq_01_mode) );

   /* If conversion ok */
    if (rc == SV_OK)
     {
      /**********************************************************/
      /* Call the SLI API.  Check for completion of call.       */
      /* If call is in progress, wait for semaphore to post     */
      /* completion.  If not successfully completed, call the   */
      /* error routine to display an error message.  Else save  */
      /* Session ID to use in subsequent verbs.                 */
      /**********************************************************/
       SLI(LuaVerbPtr);

       if (LuaVerb.common.lua_prim_rc == LUA_IN_PROGRESS)
         DOSSEMWAIT((unsigned long) &UserRamSem, (long) -1);

       if (LuaVerb.common.lua_prim_rc != LUA_OK)
         error2(LuaVerb.common.lua_prim_rc,LuaVerb.common.lua_sec_rc,
                LuaVerb.common.lua_opcode);
       else
         LU_SessionID = LuaVerb.common.lua_sid;

        rc = LuaVerb.common.lua_prim_rc;

     } /* end if */
  } /* end if */

  return(rc);                                 /* Send return code to main */
} /* End Sli_Open */


/****************************************************************************/
/****************************************************************************/
/* Function : SLI_SEND                                                      */
/* Purpose  : Send data to the host on LU Normal flow.                      */
/****************************************************************************/
unsigned pascal  Sli_Send_Data()
{
 unsigned rc;                                       /* return code          */
 ClrLuaVerb();                                      /* set record to 0      */

 /******************************************************************/
 /* Set the required fields for SLI_SEND request: verb, verb       */
 /*     length, opcode, sid, data pointer, data length,            */
 /*     posthandle, message type, certain rh bits.                 */
 /******************************************************************/
 LuaVerb.common.lua_verb = LUA_VERB_SLI;
 LuaVerb.common.lua_verb_length = sizeof(struct LUA_COMMON) + 2;
 LuaVerb.common.lua_opcode = LUA_OPCODE_SLI_SEND;
 LuaVerb.common.lua_sid = LU_SessionID;
 LuaVerb.common.lua_data_length = strlen(TestData);
 LuaVerb.common.lua_data_ptr = (char far *) TestData;
 LuaVerb.common.lua_post_handle = (unsigned long) &UserRamSem;
 LuaVerb.common.lua_rh.ri   = 1;
 LuaVerb.common.lua_rh.dr1i = 1;
 LuaVerb.common.lua_rh.bbi  = 1;
 LuaVerb.common.lua_rh.cdi  = 1;
 LuaVerb.common.lua_message_type = LUA_MESSAGE_TYPE_LU_DATA;

 /* Convert the Data to EBCDIC */
 rc = ascii_ebcdic(LuaVerb.common.lua_data_ptr,
                   LuaVerb.common.lua_data_length) ;

 /* If conversion ok */
 if (rc == SV_OK)
 {
   /**********************************************************/
   /* Call the SLI API.  Check for completion of call.       */
   /* If call is in progress, wait for semaphore to post     */
   /* completion.  If not successfully completed, call the   */
   /* error routine to display an error message.             */
   /**********************************************************/
   SLI(LuaVerbPtr);

   if (LuaVerb.common.lua_prim_rc == LUA_IN_PROGRESS)
     DOSSEMWAIT((unsigned long) &UserRamSem, (long) -1);

   if (LuaVerb.common.lua_prim_rc != LUA_OK)
     error2(LuaVerb.common.lua_prim_rc,LuaVerb.common.lua_sec_rc,
            LuaVerb.common.lua_opcode);

   rc = LuaVerb.common.lua_prim_rc;
 }
 return(rc);                                   /* send return code to main */

} /* End Sli_Send_Data */


/****************************************************************************/
/****************************************************************************/
/* Function : SLI_RECEIVE                                                   */
/* Purpose  : Receive a message from the host on LU normal flow.            */
/****************************************************************************/
unsigned pascal  Sli_Receive()
{
 unsigned rc;                                       /* return code          */
 ClrLuaVerb();                                      /* set record to 0      */

 /******************************************************************/
 /* Set the required fields for SLI_RECEIVE: verb, verb length,    */
 /*     opcode, sid, data pointer, max length, posthandle,         */
 /*     and a flag1 flow bit.                                      */
 /******************************************************************/
 LuaVerb.common.lua_verb = LUA_VERB_SLI;
 LuaVerb.common.lua_verb_length = sizeof(struct LUA_COMMON);
 LuaVerb.common.lua_opcode = LUA_OPCODE_SLI_RECEIVE;
 LuaVerb.common.lua_sid = LU_SessionID;
 LuaVerb.common.lua_max_length   = sizeof(DataBuffer) ;
 LuaVerb.common.lua_data_ptr     = (char far *) DataBuffer;
 LuaVerb.common.lua_post_handle  = (unsigned long) &UserRamSem;
 LuaVerb.common.lua_flag1.lu_norm = 1;

 /**********************************************************/
 /* Call the SLI API.  Check for completion of call.       */
 /* If call is in progress, wait for semaphore to post     */
 /* completion.  If not successfully completed, call the   */
 /* error routine to display an error message.  Otherwise  */
 /* translate the data and save the sequence number for    */
 /* the response.                                          */
 /**********************************************************/
 SLI(LuaVerbPtr);

 if (LuaVerb.common.lua_prim_rc == LUA_IN_PROGRESS)
   DOSSEMWAIT((unsigned long) &UserRamSem, (long) -1);

 if (LuaVerb.common.lua_prim_rc != LUA_OK)
  {
   error2(LuaVerb.common.lua_prim_rc,LuaVerb.common.lua_sec_rc,
          LuaVerb.common.lua_opcode);
   rc = LuaVerb.common.lua_prim_rc;
  }
 else
  {
       rc = ebcdic_ascii(LuaVerb.common.lua_data_ptr,
                         LuaVerb.common.lua_data_length);
       SavedSeqNum[0] = LuaVerb.common.lua_th.snf[0];
       SavedSeqNum[1] = LuaVerb.common.lua_th.snf[1];
  }

 return(rc);                                    /* send return code to main */

} /* End Sli_Receive */


/****************************************************************************/
/****************************************************************************/
/* Function : SLI_SEND_RESPONSE                                             */
/* Purpose  : Send a response to LU Normal data to the host.                */
/****************************************************************************/
unsigned pascal  Sli_Send_Response()
{
 unsigned rc;                                       /* return code          */
 ClrLuaVerb();                                      /* set record to 0      */

 /******************************************************************/
 /* Set the required fields for SLI_SEND response: verb, verb      */
 /*     length, opcode, sid, data pointer, data length, th snf     */
 /*     posthandle, message type, certain rh bits and a flag1 flow.*/
 /******************************************************************/
 LuaVerb.common.lua_verb = LUA_VERB_SLI;
 LuaVerb.common.lua_verb_length = sizeof(struct LUA_COMMON) + 2;
 LuaVerb.common.lua_opcode = LUA_OPCODE_SLI_SEND;
 LuaVerb.common.lua_sid = LU_SessionID;
 LuaVerb.common.lua_post_handle  = (unsigned long) &UserRamSem;
 LuaVerb.common.lua_th.snf[0] = SavedSeqNum[0];
 LuaVerb.common.lua_th.snf[1] = SavedSeqNum[1];
 LuaVerb.common.lua_rh.dr1i = 1;
 LuaVerb.common.lua_flag1.lu_norm = 1;
 LuaVerb.common.lua_message_type = LUA_MESSAGE_TYPE_RSP;

 /**********************************************************/
 /* Call SLI API.  Check for completion of call.           */
 /* If call is in progress, wait for semaphore to post     */
 /* completion.  If not successfully completed, call for   */
 /* error routine to be performed.                         */
 /**********************************************************/
 SLI(LuaVerbPtr);

 if (LuaVerb.common.lua_prim_rc == LUA_IN_PROGRESS)
   DOSSEMWAIT((unsigned long) &UserRamSem, (long) -1);

 if (LuaVerb.common.lua_prim_rc != LUA_OK)
     error2(LuaVerb.common.lua_prim_rc,LuaVerb.common.lua_sec_rc,
            LuaVerb.common.lua_opcode);

 rc = LuaVerb.common.lua_prim_rc;

 return(rc);                                    /* send return code to main */

} /* End Sli_Send_Response */


/****************************************************************************/
/****************************************************************************/
/* Function : SLI_CLOSE                                                     */
/* Purpose  : Issue an SLI_CLOSE to end the session.                        */
/****************************************************************************/
unsigned pascal Sli_Close(unsigned char CloseType)
{
 unsigned rc;                                       /* return code          */
 ClrLuaVerb();                                      /* set record to 0      */

 /******************************************************************/
 /* Set the required fields for SLI_CLOSE: verb, verb length       */
 /*     opcode, sid, posthandle, and flag1 close abend.            */
 /******************************************************************/
 LuaVerb.common.lua_verb = LUA_VERB_SLI;
 LuaVerb.common.lua_verb_length = sizeof(struct LUA_COMMON);
 LuaVerb.common.lua_opcode = LUA_OPCODE_SLI_CLOSE;
 LuaVerb.common.lua_sid = LU_SessionID;
 LuaVerb.common.lua_post_handle = (unsigned long) &UserRamSem;
 LuaVerb.common.lua_flag1.close_abend = CloseType;

 /**********************************************************/
 /* Call the SLI API.  Check for completion of call.       */
 /* If call is in progress, wait for semaphore to post     */
 /* completion.  If not successfully completed, call the   */
 /* error routine to display an error message.             */
 /**********************************************************/
 SLI(LuaVerbPtr);

 if (LuaVerb.common.lua_prim_rc == LUA_IN_PROGRESS)
   DOSSEMWAIT((unsigned long) &UserRamSem, (long) -1);

 if (LuaVerb.common.lua_prim_rc != LUA_OK)
     error2(LuaVerb.common.lua_prim_rc,LuaVerb.common.lua_sec_rc,
            LuaVerb.common.lua_opcode);

 rc = LuaVerb.common.lua_prim_rc;

 return(rc);                                     /* send return code to main */

} /* End Sli_Close */


/****************************************************************************/
/****************************************************************************/
/* Function : ASCII_EBCDIC                                                  */
/* Purpose  : Convert selected data from ASCII to EBCDIC representation.    */
/****************************************************************************/
unsigned pascal  ascii_ebcdic(unsigned char far *ptr, unsigned len)
{
 /***************************************************************/
 /* Set opcode for conversion, direction for ASCII to EBCDIC.   */
 /* Select character set.  Provide length of data to be         */
 /* converted, as well as source and target addresses.          */
 /***************************************************************/
 ConvertVerb.opcode = SV_CONVERT;
 ConvertVerb.direction = SV_ASCII_TO_EBCDIC;
 ConvertVerb.char_set = SV_AE;
 ConvertVerb.len = len;
 ConvertVerb.source = ptr;
 ConvertVerb.target = ptr;

 /****************************************************************/
 /* Send convert request to host and check return code for error */
 /****************************************************************/
 ACSSVC  ((long) &ConvertVerb);

 if (ConvertVerb.primary_rc != SV_OK)
   error1(ConvertVerb.primary_rc,ConvertVerb.secondary_rc);

 return(ConvertVerb.primary_rc);
} /* end ascii_ebcdic */


/****************************************************************************/
/****************************************************************************/
/* Function : EBCDIC_ASCII                                                  */
/* Purpose  : Convert selected data from EBCDIC to ASCII representation.    */
/****************************************************************************/
unsigned pascal  ebcdic_ascii(unsigned char far *ptr, unsigned len)
{
 /***************************************************************/
 /* Set opcode for conversion, direction for EBCDIC to ASCII.   */
 /* Select character set.  Provide length of data to be         */
 /* converted, as well as source and target addresses.          */
 /***************************************************************/
 ConvertVerb.opcode = SV_CONVERT;
 ConvertVerb.direction = SV_EBCDIC_TO_ASCII;
 ConvertVerb.char_set = SV_AE;
 ConvertVerb.len = len;
 ConvertVerb.source = ptr;
 ConvertVerb.target = ptr;

 /****************************************************************/
 /* Send convert request to host and check return code for error */
 /****************************************************************/
 ACSSVC  ((long) &ConvertVerb);

 if (ConvertVerb.primary_rc != SV_OK)
   error1(ConvertVerb.primary_rc,ConvertVerb.secondary_rc);

 return(ConvertVerb.primary_rc);
} /* end ebcdic_ascii */


/****************************************************************************/
/****************************************************************************/
/* Procedure : ERROR1                                                       */
/* Purpose   : Display return codes for conversion errors.                  */
/****************************************************************************/
void pascal  error1(unsigned short primary_rc, unsigned long secondary_rc)
{
 printf("\nAn error has occurred during the conversion process.");
 printf("\nThe primary return code is: %x .",primary_rc);
 printf("\nThe secondary return code is: %lx .",secondary_rc);
} /* End Error1 */


/****************************************************************************/
/****************************************************************************/
/* Procedure : ERROR2                                                       */
/* Purpose   : Display return codes for unsuccessful function calls.        */
/****************************************************************************/
void pascal  error2(unsigned short primary_rc,unsigned long  secondary_rc,
                    unsigned int verb)
{
 printf("\nAn error has occurred with SLI interface. Verb opcode: %x",
         verb);
 printf("\nThe primary return code is: %x .",primary_rc);
 printf("\nThe secondary return code is: %lx .",secondary_rc);
} /* End Error2 */

