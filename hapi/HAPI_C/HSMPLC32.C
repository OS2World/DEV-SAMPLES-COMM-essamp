/********************************************************************/
/*                                                                  */
/* FILE NAME: HSMPLC32.C                                            */
/*                                                                  */
/* MODULE NAME= HSMPLC32.C                                          */
/*                                                                  */
/* DESCRIPTIVE NAME= 32 BIT C SAMPLE PROGRAM FOR EHLLAPI            */
/*                                                                  */
/*   Displays EHLLAPI and session information.                      */
/*   Writes string to host.                                         */
/*   Searches for written string on host.                           */
/*   Displays host session screen.                                  */
/*   Manipulates the Presentation Manager properties of             */
/*   the emulator session to: change window title name, switch      */
/*   list name, make window invisible, query window status,         */
/*   window coordinates, change window size, and restore the        */
/*   emulator session window to its original conditions.            */
/*   Next, the structured field functions are used.  The            */
/*   communications buffer is queried, the read and write buffers   */
/*   allocated, a connection is initiated to the communications     */
/*   buffer, and an asynchronus read structured field is issued     */
/*   disabling the inbound host.  Then, the sendkey function is     */
/*   used to send the command 'IND$FILE PUT SF_TEST EXEC A'         */
/*   to the host which puts a non-existent file from the            */
/*   PC to the host using a structured field.  Next, a get          */
/*   completion request is issued to determine if the               */
/*   previous asynchronus read structured field is completed,       */
/*   Upon completion, a synchronus write structured field is        */
/*   issued, the communications buffers are de-allocated, and       */
/*   then a disconnect from structured field is issued.             */
/*                                                                  */
/*   COPYRIGHT:  XXXXXXXXX  (C) COPYRIGHT IBM CORP. 1987,1988,1991  */
/*               LICENSED MATERIAL - PROGRAM PROPERTY OF IBM        */
/*               ALL RIGHTS RESERVED                                */
/*                                                                  */
/* NOTES=                                                           */
/*    Compile with default options                                  */
/*                                                                  */
/*                                                                  */
/* MODULE TYPE:  IBM C Set/2   (Toronto 32 bit compiler)            */
/*                                                                  */
/*                                                                  */
/*   CHANGE ACTIVITY =                                              */
/*   CS2 => Indicates changes(C) and additions(A) to support        */
/*          IBM C Set/2 32 bit applications.                        */
/*                                                                  */
/**********************-END OF SPECIFICATIONS-***********************/

/********************************************************************/
/********************** BEGIN INCLUDE FILES   ***********************/
/********************************************************************/

#define E32TO16                        /* Setup header file for CS2A*/
                                       /* IBM C Set/2 compiler      */

#include "stdio.h"

#include "string.h"

#include "hapi_c.h"                    /* Get EHLLAPI include file  */


/********************************************************************/
/************************ FUNCTION EXTERNS **************************/
/********************************************************************/


  extern unsigned VIO16WRTCELLSTR (    /*                       CS2C*/
        CHAR PTR16,               /* String to be written           */
        USHORT,                   /* Length of string               */
        USHORT,                   /* Starting pos for output (row)  */
        USHORT,                   /* Starting pos for output (col)  */
        USHORT);                  /* Vio Handle                     */
  extern USHORT DOS16SLEEP(LONG);      /*                       CS2C*/

#pragma linkage (VIO16WRTCELLSTR, far16 pascal)               /*CS2A*/
#pragma linkage (DOS16SLEEP, far16 pascal)                    /*CS2A*/

#pragma stack16 (4096)                 /* used as 16 bit stack  CS2A*/
#define hllc hllapi                    /* use only direct calls CS2A*/


/*********************************************************************/
/****************************  DEFINES *******************************/
/*********************************************************************/

#define MAX_DATA_SIZE 3840              /* The maximum data          */
                                        /* size for this             */
                                        /* application.              */
#define EABS 0x80                       /* Extended attribute        */
                                        /* bit.                      */
#define PSS 0x40                        /* Programmed Symbol         */
                                        /* Set bit.                  */
#define TIMEOUT  5000L                  /* semaphore timeout         */

/*********************************************************************/
/******************* BEGIN STATIC VARIABLES    ***********************/
/*********************************************************************/

unsigned char press_ent_msg[] =
{
  "\n\nPress ENTER to continue..."
}
;
unsigned char blank_screen[] =             /* Poor mans blank screen  */
{
 "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n"

}
;
unsigned char dft_sess = 0;               /* Session to write str CS2C*/
                                          /* to and search from       */
unsigned char host_text[] =
{
  "EHLLAPI"                               /* String to send to        */
}                                         /* host and to search for   */
;
unsigned char invis_text[] =
{
  "INVISIBLE WINDOW WRITE"                /* String to send to host   */
}
;
unsigned char home_key[] =
{
  "@0"                                    /* The Home Key             */
}
;
unsigned char setparm_text[] =
{
  "NOATTRB EAB XLATE"                    /* String used for set parms */
}
;
unsigned char test_name[ ] =
{
  "Sample_Test_Name"                    /* window title test name     */
}
;
unsigned char sf_test_str[] =           /* command string to create a */
{                                       /* structured field           */
  "IND$FILE PUT SF_TEST EXEC A@E"
}
;

/******************** EHLLAPI variables *******************************/

USHORT hfunc_num;                       /* EHLLAPI function numberCS2C*/

unsigned char hdata_str[MAX_DATA_SIZE]; /* EHLLAPI data string        */

USHORT hds_len;                         /* EHLLAPI data string lenCS2C*/

USHORT hrc;                             /* EHLLAPI return code    CS2C*/

/******************* structured field variables ***********************/

USHORT   sf_doid;                       /* destination origin id  CS2C*/
unsigned char * sf_buffer_address;      /* 4 byte pointer to buffer   */
SHORT    optimal_buffer_len;            /* optimal buffer length  CS2C*/
SHORT    optimal_in_buffer_len;         /* optimal inbound buffer CS2C*/
unsigned char * write_buffer;           /* write buffer address       */
SHORT    optimal_out_buffer_len;        /* optimal outbound bufferCS2C*/
unsigned char * status_sem;             /* status notification semaph.*/
unsigned char * sf_asem;                /* 4 byte pointer to semaphore*/
unsigned char * read_buffer;            /* read buffer address        */
                                        /* DDM Base Type Query Reply  */
char query_reply[12] = {0x00,0x0c,0x81,0x95,0x00,0x00,0x01,0x00,0x01,0x00,0x01,0x01};



/*******************************************************************/
/*************************** BEGIN CODE ****************************/
/*******************************************************************/


