
                           READ.ME (FILE)



                  IBM EXTENDED SERVICES FOR OS/2
            COMMUNICATIONS MANAGER API SAMPLE PROGRAMS SUPPORT


   Version 1.0 (c) Copyright International Business Machines Corp., 1988, 1991

   LICENSE: You may copy and distribute the sample programs on this
            diskette in any form without payment to IBM, for the purpose
            of developing, using, marketing or distributing application
            programs for use with Extended Services.

   The Sample Programs are available on one 3.5 inch diskette and on one
   5.25 diskette.  The sample diskettes are labeled "Extended Services
   for OS/2 Sample Programs".

   The material on these diskettes could include technical inaccuracies, or
   incompatibilities with IBM products, and could be modified or excluded
   from the generally available reference materials.  IBM expressly reserves
   the right, without notice to you, to implement or modify IBM Operating
   System/2 and other IBM products in a manner that affects the compatibility
   or useability of application programs developed by you using the materials
   on these diskettes.

   International Business Machines Corporation provides these diskettes
   "as is" without warranty of any kind, either express or implied, including,
   but not limited to, the implied warranties of merchantability or fitness
   for a particular purpose.

   It is possible that these diskettes may contain references to, or
   information about, IBM products (machines and programs), programming or
   services that are not announced in your country.  Such references or
   information must not be construed to mean that IBM intends to announce
   such IBM products, programming or services in your country.

  The diskettes contain documentation, source files, and the supporting
  files of the sample programs for the following Communications Manager
  Application Program Interfaces (APIs):

     1) APPC, CPI-C and System Management Interfaces
     2) Asynchronous Communication Device Interface (ACDI)
     3) Server-Requester Programming Interface (SRPI)
     4) Emulator High Level Language API (EHLLAPI)
     5) X.25 API (X25)
     6) Conventional LU Application API (LUA)
     7) X.25 adapter reactivation sample program
     8) Stopping Communications Manager sample program
     9) Sample REXX procedures for Programmable Configuration
    10) NETBIOS API
    11) IEEE 802.2 API

  The assembler sample programs and related instructions have been developed
  for use with IBM Macro Assembler/2 (or equivalent).

  The COBOL sample programs and related instructions have been developed
  for use with Microfocus COBOL (or equivalent).

  The C sample programs and related instructions have been developed
  for use with one or both of the following C compilers:
     o  IBM C Set/2 (for 32-bit sample programs)
     o  Microsoft C Version 6.0 or equivalent (for 16-bit sample programs)

  The BASIC sample programs and related instructions have been developed
  for use with IBM BASIC/2 (or equivalent).

  The REXX sample programs and related instructions have been developed
  for use with IBM REXX.

  The following files are found in the root directory of the Communications
  Manager Sample Programs Diskette:

    READ.ME       Sample program information
    CPYRIGHT.DAT  Copyright information
    SNA.ZIP       APPC, CPI-C, System Management samples
    ACDI.ZIP      ACDI samples
    SRPI.ZIP      SRPI samples
    HAPI.ZIP      EHLLAPI samples
    X25.ZIP       X.25 samples
    LUA.ZIP       LUA samples
    REACTX25.ZIP  X.25 adapter reactivation sample
    CMSTOP.ZIP    Sample to stop Communications Manager
    PGM_CFG.ZIP   Sample REXX procedures for Programmable Configuration
    NETBIOS.ZIP   NETBIOS sample
    IEEE8022.ZIP  IEEE 802.2 sample

   /*******************************************************************/
   /                                                                   /
   /            INSTALLATION INSTRUCTIONS                              /
   /                                                                   /
   /*******************************************************************/

    The sample programs and associated files are stored in a compressed
    format on the sample programs diskette.  The PKUNZIP2.EXE program
    is required to unpack the sample programs.  PKUNZIP2.EXE is automatically
    installed in d:\OS2\INSTALL by the Extended Services installation
    program (where d: is the boot drive active when Extended Services
    is installed).

    To install:

    1.  Insert the CM Sample Programs Diskette into your diskette drive.
        In the examples below, the A: drive is used.
    2.  From the OS/2 command line, type one or more of the following
        commands.  If "d:outpath" is omitted, the current working directory
        will be used as "outpath".  Note that the column on the right
        describes the approximate amount of storage required in megabytes
        on the target drive, and is not part of the command used to
        unpack the files:

        pkunzip2 -d a:sna.zip d:outpath                        (1.30 Mb)
        pkunzip2 -d a:acdi.zip d:outpath                       ( .25 Mb)
        pkunzip2 -d a:srpi.zip d:outpath                       ( .20 Mb)
        pkunzip2 -d a:hapi.zip d:outpath                       ( .40 Mb)
        pkunzip2 -d a:x25.zip d:outpath                        ( .25 Mb)
        pkunzip2 -d a:lua.zip d:outpath                        ( .25 Mb)
        pkunzip2 -d a:reactx25.zip d:outpath                   ( .04 Mb)
        pkunzip2 -d a:cmstop.zip d:outpath                     ( .04 Mb)
        pkunzip2 -d a:pgm_cfg.zip d:outpath                    ( .05 Mb)
        pkunzip2 -d a:netbios.zip d:outpath                    ( .03 Mb)
        pkunzip2 -d a:ieee8022.zip d:outpath                   ( .06 Mb)

        The following directories will be created when you install SNA.ZIP
             outpath\SNA
             outpath\SNA\APPCMGMT
             outpath\SNA\APPCTELL
             outpath\SNA\CPICFILE
             outpath\SNA\CPICOBOL
             outpath\SNA\CPICREXX
             outpath\SNA\DISPLAY
             outpath\SNA\FILE
             outpath\SNA\FILECFG

        The following directories will be created when you install ACDI.ZIP
             outpath\ACDI
             outpath\ACDI\ACDI_C
             outpath\ACDI\ACDI_M

        The following directories will be created when you install SRPI.ZIP
             outpath\SRPI
             outpath\SRPI\SRPI_C
             outpath\SRPI\SRPI_M
             outpath\SRPI\SRPI_CBL
             outpath\SRPI\HOST

        The following directories will be created when you install HAPI.ZIP
             outpath\HAPI
             outpath\HAPI\HAPI_C
             outpath\HAPI\HAPI_M
             outpath\HAPI\HAPI_CBL
             outpath\HAPI\HAPI_BAS
             outpath\HAPI\HAPI_REX

        The following directories will be created when you install X25.ZIP
             outpath\X25
             outpath\X25\X25_C
             outpath\X25\X25_M

        The following directories will be created when you install LUA.ZIP
             outpath\LUA
             outpath\LUA\LUA_C
             outpath\LUA\LUA_M
             outpath\LUA\LUA_CBL

        The following directories will be created when you install REACTX25.ZIP
             outpath\REACTX25
             outpath\REACTX25\RX25_C

        The following directories will be created when you install CMSTOP.ZIP
             outpath\CMSTOP
             outpath\CMSTOP\CMSTOP_C

        The following directory will be created when you install PGM_CFG.ZIP
             outpath\PGM_CFG

        The following directory will be created when you install NETBIOS.ZIP
             outpath\NETBIOS

        The following directory will be created when you install IEEE8022.ZIP
             outpath\IEEE8022

  The following is a brief discussion of the documentation and sample
  programs installed with each of the ZIP files:

  SNA:
    APPCMGMT sub-directory:
      Contains documentation and source for a Subsystem Management
      sample program.  The sample program issues verbs such as Deactivate
      Logical Link and Deactivate Session.  The sample is written in C.
      APPCMGMT.DOC contains information which may be useful when
      configuring, compiling and running the sample program.  APPCMGMT.C
      can be used to create either a 32-bit or 16-bit program.

    APPCTELL sub-directory:
      Contains documentation and source for an APPC sample program that
      sends a VIOPopUp message to the specified Partner LU.  The sample
      is written in C.  APPCTELL.DOC contains information which may be
      useful when configuring, compiling and running the sample program.
      APPCTELL.C can be used to create either a 32-bit or 16-bit program.

    CPICOBOL sub-directory:
      Contains documentation and source for sample programs obtained from
      The CPI-Communications Reference.  The samples are written in
      COBOL.  CPICOBOL.DOC contains information which may be
      useful when configuring, compiling and running the sample program.

    CPICFILE sub-directory:
      Contains documentation and source for sample file transfer programs
      using CPI-Communications.  The samples are written in C.
      CPICFILE.DOC contains information which may be useful when configuring,
      compiling and running the sample programs.  CPICCSVR.C and
      CPICCREQ.C can be used to create either 16-bit or 32-bit programs.

    CPICREXX sub-directory:
      Contains documentation and source for sample programs that set and
      extract CPI-Communications side information.  The samples are
      written in REXX.  CPICREXX.DOC contains information which may be
      useful when configuring, compiling and running the sample program.

    DISPLAY sub-directory:
      Contains documentation and source for sample programs that request
      and format information using the DISPLAY and DISPLAY_APPN verbs.
      Both command line and Presentation Manager versions are included. The
      samples are written in C.  DISPLAY.DOC, PMDSPLAY.DOC and RDSPSRVR.DOC
      contain information which may be useful when configuring, compiling and
      running the sample programs.

    FILE sub-directory:
      Contains documentation and source for sample APPC file transfer
      programs.  The samples are written in C.  FILE.DOC contains
      information which may be useful when configuring, compiling and
      running the sample program.

    FILECFG sub-directory:
      Contains documentation and source for a configuration sample
      program.  The sample issues verbs such as Define Local LU and
      Define TP.  The sample is written in C.  FILECFG.DOC contains
      information which may be useful when configuring, compiling and
      running the sample program.

  ACDI:

    The ACDI directory contains documentation and sample programs for the
    ACDI API. ACDI.DOC file contains the documentation and the instructions
    for the ACDI sample programs. The subdirectories contain the sample
    programs source code and the supporting files.

  SRPI:

    The SRPI directory contains documentation and sample programs for the
    SRPI API. SRPI.DOC file contains the documentation and the instructions
    for the SRPI sample programs. The subdirectories contain the sample
    programs source code and the supporting files.

  HAPI:

    The HAPI directory contains documentation and sample programs for the
    EHLLAPI API. HAPI.DOC file contains the documentation and the
    instructions for the EHLLAPI sample programs and EHLLAPI Program
    Sampler. The subdirectories contain the sample programs source code
    and the supporting files.

  X25:

    The X25 directory contains documentation and sample programs for the
    X.25 API. X25.DOC file contains the documentation and the instructions
    for the X.25 API sample programs. The subdirectories contain the
    sample programs source code and the supporting files.

  LUA:

    The LUA directory contains documentation and sample programs for the
    LUA API. LUA.DOC file contains the documentation and the instructions
    for the LUA API sample programs. The subdirectories contain the
    sample programs source code and the supporting files.

  REACTX25:

    The REACTX25 directory contains documentation and a sample program for
    the automatic reactivation of the X.25 adapter when either a link
    or adapter failure occurs.  REACTX25.DOC contains the documentation
    and the instructions for the REACTX25 sample program.  The subdirectory
    contains the sample program source code and the supporting files.
    The sample program is written in C.

  CMSTOP:

    The CMSTOP directory contains documentation and a sample program for
    stopping Communications Manager.  CMSTOP.DOC contains the documentation
    and the instructions for the CMSTOP sample program.  The subdirectory
    contains the sample program source code and the supporting files.
    The sample program is written in C.

  PGM_CFG:

    The PGM_CFG directory contains documentation and sample REXX programs
    for use with Programmable Configuration.  PGM_CFG.DOC contains the
    documentation and the instructions for the sample REXX procedures.

  NETBIOS:

    The NETBIOS directory contains documentation and a sample program for
    the NETBIOS Dynamic Link interface.  NETBIOS.DOC contains the
    documentation and the instructions for the NETBIOS sample program.
    The NETBIOS directory contains the sample program source code and the
    supporting files.  The sample program is written in C.

  IEEE8022:

    The IEEE8022 directory contains documentation and a sample program for
    the IEEE 802.2 Dynamic Link interface.  IEEE8022.DOC contains the
    documentation and the instructions for the IEEE 802.2 sample program.
    The IEEE8022 directory contains the sample program source code and the
    supporting files.  The sample program is written in C.

  It is recommended that you print the READ.ME and associated documentation
  files (all with an extension of .DOC) using the IBM PC DOS PRINT command
  or OS/2 PRINT command. If you do not have access to a printer, use the
  DOS TYPE or OS/2 TYPE command or an available editor to view these files.

  The following is a detailed listing of the various files that will be
  installed with each of the ZIP files:

      outpath\SNA directory contents:
        APPCMGMT    - APPC subsystem management directory
        APPCTELL    - APPC tell program directory
        CPICFILE    - CPIC file transfer directory
        CPICOBOL    - CPIC COBOL directory
        CPICREXX    - CPIC REXX directory
        DISPLAY     - Display sample program directory
        FILE        - APPC file transfer program directory
        FILECFG     - Configuration program directory

        outpath\SNA\APPCMGMT directory contents:
          APPCMGMT.C
          APPCMGMT.DEF
          APPCMGMT.DOC
          APPCMGMT.EXE
          APPCMGMT.LRF
          APPCMGMT.MAK
          B32MGMT.CMD

        outpath\SNA\APPCTELL directory contents:
          APPCTELL.C
          APPCTELL.DEF
          APPCTELL.DOC
          APPCTELL.EXE
          APPCTELL.LRF
          APPCTELL.MAK
          B32TELL.CMD

        outpath\SNA\CPICFILE directory contents:
          APC.MSG
          APC.TXT
          CPICBOTH.NDF
          CPICCREQ.BND
          CPICCREQ.C
          CPICCREQ.DEF
          CPICCREQ.EXE
          CPICCREQ.LRF
          CPICCREQ.MAK
          CPICCSVR.BND
          CPICCSVR.C
          CPICCSVR.DEF
          CPICCSVR.EXE
          CPICCSVR.LRF
          CPICCSVR.MAK
          CPICFILE.DOC
          CPICREQ.NDF
          CPICSVR.NDF
          B32CPICS.CMD
          B32CPICR.CMD

        outpath\SNA\CPICOBOL directory contents:
          CPICOBOL.CMD
          CPICOBOL.DOC
          CREDRPT.CBL
          CREDRPT.DEF
          CREDRPT.EXE
          CREDRPT.NDF
          DEFSIDE.CBL
          DEFSIDE.DEF
          DEFSIDE.EXE
          DELSIDE.CBL
          DELSIDE.DEF
          DELSIDE.EXE
          SALESRPT.CBL
          SALESRPT.DEF
          SALESRPT.EXE
          SALESRPT.NDF

        outpath\SNA\CPICREXX directory contents:
          CPICREXX.DOC
          XCMESI.CMD
          XCMSSI.CMD

        outpath\SNA\DISPLAY directory contents:
          ADAPTER.C
          AM.C
          APPCUTIL.C
          CN.C
          COS.C
          DEFAULTS.C
          DIR.C
          DISPLAY.C
          DISPLAY.DEF
          DISPLAY.DOC
          DISPLAY.H
          DISPLAY.MAK
          DISPUTIL.C
          EXECDISP.C
          GW.C
          ISR.C
          LINKS.C
          LINK_DEF.C
          LU03.C
          LU62.C
          LU_DEF.C
          MODE_DEF.C
          MS.C
          MSGID.H
          NODE.C
          PLU_DEF.C
          PMDSPLAY.C
          PMDSPLAY.DEF
          PMDSPLAY.DOC
          PMDSPLAY.H
          PMDSPLAY.ICO
          PMDSPLAY.IPF
          PMDSPLAY.MAK
          PMDSPLAY.RC
          PMDSPRC.DEF
          PMD_DLGS.C
          PMD_MAIN.C
          PMD_UTIL.C
          PMRCSTUB.C
          RDSPSRVR.C
          RDSPSRVR.DEF
          RDSPSRVR.DOC
          RDSPSRVR.MAK
          SESS.C
          SNA.C
          TOP.C
          TP.C
          X25.C
          APD.TXT

        outpath\SNA\FILE directory contents:
          APX.MSG
          APX.TXT
          FILE.DOC
          FILEBOTH.NDF
          FILECREQ.BND
          FILECREQ.C
          FILECREQ.DEF
          FILECREQ.EXE
          FILECREQ.LRF
          FILECREQ.MAK
          FILECSVR.BND
          FILECSVR.C
          FILECSVR.DEF
          FILECSVR.EXE
          FILECSVR.LRF
          FILECSVR.MAK
          FILEREQ.NDF
          FILESVR.NDF

        outpath\SNA\FILECFG directory contents:
          FILECFG.C
          FILECFG.DEF
          FILECFG.DOC
          FILECFG.EXE
          FILECFG.LRF
          FILECFG.MAK

      outpath\ACDI directory contents:
        ACDI.DOC - ACDI Sample Program documentation
        ACDI_C   - ACDI C language files subdirectory
        ACDI_M   - ACDI ASSEMBLER language files subdirectory

        outpath\ACDI\ACDI_C directory contents:
          ACDICRCV.C   - Source file
          ACDICRCV     - Make file
          ACDICRCV.DEF - Definition file
          ACDICRCV.LRF - Link file
          ACDICRCV.BND - Bind file
          ACDICXMT.C   - Source File
          ACDICXMT     - Make file
          ACDICXMT.DEF - Definition file
          ACDICXMT.LRF - Link file
          ACDICXMT.BND - Bind file
          ACX.TXT      - Message Text file

        outpath\ACDI\ACDI_M directory contents:
          ACDIMRCV.ASM - Source file
          ACDIMRCV     - Make file
          ACDIMRCV.DEF - Definition file
          ACDIMRCV.LRF - Link file
          ACDIMRCV.BND - Bind file
          ACDIMXMT.ASM - Source File
          ACDIMXMT     - Make file
          ACDIMXMT.DEF - Definition file
          ACDIMXMT.LRF - Link file
          ACDIMXMT.BND - Bind file
          ACX.TXT      - Message Text file

      outpath\SRPI directory contents:
        SRPI.DOC - SRPI Sample Program documentation
        SRPI_C   - SRPI C language files subdirectory
        SRPI_M   - SRPI ASSEMBLER language files subdirectory
        SRPI_CBL - SRPI COBOL language files subdirectory
        HOST     - SRPI Server files subdirectory

        outpath\SRPI\SRPI_C directory contents:
          CSAMPL.C     - Source file
          CSAMPL       - Make file
          CSAMPL.DEF   - Definition file
          CSAMPL.LRF   - Link file

        outpath\SRPI\SRPI_M directory contents:
          MSAMPL.ASM   - Source file
          MSAMPL       - Make file
          MSAMPL.DEF   - Definition file
          MSAMPL.LRF   - Link file

        outpath\SRPI\SRPI_CBL directory contents:
          BSAMPL.CBL   - Source file
          BSAMPMAK.CMD - Command file to compile and link
          BSAMPL.DEF   - Definition file
          BSAMPL.LRF   - Link file

        outpath\SRPI\HOST directory contents:
          IBMABASE.ASM
          ALGABASE.EXC
          INPUT.FIL

      outpath\HAPI directory contents:
        HAPI.DOC    - EHLLAPI Sample Program documentation
        HAPI_C      - EHLLAPI C language files subdirectory
        HAPI_M      - EHLLAPI ASSEMBLER language files subdirectory
        HAPI_CBL    - EHLLAPI COBOL language files subdirectory
        HAPI_BAS    - EHLLAPI BASIC Compiler language files subdirectory
        HAPI_REX    - EHLLAPI REXX language files subdirectory
        HLLSAMP.EXE - EHLLAPI Program Sampler executable file.

        outpath\HAPI\HAPI_C directory contents:
          HSMPLC.C     - Source file
          HSMPLC       - Make file
          HSMPLC.DEF   - Definition file
          HSMPLC.LRF   - Link file
          HSMPLC32.C   - 32-bit source file
          HSMPLC32     - 32-bit make file
          HSMPLC32.DEF - 32-bit definition file
          HSMPLC32.LRF - 32-bit link file

        outpath\HAPI\HAPI_M directory contents:
          HSMPLM.ASM   - Source file
          HSMPLM       - Make file
          HSMPLM.DEF   - Definition file
          HSMPLM.LRF   - Link file

        outpath\HAPI\HAPI_CBL directory contents:
          HSMPLCBL.CBL - Source file
          HSMPLCBB.CMD - Command file to compile and link
          HSMPLCBL.DEF - Definition file
          HSMPLCBL.LRF - Link file

        outpath\HAPI\HAPI_BAS directory contents:
          HSMPLBAS.BAS - Source file
          HSMPLBSB.CMD - OS/2 Protect mode batch file to compile/link
          HSMPLBAS.DEF - Definition file
          HSMPLBAS.LRF - Link file

        outpath\HAPI\HAPI_REX directory contents:
          QTIME.CMD    - REXX command file (Query time)
          CMMACRO.CMD  - REXX command file (record/replay host keystrokes)

      outpath\X25 directory contents:
        X25.DOC     - X.25 sample program documentation
        X25_C       - X.25 C language files subdirectory
        X25_M       - X.25 ASSEMBLER language files subdirectory

        outpath\X25\X25_C directory contents:
          X25CRCV.C    - Source file
          X25CRCV      - Make file
          X25CRCV.DEF  - Definition file
          X25CRCV.LRF  - Link file
          X25CXMIT.C   - Source File
          X25CXMIT     - Make file
          X25CXMIT.DEF - Definition file
          X25CXMIT.LRF - Link file

        outpath\X25\X25_M directory contents:
          X25MRCV.ASM  - Source file
          X25MRCV      - Make file
          X25MRCV.DEF  - Definition file
          X25MRCV.LRF  - Link file
          X25MXMIT.ASM - Source File
          X25MXMIT     - Make file
          X25MXMIT.DEF - Definition file
          X25MXMIT.LRF - Link file

        outpath\LUA directory contents:
          LUA.DOC      - LUA sample program document
          LUA_C        - LUA C language files subdirectory
          LUA_M        - LUA ASSEMBLER language files subdirectory
          LUA_O        - LUA COBOL language files subdirectory
          VTAMPGMA.JCL - Sample VTAM application

        outpath\LUA\LUA_C directory contents:
          LUASAMPC.C   - Source file
          LUASAMPC     - Make file
          LUASAMPC.DEF - Definition file
          LUASAMPC.LRF - Link file

        outpath\LUA\LUA_M directory contents:
          LUASAMPM.ASM - Source file
          LUASAMPM     - Make file
          LUASAMPM.DEF - Definition file
          LUASAMPM.LRF - Link file

        outpath\LUA\LUA_CBL directory contents:
          LUASAMPO.CBL - Source file
          LUASAMPO.CMD - Command file to compile/link
          LUASAMPO.DEF - Definition file
          LUASAMPO.LRF - Link file

        outpath\REACTX25 directory contents:
          REACTX25.DOC - REACTX25 sample program documentation
          RX25_C       - REACTX25 C language files subdirectory

        outpath\REACTX25\RX25_C directory contents:
          REACTX25.C   - Source file
          REACTX25     - Make file
          REACTX25.DEF - Definition file
          REACTX25.LRF - Link file

        outpath\CMSTOP directory contents:
          CMSTOP.DOC   - CMSTOP sample program documentation
          CMSTOP_C     - CMSTOP C language files subdirectory

        outpath\CMSTOP\CMSTOP_C directory contents:
          CMSTOP.C     - Source file
          CMSTOP       - Make file
          CMSTOP.DEF   - Definition file
          CMSTOP.LRF   - Link file
          CMSTOP.EXE   - Executable

        outpath\PGM_CFG directory contents:
          PGM_CFG.DOC  - Programmable configuration REXX sample documentation
          ACDI.CMD     - Sample REXX procedure to configure ACDI
          TWINAX.CMD   - Sample REXX procedure to configure twinax
          X25.CMD      - Sample REXX procedure to configure X.25
          3270.CMD     - Sample REXX procedure to configure 3270
          5250.CMD     - Sample REXX procedure to configure 5250

        outpath\NETBIOS directory contents:
          NETBIOS.DOC  - NETBIOS sample program documentation
          NETSAMPO.C   - Source file
          NETERROR.H   - Header file
          NETGBLV.H    - Header file
          NETSAM16.MAK - 16 bit make file
          NETSAMPO.MAK - 32 bit make file
          NETSAMPO.DEF - Definition file

        outpath\IEEE8022 directory contents:
          IEEE8022.DOC - IEEE 802.2 sample program documentation
          DLCSAMPO.C   - Source file
          DIRINTF.H    - Header file
          DLCERROR.H   - Header file
          DLCGBLV.H    - Header file
          DLCINTF.H    - Header file
          DLCSAM16.MAK - 16 bit make file
          DLCSAMPO.MAK - 32 bit make file
          DLCSAMPO.DEF - Definition file
