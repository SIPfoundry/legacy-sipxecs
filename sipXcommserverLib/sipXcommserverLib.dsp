# Microsoft Developer Studio Project File - Name="sipXcommserverLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sipXcommserverLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXcommserverLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXcommserverLib.mak" CFG="sipXcommserverLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXcommserverLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sipXcommserverLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXcommserverLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /I "include" /I "include\fastdb" /I "..\sipXportLib\include" /I "..\sipXportLib\include\glib" /I "..\sipXtackLib\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D SIPX_TMPDIR=\"..\" /D SIPX_DBDIR=\"..\" /D SIPX_CONFDIR=\".\" /D SIPX_LOGDIR=\".\" /D "DISABLE_MEM_POOLS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sipXcommserverLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /I "include" /I "include\fastdb" /I "..\sipXportLib\include" /I "..\sipXportLib\include\glib" /I "..\sipXtackLib\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D SIPX_TMPDIR=\"..\" /D SIPX_DBDIR=\"..\" /D SIPX_CONFDIR=\".\" /D SIPX_LOGDIR=\".\" /D "DISABLE_MEM_POOLS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "sipXcommserverLib - Win32 Release"
# Name "sipXcommserverLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\sipdb\AliasDB.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipdb\AuthexceptionDB.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\cgistub.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\class.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\compiler.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\container.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipdb\CredentialDB.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\cursor.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\database.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipdb\DialByNameDB.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipdb\ExtensionDB.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\file.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\hashtab.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipdb\HuntgroupDB.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\localcli.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipdb\PermissionDB.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\query.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipdb\RegistrationDB.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\repsock.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipdb\ResultSet.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\server.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipdb\SIPDBManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipdb\SIPXAuthHelper.cpp
# End Source File
# Begin Source File

SOURCE=.\src\sipdb\SubscriptionDB.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\symtab.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\sync.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\ttree.cpp
# End Source File
# Begin Source File

SOURCE=.\src\digitmaps\UrlMapping.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\w32sock.cpp
# End Source File
# Begin Source File

SOURCE=.\src\fastdb\wwwapi.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\sipdb\AliasDB.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\AliasRow.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\array.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\AuthexceptionDB.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\AuthexceptionRow.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\bugdb.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\class.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\cli.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\clidb.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\cliproto.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\compiler.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\container.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\CredentialDB.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\CredentialRow.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\cursor.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\database.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\date.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\datetime.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\DialByNameDB.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\DialByNameRow.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\exception.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\ExtensionDB.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\ExtensionRow.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\fastdb.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\file.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\harray.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\hashtab.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\HuntgroupDB.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\HuntgroupRow.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\localcli.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\PermissionDB.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\PermissionRow.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\query.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\reference.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\RegistrationDB.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\RegistrationRow.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\repsock.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\ResultSet.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\server.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\set.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\SIPDBManager.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\SIPXAuthHelper.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\sockio.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\stdtp.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\SubscriptionDB.h
# End Source File
# Begin Source File

SOURCE=.\include\sipdb\SubscriptionRow.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\subsql.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\symtab.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\sync.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\ttree.h
# End Source File
# Begin Source File

SOURCE=.\include\digitmaps\UrlMapping.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\w32sock.h
# End Source File
# Begin Source File

SOURCE=.\include\fastdb\wwwapi.h
# End Source File
# End Group
# End Target
# End Project
