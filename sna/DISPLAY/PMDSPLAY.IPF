.****************************************************************************
.*                                                                          *
.*  MODULE NAME : PMDSPLAY.IPF                                              *
.*                                                                          *
.*  DESCRIPTIVE : COMMUNICATIONS MANAGER DISPLAY SAMPLE PROGRAM             *
.*  NAME                                                                    *
.*                                                                          *
.*  FUNCTION:   Source for help file PMDSPLAY.HLP                           *
.*                                                                          *
.*  MODULE:     IBM Information Presentation Facility Compiler Version 1.00 *
.*  TYPE        (IPFC.EXE; part of IBM OS/2 Programming Tools and           *
.*              Information Versions 1.2 and 1.3)                           *
.*                                                                          *
.*  ASSOCIATED FILES:  See PMDSPLAY.MAK                                     *
.*                                                                          *
.****************************************************************************
.*
.*/*---------------------------------------------------------------------*\
.* | The :userdoc tag MUST be on the first non-comment line in the file. |
.* | There may not be any blank lines preceeding it.  Since IPFC doesn't |
.* | have a pre-processor, and hence doesn't allow defines or includes,  |
.* | the panel-id's must be hard coded.                                  |
.*\*---------------------------------------------------------------------*/
:userdoc.

:prolog.

:title.Help

:eprolog.

:body.