/*******************************************************************/
/* MAIN - Main code calls routines to do real work.                */
/*                                                                 */
/* INPUT                                                           */
/*                                                                 */
/* OUTPUT                                                          */
/*                                                                 */
/*                                                                 */
/*******************************************************************/
main(){                                         /* Entry from DOS. */

  int  rc;                              /* return code             */
  char key = 'a' ;                      /* input character         */

  setvbuf(stdout,NULL,_IONBF,0);        /* Set buffers to nobufCS2A*/
  printf(blank_screen);                 /* Clear the screen        */
  rc = disp_ehllapi_info();             /* Call routine to         */
                                        /* display EHLLAPI info    */

  if (rc == 0){
    printf(press_ent_msg);
    fgetc(stdin);                       /*Make user press ENTERCS2C*/
    rc = disp_session_info();           /* Call routine to display */
                                        /* Host session info       */
  }

  if (rc == 0){

     if (dft_sess){                      /* At least 1 dft sessCS2C*/

       printf(blank_screen);             /* Clear the screen.      */
       printf("Press ENTER to send string '");
       printf(host_text);
       printf("' to session short name ");
       fputc(dft_sess,stdout);
       printf ("...");
       fgetc(stdin);                     /*Make user press EnteCS2C*/

       rc = write_str_2_host(host_text); /* Call routine to write  */
                                         /* string to host session */
     }
     else {

       printf("NO DFT SESSION SESSION STARTED.\n");
       rc = 1;                           /* Get out of program     */
     }
  }

  if (rc == 0){

     printf("Press ENTER to search for string '");
     printf(host_text);
     printf("' on Host Presentation Space...");
     fgetc(stdin);                       /*Make user press EnteCS2C*/
     rc = search_str_on_host();          /* Routine to search for  */
                                         /* string on host session */
  }

  if (rc == 0){

     printf("Press ENTER to display first 1920 bytes of Host presentation space...");
     fgetc(stdin);                       /*Make user press ENTERCS2C*/
     rc = disp_host_scr();               /* Call routine to display */
                                         /* Host session screen     */
  }

  if (rc == 0){

     printf("Press ENTER to change the title of the presentation space...");
     fgetc(stdin);                      /* Make user press ENTERCS2C*/
     rc = change_PS_window_name();      /* Call routine to          */
                                        /* change window name       */
  }

  if (rc == 0){

     printf("Press ENTER to change the switch list name...");
     fgetc(stdin);                      /* Make user press ENTERCS2C*/
     rc = change_switch_list_LT_name(); /* Call routine to          */
                                        /* change switch list name  */
  }

  if (rc == 0){

     printf("Press ENTER to query the Presentation\
 Manager window status...");
     fgetc(stdin);                       /* Make user press ENTERCS2C*/
     printf(blank_screen);               /* Clear the screen         */
     rc = query_PM_window_status();      /* Call routine to          */
                                         /* query window status      */
  }

  if (rc == 0){

     printf("Press ENTER to make the Presentation Manager window invisible...");
    fgetc(stdin);                         /* Make user press ENTERCS2C*/
    rc = make_PM_window_invisible();      /* Call routine to make the */
                                          /* PM window invisible      */
  }

  if (rc == 0){

     printf("Press ENTER to send string '");
     printf(invis_text);
     printf("' to session short name ");
     fputc(dft_sess,stdout);
     printf ("...");
     fgetc(stdin);                      /* Make user press ENTER CS2C */
     rc = write_str_2_host(invis_text); /* Call routine to write      */
                                        /* string to host session     */
  }

  if (rc == 0){

     printf("Press ENTER to display first 1920 bytes of Host presentation space...");
     fgetc(stdin);                     /* Make user press ENTER   CS2C*/
     rc = disp_host_scr();             /* Call routine to             */
                                       /* display Host session screen */
  }

  if (rc == 0){

     printf("Press ENTER to maximize the PM window and make it visible...");
     fgetc(stdin);                    /* Make user press ENTER    CS2C*/
     rc = make_PM_window_visible ();  /* Call routine to              */
                                      /* make window visible          */
  }

  if (rc == 0){

     printf("Press ENTER to disconnect from the Presentation Manager window...");
     fgetc(stdin);                        /* Make user press ENTERCS2C*/
     rc = disconnect_PM_window_service(); /* Call routine to          */
                                          /* disconnect PM window     */
  }

  if (rc == 0){

     printf("Press ENTER to restore the PM window name, ");
     printf("switch name, and window size...");
     fgetc(stdin);                        /* Make user press ENTERCS2C*/
     rc = reset_switch_and_window_name(); /* Call routine to reset    */
                                          /* window conditions        */
  }

  if (rc == 0){

     printf("The sample program continues with structured field EHLLAPI calls.\n");
     printf("The host session must be active and have access to\n");
     printf("the IND$FILE file transfer application.\n");
     printf("Do you wish to continue ? (y/n) \n");

     do {
        key = fgetc(stdin);               /* Get user input       CS2C*/
        }while ((key != 'Y') && (key != 'y') && (key != 'N') && (key != 'n'));
  }

  if ((key == 'Y') || (key == 'y')){

     printf(blank_screen);
     rc = reset_system();

     if (rc ==0) {
        printf("Query The Communications Buffer Size.\n");
        rc = query_com_buffer_size();
     }

    if (rc == 0) {
       printf("Allocate The Read Buffer.\n");
       optimal_buffer_len = optimal_in_buffer_len;
       rc = allocate_sf_buffer();
       read_buffer = sf_buffer_address;
    }

    if (rc == 0) {
       printf("Allocate The Write Buffer.\n");
       optimal_buffer_len = optimal_out_buffer_len;
       rc = allocate_sf_buffer();
       write_buffer =  sf_buffer_address;
    }

    if (rc == 0) {
       fflush(stdin);                       /* Flush input buffer CS2A*/
       printf(press_ent_msg);
       fgetc(stdin);                      /* Make user press ENTERCS2C*/
       printf(blank_screen);
       printf("\nConnect To The Communications Buffer.\n");
       rc = connect_structured_field();
    }

    if (rc == 0) {
       printf(press_ent_msg);
       fgetc(stdin);                      /* Make user press ENTERCS2C*/
       printf("\nPerform An Asynchronus Read Structured Field.\n");
       rc = read_sf_async();
    }


    if (rc == 0) {
       printf(press_ent_msg);
       fgetc(stdin);                      /* Make user press ENTERCS2C*/
       printf("\nCreating A Structured Field ...\n");
       rc = create_a_structured_field();
    }

    if (rc == 0) {
       printf(press_ent_msg);
       fgetc(stdin);                      /* Make user press ENTERCS2C*/
       printf("\nPerform a Get Asynchronus Completion Request.\n");
       rc = get_completion_request();
    }

    if (rc == 0) {
       printf(press_ent_msg);
       fgetc(stdin);                      /* Make user press ENTERCS2C*/
       printf("\nPerform Synchronus Write Structured Field.\n");
       rc = write_sf_sync();
    }

    if (rc == 0) {
       printf(press_ent_msg);
       fgetc(stdin);                      /* Make user press ENTERCS2C*/
       printf("\nReturn Communication Buffers To Memory.\n");
       rc = free_com_buffers();
    }


    if (rc == 0) {
       printf(press_ent_msg);
       fgetc(stdin);                      /* Make user press ENTERCS2C*/
       printf("\nDisconnect From Structured Field Communications.\n");
       rc = disconnect_structured_field();
    }

  }

  if (rc == 0){
     fflush(stdin);                       /* Flush input buffer   CS2A*/
     printf( "\nSAMPLE PROGRAM DONE.  To Exit Program Press ENTER..." );
     fgetc(stdin);                        /* Make user press ENTERCS2C*/
  }
}






/*********************************************************************/
/* DISP_EHLLAPI_INFO - CALLs EHLLAPI QUERY_SYSTEM and then displays  */
/*                     the requested info.                           */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

