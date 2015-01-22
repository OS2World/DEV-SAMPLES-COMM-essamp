***********************************************************************         
* TITLE: IBMABASE MAINLINE                                                      
*                                                                               
* LOGIC: Determine the function requested, and perform that function.           
*                                                                               
* OPERATION:                                                                    
* 1. Establish addressability to the server parameters.                         
* 2. Determine that we have a server request.                                   
* 3. Establish addressability to the CPRB.                                      
* 4. If the cms files are not open:                                             
*      - Open them.                                                             
* 5. Determine the function requested.                                          
* 6. If function 1 is requested:                                                
*      - Issue the FSREAD macro to read in a record.                            
*      - If the end of file was encountered:                                    
*          a. Close the data sets.                                              
*          b. Set end of file return code                                       
*      - Else, no end of file encountered:                                      
*          a. If the transaction should be logged:                              
*               - Issue the FSWRITE macro to output the log message.            
*          b. Translate the reply data into ASCII.                              
* 7. If function 2 is requested:                                                
*      - Translate the request data into EBCDIC.                                
*      - Issue the FSWRITE macro to write the record.                           
*      - If the transaction should be logged:                                   
*          a. Issue the FSWRITE macro to output the log message.                
* 8. Else set invalid function return code.                                     
* 9. Return to the caller with return code.                                     
***********************************************************************         
IBMABASE CSECT                                                                  
         STM   R14,R12,R12(R13)      Save the caller's registers                
         LR    R11,R15               Load base register                         
         USING IBMABASE,R11,R12      Establish addressability                   
         L     R12,BASE2                                                        
         ST    R13,SAVEAREA+4        Save the callers savearea address          
         LA    R9,SAVEAREA           Obtain our savearea address                
         ST    R9,R8(,R13)           Chain it in the caller's savearea          
         LR    R13,R9                Point register 13 to our savearea          
***********************************************************************         
* GET THE SERVER PARAMETER LIST                                                 
***********************************************************************         
         USING PARAMTR,R1            ADDRESSABILITY TO PARM LIST                
         OI    STATUS,TERMIN8        Assume Termination (no CPRB)               
         CLC   REQTYP,=2F'-1'        Test for no parameter                      
         BNE   PROCPARM              YES, PROCESS THE PARAMETER                 
         B     REQ                   Assume request, not from SENDREQ           
BASE2    DC    A(IBMABASE+4096)                                                 
PROCPARM MVC TYPE,REQTYP             SAVE NOTIFICATION TYPE                     
         DROP  R1                                                               
         CLC   TYPE,SENDREQ          Check for SENDREQ Notification             
         BE    REQ                   YES-> Must be from SENDREQ                 
***********************************************************************         
* IF THIS IS AN ENDCMD NOTIFICATION THEN ISSUE A MESSAGE STATING                
* THIS AND RETURN.  CLOSE THE FILES AND CLEAN UP LOCKS HERE.                    
***********************************************************************         
         CLC   TYPE,ENDCMD           Check for ENDCMD notification              
         BNE   CHKWSC                If not ENDCMD go to next check             
         MVC   REPMSG(30),MSGEND     Set message - this is ENDCMD               
         B     CLOSELOG              Log the message and end                    
***********************************************************************         
* IF THIS IS A WORK STATION COMMUNICATION TERMINATION NOTIFICATION              
* THEN ISSUE A MESSAGE STATING THIS AND RETURN                                  
***********************************************************************         
CHKWSC   CLC   TYPE,WSCOMM           Check for WSCOMM notification              
         BNE   CHKPUR                If not WSCOMM go to next check             
         MVC   REPMSG(30),MSGWSC     Set message - this is WSCOMM               
         DELENTRY IBMABASE           Drop the notification entry                
         LTR   R15,R15               Was it Successful?                         
         BZ    CLOSELOG              Log the message and end                    
         CH    R15,=H'4'             Reply sent not confirmed                   
         BNE   CHKWSC01              NO-> Check other                           
         LA    R9,MSGSNOC            Tell sent but not confirmed                
         B     CHKWSC99              Update the error log                       
