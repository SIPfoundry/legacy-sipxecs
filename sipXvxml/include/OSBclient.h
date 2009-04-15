/****************License************************************************
 *
 * Copyright 2000-2001.  SpeechWorks International, Inc.  
 *
 * Use of this software is subject to notices and obligations set forth
 * in the SpeechWorks Public License - Software Version 1.1 which is
 * included with this software.
 *
 * SpeechWorks is a registered trademark, and SpeechWorks Here, 
 * DialogModules and the SpeechWorks logo are trademarks of SpeechWorks 
 * International, Inc. in the United States and other countries. 
 * 
 ***********************************************************************
 *
 * Data structures used by OSBclient.cpp
 *
 ************************************************************************
 */

#ifndef _OSBCLIENT_H
#define _OSBCLIENT_H

#include <VXIlog.h>
#include <VXIjsi.h>
#include <VXIinet.h>
#include <VXIcache.h>
#include <VXItel.h>
#include <VXIrec.h>
#include <VXIprompt.h>
#include <OSBobject.h>
#include <VXIinterpreter.h>
#include <VXItrd.h>

#include <SBclientConfig.h>    /* For CLIENT_[...] configuration properties */
#include <cp/CallManager.h>

 /** 
  * @name OSBclient
  * @memo OSBclient implementation of VXIplatform
  * @doc
  * Provides an implementation of the VXIplatform abstract interface
  * for creating and destroying the VXI resources required by the
  * browser and to isolate the main from the actual implementation
  * details of the resource management.  This set of convenience
  * functions is used by the reference application manager.  
  */

/*@{*/

/**
 * @memo OSBclient defined VXIplatform structure, returned upon initialization 
 * by VXIplatformCreateResources()
 */
struct VXIplatform {
  VXIunsigned                channelNum;

  VXItelInterface           *VXItel;
  VXIlogInterface           *VXIlog;
  VXIinetInterface          *VXIinet;
  VXIcacheInterface         *VXIcache;
  VXIpromptInterface        *VXIprompt;
  VXIrecInterface           *VXIrec;
  VXIjsiInterface           *VXIjsi;
  VXIobjectInterface        *VXIobject;
  VXIinterpreterInterface   *VXIinterpreter;

  VXIMap                    *telephonyProps;
  OSBobjectResources         objectResources;
  VXIresources               resources;
  VXIbool                    acceptCookies;

  CallManager				*pCallMgr;
};

/*@}*/

#endif /* include guard */