int disp_ehllapi_info(){

#pragma seg16(dsp_struct)               /*                       CS2A*/
  _Packed struct qsys_struct dsp_struct;/*                       CS2A*/

  _Packed struct qsys_struct * data_ptr;
                                        /* assign pointer        CS2C*/
                                        /* EHLLAPI data string.      */

  unsigned int rc = 0;                  /* return code               */
  data_ptr = &dsp_struct;               /*                       CS2A*/


  hfunc_num = HA_QUERY_SYSTEM;          /* Issue query system        */

  hllc(&hfunc_num, (char * )data_ptr, &hds_len, &hrc);/* Call EHLLAPICS2C*/

  if (hrc == HARC_SUCCESS){             /* If good rc                */

    printf("                       EHLLAPI INFORMATION\n\n");

    printf("  EHLLAPI version              : ");

    fputc(data_ptr->qsys_hllapi_ver,stdout);

    printf("\n");

    printf("  EHLLAPI level                : ");

    fputc(data_ptr->qsys_hllapi_lvl[0],stdout);

    fputc(data_ptr->qsys_hllapi_lvl[1],stdout);

    printf("\n");

    printf("  EHLLAPI release date         : ");

    wrt_str(data_ptr->qsys_hllapi_date, 6);

    printf("\n");

    printf("  EHLLAPI LIM version          : ");

    fputc(data_ptr->qsys_lim_ver,stdout);
    printf("\n");

    printf("  EHLLAPI LIM level            : ");

    fputc(data_ptr->qsys_lim_lvl[0],stdout);

    fputc(data_ptr->qsys_lim_lvl[1],stdout);

    printf("\n");

    printf("  EHLLAPI hardware base        : ");

    fputc(data_ptr->qsys_hardware_base,stdout);

    printf(" = ");

    if (data_ptr->qsys_hardware_base == 'Z'){

       printf("(See System model/submodel below)");

    }

    printf("\n");

    printf("  EHLLAPI CTRL program type    : ");

    fputc(data_ptr->qsys_ctrl_prog_type,stdout);

    printf(" = ");

    if (data_ptr->qsys_ctrl_prog_type == 'X'){

      printf("OS/2");

    }

    printf("\n");

    printf("  EHLLAPI sequence number      : ");

    fputc(data_ptr->qsys_seq_num[0],stdout);

    fputc(data_ptr->qsys_seq_num[1],stdout);

    printf("\n");

    printf("  EHLLAPI CTRL program version : ");

    fputc(data_ptr->qsys_ctrl_prog_ver[0],stdout);

    fputc(data_ptr->qsys_ctrl_prog_ver[1],stdout);

    printf("\n");

    printf("  EHLLAPI PC session name      : ");

    fputc(data_ptr->qsys_pc_sname,stdout);
    printf("\n");

    printf("  EHLLAPI extended error 1     : ");

    wrt_str(data_ptr->qsys_err1, 4);
    printf("\n");

    printf("  EHLLAPI extended error 2     : ");

    wrt_str(data_ptr->qsys_err2, 4);
    printf("\n");


    printf("  EHLLAPI system model/submodel: %02X%02X",
         data_ptr->qsys_sys_model, data_ptr->qsys_sys_submodel);

    printf(" HEX  ");
    if ((data_ptr->qsys_sys_model == 0xFC)
       && (data_ptr->qsys_sys_submodel == 0x00)){

      printf("= Model PC AT");
    }

    if ((data_ptr->qsys_sys_model == 0xFC)
       && (data_ptr->qsys_sys_submodel == 0x01)){

      printf("= Model PC AT ENHANCED");
    }

    if ((data_ptr->qsys_sys_model == 0xFC)
       && (data_ptr->qsys_sys_submodel == 0x02)){

      printf("= Model PC XT Model 286");
    }

    if ((data_ptr->qsys_sys_model == 0xFC)
       && (data_ptr->qsys_sys_submodel == 0x04)){

      printf("= Model 50");
    }

    if ((data_ptr->qsys_sys_model == 0xFC)
       && (data_ptr->qsys_sys_submodel == 0x05)){

      printf("= Model 60");
    }

    if ((data_ptr->qsys_sys_model == 0xF8)
       && (data_ptr->qsys_sys_submodel == 0x00)){

      printf("= Model 80");
    }

    if ((data_ptr->qsys_sys_model == 0xF8)
       && (data_ptr->qsys_sys_submodel == 0x09)){

      printf("= Model 70");
    }

    printf("\n");

    printf("  EHLLAPI National Language    : %d\n",
         data_ptr->qsys_pc_nls);

    printf("  EHLLAPI monitor type         : ");

    fputc(data_ptr->qsys_monitor_type,stdout);

    printf(" = ");
    if (data_ptr->qsys_monitor_type == 'M'){


      printf("PC MONOCHROME");
    }

    if (data_ptr->qsys_monitor_type == 'C'){

      printf("PC CGA");
    }

    if (data_ptr->qsys_monitor_type == 'E'){

      printf("PC EGA");
    }

    if (data_ptr->qsys_monitor_type == 'A'){

      printf("PS MONOCHROME");
    }

    if (data_ptr->qsys_monitor_type == 'V'){

      printf("PS 8512");
    }

    if (data_ptr->qsys_monitor_type == 'H'){

      printf("PS 8514");
    }

    if (data_ptr->qsys_monitor_type == 'U'){

      printf("UNKNOWN monitor type");
    }
    printf("\n");
  }

  else  {                                 /* Bad return code         */

    rc = hrc;
    error_hand(hfunc_num, rc);
  }

  return(rc);
}



/*********************************************************************/
/* DISP_SESSION_INFO - CALLs EHLLAPI QUERY funtions and then displays*/
/*                     the requested session info.                   */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