CHKWSC01 CH    R15,=H'8'             NO Reply successfully sent                 
         BNE   CHKWSC02              NO-> Check other                           
         LA    R9,MSGNRSS            Tell no reply successfully sent            
         B     CHKWSC99              Update the error log                       
CHKWSC02 LA    R9,MSGEDEL            Tell DELENTRY other errors                 
CHKWSC99 BAL   R10,LOGINFO           GO UPDATE LOG FILE                         
         B     LOGMSG                LOG THE MESSAGE AND END                    
***********************************************************************         
* IF THIS IS AN ABNORMAL TERMINATION (CMS ABEND) NOTIFICATION                   
* THEN ISSUE A MESSAGE STATING THIS AND RETURN                                  
***********************************************************************         
CHKPUR   CLC   TYPE,PURGE            Check for PURGE notification               
         BNE   CHKRESET              If not PURGE go to next check              
         MVC   REPMSG(30),MSGPUR     Set message - this is PURGE                
         B     CLOSELOG              Log the message and end                    
***********************************************************************         
* IF THIS IS AN EXPLICIT SERVER UNLOAD (NUCXDROP) NOTIFICATION                  
* YOU NEED TO CLEAN UP STORAGE HERE IF ANY WAS OBTAINED                         
* THEN ISSUE A MESSAGE STATING THIS AND RETURN                                  
***********************************************************************         
CHKRESET CLC   TYPE,RESET            Check for RESET notification               
         BNE   REQ                   If not RESET go it is a request            
         MVC   REPMSG(30),MSGRES     Set message - this is RESET                
         B     LOGMSG                Log the message and end                    
***********************************************************************         
* WE HAVE A REQUEST.  GET ADDRESS OF EXTENDED PARAMETER LIST.  GET              
* ADDRESS OF CPRB AND CHECK SELF IDENTIFIER FIELD TO VERIFY THAT IT             
* IS A CPRB.                                                                    
***********************************************************************         
REQ      LTR   R3,R0                 Load address of EPLIST                     
         BZ    NOCPRBM               IF NO EPLIST SET MESSAGE                   
         USING PTRS,R3               Addressability of eplist                   
         ICM   R2,B'1111',CRBPTR     Get address of CPRB                        
         BZ    NOCPRBM               If not CPRB  set message                   
         DROP  R3                                                               
         USING CPRB,R2               Addressability to CPRB                     
         CLC   CRBCPRB,CPRBCHK       Check self identifier                      
         BNE   NOCPRBM               Fail->  Send error codes                   
         L     R4,CRBRPDAT           Get Reply Data pointer                     
         L     R5,CRBRQDAT           Get Request Data pointer                   
         L     R6,CRBRQPRM           Get Request Parm pointer                   
         NI    STATUS,255-TERMIN8    Reset Termination entry                    
         B     GETFILES              If OK continue                             
NOCPRBM  MVC   REPMSG,MSGCPR         Else set message - no CPRB found           
         B     NOCPRB                If not CPRB then return                    
***********************************************************************         
* WE HAVE A SERVER REQUEST AND A CPRB.  NEXT PREPARE THE CMS FILES              
***********************************************************************         
GETFILES TM    STATUS,OPENED         Are the files already opened?              
         BO    OPEN                  Yes, then don't try to open them           
         FSOPEN FSCB=INPUT,ERROR=OPENERR   Open the INPUT file                  
         FSOPEN FSCB=OUTPUT          Open the OUTPUT file                       
         FSOPEN FSCB=LOG             Open the LOG file                          
         ADDENTRY IBMABASE           Add the notification entry                 
         OI    STATUS,OPENED         Indicate that they are open                
