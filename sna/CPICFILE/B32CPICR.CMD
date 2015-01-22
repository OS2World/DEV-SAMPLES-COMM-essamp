@echo OFF
cls
ECHO ************************************************************************
ECHO * CPICCREQ Sample Program: CPICCREQ.C                                  *
ECHO *                                                                      *
ECHO * COPYRIGHT:                                                           *
ECHO * ----------                                                           *
ECHO * Copyright (C) International Business Machines Corp., 1991            *
ECHO *                                                                      *
ECHO *                                                                      *
ECHO *  File list:                                                          *
ECHO *                                                                      *
ECHO *   CPICCREQ.C    - source code to create the EXE                      *
ECHO *   CPICCREQ.H    - header file for CPICCREQ.C                         *
ECHO *   CPICCREQ.DEF  - module definition file for CPICCREQ.C              *
ECHO *   B32CPICR.CMD  - command file to link the program dynamically       *
ECHO ************************************************************************
PAUSE
SETLOCAL
ECHO ***************************************************************************
ECHO * Building program                                                        *
ECHO *                                                                         *
ECHO * icc  /Gt  /C+ /G3  /Sm cpiccreq.c                                       *
ECHO *                                                                         *
ECHO * compiler options:                                                       *
ECHO *                                                                         *
ECHO *   /Gt   :  enable all varibles to be passed to 16 bit functions         *
ECHO *   /C+   :  perform compile only, no link                                *
ECHO *   /G3   :  optimize code for a 386 processor; code will run on a 486    *
ECHO *   /Sm   :  Allow migration extensions                                   *
ECHO *                                                                         *
ECHO ***************************************************************************
icc  /Gt  /C+ /G3  /Sm cpiccreq.c
IF ERRORLEVEL 0 GOTO LINK
ECHO ERROR:  Failed to compile CPICCREQ.C
ECHO REMEDY: Make sure that you have installed the correct components,
ECHO         and try again
PAUSE
GOTO END
:LINK
ECHO ***************************************************************************
ECHO * Linking program                                                         *
ECHO *                                                                         *
ECHO * link386  /NOE /NOI cpiccreq,cpiccreq,cpiccreq,acs,cpiccreq.def          *
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
ECHO *    CPIC     - CPIC library from CMLIB                                   *
ECHO *    OS2386   - OS/2 import library                                       *
ECHO *                                                                         *
ECHO *  CPICCREQ.DEF - module definition file                                  *
ECHO *                                                                         *
ECHO ***************************************************************************
link386 /NOE /NOI cpiccreq,cpiccreq,cpiccreq,acs+cpic,cpiccreq.def
IF ERRORLEVEL 0 GOTO END
ECHO ERROR:  Failed to create CPICCREQ.EXE
ECHO REMEDY: Make sure that you have installed the correct components,
ECHO         and try again
PAUSE
:END
ENDLOCAL