int disp_session_info(){                /* Routine to display        */
                                        /* Host session info         */

#pragma seg16(dsp_struct)               /*                       CS2A*/

  _Packed struct qses_struct * data_ptr;
                                        /* assign pointer        CS2C*/
                                        /* EHLLAPI data string.      */

#pragma seg16(sess_stuc)                /*                       CS2A*/
  _Packed struct qsst_struct sess_stuc; /* Query Session             */
                                        /* structure.                */

  unsigned int i;                       /* Array index               */
  unsigned int num_sess;                /* Number of session started */
  unsigned int rc = 0;                  /* return code               */
  data_ptr = (_Packed struct qses_struct *)hdata_str;/*          CS2A*/

  printf("\n\n\n\n\n\n\n\n\n\n\n\n");

  printf("                           SESSION INFO\n\n");


  hfunc_num = HA_QUERY_SESSIONS;        /* Issue query sessions      */

  hds_len = MAX_DATA_SIZE / 12 * 12;    /* Make sure len is          */
                                        /* multiple of 12            */

  hllc(&hfunc_num, (char *)data_ptr, &hds_len, &hrc);/* Call EHLLAPICS2C*/


  if (hrc == HARC_SUCCESS){             /* If good rc                */

    num_sess = hds_len;                 /* Number of sessions started*/

    printf("Number of started sessions = %d\n\n\n", num_sess);

    for(i = 0;((i < num_sess) && (rc == 0)); i++)
                                        /* LOOP thru queried sessions */
    {
      printf("Session number     : %d\n",i+1);
      printf("Session Long name  : ");
      wrt_str(data_ptr[i].qses_longname, 8);

      printf("\n");

      printf("Session Short name : ");
      wrt_str(&data_ptr[i].qses_shortname, 1);

      printf("\n");

      printf("Session Type       : ");
      wrt_str(&data_ptr[i].qses_sestype, 1);

      printf(" = ");
      if (data_ptr[i].qses_sestype == 'H')

      {
        printf("Host");

        if (!(dft_sess)){               /* First HOST not set alreaCS2C*/

          dft_sess = data_ptr[i].qses_shortname; /* Set the session to */
                                                 /*write string to     */
        }
      }
      if (data_ptr[i].qses_sestype == 'P'){

         printf("PC");
      }

      printf("\n");

      printf("Session PS size    : %d\n"
           ,data_ptr[i].qses_pssize);

      hfunc_num = HA_QUERY_SESSION_STATUS;/* Issue query session status */

      hds_len = 18;                       /* Set length                 */

      sess_stuc.qsst_shortname = data_ptr[i].qses_shortname;
                                        /* Set the session short name   */

      hllc(&hfunc_num, (char *)&sess_stuc, &hds_len, &hrc);       /*CS2C*/

      if (hrc == HARC_SUCCESS){         /* If good rc                   */

        printf("Session PS rows    : %d\n"
             ,sess_stuc.qsst_ps_rows);

        printf("Session PS columns : %d\n"
             ,sess_stuc.qsst_ps_cols);

        printf("Session type 2     : ");

        wrt_str(&sess_stuc.qsst_sestype, 1);

        printf(" = ");
        if (sess_stuc.qsst_sestype == 'F'){

          printf("5250");
        }

        if (sess_stuc.qsst_sestype == 'G'){

          printf("5250 Printer Session");
        }

        if (sess_stuc.qsst_sestype == 'D'){

          printf("DFT Host");
        }

        if (sess_stuc.qsst_sestype == 'P'){

          printf("PC");
        }

        printf("\n");

        printf("Session supports Extended attributes (EABs)? : ");

        if (sess_stuc.qsst_char & EABS){   /* if eabs on.             */

          printf("YES\n");
        }

        else                              /* no eabs                  */

        {
          printf("NO\n");
        }

        printf("Session supports Program Symbols (PSS)?      : ");

        if (sess_stuc.qsst_char & PSS){ /* if programmed symbol set on */

          printf("YES\n");
        }

        else                            /* no PSS                    */

        {
          printf("NO\n");

        }
        printf(press_ent_msg);
        fgetc(stdin);                   /* Make user press ENTERCS2C */
                                        /* ENTER.                    */
      }

      else                              /* Bad return code           */

      {
        rc = hrc;
        error_hand(hfunc_num, rc);
      }
    }

  }

  else                                  /* Bad return code           */

  {
    rc = hrc;
    error_hand(hfunc_num, rc);
  }

  return(rc);

}





/*********************************************************************/
/* WRITE_STR_2_HOST  - Connects to first session and writes home_key */
/*                     and string to host                            */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

int write_str_2_host(char  *in_str)     /* Call routine to           */
                                        /* write string to host      */
{

  unsigned int rc = 0;                  /* return code               */
  hdata_str[0] = dft_sess;              /* Set session id for connect*/

  hfunc_num = HA_CONNECT_PS;            /* Issue Connect             */
                                        /* Presentation Space        */
  hllc(&hfunc_num, (char * )hdata_str, &hds_len, &hrc);
                                        /* Call EHLLAPI          CS2C*/

  if (hrc == HARC_SUCCESS){             /* If good return code       */

    hfunc_num = HA_SENDKEY;             /* Issue sendkey             */

    strcpy(hdata_str, home_key);        /* String to send to Host    */

    hds_len = sizeof(home_key) - 1;     /* Set length of string      */
                                        /* minus null character      */

    hllc(&hfunc_num, (char * )hdata_str, &hds_len, &hrc);
                                        /* Call EHLLAPI          CS2C*/

    if (hrc == HARC_SUCCESS){           /* If good return code       */

       hfunc_num = HA_SENDKEY;          /* Issue sendkey             */

       strcpy(hdata_str,in_str);        /* String to send to Host    */

       hds_len = strlen(in_str) ;       /* Set length of string      */
                                        /* minus null character      */

       hllc(&hfunc_num, (char * )hdata_str, &hds_len, &hrc);
                                        /* Call EHLLAPI          CS2C*/

       if (hrc == HARC_SUCCESS){        /* If good return code       */

        printf ("Sent String to Host.\n\n\n");
      }

      else                              /* Bad return code           */
      {
        rc = hrc;                       /* Set return code           */
        error_hand(hfunc_num, rc);
      }

    }
    else
    {

      rc = hrc;                         /* Set return code           */
      error_hand(hfunc_num, rc);
    }
  }
  else
  {

    rc = hrc;                           /* Set return code           */
    error_hand(hfunc_num, rc);
  }
  return(rc);

}





/*********************************************************************/
/* SEARCH_STR_ON_HOST- Searches for string on host.                  */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

int search_str_on_host(){               /* Routine to search for     */
                                        /* string on host session    */

  unsigned int rc = 0;                  /* return code               */

  hfunc_num = HA_SEARCH_PS;             /* Issue search PS           */

  strcpy(hdata_str, host_text);      /* String to search for on host */

  hds_len = sizeof(host_text) - 1;      /* Set length of string      */
                                        /* minus null character      */

  hllc(&hfunc_num, (char * )hdata_str, &hds_len, &hrc);
                                        /* Call EHLLAPI          CS2C*/

  if (hrc == HARC_SUCCESS){             /* If good return code       */

    printf("Found string '");
    printf(host_text);
    printf("' at PS position %d.\n",hds_len);
    printf("(press CONTROL-ESCAPE to verify)\n\n\n");
  }

  else

  {
    rc = hrc;                           /* Set return code           */
    error_hand(hfunc_num, rc);
  }
  return(rc);

}



/*********************************************************************/
/* DISP_HOST_SCR - Displays first 1920 bytes of host screen.         */
/*                                                                   */
/*                                                                   */
/*********************************************************************/

int disp_host_scr()                     /* Routine to display        */
                                        /* host screen               */

{
  unsigned int rc = 0;                  /* return code               */

  hfunc_num = HA_SET_SESSION_PARMS;     /* Issue Set session Parms   */

  hds_len = sizeof(setparm_text) - 1;   /* Copy the first 1920 bytes */
                                        /* of presentation space     */

  hllc(&hfunc_num, setparm_text, &hds_len, &hrc);
                                        /* Call EHLLAPI          CS2C*/

  if (hrc == HARC_SUCCESS){             /* If good return code       */


     hfunc_num = HA_COPY_PS_TO_STR;     /* Issue Copy PS to string   */

     hds_len = MAX_DATA_SIZE;           /* Copy the first 1920 bytes */
                                        /* of presentation space     */

     hrc = 1;                           /* Set PS position to        */
                                        /* top,left corner           */

     hllc(&hfunc_num, hdata_str, &hds_len, &hrc);
                                        /* Call EHLLAPI          CS2C*/
     if (hrc == HARC_SUCCESS){          /* If good return code       */

      VIO16WRTCELLSTR (hdata_str, MAX_DATA_SIZE, 0, 0, 0);
                                        /* Write the string in color */

    }
    else                                /* Bad return code           */
    {
      rc = hrc;                         /* Set return code           */
      error_hand(hfunc_num, rc);
    }

  }
  else                                  /* Bad return code           */
  {
    rc = hrc;                           /* Set return code           */
    error_hand(hfunc_num, rc);
  }

  return(rc);

}




/*********************************************************************/
/* WRT_STR - Writes char sting to standard output.                   */
/*                                                                   */
/* INPUT                                                             */
/*                                                                   */
/* OUTPUT                                                            */
/*                                                                   */
/*********************************************************************/