OPEN     CLC   CRBFID,=H'1'          Is function one requested?                 
         BE    FUNCTN1               Yes, branch to the function.               
         CLC   CRBFID,=H'2'          Is function two requested?                 
         BE    FUNCTN2               Yes, branch to the function.               
         LA    R9,MSGBAD             Invalid Request                            
         BAL   R10,LOGINFO           Go Log the information message             
         LA    R15,8                 Else, bad function id.                     
         B     CLOSEXIT              Close and Exit the server.                 
***********************************************************************         
* Process Requestors TYPE 1  function                                           
***********************************************************************         
FUNCTN1  FSREAD FSCB=INPUT,BUFFER=REPLY   Get the record.                       
         CH    R15,=H'12'            End of file?                               
         BE    ENDOFILE              Go to end of file routine.                 
         LTR   R15,R15               Any other errors?                          
         BNZ   ERRORRC1              YES-> Tell requestor                       
         CLI   0(R6),X'01'           Should we log the transaction?             
         NOP   NOINLOG               No, branch around logging.                 
         BAL   R10,LOGTRANS          Put data pass into Log file                
NOINLOG  XC CRBSRTNC,CRBSRTNC        ZERO is a good return code                 
         L     R7,INPUT+(FSCBSIZE-FSCBD)   Get the size                         
         C     R7,CRBRPDLN           Will it fit in request area?               
         BE    CPRBPDLN              YES-> Return data                          
         LA    R9,MSGLEN             Set up log message                         
         BAL   R10,LOGINFO           Go Log the information message             
         MVC   CRBSRTNC,=AL4(CSMSREPD)   Tell data too big for buffer           
         LA    R15,CSMSREPD          Set Reg return code also                   
CPRBPDLN BCTR R7,0                   For execute                                
         LR    R8,R7                 Exclude Balance field from the             
         SH    R8,=H'4'                ASCII translation                        
         EX    R8,ETOA               Translate the record to ASCII.             
         EX    R7,TOCPRB             Set return data in CPRB                    
         LA    R7,1(,R7)             Restore length of the reply,               
         ST    R7,CRBRPDLN           and store it into the CPRB.                
         LA    R7,0                  Set the reply parameter length,            
         ST    R7,CRBRPPLN           and store it into the CPRB.                
         LTR   R15,R15               Is there an error?                         
         BNZ   CLOSEXIT              YES-> Close and Exit the server.           
         TM    STATUS,TERMIN8        Is there a CPRB present?                   
         BO    CLOSEXIT              NO-> Close and Exit the server.            
         XC    CRBSRTNC(4),CRBSRTNC  Clear Return code for Requestor            
         B     EXIT                  Exit the server.                           
         SPACE 2                                                                
***********************************************************************         
* Process Requestors TYPE 2  function                                           
***********************************************************************         
FUNCTN2  L     R7,CRBRPDLN           Get the size                               
         BCTR  R7,0                  Minus one for execute                      
         L     R9,CRBRPDAT           Get data pointer                           
         EX    R7,FROMCPRB           Move data from CPRB.                       
         LR    R8,R7                 Exclude Balance field from the             
         SH    R8,=H'4'                EBCDIC translation                       
         EX    R8,ATOEDAT            Translate the record to EBCDIC.            
         LA    R7,1(,R7)             Restore length of the reply,               
         FSWRITE FSCB=OUTPUT,BUFFER=LOGBUFF,BSIZE=(R7)   Update Record          
         CLI   0(R6),X'01'           Should we log the transaction?             
         NOP   NOOUTLOG              No, branch around logging.                 
         BAL   R10,LOGTRANS          Put data pass into Log file                
NOOUTLOG LA R15,0                    Set the return code.                       
         TM    STATUS,TERMIN8        Is there a CPRB present?                   
         BO    EXIT                  NO-> Exit now                              
         XC    CRBSRTNC,CRBSRTNC     ZERO is a good return code                 
         B     EXIT                  Return to the Requestor                    
***********************************************************************         
*                           ERROR processing                                    
***********************************************************************         
OPENERR  ST    R15,SAVERC            Save the Return code                       
         LA    R9,MSGFNF             Set up log message                         
         CH    R15,=H'28'            Is the file missing?                       
         BE    OPENERR1              Yes send error message                     
         LA    R9,MSGOPN             Set up log message                         
