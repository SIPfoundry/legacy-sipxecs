# Microsoft Developer Studio Project File - Name="sipXforkingProxy" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=sipXforkingProxy - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXforkingProxy.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXforkingProxy.mak" CFG="sipXforkingProxy - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXforkingProxy - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "sipXforkingProxy - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXforkingProxy - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "include" /I "include\fastdb" /I "..\sipXportLib\include" /I "..\sipXtackLib\include" /I "..\sipXcommserverLib\include" /I "src\sipforkingProxy" /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D SIPX_CONFDIR=\".\" /D SIPX_LOGDIR=\".\" /D SIPX_VERSION=\"2.7\" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib sipXportLib.lib sipXtackLib.lib sipXcommserverLib.lib libpcre.a /nologo /subsystem:console /machine:I386 /libpath:"../sipXportLib/release" /libpath:"../sipXtackLib/release" /libpath:"../sipXcommserverLib/release"

!ELSEIF  "$(CFG)" == "sipXforkingProxy - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sipXforkingProxy___Win32_Debug"
# PROP BASE Intermediate_Dir "sipXforkingProxy___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "include" /I "include\fastdb" /I "..\sipXportLib\include" /I "..\sipXtackLib\include" /I "..\sipXcommserverLib\include" /I "src\sipforkingProxy" /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "_MBCS" /D SIPX_CONFDIR=\".\" /D SIPX_LOGDIR=\".\" /D SIPX_VERSION=\"2.7\" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib sipXportLib.lib sipXtackLib.lib sipXcommserverLib.lib libpcre.a /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept /libpath:"../sipXportLib/debug" /libpath:"../sipXtackLib/debug" /libpath:"../sipXcommserverLib/debug"

!ENDIF 

# Begin Target

# Name "sipXforkingProxy - Win32 Release"
# Name "sipXforkingProxy - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\cse\CallStateEventBuilder.cpp
# End Source File
# Begin Source File

SOURCE=.\src\cse\CallStateEventBuilder_XML.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipforkingproxy\ForkingProxyCseObserver.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipforkingproxy\ForwardRules.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipforkingproxy\SipForkingProxyMain.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipforkingproxy\SipRouter.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\src\cse\CallStateEventBuilder_XML.h
# End Source File
# Begin Source File

SOURCE=.\src\sipforkingproxy\ForkingProxyCseObserver.h
# End Source File
# Begin Source File

SOURCE=.\include\sipforkingproxy\ForwardRules.h
# End Source File
# Begin Source File

SOURCE=.\include\sipforkingproxy\SipRouter.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