wrt_str(text_ptr, x)                    /* Prints x number of        */
                                        /* characters                */
unsigned char * text_ptr;               /* Pointer to string         */
                                        /* to write                  */
unsigned int x;                         /* Number of bytes in string */

{
  unsigned int i;                       /* counter                   */

  for (i = 0; i < x; i++)
  {
    fputc(text_ptr[i],stdout);

  }
}




/*******************************************************************/
/*  QUERY_PM_WINDOW_STATUS - Query PM window status information    */
/*                                                                 */
/*                                                                 */
/*                                                                 */
/*******************************************************************/

int query_PM_window_status()
{
        int rc = 0;                          /* return code        */
#pragma seg16(connect_struct)         /*                       CS2A*/
        _Packed struct stpm_struct connect_struct;
#pragma seg16(status_struct)          /*                       CS2A*/
        _Packed struct cwin_struct status_struct;
        connect_struct.stpm_shortname = dft_sess; /* setsession id */

        hfunc_num = HA_CONNECT_PM_SRVCS;     /* issue connect      */
                                             /* presentation space */

        hllapi(&hfunc_num,(char *)&connect_struct,&hds_len,&hrc);
                                                             /*CS2C*/

        if (hrc == HARC_SUCCESS){            /* if return code good*/

            hfunc_num = HA_PM_WINDOW_STATUS ;/* choose PM window   */
                                             /* status function    */
            status_struct.cwin_shortname = dft_sess;
            status_struct.cwin_option = 0x2;
            hllapi(&hfunc_num,(char *)&status_struct,&hds_len,&hrc);
                                                             /*CS2C*/

        }

        if (hrc == HARC_SUCCESS){

            printf("                     PM WINDOW STATUS\n\n" );

            if (status_struct.cwin_flags & 0x0008)
                printf(" STATUS  :     The window is visible. \n" );

            if (status_struct.cwin_flags & 0x0010)
                printf(" STATUS  :     The window is invisible. \n" );

            if (status_struct.cwin_flags & 0x0080)
                printf(" STATUS  :     The window is activated. \n" );

            if (status_struct.cwin_flags & 0x0100)
                printf(" STATUS  :     The window is deactivated. \n" );

            if (status_struct.cwin_flags & 0x0400)
                printf(" STATUS  :     The window is minimized. \n" );

            if (status_struct.cwin_flags & 0x0800)
                printf(" STATUS  :     The window is maximized. \n" );

            printf("\n\n");

        }

        if (hrc == HARC_SUCCESS){

            rc = query_window_coords();
        }

        else

        {
          rc = hrc;
          error_hand(hfunc_num,rc);
        }

        return (rc);
}




/************************************************************************/
/*  QUERY_PM_WINDOW_COORDS - Query PM window coordinates in pells       */
/*                                                                      */
/*                                                                      */
/*                                                                      */
/************************************************************************/

int query_window_coords()
{

#pragma seg16(window_struct)               /*                       CS2A*/
        _Packed struct gcor_struct window_struct;

        int rc = 0;

        hfunc_num = HA_QUERY_WINDOW_COORDS;      /*  select query       */
        hrc = rc;                                /*  window coordinates */

        window_struct.gcor_shortname = dft_sess;
        hllc(&hfunc_num,(char *)&window_struct,&hds_len,&hrc);  /* call EhllapiCS2A*/


        if (hrc == HARC_SUCCESS){

            printf("                  PM WINDOW COORDINATES\n\n");
            printf(" XLEFT    :     %lx \n",window_struct.gcor_xLeft);
            printf(" YBOTTOM  :     %lx \n",window_struct.gcor_yBottom);
            printf(" XRIGHT   :     %lx \n",window_struct.gcor_xRight);
            printf(" YTOP     :     %lx \n",window_struct.gcor_yTop);
            printf("\n");
            printf("(press CONTROL-ESCAPE to verify)\n\n\n");
        }

        else  {

            error_hand(hfunc_num,hrc);
        }

        return (rc);
}



/***********************************************************************/
/* MAKE_PM_WINDOW_INVISIBLE - Make the PM window invisible             */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

int make_PM_window_invisible()
{
        unsigned int rc = 0;                            /* return code */
#pragma seg16(data_struct)                /*                       CS2A*/
        _Packed struct cwin_struct data_struct;

        hdata_str[0] = dft_sess;          /* setsession id to connect  */

        hfunc_num = HA_CONNECT_PM_SRVCS;  /* issue connect pm services */

        hllapi(&hfunc_num,hdata_str,&hds_len,&hrc);   /* call Ehllapi  */

        if (hrc == HARC_SUCCESS){         /* if return code good       */
                                          /* connection was successful */

           hfunc_num = HA_PM_WINDOW_STATUS ;/* choose PM window        */
                                            /* status function         */
           data_struct.cwin_shortname = dft_sess;
           data_struct.cwin_option = 0x1;
           data_struct.cwin_flags = 0x0010;

           hllapi(&hfunc_num,(char *)&data_struct,&hds_len,&hrc);
                                                             /*    CS2C*/
        }

        if (hrc == HARC_SUCCESS){

            printf("The PM window is now invisible. \n" );
            printf("(press CONTROL-ESCAPE to verify)\n\n\n");
        }

        else {

            rc = hrc;                    /* make PM window invisible   */
            error_hand(hfunc_num,rc);    /* unsuccessful               */
        }

        return (rc);
}




/***********************************************************************/
/* MAKE_PM_WINDOW_VISIBLE - make the PM window visible and             */
/*                                      maximimize it                  */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

int make_PM_window_visible()
{
        unsigned int rc = 0;                /* return code             */
#pragma seg16(data_struct)                /*                       CS2A*/
        _Packed struct cwin_struct data_struct;     /* local data string       */

        hdata_str[0] = dft_sess;            /* setsession id to connect*/

        hfunc_num = HA_CONNECT_PM_SRVCS;    /* issue connect pm services*/

        hllapi(&hfunc_num,hdata_str,&hds_len,&hrc); /* call Ehllapi    */

        if (hrc == HARC_SUCCESS){          /* if return code good,     */
                                           /* connection was successful*/

          hfunc_num = HA_PM_WINDOW_STATUS ;/* choose PM window         */
                                           /* status function          */
          data_struct.cwin_shortname = dft_sess;
          data_struct.cwin_option = 0x1;
          data_struct.cwin_flags = 0x0808;

          hllapi(&hfunc_num,(char * ) &data_struct,&hds_len,&hrc);
                                                           /*      CS2C*/
        }

        if (hrc == HARC_SUCCESS){

            printf("The PM window is now visible and maximized. \n");
            printf("(press CONTROL-ESCAPE to verify)\n\n\n");
        }
        else {

            rc = hrc;                        /* make PM window visible */
            error_hand(hfunc_num,rc);        /* was unsuccessful       */
        }

        return (rc);
}



/***********************************************************************/
/*  CHANGE_PS_WINDOW_NAME - Change the presentation space window name  */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