OPENERR1 ST    R15,SAVERC            Preserve requestors return code            
         BAL   R10,LOGINFO           Go Log the information message             
         L     R15,SAVERC            Preserve requestors return code            
ERRORRC1 ST   R15,SAVERC             Save the Return code                       
         DELENTRY IBMABASE           Drop the notification entry                
         LTR   R15,R15               Was it Successful?                         
         BZ    ERRORRC2              Nothing to add                             
         A     R15,SAVERC            Include Close RC with Error RC             
ERRORRC2 TM    STATUS,TERMIN8        Is there a CPRB present?                   
         BO    CLOSEXIT              NO-> EXIT now                              
         ST    R15,CRBSRTNC          Set RC in CPRB Requestor                   
         B     CLOSEXIT                                                         
***********************************************************************         
* End Of File processing from the INPUT FILE A                                  
***********************************************************************         
ENDOFILE BAL   R10,CLOSE             Go Close all of the files                  
         MVI   STATUS,CLOSED         Indicate that they are closed.             
         LTR   R15,R15               Any Close errors?                          
         BNZ   ERRORRC1              YES-> Send additional codes                
         LA    R15,4                 Set end of file return code.               
         MVC   CRBSRTNC,=A(4)        Set EOF in CPRB Requestor RC               
         B     EXIT                                                             
***********************************************************************         
* CLOSE all of the files                                                        
***********************************************************************         
CLOSE    FSCLOSE FSCB=INPUT          Close the INPUT file.                      
         ST    R15,CLOSERC           Save the return code                       
         FSCLOSE FSCB=OUTPUT         Close the OUTPUT file.                     
         A     R15,CLOSERC           Accumulate RC                              
         ST    R15,CLOSERC           Save the return code                       
         FSCLOSE FSCB=LOG            Close the LOG file.                        
         A     R15,CLOSERC           Send any Errors back to caller             
         BR    R10                   Return to the caller.                      
***********************************************************************         
* Log informational message in LOG FILE A                                       
***********************************************************************         
CLOSELOG ST    R15,SAVERC            Preserve requestors return code            
         BAL   R10,CLOSE             Go Close all of the files                  
         A     R15,SAVERC            Include Close RC with Error RC             
LOGMSG   LA    R9,REPMSG             Set up log message                         
         TM    STATUS,TERMIN8        Is there a CPRB present?                   
         BO    LOGERR                NO-> Log it now                            
         XC    CRBSRTNC,CRBSRTNC     ZERO is a good return code                 
         B     LOGERR                Write message to log                       
NOCPRB   LA    R9,REPMSG             CPRB not found message.                    
         LA    R15,CSMLGENR          Tell No CPRB                               
         TM    STATUS,TERMIN8        Is there a CPRB present?                   
         BO    LOGERR                NO-> Log it now                            
         MVC   CRBSRTNC,=AL4(CSMLGENR) No communications established            
LOGERR   ST    R15,SAVERC            Preserve requestors return code            
         BAL   R10,LOGINFO           Go Log the information message             
         L     R15,SAVERC            Restore the saved return code              
CLOSEXIT CLI   STATUS,CLOSED         Check if files are closed.                 
         BE    EXIT                  BR-> FIles all closed                      
         LTR   R15,R15               Did we have an error?                      
         BZ    EXIT                  BR-> no error                              
         ST    R15,SAVERC            Preserve requestors return code            
         BAL   R10,CLOSE             Go Close all of the files                  
         MVI   STATUS,CLOSED         Indicate that they are closed.             
         A     R15,SAVERC            Include Close RC with Error RC             
EXIT     L     R13,SAVEAREA+4        Restore caller's saveareas.                
         L     R14,R12(,R13)         Restore the caller's registers             
         LM    R0,R12,20(R13)        except for 15 (return code).               
         BR    R14                   Return to caller with return code.         
