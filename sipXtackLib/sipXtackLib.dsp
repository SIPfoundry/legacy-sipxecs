# Microsoft Developer Studio Project File - Name="sipXtackLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sipXtackLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sipXtackLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sipXtackLib.mak" CFG="sipXtackLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sipXtackLib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sipXtackLib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "Perforce Project"
# PROP Scc_LocalPath "."
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sipXtackLib - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /I "include" /I "..\sipXportLib\include" /I "..\sipXportLib\include\glib" /I "..\sipXcallLib\include" /D "NDEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D SIP_STACK_VERSION=\"2.5.2\" /D SIPX_CONFDIR=\".\" /D SIPX_LOGDIR=\".\" /D "DISABLE_MEM_POOLS" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "sipXtackLib - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /I "include" /I "..\sipXportLib\include" /I "..\sipXportLib\include\glib" /I "..\sipXcallLib\include" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "_LIB" /D SIP_STACK_VERSION=\"2.5.2\" /D SIPX_CONFDIR=\".\" /D SIPX_LOGDIR=\".\" /D "DISABLE_MEM_POOLS" /FR /YX /FD /GZ /c
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

# Name "sipXtackLib - Win32 Release"
# Name "sipXtackLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\resparse\bzero.c
# End Source File
# Begin Source File

SOURCE=.\src\net\HttpBody.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\HttpMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\HttpRequestContext.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\HttpServer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\resparse\wnt\inet_addr.c
# End Source File
# Begin Source File

SOURCE=.\src\net\MailAttachment.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\MailMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\resparse\memset.c
# End Source File
# Begin Source File

SOURCE=.\src\net\MimeBodyPart.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\NameValuePair.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\NameValuePairInsensitive.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\NameValueTokenizer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\NetAttributeTokenizer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\NetBase64Codec.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\NetMd5Codec.cpp
# End Source File
# Begin Source File

SOURCE=.\src\resparse\ns_name.c
# End Source File
# Begin Source File

SOURCE=.\src\resparse\ns_netint.c
# End Source File
# Begin Source File

SOURCE=.\src\net\PidfBody.cpp
# End Source File
# Begin Source File

SOURCE=.\src\resparse\res_comp.c
# End Source File
# Begin Source File

SOURCE=.\src\resparse\res_copy.c
# End Source File
# Begin Source File

SOURCE=.\src\resparse\res_data.c
# End Source File
# Begin Source File

SOURCE=.\src\resparse\res_free.c
# End Source File
# Begin Source File

SOURCE=.\src\resparse\res_info.c
# End Source File
# Begin Source File

SOURCE=.\src\resparse\res_init.c
# End Source File
# Begin Source File

SOURCE=.\src\resparse\res_mkquery.c
# End Source File
# Begin Source File

SOURCE=.\src\resparse\res_parse.c
# End Source File
# Begin Source File

SOURCE=.\src\resparse\res_print.c
# End Source File
# Begin Source File

SOURCE=.\src\resparse\res_query.c
# End Source File
# Begin Source File

SOURCE=.\src\resparse\res_send.c
# End Source File
# Begin Source File

SOURCE=.\src\net\SdpBody.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SdpCodec.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SdpCodecFactory.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipClient.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipConfigServerAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipContactDb.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipDialog.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipDialogEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipDialogMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipLine.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipLineCredentials.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipLineEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipLineList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipLineMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipMessage.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipMessageEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipMessageList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipNonceDb.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipNotifyStateTask.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipObserverCriteria.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipPimClient.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipProtocolServerBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipPublishContentMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipRefreshManager.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipRefreshMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipServerBroker.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipSession.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipSrvLookup.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipSubscribeClient.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipSubscribeServer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipSubscribeServerEventHandler.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipSubscriptionMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipTcpServer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipTlsServer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipTransaction.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipTransactionList.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipUdpServer.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipUserAgent.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipUserAgentBase.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SipUserAgentStateless.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\SmimeBody.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\TapiMgr.cpp
# End Source File
# Begin Source File

SOURCE=.\src\net\Url.cpp
# End Source File
# Begin Source File

SOURCE=.\src\resparse\wnt\writev.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\resparse\bzero.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\wnt\crypt.h
# End Source File
# Begin Source File

SOURCE=.\include\net\HttpBody.h
# End Source File
# Begin Source File

SOURCE=.\include\net\HttpMessage.h
# End Source File
# Begin Source File

SOURCE=.\include\net\HttpRequestContext.h
# End Source File
# Begin Source File

SOURCE=.\include\net\HttpServer.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\wnt\netinet\in.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\wnt\arpa\inet.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\wnt\inet_aton.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\sys\isa_defs.h
# End Source File
# Begin Source File

SOURCE=.\include\net\MailAttachment.h
# End Source File
# Begin Source File

SOURCE=.\include\net\MailMessage.h
# End Source File
# Begin Source File

SOURCE=.\include\net\MimeBodyPart.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\wnt\arpa\nameser.h
# End Source File
# Begin Source File

SOURCE=.\include\net\NameValuePair.h
# End Source File
# Begin Source File

SOURCE=.\include\net\NameValuePairInsensitive.h
# End Source File
# Begin Source File

SOURCE=.\include\net\NameValueTokenizer.h
# End Source File
# Begin Source File

SOURCE=.\include\net\NetAttributeTokenizer.h
# End Source File
# Begin Source File

SOURCE=.\include\net\NetBase64Codec.h
# End Source File
# Begin Source File

SOURCE=.\include\net\NetMd5Codec.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\ns_name.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\wnt\nterrno.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\wnt\sys\param.h
# End Source File
# Begin Source File

SOURCE=.\include\net\PidfBody.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\poll.h
# End Source File
# Begin Source File

SOURCE=.\include\net\QoS.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\res_config.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\res_info.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\wnt\res_signal.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\wnt\resolv\resolv.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\rr.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SdpBody.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SdpCodec.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SdpCodecFactory.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipClient.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipConfigServerAgent.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipContactDb.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipDialog.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipDialogMgr.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipLine.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipLineCredentials.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipLineEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipLineList.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipLineMgr.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipMessage.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipMessageEvent.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipMessageList.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipNonceDb.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipNotifyStateTask.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipObserverCriteria.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipPimClient.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipProtocolServerBase.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipPublishContentMgr.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipRefreshManager.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipRefreshMgr.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipServerBroker.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipSession.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipSrvLookup.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipSubscribeClient.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipSubscribeServer.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipSubscribeServerEventHandler.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipSubscriptionMgr.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipTcpServer.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipTlsServer.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipTransaction.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipTransactionList.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipUdpServer.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipUserAgent.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipUserAgentBase.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SipUserAgentStateless.h
# End Source File
# Begin Source File

SOURCE=.\include\net\SmimeBody.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\wnt\sol_search.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\wnt\sysdep.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\types.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\wnt\sys\uio.h
# End Source File
# Begin Source File

SOURCE=.\include\net\Url.h
# End Source File
# Begin Source File

SOURCE=.\include\resparse\wnt\utilNT.h
# End Source File
# Begin Source File

SOURCE=.\include\net\version.h
# End Source File
# End Group
# End Target
# End Project