int change_PS_window_name()
{

        unsigned int rc = 0;              /* return code               */
#pragma seg16(data_struct)                /*                       CS2A*/
        _Packed struct chlt_struct data_struct;   /* local data structure      */

        hfunc_num = HA_CONNECT_PM_SRVCS;  /* issue connect pm services */
        hdata_str[0] = dft_sess;
        hllapi(&hfunc_num,hdata_str,&hds_len,&hrc); /* call Ehllapi    */

        if (hrc == HARC_SUCCESS){         /* if return code good       */

           hfunc_num = HA_CHANGE_WINDOW_NAME;
                                          /* change window name func   */
           data_struct.chlt_shortname = dft_sess;
           data_struct.chlt_option = 0x1;
           strcpy(data_struct.chlt_ltname,test_name);
           hds_len = sizeof(test_name);   /* calculate length of new   */
                                          /* window title name         */
           hds_len +=2;                   /* len of str sent to Ehllapi*/

           hllapi(&hfunc_num,(char *)&data_struct,&hds_len,&hrc);
                                                            /*     CS2C*/
        }

        if (hrc == HARC_SUCCESS){

           printf("Window Title Changed.\n" );
           printf("(press CONTROL-ESCAPE to verify)\n\n\n");
        }

        else {

           rc = hrc;                            /* window title change */
           error_hand(hfunc_num,rc);            /* was unsuccessful    */
        }

        return (rc);
}



/***********************************************************************/
/*  CHANGE_SWITCH_LIST_LT_NAME - Change The Switch List LT Name        */
/*                                        LT - logical terminal        */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

int change_switch_list_LT_name()
{
        unsigned int rc = 0;                /* return code             */
#pragma seg16(data_struct)                /*                       CS2A*/
        _Packed struct chsw_struct data_struct;     /* local data structure    */

        hfunc_num = HA_CONNECT_PM_SRVCS;    /* issue connect pm services*/
        hdata_str[0] = dft_sess;
        hllapi(&hfunc_num,hdata_str,&hds_len,&hrc);    /* call Ehllapi */

        if (hrc == HARC_SUCCESS) {        /* if return code good,      */
                                          /* connection was successful */

           hfunc_num = HA_CHANGE_SWITCH_NAME ;
                                          /* choose change switch list */
                                          /* LT name function          */

           data_struct.chsw_shortname = dft_sess;
           data_struct.chsw_option = 0x1;
           strcpy(data_struct.chsw_swname,test_name);

           hds_len = sizeof(test_name)  ; /* calc length of new switch */
                                          /* list name                 */

           hds_len +=2;            /* length of string sent to Ehllapi */

           hllapi(&hfunc_num,(char *)&data_struct,&hds_len,&hrc);
                                                            /*     CS2C*/

         }
         if (hrc == HARC_SUCCESS){

             printf("Switch List Name Changed.\n" );
             printf("(press CONTROL-ESCAPE to verify)\n\n\n");
         }
         else {

             rc = hrc;                  /* connection to presentation */
             error_hand(hfunc_num,rc);  /* space unsuccessful */
         }

        return (rc);
}



/***********************************************************************/
/*  DISCONNECT PM WINDOW SERVICE - disconnect from PM window session   */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

int disconnect_PM_window_service()
{
        unsigned int rc = 0;              /* return code               */

        hdata_str[0] = dft_sess;          /* setsession id to connect  */

        hfunc_num = HA_CONNECT_PM_SRVCS;  /* issue connect pm services */

        hllapi(&hfunc_num,hdata_str,&hds_len,&hrc);   /* call Ehllapi  */

        if (hrc == HARC_SUCCESS){         /* if return code good,      */
                                          /* connection was successful */

           hdata_str[0] = dft_sess;             /* session id          */

           hfunc_num = HA_DISCONNECT_PM_SRVCS ; /* choose disconnect   */
                                                /* PM services function*/

           hllapi(&hfunc_num,hdata_str,&hds_len,&hrc); /* call Ehllapi */
         }

         if (hrc == HARC_SUCCESS)

             printf("PM window disconnected.\n\n\n" );

         else {

              rc = hrc;
              error_hand(hfunc_num,rc);
         }

         return (rc);
}




/***********************************************************************/
/*  RESET SWITCH AND WINDOW NAME - Reset the switch and window names   */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

int reset_switch_and_window_name(){

        unsigned int rc = 0;              /* return code               */

#pragma seg16(window_struct)              /*                       CS2A*/
        _Packed struct chlt_struct window_struct; /* local data structures     */
#pragma seg16(switch_struct)              /*                       CS2A*/
        _Packed struct chsw_struct switch_struct;
#pragma seg16(status_struct)              /*                       CS2A*/
        _Packed struct cwin_struct status_struct;

        hdata_str[0] = dft_sess;          /* setsession id to connect  */

        hfunc_num = HA_CONNECT_PM_SRVCS;  /* issue connect pm space    */

        hllapi(&hfunc_num,hdata_str,&hds_len,&hrc);   /* call Ehllapi  */

        if (hrc == HARC_SUCCESS){         /* if return code good,      */
                                          /* connection was successful */

           hfunc_num = HA_CHANGE_SWITCH_NAME ;

                                          /* choose change switch list */
                                          /* LT name function          */

          switch_struct.chsw_shortname = dft_sess;/* set input string to*/
                                                  /* reset switch name  */
          switch_struct.chsw_option = 0x02;
          hds_len = 2;


          hllapi(&hfunc_num,(char *)&switch_struct,&hds_len,&hrc);
                                                            /*     CS2C*/

       }

       if (hrc == HARC_SUCCESS)

           printf("Switch List Name Reset.\n");

           if (hrc == HARC_SUCCESS)        /* if return code good,     */
                                           /* connection was sucessful */
              {
               hfunc_num = HA_CHANGE_WINDOW_NAME ;

                                           /* choose change switch     */
                                           /* list LT name function    */
                                           /* to reset window name     */

             window_struct.chlt_shortname = dft_sess;
             window_struct.chlt_option = 0x02;
             hds_len = 2;

             hllapi(&hfunc_num,(char *)&window_struct,&hds_len,&hrc);
                                                            /*     CS2C*/
           }

           if (hrc == HARC_SUCCESS)

              printf("Window Name Reset.\n");

           if (hrc == HARC_SUCCESS){       /* if return code good,     */
                                           /* connection was successful*/

              hfunc_num = HA_PM_WINDOW_STATUS;

                                          /* set window status function*/
              status_struct.cwin_shortname = dft_sess;
                                          /* set input string  to      */
                                          /* restore the window size   */
              status_struct.cwin_option = 0x01;
              status_struct.cwin_flags= 0x1000;


              hllapi(&hfunc_num,(char *)&status_struct,&hds_len,&hrc);
                                                            /*     CS2C*/
           }

           if (hrc == HARC_SUCCESS) {

               printf("Window Size Restored.\n" );
               printf("(press CONTROL-ESCAPE to verify)\n\n\n");
           }

           else {

               rc = hrc;                 /* connection to presentation */
               error_hand(hfunc_num,rc); /* was space unsuccessful     */
           }

           return (rc);
}


/***********************************************************************/
/*  RESET SYSTEM - Return Ehllapi to its initial conditions            */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

int reset_system(){

        unsigned int rc = 0;              /* return code               */

        hfunc_num = HA_RESET_SYSTEM;      /* issue connect pm services */

        hllapi(&hfunc_num,hdata_str,&hds_len,&hrc);  /* call Ehllapi   */

        if (hrc == HARC_SUCCESS){         /* if return code good,      */

            printf("Ehllapi Reset To Original Conditions. \n\n");
        }
        else {

            rc = hrc;                     /* return system error       */
            error_hand(hfunc_num,rc);
        }

        return (rc);
}




/***********************************************************************/
/*  CONNECT STRUCTURED FIELD - Connect to a Structured field           */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