***********************************************************************         
* Log transaction of data to LOG FILE A                                         
***********************************************************************         
LOGTRANS STM   R0,R15,SUBSAVE        Preserve all the callers regs              
         CLC   CRBFID,=H'1'          Is function one requested?                 
         BNE   LOGOUTFN              NO-> Check 2                               
         LA    R9,INLOG              Point to heading                           
         BAL   R10,LOGINFO           Log Transaction                            
         OI    STATUS,LOGINP         Mark it written                            
         L     R7,INPUT+(FSCBSIZE-FSCBD) Get the size                           
         BCTR  R7,0                  Minus one for execute                      
         LA    R9,REPLY              Get Record read                            
         EX    R7,FROMCPRB           Move data from REPLY buffer                
         LA    R7,1(,R7)             Restore length of the reply,               
         BAL   R10,LOGBYSIZ          Log message by size                        
LOGOUTFN CLC   CRBFID,=H'2'          Is function two requested?                 
         BNE   LOGREST               NO-> Check for the rest to log             
         LA    R9,OUTLOG             Point to heading                           
         BAL   R10,LOGINFO           Log Transaction                            
         OI    STATUS,LOGOUT         Mark it written                            
         L     R7,CRBRPDLN           Get the size                               
         BCTR  R7,0                  Minus one for execute                      
         L     R9,CRBRPDAT           Get data pointer                           
         EX    R7,FROMCPRB           Move data from CPRB.                       
         LR    R8,R7                 Exclude Balance field from the             
         SH    R8,=H'4'                EBCDIC translation                       
         EX    R8,ATOEDAT            Translate the record to EBCDIC.            
         LA    R7,1(,R7)             Restore length of the reply,               
         BAL   R10,LOGBYSIZ          Log message by size                        
LOGREST  ICM   R7,B'1111',CRBRQDLN   Get requester data length.                 
         BZ    NOREQDAT              No request data.                           
         BCTR  R7,0                  For execute                                
         ICM   R9,B'1111',CRBRQDAT   Get data pointer                           
         BZ    NOREQDAT              BR-> No data pointer                       
         EX    R7,FROMCPRB           Move data from CPRB.                       
         LR    R8,R7                 Exclude Balance field from the             
         SH    R8,=H'4'                EBCDIC translation                       
         EX    R8,ATOEDAT            Translate the record to EBCDIC.            
         LA    R7,1(,R7)             Restore length of the reply,               
         ST    R7,LOG+(FSCBSIZE-FSCBD) Set size.                                
         BAL   R10,LOGBYSIZ          Log message by size                        
NOREQDAT ICM   R7,B'1111',CRBRQPLN   Get requester parameter length.            
         BZ    NOPARDAT              No request data.                           
         BCTR  R7,0                  For execute                                
         ICM   R9,B'1111',CRBRQPRM   Get parameter pointer                      
         BZ    NOPARDAT              BR-> No parameter pointer                  
         EX    R7,FROMCPRB           Move data from CPRB.                       
         EX    R7,ATOEDAT            Translate the record to EBCDIC.            
         LA    R7,1(,R7)             Restore length of the reply,               
         ST    R7,LOG+(FSCBSIZE-FSCBD) Set size.                                
         BAL   R10,LOGBYSIZ          Log message by size                        
NOPARDAT LA    R9,BLANK              Set to write a blank line                  
         BAL   R10,LOGINFO           Put in delimiter                           
         LM    R0,R10,SUBSAVE        Restore callers regs                       
         BR    R10                   Return to caller                           
***********************************************************************         
* Common ERROR/INFORMATIONAL  message logging routines                          
***********************************************************************         
LOGINFO  MVC   LOGBUFF,BLANK         Clear the buffer first                     
         MVC   LOGBUFF(30),0(R9)     Move in the diagnostic message             
         FSWRITE FSCB=LOG,BUFFER=LOGBUFF,BSIZE=109    Write a record            
         BR    R10                   Return to caller. R15 = RETCODE            
