/************************************************************************/
/*                                                                      */
/*   MODULE NAME: CMSTOP.C                                              */
/*                                                                      */
/*   DESCRIPTIVE NAME: SAMPLE PROGRAM TO STOP COMMUNICATIONS MANAGER    */
/*                     OS/2 EXTENDED SERVICES Version 1.0               */
/*                                                                      */
/*   COPYRIGHT:  (C) COPYRIGHT IBM CORP. 1991                           */
/*               LICENSED MATERIAL - PROGRAM PROPERTY OF IBM            */
/*               ALL RIGHTS RESERVED                                    */
/*                                                                      */
/*   FUNCTION: The sample program will use the API interface to stop    */
/*             the IBM Communications Manager.  It will call the        */
/*             CmkDeactivateService entry point with the stop type      */
/*             selected.                                                */
/*                                                                      */
/*              Uses the following Communication Manager Verb:          */
/*                                                                      */
/*                CmkDeactivateService                                  */
/*                                                                      */
/*   NOTE:     The program assumes a SOFT stop unless a parameter       */
/*             is included that starts with an 'H',  and then a         */
/*             HARD stop is issued.   A successful return code          */
/*             from CmkDeactivate does not mean that CM is stopped,     */
/*             only that the request has been received.                 */
/*                                                                      */
/*   MODULE TYPE = IBM Personal Computer C/2 Compiler Version 1.1       */
/*                 (Compiler options - /AL /Zd /Od)                     */
/*                                                                      */
/*   PREREQS = Requires CM DLL file "REMAPI.DLL" at runtime.            */
/*                                                                      */
/************************************************************************/
#include <os2def.h>
#define INCL_DOS
#include <bsedos.h>

  /****  CmkDeactivateService - Deactivate the Communication Manager  */
  /*                                                                  */
  /*     This command will stop the IBM Communications Manager        */
  /*                                                                  */
  /********************************************************************/

USHORT APIENTRY CmkKernelReg(USHORT,          /* Reserved                 */
                             PSZ,             /* Reserved                 */
                             PSZ);            /* Application Name         */

USHORT APIENTRY CmkDeactivateService(USHORT,  /* type of stop             */
                                     ULONG,   /* service to deactivate    */
                                     PVOID,   /* Reserved                 */
                                     USHORT); /* Reserved                 */

#define CMK_ALL_FEATURES     1                /* autostartable features   */

/* StopType codes */
#define CMK_SOFT             0                /* soft stop                */
#define CMK_HARD             1                /* hard stop                */

#define CMK_SUCCESSFUL                  0
#define CMK_ERR_INVALID_SERVICE        22
#define CMK_ERR_SYSTEM_ERROR           23
#define CMK_ERR_CM_NOT_ACTIVE          37
#define CMK_ERR_NOT_REGISTERED         41

#define CMKDEACTIVATESERVICE_ERROR "ERROR in CmkDeactivateService.\r\n"



main(argc, argv)
   int argc;
   char *argv[];
{

  unsigned short int rc;
  unsigned short int stop_type;
  unsigned short dummy;


     /***********************************************************/
     /*                                                         */
     /*    Determine if parameter starts with an 'H'            */
     /*    If yes then we will stop HARD                        */
     /*                                                         */
     /***********************************************************/

  stop_type = CMK_SOFT;
  if (argc >= 2) {                            // Is there a parameter ?
    if (toupper(argv[1][0]) == 'H') {         // Is first char 'h' or 'H'
      stop_type = CMK_HARD;                   // Yes then HARD STOP
    } /* endif */
  } /* endif */

     /***********************************************************/
     /**                                                        */
     /**   Call API to Stop Communication Manager               */
     /**                                                        */
     /***********************************************************/
  rc = 0;
  if (!CmkKernelReg((USHORT)NULL, (PSZ)NULL, "cmstop")) {
     rc = CmkDeactivateService (stop_type, (ULONG)CMK_ALL_FEATURES, 0L, 0);
  } /* endif */

     /************************************************************/
     /*    If return code is non zero then and error occured     */
     /*                                                          */
     /*    If a Zero is returned the command request was accepted*/
     /*    but it does not mean Communications have stopped      */
     /************************************************************/

  if (rc)
  {
    DosWrite (2,CMKDEACTIVATESERVICE_ERROR,
                sizeof(CMKDEACTIVATESERVICE_ERROR),
                &dummy);
    return rc;
  }

}