int connect_structured_field(){

#pragma seg16(consf_struct)                   /*                   CS2A*/
        _Packed struct stsf_struct consf_struct; /*                CS2A*/
        _Packed struct stsf_struct *data_str;

        unsigned int rc = 0;                  /* return code           */
        data_str = &consf_struct;             /*                   CS2A*/

        data_str->stsf_shortname = dft_sess;  /* fill data string with */
                                              /* structured field      */
                                              /* input parameters      */
        data_str->stsf_query = (char *) query_reply;

        hrc = rc;
        hds_len = 11;

        hfunc_num = HA_START_STRUCTURED_FLD;     /* issue start        */
                                                 /* structured field   */

        hllapi(&hfunc_num,(char *)data_str,&hds_len,&hrc);
                                                            /*     CS2C*/

        if (hrc == HARC_SUCCESS){

           sf_doid  = data_str->stsf_doid;
           status_sem = data_str->stsf_asem;
           printf("Session Shortname      %c\n",dft_sess);
           printf("Destination/Origin ID: %x\n",sf_doid);
           printf("Structured Field Connect Initiated.\n\n");
        }
        else {

           rc = hrc;
           error_hand(hfunc_num,rc);
        }

        return (rc);
}



/***********************************************************************/
/*  QUERY COM BUFFER SIZE - Query Communications Buffer Size           */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

int query_com_buffer_size(){

#pragma seg16(combuf_struc)               /*                       CS2A*/
     _Packed struct qbuf_struct combuf_struc;/*                    CS2A*/
     _Packed struct qbuf_struct  * data_str ;/*                    CS2C*/
     unsigned int rc = 0;                           /* return code     */
     data_str = &combuf_struc;            /*                       CS2A*/

     hds_len = 9;
     data_str->qbuf_shortname =  dft_sess;
     printf("Session Shortname:  %c\n",data_str->qbuf_shortname);
     hfunc_num = HA_QUERY_BUFFER_SIZE;              /* issue query     */
                                                    /* buffer sizes    */
     hllapi(&hfunc_num,(char *)data_str,&hds_len,&hrc);
                                                            /*     CS2C*/

     optimal_in_buffer_len = data_str->qbuf_opt_inbound;
     optimal_out_buffer_len = data_str->qbuf_opt_outbound;

     if (hrc == HARC_SUCCESS){

printf("Optimal Inbound Buffer Size:  %x\n",data_str->qbuf_opt_inbound);
printf("Maximum Inbound Buffer Size:  %x\n",data_str->qbuf_max_inbound);
printf("Optimal Outbound Buffer Size: %x\n",data_str->qbuf_opt_outbound);
printf("Maximum Outbound Buffer Size: %x\n",data_str->qbuf_max_outbound);
printf("Query Buffer Complete.\n\n");

     }

     else {

       rc = hrc;
       error_hand(hfunc_num,rc);
     }

     return (rc);
}




/***********************************************************************/
/*  ALLOCATE SF BUFFER - Allocate a Structured Field Buffer from the   */
/*                                  EHLLAPI Shared Memory              */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

int allocate_sf_buffer(){

#pragma seg16(alcbuf_struc)               /*                       CS2A*/
        _Packed struct abuf_struct alcbuf_struc;/*                 CS2A*/
        _Packed struct abuf_struct  * data_str;
        int i;
        unsigned int rc = 0;                      /* return code       */
        data_str = &alcbuf_struc;         /*                       CS2A*/


        hfunc_num = HA_ALLOCATE_COMMO_BUFF;       /* issue allocate    */
        hds_len =   6;                            /* structured field  */
        data_str->abuf_length  = optimal_buffer_len;
        data_str->abuf_address = 0L;

        printf("Buffer Length:   %x\n",data_str->abuf_length);


        hllapi(&hfunc_num,(char *)data_str,&hds_len,&hrc);
                                                  /*               CS2C*/

        sf_buffer_address = (char *)data_str->abuf_address;

        if (hrc == HARC_SUCCESS){

           printf("Buffer Address:  %lx\n",sf_buffer_address);
           printf("Allocate Buffer Complete.\n\n");
         }
         else {

           rc = hrc;
           error_hand(hfunc_num,rc);
        }

        return (rc);
}



/**********************************************************************/
/*  READ_SF_ASYNC - Read asynchronously from a structured field       */
/*                                                                    */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

int read_sf_async()
{
#pragma seg16(rdabuf_struc)              /*                       CS2A*/
        _Packed struct rdsf_struct rdabuf_struc; /*               CS2A*/
        _Packed struct rdsf_struct *data_str; /* pointer to structure */
        unsigned int rc = 0;                  /*   return code        */
        int i;
        data_str = &rdabuf_struc;        /*                       CS2A*/


        hfunc_num = HA_READ_STRUCTURED_FLD;       /*    issue read    */
        hds_len = 14;                             /* structured field */
        data_str->rdsf_shortname  = dft_sess;
        data_str->rdsf_option = 'A';
        data_str->rdsf_doid   = sf_doid;
        data_str->rdsf_buffer =  read_buffer;
        data_str->rdsf_requestid = 0;
        data_str->rdsf_asem = 0L;
        for (i=0; i<5;i++){
          ((USHORT  *) data_str->rdsf_buffer)[i] = 0x00;
        }
        ((USHORT  *) data_str->rdsf_buffer)[2] = 0x0500;

        printf("Session Shortname:       %c\n",data_str->rdsf_shortname);
        printf("Control Option Selected: %c\n",data_str->rdsf_option);
        printf("Destination / Origin ID: %x\n",data_str->rdsf_doid);

        hllapi(&hfunc_num,(char *)data_str,&hds_len,&hrc);
                                                             /*   CS2C*/

        if (hrc == HARC_INBOUND_DISABLED) {
                                          /* read succesful, the      */
                                          /* host is inbound disabled */

            sf_asem = data_str->rdsf_asem;
            printf("Semaphore Address:       %lx\n",(char *)sf_asem);
            printf("Semaphore Contents:      %x\n",*(char *)sf_asem);
            printf("Read SF Complete.\n\n");
        }

        else {

            rc = hrc;
            error_hand(hfunc_num,rc);
        }

        return (rc);
}




/************************************************************************/
/* CREATE A STRUCTURED FIELD - create a structured field using ind$file */
/*                                                                      */
/*                                                                      */
/************************************************************************/

int create_a_structured_field(){         /* Call routine to             */
                                         /* write string to host        */

    unsigned int rc = 0;                 /* return code                 */

    hdata_str[0] = dft_sess;             /* Set session id to connect   */

    hfunc_num = HA_CONNECT_PS;           /* Issue Connect to PS         */

    hllapi(&hfunc_num, hdata_str, &hds_len, &hrc);   /* Call EHLLAPICS2C*/

    if (hrc == HARC_SUCCESS){

       hfunc_num = HA_SENDKEY;           /* Issue sendkey               */

       strcpy(hdata_str,home_key);       /* String to send to host      */

       hds_len = sizeof(home_key)-1;     /* Sub null char from length   */

       hllapi(&hfunc_num,hdata_str,&hds_len, &hrc);  /* Call EHLLAPICS2C*/
    }

    if (hrc == HARC_SUCCESS) {

       hfunc_num = HA_SENDKEY;              /* Issue sendkey            */

       strcpy(hdata_str,sf_test_str);       /* String to send to Host   */

       hds_len = sizeof(sf_test_str) - 1;   /* Set length of string     */
                                            /* minus null charatcter    */

       hllapi(&hfunc_num, hdata_str, &hds_len, &hrc);   /* Call EHLLCS2C*/
    }

    if (hrc != HARC_SUCCESS)                 /* If bad return code      */

    {

       rc = hrc;                             /* Set return code         */
       error_hand(hfunc_num, rc);

    }

    return(rc);

}