LOGBYSIZ FSWRITE FSCB=LOG,BUFFER=LOGBUFF,BSIZE=(R7)      Log input rcd          
         BR    R10                   Return to caller. R15 = RETCODE            
         EJECT                                                                  
***********************************************************************         
* Execute Instructions                                                          
***********************************************************************         
TOCPRB    MVC  0(0,R4),REPLY          For Execute to CPRB data                  
FROMCPRB MVC   LOGBUFF(0),0(R9)       Move from CPRB to Log Buffer.             
ETOA     TR    REPLY(0),TRASCII       Translate the record to ASCII.            
ATOEDAT  TR    LOGBUFF(0),TREBCDIC    Translate the record to EBCDIC.           
***********************************************************************         
* Data section.                                                                 
***********************************************************************         
* EBCDIC to ASCII translate table.                                              
***********************************************************************         
         SPACE                                                                  
TRASCII  DS    0CL256                                                           
         DC    X'00010203CF09D37FD4D5C30B0C0D0E0F'                              
         DC    X'10111213C7B408C91819CCCD831DD21F'                              
         DC    X'81821C84860A171B89919295A2050607'                              
         DC    X'E0EE16E5D01EEA048AF6C6C21415C11A'                              
         DC    X'20A6E180EB909FE2AB8B9B2E3C282B7C'                              
         DC    X'26A9AA9CDBA599E3A89E21242A293B5E'                              
         DC    X'2D2FDFDC9ADDDE989DACBA2C255F3E3F'                              
         DC    X'D78894B0B1B2FCD6FB603A2340273D22'                              
         DC    X'F861626364656667686996A4F3AFAEC5'                              
         DC    X'8C6A6B6C6D6E6F7071729787CE93F1FE'                              
         DC    X'C87E737475767778797AEFC0DA5BF2F9'                              
         DC    X'B5B6FDB7B8B9E6BBBCBD8DD9BF5DD8C4'                              
         DC    X'7B414243444546474849CBCABEE8ECED'                              
         DC    X'7D4A4B4C4D4E4F505152A1ADF5F4A38F'                              
         DC    X'5CE7535455565758595AA0858EE9E4D1'                              
         DC    X'30313233343536373839B3F7F0FAA7FF'                              
***********************************************************************         
* ASCII to EBCDIC translate table.                                              
***********************************************************************         
         SPACE                                                                  
TREBCDIC DS    0CL256                                                           
         DC    X'00010203372D2E2F1605250B0C0D0E0F'                              
         DC    X'101112133C3D322618193F27221D351F'                              
         DC    X'405A7F7B5B6C507D4D5D5C4E6B604B61'                              
         DC    X'F0F1F2F3F4F5F6F7F8F97A5E4C7E6E6F'                              
         DC    X'7CC1C2C3C4C5C6C7C8C9D1D2D3D4D5D6'                              
         DC    X'D7D8D9E2E3E4E5E6E7E8E9ADE0BD5F6D'                              
         DC    X'79818283848586878889919293949596'                              
         DC    X'979899A2A3A4A5A6A7A8A9C04FD0A107'                              
         DC    X'4320211C23EB249B7128384990BAECDF'                              
         DC    X'45292A9D722B8A9A6756644A53685946'                              
         DC    X'EADA2CDE8B5541FE5851524869DB8E8D'                              
         DC    X'737475FA15B0B1B3B4B56AB7B8B9CCBC'                              
         DC    X'AB3E3B0ABF8F3A14A017CBCA1A1B9C04'                              
         DC    X'34EF1E0608097770BEBBAC5463656662'                              
         DC    X'30424757EE33B6E1CDED3644CECF31AA'                              
         DC    X'FC9EAE8CDDDC39FB80AFFD7876B29FFF'                              
         EJECT                                                                  
