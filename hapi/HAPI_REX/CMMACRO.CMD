/****************************************************************
* Module Name:  CMMACRO.CMD                                     *
*                                                               *
* Descriptive Name: Rexx EHLLAPI Sample Program                 *
*                                                               *
* Copyright:    (C) Copyright IBM Corp. 1988, 1991              *
*               Licensed Material - Program Property of IBM     *
*               All Rights Reserved                             *
*                                                               *
* Status:       Release 1.0 Modification 0                      *
*                                                               *
* Function:     Record and Playback Host-session keystrokes.    *
*                                                               *
*               This sample program allows the user to 'record' *
*               and 'playback' sequences of keystrokes in the   *
*               host 3270 session.                              *
*                                                               *
* Restrictions: This sample program is provided solely as       *
*               an example of how Rexx may be used to invoke    *
*               Communications Manager EHLLAPI functions.       *
*                                                               *
* Module Type:  IBM Rexx Procedures Language                    *
****************************************************************/
parse arg session .
if session = "" then session = "A"
start_key = "@rr"; start_key_text = "Ctrl-R"
play_key  = "@rp"; play_key_text  = "Ctrl-P"
done_key  = "@rq"; done_key_text  = "Ctrl-Q"

say "Keystroke Macro Facility"
say "The following keys are redefined in" session "session."
say "  Start recording keystrokes   " start_key_text
say "  Stop  recording keystrokes   " start_key_text
say "  Play back recorded keystrokes" play_key_text
say "  Quit                         " done_key_text
say
say "Beeps are made to give audio feedback."
say "  1 beep  - start recording"
say "  2 beeps - stop recording"
say "  3 beeps - quitting"

if rxfuncquery("hllapi") then
   call rxfuncadd "hllapi","saahlapi","hllapisrv"

rc = hllapi("Set_session_parms","CONPHYS NORESET")
rc = hllapi("Connect",session)
if (rc \= 0) then return rc
rc = hllapi("Start_keystroke_intercept",session,"L")

done = 0; record = 0
do while \done
   key = hllapi("Get_key",session)
   select
      when (key = start_key) & record then
         do; warn(2); record = 0; end
      when (key = start_key) & \record then
         do; warn(1); record = 1; string = ""; end
      when (key = play_key) then
         rc = hllapi("Sendkey",string)
      when (key = done_key) then
         do; warn(3); done = 1; end
      when record then
         do; string = string || key; rc = hllapi("Sendkey",key); end
      when \record then
         rc = hllapi("Sendkey",key)
      otherwise nop
   end
end

rc = hllapi("Stop_keystroke_intercept",session)
rc = hllapi("Disconnect")
exit

warn: procedure
   do arg(1); beep(1000,250); end
   return ""
                                                                              