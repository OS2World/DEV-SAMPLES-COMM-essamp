@echo OFF
cls
ECHO ************************************************************************
ECHO * APPCMGMT Sample Program: APPCMGMT.C                                  *
ECHO *                                                                      *
ECHO * COPYRIGHT:                                                           *
ECHO * ----------                                                           *
ECHO * Copyright (C) International Business Machines Corp., 1991            *
ECHO *                                                                      *
ECHO *                                                                      *
ECHO *  File list:                                                          *
ECHO *                                                                      *
ECHO *   APPCMGMT.C    - source code to create the EXE                      *
ECHO *   APPCMGMT.H    - header file for APPCMGMT.C                         *
ECHO *   APPCMGMT.DEF  - module definition file for APPCMGMT.C              *
ECHO *   B32MGMT.CMD   - command file to link the program dynamically       *
ECHO ************************************************************************
PAUSE
SETLOCAL
ECHO ***************************************************************************
ECHO * Building program                                                        *
ECHO *                                                                         *
ECHO * icc  /Gt  /C+ /G3  /Sm appcmgmt.c                                       *
ECHO *                                                                         *
ECHO * compiler options:                                                       *
ECHO *                                                                         *
ECHO *   /Gt   :  enable all varibles to be passed to 16 bit functions         *
ECHO *   /C+   :  perform compile only, no link                                *
ECHO *   /G3   :  optimize code for a 386 processor; code will run on a 486    *
ECHO *   /Sm   :  Allow migration extensions                                   *
ECHO *                                                                         *
ECHO ***************************************************************************
icc  /Gt  /C+ /G3  /Sm appcmgmt.c
IF ERRORLEVEL 0 GOTO LINK
ECHO ERROR:  Failed to compile APPCMGMT.C
ECHO REMEDY: Make sure that you have installed the correct components,
ECHO         and try again
PAUSE
GOTO END
:LINK
ECHO ***************************************************************************
ECHO * Linking program                                                         *
ECHO *                                                                         *
ECHO * link386 /NOE /NOI appcmgmt,appcmgmt,appcmgmt,acs,appcmgmt.def           *
ECHO *                                                                         *
ECHO * link options:                                                           *
ECHO *                                                                         *
ECHO *   /NOE  - no extra dictionary                                           *
ECHO *   /NOI  - differentiate between cases                                   *
ECHO *                                                                         *
ECHO *  LIBRARIES:                                                             *
ECHO *    DDE4SBS  - Statically bound, single thread, standard library         *
ECHO *    DDE4SBM  - Statically bound, single thread, migration library        *
ECHO *    ACS      - Communication library from CMLIB                          *
ECHO *    OS2386   - OS/2 import library                                       *
ECHO *                                                                         *
ECHO *  APPCMGMT.DEF - module definition file                                  *
ECHO *                                                                         *
ECHO ***************************************************************************
link386  /NOE /NOI appcmgmt,appcmgmt,appcmgmt,acs,appcmgmt.def
IF ERRORLEVEL 0 GOTO END
ECHO ERROR:  Failed to create APPCMGMT.EXE
ECHO REMEDY: Make sure that you have installed the correct components,
ECHO         and try again
PAUSE
:END
ENDLOCAL