SAVERC   DC    F'0'                  Save Requestors Return code.               
CLOSERC  DC    F'0'                  Save Close Return codes.                   
TYPE     DS    CL8                                                              
SAVEAREA DC    18F'0'                IBMABASE's save area.                      
SUBSAVE  DC    18F'0'                IBMABASE subroutine's save area.           
STATUS   DC    X'00'                 Status word.                               
TERMIN8  EQU   X'08'                 Termination processing (no CPRB)           
LOGOUT   EQU   X'04'                 Output header info is written              
LOGINP   EQU   X'02'                 Input header info is written               
OPENED   EQU   X'01'                 Data sets are opened.                      
CLOSED   EQU   X'00'                 Data sets are closed.                      
CPRBCHK  DC    CL4'CPRB'             CPRB self identifier                       
SENDREQ  DC    CL8'CPRB'             SENDREQ parameter identifier               
ENDCMD   DC    CL8'ENDCMD'           End-of-command identifier                  
WSCOMM   DC    CL8'WSCOMM'           Comm module termination identifier         
PURGE    DC    CL8'PURGE'            Abnormal termination identifier            
RESET    DC    CL8'RESET'            Server unload identifier                   
         LTORG                                                                  
         EJECT                                                                  
***********************************************************************         
* FSCB mapping                                                                  
***********************************************************************         
         SPACE                                                                  
INPUT    FSCB 'INPUT FILE A',FORM=E                                             
OUTPUT   FSCB 'OUTPUT FILE A',FORM=E                                            
LOG      FSCB 'LOG FILE A',FORM=E,RECFM=V                                       
         EJECT                                                                  
***********************************************************************         
* CPRB reply buffer mapping.                                                    
***********************************************************************         
             SPACE                                                              
LOGBUFF      DC    CL109' '                                                     
LOGLEN       EQU   *-LOGBUFF                                                    
REPLY        DC    CL109' '                                                     
REPLYLEN     EQU   *-REPLY                                                      
             SPACE                                                              
BLANK        DC    CL109' '                                                     
***********************************************************************         
* MESSAGES TO BE ISSUED                                                         
***********************************************************************         
REPMSG   DC    CL109' '              LOG_MSG Area                               
MSGBAD   DC    CL30'INVALID SERVICE REQUEST'                                    
MSGFNF   DC    CL30'INPUT FILE NOT FOUND'                                       
MSGOPN   DC    CL30'INPUT FILE OPEN ERRORS'                                     
MSGEND   DC    CL30'ENDCMD RECEIVED BY SERVER'                                  
MSGPUR   DC    CL30'PURGE RECEIVED BY SERVER'                                   
MSGRES   DC    CL30'RESET RECEIVED BY SERVER'                                   
MSGWSC   DC    CL30'WSCOMM RECEIVED BY SERVER'                                  
MSGLEN   DC    CL30'REPLY LENGTH TOO SMALL'                                     
MSGCPR   DC    CL30'CPRB NOT FOUND'                                             
MSGSNOC  DC    CL30'REPLY SENT BUT NOT CONFIRMED'                               
MSGEDEL  DC    CL30'NO REPLY SENT FOR LAST REQUEST'                             
MSGNRSS  DC    CL30'DELENTRY PROCESSING ERROR'                                  
INLOG    DC    CL30'CUSTOMER RECORDS READ ***'                                  
OUTLOG   DC    CL30'CUSTOMER RECORDS UPDATED'                                   
***********************************************************************         
* Server parameter list mapping.                                                
***********************************************************************         
PARAMTR    DSECT                                                                
           DS   CL8                                                             
REQTYP     DS   CL8                                                             
PTRS       DSECT                                                                
           DS   3F                                                              
CRBPTR     DS   F                                                               
         SPACE                                                                  
***********************************************************************         
* CPRB mapping                                                                  
***********************************************************************         
         CPRB DSECT=YES                                                         
         CSMRETCD                                                               
***********************************************************************         
* VM/SP DSECT Mappings                                                          
***********************************************************************         
         SPACE                                                                  
         REGEQU                                                                 
         SPACE                                                                  
FSCBD    FSCBD                                                                  
         EJECT                                                                  
         END   IBMABASE                                                         
