
/**
 *
 * Copyright (c) eZuce, Inc. All rights reserved.
 * Contributed to SIPfoundry under a Contributor Agreement
 *
 * This software is free software; you can redistribute it and/or modify it under
 * the terms of the Affero General Public License (AGPL) as published by the
 * Free Software Foundation; either version 3 of the License, or (at your option)
 * any later version.
 *
 * This software is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
 * details.
 * 
 */


#include <iostream>
#include <fstream>
#include <stdexcept>

#include "rutil/Log.hxx"
#include "rutil/Logger.hxx"
#include "rutil/DnsUtil.hxx"
#include "rutil/dns/DnsStub.hxx"
#include "rutil/GeneralCongestionManager.hxx"
#include "rutil/TransportType.hxx"

#include "resip/stack/SipStack.hxx"
#include "resip/stack/Compression.hxx"
#include "resip/stack/EventStackThread.hxx"
#include "resip/stack/InteropHelper.hxx"
#include "resip/stack/ConnectionManager.hxx"

#include "resip/dum/InMemorySyncRegDb.hxx"
#include "resip/dum/MasterProfile.hxx"
#include "resip/dum/DialogUsageManager.hxx"
#include "resip/dum/DumThread.hxx"
#include "resip/dum/TlsPeerAuthManager.hxx"
#include "resip/dum/WsCookieAuthManager.hxx"

#include "repro/AsyncProcessorWorker.hxx"
#include "repro/ReproRunner.hxx"
#include "repro/Proxy.hxx"
#include "repro/ProxyConfig.hxx"
#include "repro/BerkeleyDb.hxx"
#include "repro/Dispatcher.hxx"
#include "repro/UserAuthGrabber.hxx"
#include "repro/ProcessorChain.hxx"
#include "repro/ReproVersion.hxx"
#include "repro/WebAdmin.hxx"
#include "repro/WebAdminThread.hxx"
#include "repro/Registrar.hxx"
#include "repro/ReproServerAuthManager.hxx"
#include "repro/RegSyncClient.hxx"
#include "repro/RegSyncServer.hxx"
#include "repro/RegSyncServerThread.hxx"
#include "repro/CommandServer.hxx"
#include "repro/CommandServerThread.hxx"
#include "repro/BasicWsConnectionValidator.hxx"
#include "repro/monkeys/CookieAuthenticator.hxx"
#include "repro/monkeys/IsTrustedNode.hxx"
#include "repro/monkeys/AmIResponsible.hxx"
#include "repro/monkeys/DigestAuthenticator.hxx"
#include "repro/monkeys/LocationServer.hxx"
#include "repro/monkeys/RecursiveRedirect.hxx"
#include "repro/monkeys/SimpleStaticRoute.hxx"
#include "repro/monkeys/StaticRoute.hxx"
#include "repro/monkeys/StrictRouteFixup.hxx"
#include "repro/monkeys/OutboundTargetHandler.hxx"
#include "repro/monkeys/QValueTargetHandler.hxx"
#include "repro/monkeys/SimpleTargetHandler.hxx"
#include "repro/monkeys/GeoProximityTargetSorter.hxx"
#include "repro/monkeys/RequestFilter.hxx"
#include "repro/monkeys/MessageSilo.hxx"
#include "repro/monkeys/CertificateAuthenticator.hxx"

#include "sipx/proxy/ReproGlue.h"


#define RESIPROCATE_SUBSYSTEM resip::Subsystem::REPRO

using namespace resip;
using namespace repro;

namespace sipx {
namespace proxy {

ReproGlue::ReproGlue()
{
}

bool ReproGlue::run(const std::string& path)
{
  if(mRunning) return false;

   // Parse command line and configuration file
   assert(!mProxyConfig);
   Data defaultConfigFilename(path.c_str());
   try
   {
      mProxyConfig = new ProxyConfig();
      mProxyConfig->parseConfigFile(defaultConfigFilename);
   }
   catch(BaseException& ex)
   {
      std::cerr << "Error parsing configuration: " << ex << std::endl;
      return false;
   }

   // Non-Windows server process stuff
   if(!mRestarting)
   {
      setPidFile(mProxyConfig->getConfigData("PidFile", "", true));
      if(mProxyConfig->getConfigBool("Daemonize", false))
      {
         daemonize();
      }
   }

   // Initialize resip logger
   GenericLogImpl::MaxByteCount = mProxyConfig->getConfigUnsignedLong("LogFileMaxBytes", 5242880 /*5 Mb */);
   Data loggingType = mProxyConfig->getConfigData("LoggingType", "cout", true);
   Log::initialize(loggingType, 
                   mProxyConfig->getConfigData("LogLevel", "INFO", true), 
                   mArgv[0], 
                   mProxyConfig->getConfigData("LogFilename", "repro.log", true).c_str(),
                   /*isEqualNoCase(loggingType, "file") ? &g_ReproLogger :*/ 0); // if logging to file then write WARNINGS, and Errors to console still

   InfoLog( << "Starting repro version " << VersionUtils::instance().releaseVersion() << "...");

   // Create SipStack and associated objects
   if(!createSipStack())
   {
      return false;
   }

   // Drop privileges (can do this now that sockets are bound)
   Data runAsUser = mProxyConfig->getConfigData("RunAsUser", "", true);
   Data runAsGroup = mProxyConfig->getConfigData("RunAsGroup", "", true); 
   if(!runAsUser.empty())
   {
      InfoLog( << "Trying to drop privileges, configured uid = " << runAsUser << " gid = " << runAsGroup);
      dropPrivileges(runAsUser, runAsGroup);
   }

   // Create datastore
   if(!createDatastore())
   {
      return false;
   }

   // Create DialogUsageManager that handles ServerRegistration,
   // and potentially certificate subscription server
   createDialogUsageManager();

   // Create the Proxy and associate objects
   if(!createProxy())
   {
      return false;
   }

   // Create HTTP WebAdmin and Thread
   if(!createWebAdmin())
   {
      return false;
   }

   // Create reg sync components if required
   createRegSync();

   // Create command server if required
   if(!mRestarting)
   {
      createCommandServer();
   }

   // Make it all go - startup all threads
   mThreadedStack = mProxyConfig->getConfigBool("ThreadedStack", true);
   if(mThreadedStack)
   {
      // If configured, then start the sub-threads within the stack
      mSipStack->run();
   }
   mStackThread->run();
   if(mDumThread)
   {
      mDumThread->run();
   }
   mProxy->run();
   if(mWebAdminThread)
   {
      mWebAdminThread->run();
   }
   if(!mRestarting && mCommandServerThread)
   {
      mCommandServerThread->run();
   }
   if(mRegSyncServerThread)
   {
      mRegSyncServerThread->run();
   }
   if(mRegSyncClient)
   {
      mRegSyncClient->run();
   }

   mRunning = true;

   return true;
}

} } // sipx::proxy
