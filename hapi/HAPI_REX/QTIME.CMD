/****************************************************************
* Module Name:  QTIME.CMD                                       *
*                                                               *
* Descriptive Name: Rexx EHLLAPI Sample Program                 *
*                                                               *
* Copyright:    (C) Copyright IBM Corp. 1988, 1991              *
*               Licensed Material - Program Property of IBM     *
*               All Rights Reserved                             *
*                                                               *
* Status:       Release 1.0 Modification 0                      *
*                                                               *
* Function:     Set PC system time using the VM host time.      *
*                                                               *
*               This sample program queries the 3270 VM host    *
*               time using the CP Query Time command. It then   *
*               uses that time to set the PC time-of-day using  *
*               the OS/2 TIME command.                          *
*                                                               *
* Restrictions: This sample program is provided solely as       *
*               an example of how Rexx may be used to invoke    *
*               Communications Manager EHLLAPI functions.       *
*                                                               *
* Module Type:  IBM Rexx Procedures Language                    *
****************************************************************/
trace value all
signal on halt   name quit              /* Handle error conditions so ... */
signal on error  name quit              /* ... are sure to diconnect      */
signal on syntax name quit

session='A'                             /* Use 3270 session A */

if Rxfuncquery('hllapi') Then           /* Is Rexx EHLLAPI support loaded?*/
  call Rxfuncadd 'HLLAPI','SAAHLAPI','HLLAPISRV' /* If not, load it..   . */

rc=hllapi('Connect',session)            /* Connect to host session A      */
if rc<>0 then do                        /* If did not work, message & exit*/
  say 'Did not connect to host. RC='rc
  signal quit
  end

rc=hllapi('Wait')                       /* is host ready for keystrokes?  */
if rc<>0 then do                        /* if not, give a message & exit  */
  say 'Host keyboard locked.'
  signal quit
  end

rc=hllapi('Sendkey', '@C')              /* Send a clear key */
rc=hllapi('Wait')                       /* Wait for clear key to complete */
rc=hllapi('Sendkey', '@C')              /* Send a clear key */
rc=hllapi('Wait')                       /* Wait for clear key to complete */

rc=hllapi('Sendkey', 'CP Query Time @E')/* Send CP Query Time and Enter   */
rc=hllapi('Wait')                       /* Wait for query time to complete*/

pos=hllapi('Search_ps','TIME IS ',1)    /* Look for TIME IS phrase on host*/
if pos=0 then do                        /* If not found, message and exit */
  say 'Host could not process QUERY TIME command.'
  signal quit
  end
                                        /* Copy time XX:XX:XX from host   */
time=hllapi('Copy_ps_to_str', pos + Length('TIME IS '), 8)

'@time' time                            /* Set PC time using host time    */

rc=hllapi('Sendkey', '@C')              /* Reset host screen by clearing  */
rc=hllapi('Wait')

quit:
call hllapi 'disconnect'                /* End the rexx-hllapi connection */
call hllapi 'reset_system'
exit
                   