:h1 name=Dlg1001 res=1001.Help For PMDSPLAY
.*
:i1.PMDSPLAY sample program
.*
:p.The :hp2.PMDSPLAY:ehp2. program displays information about the
active configuration of IBM Communications Manager.
Use the :link reftype=hd res=1200.Display:elink. Action Bar Item
to select the information to display.
.*
.*
:p.For help on any selectable field, select the field and press F1.
.*
:p.The PMDSPLAY sample program demonstrates the use of the
APPC management verbs DISPLAY and DISPLAY_APPN. Source for this program
is available from the CM Sample Programs 2 Diskette.
.*
:h1 name=Dlg1200 res=1200.Display Action Bar Item
.*
:i1.Display Action Bar Item
.*
:p.The :hp2.Display:ehp2. Action Bar Item contains lists from
which you can select the types of information to display.
:p.The :link reftype=hd res=1210.General SNA:elink. Menu Item
contains a list of types of information associated with
Advanced Program-To-Program Communications
and all Logical Unit types.
:p.The :link reftype=hd res=1230.APPN:elink. Menu Item
contains a list of types of information associated with
Advanced Peer-To-Peer Networking.
:p.The :link reftype=hd res=1140.Refresh:elink. Menu Item
refreshes the information currently being displayed.
:p.The :hp2.Exit:ehp2. Menu Item allows you to end PMDSPLAY at
any time.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h2 name=Dlg1210 res=1210.General SNA Menu Item
.*
:i1.Displaying General SNA Information
.*
:p.The :hp2.General SNA:ehp2. Menu Item contains a list of types of information
associated with Advanced Program-To-Program Communications and all
Logical Unit types.
To display one of these types of information, do the following:
:ul.
:li.Select the :hp2.Display:ehp2. Action Bar Item.
:li.Select the :hp2.General SNA:ehp2. Menu Item.
:li.Select a Menu Item from the following list:
:ul compact.
:li.:link reftype=hd res=1211.Adapters:elink.
:li.:link reftype=hd res=1212.Global SNA:elink.
:li.:link reftype=hd res=1213.Link Definitions:elink.
:li.:link reftype=hd res=1214.Links:elink.
:li.:link reftype=hd res=1215.Logical Units 0 to 3:elink.
:li.:link reftype=hd res=1216.Logical Unit 6.2:elink.
:li.:link reftype=hd res=1217.LU Definitions:elink.
:li.:link reftype=hd res=1218.Management Services:elink.
:li.:link reftype=hd res=1219.Mode Definitions:elink.
:li.:link reftype=hd res=1220.Partner LU Definitions:elink.
:li.:link reftype=hd res=1221.LU 6.2 Sessions:elink.
:li.:link reftype=hd res=1222.SNA Gateway:elink.
:li.:link reftype=hd res=1223.System Defaults:elink.
:li.:link reftype=hd res=1224.Transaction Program Definitions:elink.
:li.:link reftype=hd res=1225.Transaction Programs:elink.
:li.:link reftype=hd res=1226.X.25 Physical Links:elink.
:eul.
:eul.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1211 res=1211.Adapter Menu Item
.*
:i1.Displaying Adapter Information
.*
:p.The :hp2.Adapter:ehp2. Menu Item displays
the communications adapters configured for use by SNA.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1212 res=1212.Global SNA Menu Item
.*
:i1.Displaying Global SNA Information
.*
:p.The :hp2.Global SNA:ehp2. Menu Item displays
general information about the active configuration, such as
:ul compact.
:li.The control point name
:li.The node type
:li.The Communications Manager version and release
:eul.
.*
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1213 res=1213.Link Definitions Menu Item
.*
:i1.Displaying Link Definitions Information
.*
:p.The :hp2.Link Definitions:ehp2. Menu Item displays
the defined SNA logical links.
These SNA logical links include those defined by configuration
and those defined dynamically
when other computers activate SNA logical links to this computer.
.*
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1214 res=1214.Links Menu Item
.*
:i1.Displaying Links Information
.*
:p.The :hp2.Links:ehp2. Menu Item displays
the active SNA logical links.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1215 res=1215.Logical Units 0 to 3 Menu Item
.*
:i1.Displaying Logical Units 0 to 3 Information
.*
:p.The :hp2.Logical Units 0 to 3:ehp2. Menu Item displays
logical unit types 0, 1, 2, and 3.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1216 res=1216.Logical Unit 6.2 Menu Item
.*
:i1.Displaying Logical Unit 6.2 Information
.*
:p.The :hp2.Logical Unit 6.2:ehp2. Menu Item displays
all active type 6.2 logical units, their partner LUs, and their modes.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1217 res=1217.LU Definitions Menu Item
.*
:i1.Displaying LU Definitions Information
.*
:p.The :hp2.LU Definitions:ehp2. Menu Item displays
the defined type 6.2 logical units.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1218 res=1218.Management Services Menu Item
.*
:i1.Displaying Management Services Information
.*
:p.The :hp2.Management Services:ehp2. Menu Item displays management
services (MS) information, including the number of held alerts,
information about focal points, registered MS applications, and
active MS transactions.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1219 res=1219.Mode Definitions Menu Item
.*
:i1.Displaying Mode Definitions Information
.*
:p.The :hp2.Mode Definitions:ehp2. Menu Item displays
the defined modes.
These modes include those defined by configuration
and those defined dynamically by Communications Manager.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1220 res=1220.Partner LU Definitions Menu Item
.*
:i1.Displaying Partner LU Definitions Information
.*
:p.The :hp2.Partner LU Definitions:ehp2. Menu Item displays
the defined partner LUs.
These partner LUs include those defined by configuration
and those defined dynamically by Communications Manager.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1221 res=1221.LU 6.2 Sessions Menu Item
.*
:i1.Displaying LU 6.2 Sessions Information
.*
:p.The :hp2.LU 6.2 Sessions:ehp2. Menu Item displays active sessions.
These include the SSCP-PU session,
and SSCP-LU and LU-LU sessions for LU type 6.2,
but not the SSCP-LU and LU-LU sessions for LU types 0, 1, 2, and 3.
For information on these latter sessions,
see :hp2.Logical Units 0 to 3:ehp2..
.*
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1222 res=1222.SNA Gateway Menu Item
.*
:i1.Displaying SNA Gateway Information
.*
:p.The :hp2.SNA Gateway:ehp2. Menu Item displays information
about the SNA gateway, including dedicated and pooled LUs.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1223 res=1223.System Defaults Menu Item
.*
:i1.Displaying System Defaults Information
.*
:p.The :hp2.System Defaults:ehp2. Menu Item displays
default values used by Communications Manager.
These defaults are used primarily for the dynamic definition of
modes, partner LUs, and transaction programs that are not defined
explicitly by configuration.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1224 res=1224.Transaction Program Definitions Menu Item
.*
:i1.Displaying Transaction Program Definitions Information
.*
:p.The :hp2.Transaction Program Definitions:ehp2. Menu Item displays
the defined transaction programs.
These transaction programs include those defined by configuration
and those defined dynamically by Communications Manager.
:p.This information is also known as :hp2.Attach Manager:ehp2.
information.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1225 res=1225.Transaction Programs Menu Item
.*
:i1.Displaying Transaction Programs Information
.*
:p.The :hp2.Transaction Programs:ehp2. Menu Item displays
the active transaction programs
and any active conversations they may have.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1226 res=1226.X.25 Physical Links Menu Item
.*
:i1.Displaying X.25 Physical Links Information
.*
:p.The :hp2.X.25 Physical Links:ehp2. Menu Item displays information
about X.25 circuits.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h2 name=Dlg1230 res=1230.APPN Menu Item
.*
:i1.Displaying APPN Information
.*
:p.The :hp2.APPN:ehp2. Menu Item contains a list of types of information
associated with Advanced Peer-To-Peer Networking.
To display one of these types of information, do the following:
:ul.
:li.Select the :hp2.Display:ehp2. Action Bar Item.
:li.Select the :hp2.APPN:ehp2. Menu Item.
:li.Select a Menu Item from the following list:
:ul compact.
:li.:link reftype=hd res=1231.Class of Service:elink.
:li.:link reftype=hd res=1232.Connection Networks:elink.
:li.:link reftype=hd res=1233.Directory:elink.
:li.:link reftype=hd res=1234.Intermediate Sessions:elink.
:li.:link reftype=hd res=1235.Node:elink.
:li.:link reftype=hd res=1236.Topology:elink.
:eul.
:eul.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1231 res=1231.Class of Service Menu Item
.*
:i1.Displaying Class of Service Information
.*
:p.The :hp2.Class of Service:ehp2. Menu Item displays
the class of service definitions.
:p.A class of service designates the transport network characteristics,
such as route security, transmission priority, and bandwidth
needed for a particular session.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1232 res=1232.Connection Networks Menu Item
.*
:i1.Displaying Connection Networks Information
.*
:p.The :hp2.Connection Networks:ehp2. Menu Item displays
the connection networks to which the computer is attached.
:p.A connection network is a representation within an APPN network
of a shared-access transport facility such as a token-ring.
Each computer on the facility represents its ability to connect to
other computers on the facility by defining a single connection to
the connection network.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1233 res=1233.Directory Menu Item
.*
:i1.Displaying Directory Information
.*
:p.The :hp2.Directory:ehp2. Menu Item displays
the computer's local directory database.
:p.The APPN directory is a distributed database that lists
names of resources (in particular, logical units) and records
the CP name of the node where each resource is located.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1234 res=1234.Intermediate Sessions Menu Item
.*
:i1.Displaying Intermediate Sessions Information
.*
:p.The :hp2.Intermediate Sessions:ehp2. Menu Item displays information
about sessions being routed through the computer.
:p.This information is meaningful only in an APPN network node.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1235 res=1235.Node Menu Item
.*
:i1.Displaying Node Information
.*
:p.The :hp2.Node:ehp2. Menu Item displays
the settings of APPN networking parameters in the computer.
:p.This information is meaningful only in an APPN network node.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h3 name=Dlg1236 res=1236.Topology Menu Item
.*
:i1.Displaying Topology Information
.*
:p.The :hp2.Topology:ehp2. Menu Item displays
the APPN network topology database
(all network nodes and transmission groups between network nodes),
and the computer's local topology database
(transmission groups to end nodes and LEN nodes).
Only active transmission groups are displayed.
:p.This information is meaningful only in an APPN network node.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h2 name=Dlg1240 res=1240.Exit Menu Item
.*
:i1.Closing PMDSPLAY
.*
:p.When you select :hp2.Exit:ehp2., PMDSPLAY ends.
:p.Pressing Function Key 3 (F3) will also perform this function.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h1 name=Dlg1100 res=1100.Options Action Bar Item
.*
:i1.Options Action Bar Item
.*
:p.The :hp2.Options:ehp2. Action Bar Item allows you to put information
on the screen or in a file.
It also allows you to display the configuration of a remote machine, and
to refresh the information currently selected.
.*
:p.For help on a specific Menu Item in the Options, select the
item and press F1.
.*
:h2 name=Dlg1110 res=1110.Screen Menu Item
.*
:i1.Displaying To The Screen
.*
:p.When you select :hp2.Screen:ehp2., PMDSPLAY will display the
information you select in the window on the screen.
:p.Pressing Function Key 4 (F4) will also perform this function.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h2 name=Dlg1120 res=1120.File Menu Item
.*
:i1.Displaying To A File
.*
:p.When you select :hp2.File:ehp2., PMDSPLAY will write the
information you select to a file of your choice.
The special file names for printers (:hp2.PRN:ehp2., :hp2.LPT1:ehp2.,
:hp2.LPT2:ehp2., :hp2.LPT3:ehp2., and :hp2.LPT4:ehp2.) are allowed.
:p.Pressing Function Key 5 (F5) will also perform this function.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h2 name=Dlg1130 res=1130.Target Menu Item
.*
:i1.Displaying A Target Machine's Configuration
.*
:p.When you select :hp2.Target:ehp2., PMDSPLAY will attempt to start
a conversation with the target machine of your choice.  If it is
successful,
you can display the active configuration of that target machine.
:p.Pressing Function Key 6 (F6) will also perform this function.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h2 name=Dlg1140 res=1140.Refresh Menu Item
.*
:i1.Refreshing Information
.*
:p.The :hp2.Refresh:ehp2. Menu Item refreshes the information currently
being displayed.  To perform this function, do the following:
:ul.
:li.Select the :hp2.DISPLAY:ehp2. Action Bar Item.
:li.Select the :hp2.Refresh:ehp2. Menu Item.
:eul.
:p.Pressing Control-R (Ctrl-R) also performs this function.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h1 name=Dlg1300 res=1300.Help Action Bar Item
.*
:i1.Help Action Bar Item
.*
:p.The :hp2.Help:ehp2. Action Bar Item contains the following
Menu Items:
:dl break=all tsize=4.
:dt.Help for help
:dd.Displays information on how to use the help system
:dt.Extended help
:dd.Displays general help on the :hp2.PMDSPLAY:ehp2. program
:dt.Keys help
:dd.Displays help on the keys that can be used in the
:hp2.PMDSPLAY:ehp2. program
:dt.Help index
:dd.Displays a list of topics within the :hp2.PMDSPLAY:ehp2.
program for which help is available
:dt.About
:dd.Displays the program name.
:edl.
:p.For more information on each of these help items,
move the selection cursor to the item and press F1.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h2 name=Dlg1310 res=1310.Help for Help Menu Item
.*
:p.Select :hp2.Help for help:ehp2. if you want to know how to use the
help system.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h2 name=Dlg1320 res=1320.Extended Help Menu Item
.*
:p.Select :hp2.Extended Help:ehp2. if you want general help
on the :hp2.PMDSPLAY:ehp2. program.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h2 name=Dlg1330 res=1330.Help Index Menu Item
.*
:p.Select this item if you want a list of topics within the
:hp2.PMDSPLAY:ehp2. program for which help is available.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h2 name=Dlg1340 res=1340.Keys Help Menu Item
.*
:p.Select :hp2.Keys help:ehp2. to see a list of keys that
are used in the :hp2.PMDSPLAY:ehp2. program.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h2 name=Dlg1350 res=1350.Keys Help
.*
:p.The :hp2.PMDSPLAY:ehp2. program assigns the following
special functions to keys:
:dl.
:dt.F1
:dd.Invoke the Help Manager.
:dt.F3
:dd.Exit the PMDSPLAY program.
:dt.F4
:dd.Direct the results of the display to the screen.
:dt.F5
:dd.Direct the results of the display to a file.
:dt.F6
:dd.Display the configuration of a remote machine.
:dt.F10
:dd.Move the cursor to the Action Bar.
:dt.Ctrl-R
:dd.Refresh the Current Type of Information displayed.
:edl.
:p.For a list of the keys that can be used in Presentation Manager,
see :hp2.Keys help:ehp2..
This is available from every help panel :hp2.Help:ehp2. Action Bar Item.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h2 name=Dlg1360 res=1360.About Menu Item
.*
:i1.About Menu Item
.*
:p.Select :hp2.About:ehp2. if you want to know the full name of the
:hp2.PMDSPLAY:ehp2. sample program.
:p.For general information on PMDSPLAY, press F2 while
viewing this panel.
.*
:h1 name=Dlg1002 res=1002.File Selection Dialog
.*
:i1.File Selection Dialog
.*
:p.The :hp2.File Selection Dialog:ehp2. window allows you to choose
the file to which you wish to write the information you will select.
:p.You may choose the filename in several ways:
:dl.
:dt.Drives
:dd.Select the drive from this list box.
:dt.Directories
:dd.Select the directory from this list box.
:dt.Files
:dd.Select the filename from this list box.
:dt.Filename
:dd.Enter the fully or partially qualified filename in this field.
:edl.
:p.The sample uses the :hp2.Help Manager:ehp2. to display help on each
of the selectable fields in the window.
:p.For help on any selectable field, select the field and press F1.
.*
:h2 name=Dlg1400 res=1400.Drive List Box
.*
:i1.Drive List Box
.*
:p.The :hp2.Drive:ehp2. list box allows you to select the drive
of the file in which you wish to store the DISPLAY information.
:p.For help on a specific field, select the field and press F1.
:p.For general help on the :hp2.File Selection Dialog:ehp2.,
press F2 while in this panel.
.*
:h2 name=Dlg1410 res=1410.Directory List Box
.*
:i1.Directory List Box
.*
:p.The :hp2.Directory:ehp2. list box allows you to select the directory
of the file in which you wish to store the DISPLAY information.
:p.For help on a specific field, select the field and press F1.
:p.For general help on the :hp2.File Selection Dialog:ehp2.,
press F2 while in this panel.
.*
:h2 name=Dlg1420 res=1420.File List Box
.*
:i1.File List Box
.*
:p.The :hp2.File:ehp2. list box allows you to select the filename
of the file in which you wish to store the DISPLAY information.
:p.For help on a specific field, select the field and press F1.
:p.For general help on the :hp2.File Selection Dialog:ehp2.,
press F2 while in this panel.
.*
:h2 name=Dlg1430 res=1430.Filename Entry Field
.*
:i1.Filename Entry Field
.*
:p.The :hp2.Filename:ehp2. entry field allows you to enter the
fully or partially qualified name
of the file in which you wish to store the DISPLAY information.
:p.For help on a specific field, select the field and press F1.
:p.For general help on the :hp2.File Selection Dialog:ehp2.,
press F2 while in this panel.
.*
:h2 name=Dlg1440 res=1440.Append Check Box
.*
:i1.Append Check Box
.*
:p.The :hp2.Append:ehp2. check box allows you to
append the DISPLAY information to an existing file.
:p.For help on a specific field, select the field and press F1.
:p.For general help on the :hp2.File Selection Dialog:ehp2.,
press F2 while in this panel.
.*
:h2 name=Dlg1450 res=1450.OK Pushbutton
.*
:i1.OK Pushbutton (File Selection Dialog)
.*
:p.The :hp2.OK:ehp2. pushbutton completes the dialog.
Subsequent DISPLAY information will be copied to the file you chose.
:p.For help on a specific field, select the field and press F1.
:p.For general help on the :hp2.File Selection Dialog:ehp2.,
press F2 while in this panel.
.*
:h2 name=Dlg1460 res=1460.Cancel Pushbutton
.*
:i1.Cancel Pushbutton (File Selection Dialog)
.*
:p.The :hp2.Cancel:ehp2. pushbutton cancels the dialog
and returns to the main window without making any changes.
:p.For help on a specific field, select the field and press F1.
:p.For general help on the :hp2.File Selection Dialog:ehp2.,
press F2 while in this panel.
.*
:h2 name=Dlg1470 res=1470.Help Pushbutton
.*
:i1.Help Pushbutton (File Selection Dialog)
.*
:p.The :hp2.Help:ehp2. pushbutton requests help for the field
on which the cursor is resting.
:p.For help on a specific field, select the field and press F1.
:p.For general help on the :hp2.File Selection Dialog:ehp2.,
press F2 while in this panel.
.*
:h1 name=Dlg1003 res=1003.Target Selection Dialog
.*
:i1.Target Selection Dialog
.*
:p.The :hp2.Target Selection Dialog:ehp2. window allows you to choose
the machine whose configuration you wish to view.
:p.You may choose the target machine in one of the following ways:
:dl.
:dt.Partner LU Aliases and Names
:dd.You may select the partner LU name from this list box.
:dt.Partner LU
:dd.You may enter the fully qualified partner LU name or partner
LU alias in this field.
:edl.
:p.The sample uses the :hp2.Help Manager:ehp2. to display help on each
of the selectable fields in the window.
:p.For help on any selectable field, select the field and press F1.
.*
:h2 name=Dlg1500 res=1500.Partner LU Names List Box
.*
:i1.Partner LU Names List Box
.*
:p.The :hp2.Partner LU Names:ehp2. list box lists your currently
defined partner LUs and allows you to select one whose active
configuration you wish to display.
:p.For help on a specific field, select the field and press F1.
:p.For general help on the :hp2.Target Selection Dialog:ehp2.,
press F2 while in this panel.
.*
:h2 name=Dlg1510 res=1510.Partner LU Entry Field
.*
:i1.Partner LU Entry Field
.*
:p.The :hp2.Partner LU:ehp2. entry field allows you to enter the
name of the partner LU whose active configuration you wish to display.
:p.You may enter either a fully qualified partner LU name or a partner
LU alias in this field.
:p.For help on a specific field, select the field and press F1.
:p.For general help on the :hp2.Target Selection Dialog:ehp2.,
press F2 while in this panel.
.*
:h2 name=Dlg1520 res=1520.OK Pushbutton
.*
:i1.OK Pushbutton (Target Selection Dialog)
.*
:p.The :hp2.OK:ehp2. pushbutton completes the dialog.
PMDPSLAY will establish a conversation with the remote DISPLAY
server program, RDSPSRVR, in the computer containing the partner LU
you've selected, allowing you to display that computer's active
configuration.
:p.For help on a specific field, select the field and press F1.
:p.For general help on the :hp2.Target Selection Dialog:ehp2.,
press F2 while in this panel.
.*
:h2 name=Dlg1530 res=1530.Cancel Pushbutton
.*
:i1.Cancel Pushbutton (Target Selection Dialog)
.*
:p.The :hp2.Cancel:ehp2. pushbutton cancels the dialog
and returns to the main window without making any changes.
:p.For help on a specific field, select the field and press F1.
:p.For general help on the :hp2.Target Selection Dialog:ehp2.,
press F2 while in this panel.
.*
:h2 name=Dlg1540 res=1540.Help Pushbutton
.*
:i1.Help Pushbutton (Target Selection Dialog)
.*
:p.The :hp2.Help:ehp2. pushbutton request helps for the field
on which the cursor is resting.
:p.For help on a specific field, select the field and press F1.
:p.For general help on the :hp2.Target Selection Dialog:ehp2.,
press F2 while in this panel.
.*
.*
:ehelp.
:euserdoc.