/********************************************************************/
/*  GET COMPLETION REQUEST - Check if last asyncronus               */
/*                           process finished                       */
/*                                                                  */
/*                                                                  */
/********************************************************************/

int get_completion_request(){

#pragma seg16(data_str)                   /*                    CS2A*/
     _Packed struct gcmp_struct data_str ;/* disconnect data struct */
     unsigned int rc = 0;                 /* return code            */

     data_str.gcmp_shortname = dft_sess;  /* fill data string       */
                                          /* with completion request*/
                                          /* input parameters       */
     data_str.gcmp_option = 'W';
     data_str.gcmp_requestid = 0;
     data_str.gcmp_ret_functid = 0;
     data_str.gcmp_ret_datastr = 0L;
     data_str.gcmp_ret_length  = 0;
     data_str.gcmp_ret_retcode = 0;
     hds_len = 14;

     printf("Session Shortname:  %c\n",data_str.gcmp_shortname);

                                         /* wait for semaphore     */
                                         /* to clear               */


     hfunc_num = HA_GET_ASYNC_COMPLETION;

     hllapi(&hfunc_num,(char * )&data_str,&hds_len,&hrc);   /*   CS2C*/

     if (hrc == HARC_SUCCESS){

        printf("Semaphore Address:  %lx\n",(char *)sf_asem);
        printf("Semaphore Contents: %x\n",*(char *)sf_asem);
        printf("Completion Request Successful.\n");
       }

      else {

        rc = hrc;
        error_hand(hfunc_num,rc);
     }

    return (rc);
}




/**********************************************************************/
/*  WRITE_SF_SYNC - Write synchronously to a structured field         */
/*                                                                    */
/*                                                                    */
/*                                                                    */
/**********************************************************************/

int write_sf_sync(){

#pragma seg16(wsfbuf_struc)              /*                       CS2A*/
    _Packed struct wrsf_struct wsfbuf_struc;  /*                  CS2A*/
    _Packed struct wrsf_struct *data_str;     /*                  CS2A*/
    unsigned int rc = 0;                             /* return code   */
    int i;
    data_str = &wsfbuf_struc;            /*                       CS2A*/

    hfunc_num = HA_WRITE_STRUCTURED_FLD;          /*    issue write   */
    hds_len = 8;                                  /* structured field */
    data_str->wrsf_shortname  = dft_sess;
    data_str->wrsf_option = 'S';
    data_str->wrsf_doid   = sf_doid;

                                            /* fill buffer with zeros */
    data_str->wrsf_buffer =  write_buffer;
    for (i=0; i<0x500; i++) {
       ((char *) data_str->wrsf_buffer)[i] = '\0';/*from far* to *CS2C*/
    }
                                      /* fill structured field header */
    ((char *)data_str->wrsf_buffer)[2]  = 0x05;   /*from far* to *CS2C*/
    ((char *)data_str->wrsf_buffer)[9]  = 0x05;   /*from far* to *CS2C*/
    ((char *)data_str->wrsf_buffer)[10] = 0xD0;   /*from far* to *CS2C*/
    ((char *)data_str->wrsf_buffer)[12] = 0x09;   /*from far* to *CS2C*/

    printf("Session Shortname:       %c\n",data_str->wrsf_shortname);
    printf("Control Option Selected: %c\n",data_str->wrsf_option);
    printf("Destination / Origin ID: %x\n",data_str->wrsf_doid);

    hllapi(&hfunc_num,(char *)data_str,&hds_len,&hrc);      /*    CS2C*/

    if (hrc == HARC_SUCCESS) {

       printf("Write Structured Field Complete.\n");
       DOS16SLEEP((LONG) TIMEOUT);              /* Allow time for   */
    }                                           /* host to complete */
                                                /* response before  */
    else {                                      /* continuing       */

       rc = hrc;
       error_hand(hfunc_num,rc);
    }

    return (rc);
}





/********************************************************************/
/*  FREE COMMUNICATIONS BUFFER - Return to the shared memory pool   */
/*                  the communications buffers no longer being used */
/*                                                                  */
/*                                                                  */
/********************************************************************/

int free_com_buffers(){

#pragma seg16(fcombuf_struc)             /*                       CS2A*/
  _Packed struct fbuf_struct fcombuf_struc; /*                    CS2A*/
  _Packed struct fbuf_struct *data_str;
  unsigned int rc=0;
  data_str = &fcombuf_struc;              /*                      CS2A*/


  hfunc_num = HA_FREE_COMMO_BUFF;
  hds_len = 6;
  data_str->fbuf_length = optimal_in_buffer_len;
  data_str->fbuf_address = read_buffer;

  hllapi(&hfunc_num,(char *)data_str,&hds_len,&hrc);       /*     CS2A*/

  if (hrc == HARC_SUCCESS) {

    printf("Read Communications Buffer De-allocated.\n");

    hfunc_num = HA_FREE_COMMO_BUFF;
    hds_len = 6;
    data_str->fbuf_length = optimal_out_buffer_len;
    data_str->fbuf_address = write_buffer;
    hllapi(&hfunc_num,(char *)data_str,&hds_len,&hrc);     /*     CS2C*/
  }

 if (hrc == HARC_SUCCESS)

    printf("Write Communications Buffer De-allocated.\n");

 else {

   rc = hrc;
   error_hand(hfunc_num,rc);
 }

 return(rc);
}




/***********************************************************************/
/*  DISCONNECT STRUCTURED FIELD - Disonnect to a Structured field      */
/*                                                                     */
/*                                                                     */
/*                                                                     */
/***********************************************************************/

int disconnect_structured_field(){

#pragma seg16(dsfbuf_struc)                  /*                    CS2A*/
     _Packed struct spsf_struct dsfbuf_struc;/*                    CS2A*/
     _Packed struct spsf_struct *data_str ;  /* disconnect data struct */
     unsigned int rc = 0;                    /* return code            */
     data_str = &dsfbuf_struc;               /*                    CS2A*/

     data_str->spsf_shortname = dft_sess;    /* fill data string 2     */
                                             /* with structured field  */
                                             /* input parameters       */
     data_str->spsf_doid = sf_doid;
     hds_len = 3;
     hfunc_num = HA_STOP_STRUCTURED_FLD;           /* issue stop       */
                                                   /* structured field */

     printf("Short-Session Name:      %c\n",data_str->spsf_shortname);
     printf("Destination / Origin ID: %x\n",data_str->spsf_doid);

     hllapi(&hfunc_num,(char *)data_str,&hds_len,&hrc);     /*     CS2C*/

     if (hrc == HARC_SUCCESS)

         printf("Structured Field Disconnected.\n\n");

     else {

         rc = hrc;
         error_hand(hfunc_num,rc);
     }
        return (rc);
}




/*********************************************************************/
/* ERROR_HAND - Error handler.                                       */
/*                                                                   */
/*********************************************************************/

int error_hand(func, rc)                /* Error handler.            */

unsigned int func;
unsigned int rc;
{
  printf("UNEXPECTED RETURN CODE %d from FUNCTION #%d.",rc,
       func);
}

