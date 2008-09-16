// 
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.  
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
// 
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsFS.h"
#include "os/OsSysLog.h"
#include "xmlparser/tinyxml.h"
#include "xmlparser/XmlErrorMsg.h"
#include "xmlparser/ExtractContent.h"
#include "sipXecsService/SipXecsService.h"

#include "SipxProcessCmd.h"
#include "SipxResource.h"
#include "FileResource.h"
#include "SipxProcessResource.h"
#include "SipxProcessResourceManager.h"
#include "SipxProcessManager.h"
#include "SipxProcess.h"

// DEFINES
// CONSTANTS

const UtlContainableType SipxProcess::TYPE = "SipxProcess";

const char* SipXecsProcessRootElement = "sipXecs-process";
const char* SipXecsProcessNamespace =
   "http://www.sipfoundry.org/sipX/schema/xml/sipXecs-process-01-00";

const char* SipxProcessStateDir = "process-state";
const char* SipxProcessConfigVersionDir = "process-cfgver";

const char* SipxProcessStateName[/* State value */] = // must match SipxProcess::State
{
   "Undefined",
   "Disabled",
   "Testing",
   "ResourceRequired",
   "ConfigurationMismatch",
   "ConfigurationTestFailed",
   "Starting",
   "Running",
   "AwaitingReferences",
   "Stopping",
   "Failed"
};

const char* SipxProcessEventName[/* Event value */] = // must match SipxProcess::Event
{
   "Startup",
   "ConfigurationChange",
   "ConfigurationVersionUpdate",
   "TestPass",
   "TestFail",
   "CheckState",
   "ProcessRunning",
   "ProcessExit",
   "Shutdown"
};

// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
SipxProcess::SipxProcess(const UtlString& name,
                         const UtlString& version,
                         const OsPath& definitionFile) :
   UtlString(name),
   mLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mVersion(version),
   mDesiredState(Undefined),
   mState(Undefined),
   mpProcessTask(NULL),
   mConfigtest(NULL),
   mStart(NULL),
   mStop(NULL),
   mReconfigure(NULL),
   mDefinitionFile(definitionFile)
{
   /*
    * Get the SipxProcessResource for this SipxProcess; it may have already been created
    * by some other SipxProcess that declared this one as a resource.
    */
   SipxProcessResourceManager* processResourceManager = SipxProcessResourceManager::getInstance();
   if (!(mSelfResource = processResourceManager->find(name.data())))
   {
      // No other SipxProcess has declared this one as a resource yet,
      // so create the SipxProcessResource 
      mSelfResource = new SipxProcessResource(name.data());
      processResourceManager->save(mSelfResource);
   }
   else
   {
      /* the resource has already been created by some other SipxProcess
       * listing this one as a resource.
       */
   }
};

/// Read a process definition and return a process if definition is valid.
SipxProcess* SipxProcess::createFromDefinition(const OsPath& definitionFile)
{
   SipxProcess* process = NULL;
   SipxProcessManager* processManager = SipxProcessManager::getInstance();
   
   UtlString errorMsg;

   TiXmlDocument processDefinitionDoc(definitionFile);
   if (processDefinitionDoc.LoadFile())
   {
      // definition is well formed xml, at least
      TiXmlElement* processDefElem;
      if ((processDefElem = processDefinitionDoc.RootElement()))
      {
         const char* rootElementName = processDefElem->Value();
         const char* definitionNamespace = processDefElem->Attribute("xmlns");
         if (   rootElementName && definitionNamespace
             && 0 == strcmp(rootElementName, SipXecsProcessRootElement)
             && 0 == strcmp(definitionNamespace, SipXecsProcessNamespace)
             )
         {
            // step through the top level elements

            bool definitionValid = true;
            
            TiXmlElement* nameElement = NULL;
            UtlString name;
            TiXmlElement* versionElement = NULL;
            UtlString version;
            TiXmlElement* commandsElement = NULL;
            TiXmlElement* statusElement = NULL;
            TiXmlElement* resourcesElement = NULL;

            // Get the 'name' element
            nameElement = processDefElem->FirstChildElement();
            if (nameElement && (0 == strcmp("name",nameElement->Value())))
            {
            
               if (   ! textContent(name, nameElement)
                   || name.isNull()
                   )
               {
                  definitionValid = false;
                  XmlErrorMsg(processDefinitionDoc,errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                                "'name' element content is invalid %s",
                                errorMsg.data()
                                );
               }
               else if ((process = processManager->findProcess(name)))
               {
                  definitionValid = false;
                  XmlErrorMsg(processDefinitionDoc,errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                                "duplicate process name '%s'\n"
                                "  %s\n"
                                "  previously defined in '%s'",
                                name.data(), errorMsg.data(), process->mDefinitionFile.data()
                                );
                  process = NULL; // so that we don't return or delete it
               }
            }
            else
            {
               definitionValid = false;
               XmlErrorMsg(processDefinitionDoc,errorMsg);
               OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                             "required 'name' element is missing %s",
                             errorMsg.data()
                             );
            }
   
            // Get the 'version' element
            if ( definitionValid )
            {
               if (   (versionElement = nameElement->NextSiblingElement())
                   && (0 == strcmp("version",versionElement->Value())))
               {
                  if (   ! textContent(version, versionElement)
                      || version.isNull()
                      )
                  {
                     definitionValid = false;
                     XmlErrorMsg(processDefinitionDoc,errorMsg);
                     OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                                   "'version' element content is invalid %s",
                                   errorMsg.data()
                                   );
                  }
               }
               else
               {
                  definitionValid = false;
                  XmlErrorMsg(processDefinitionDoc,errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                                "required 'version' element is missing %s",
                                errorMsg.data()
                                );
               }
            }

            // Get the 'commands' element
            if ( definitionValid )
            {
               if (   (commandsElement = versionElement->NextSiblingElement())
                   && (0 == strcmp("commands",commandsElement->Value())))
               {
                  // defer parsing commands until SipxProcess object is created below
               }
               else
               {
                  definitionValid = false;
                  XmlErrorMsg(processDefinitionDoc,errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                                "required 'commands' element is missing %s",
                                errorMsg.data()
                                );
               }
            }

            // Get the 'status' element
            if ( definitionValid )
            {
               if (   (statusElement = commandsElement->NextSiblingElement())
                   && (0 == strcmp("status",statusElement->Value())))
               {
                  // defer parsing status until SipxProcess object is created below
               }
               else
               {
                  definitionValid = false;
                  XmlErrorMsg(processDefinitionDoc,errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                                "required 'status' element is missing %s",
                                errorMsg.data()
                                );
               }
            }

            // Get the 'resources' element
            if ( definitionValid )
            {
               if ((resourcesElement = statusElement->NextSiblingElement()))
               {
                  const char* elementName = resourcesElement->Value();
                  if (0 != strcmp("resources",elementName))
                  {
                     definitionValid = false;
                     XmlErrorMsg(processDefinitionDoc,errorMsg);
                     OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                                   "invalid '%s' element: expected 'resources'  %s",
                                   elementName, errorMsg.data()
                                   );
                  }
               }
            }

            /* All the required top level elements have been found, so create
             * the SipxProcess object and invoke the parsers for the components. */
            if (definitionValid)
            {
               if ((process = new SipxProcess(name, version, definitionFile)))
               {
                  // Parse the 'commands' elements
                  TiXmlElement* configtestCmdElement;
                  TiXmlElement* startCmdElement;
                  TiXmlElement* stopCmdElement;
                  TiXmlElement* reconfigureCmdElement;

                  // mConfigtest <= sipXecs-process/commands/configtest
                  configtestCmdElement = commandsElement->FirstChildElement();
                  if (   configtestCmdElement
                      && (0 == strcmp("configtest",configtestCmdElement->Value())))
                  {
                     if (! (process->mConfigtest =
                            SipxProcessCmd::parseCommandDefinition(processDefinitionDoc,
                                                               configtestCmdElement)))
                     {
                        definitionValid = false;
                        XmlErrorMsg(processDefinitionDoc,errorMsg);
                        OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                                      "'configtest' content is invalid %s",
                                      errorMsg.data()
                                      );
                     }
                  }
                  else
                  {
                     definitionValid = false;
                     XmlErrorMsg(processDefinitionDoc,errorMsg);
                     OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                                   "required 'configtest' element is missing %s",
                                   errorMsg.data()
                                   );
                  }

                  // mStart <= sipXecs-process/commands/start
                  if (definitionValid)
                  {
                     startCmdElement = configtestCmdElement->NextSiblingElement();
                     if (   startCmdElement
                         && (0 == strcmp("start",startCmdElement->Value())))
                     {
                        if (! (process->mStart =
                               SipxProcessCmd::parseCommandDefinition(processDefinitionDoc,
                                                                  startCmdElement)))
                        {
                           definitionValid = false;
                           XmlErrorMsg(processDefinitionDoc,errorMsg);
                           OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                                         "SipxProcess::createFromDefinition "
                                         "'start' content is invalid %s",
                                         errorMsg.data()
                                         );
                        }
                     }
                     else
                     {
                        definitionValid = false;
                        XmlErrorMsg(processDefinitionDoc,errorMsg);
                        OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                                      "required 'start' element is missing %s",
                                      errorMsg.data()
                                      );
                     }
                  }

                  // mStop <= sipXecs-process/commands/stop
                  if (definitionValid)
                  {
                     stopCmdElement = startCmdElement->NextSiblingElement();
                     if (   stopCmdElement
                         && (0 == strcmp("stop",stopCmdElement->Value())))
                     {
                        if (! (process->mStop =
                               SipxProcessCmd::parseCommandDefinition(processDefinitionDoc,
                                                                  stopCmdElement)))
                        {
                           definitionValid = false;
                           XmlErrorMsg(processDefinitionDoc,errorMsg);
                           OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                                         "SipxProcess::createFromDefinition "
                                         "'stop' content is invalid %s",
                                         errorMsg.data()
                                         );
                        }
                     }
                     else
                     {
                        definitionValid = false;
                        XmlErrorMsg(processDefinitionDoc,errorMsg);
                        OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                                      "required 'stop' element is missing %s",
                                      errorMsg.data()
                                      );
                     }
                  }

                  // mReconfigure <= sipXecs-process/commands/reconfigure
                  if (definitionValid)
                  {
                     reconfigureCmdElement = stopCmdElement->NextSiblingElement();
                     if (   reconfigureCmdElement
                         && (0 == strcmp("reconfigure",reconfigureCmdElement->Value())))
                     {
                        if (!(process->mReconfigure =
                              SipxProcessCmd::parseCommandDefinition(processDefinitionDoc,
                                                                 reconfigureCmdElement)))
                        {
                           definitionValid = false;
                           XmlErrorMsg(processDefinitionDoc,errorMsg);
                           OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                                         "SipxProcess::createFromDefinition "
                                         "'reconfigure' content is invalid %s",
                                         errorMsg.data()
                                         );
                        }
                     }
                     else
                     {
                        // reconfigure is optional, so this is ok.
                     }
                  }

                  if (definitionValid)
                  {
                     // Parse the 'status' elements
                     TiXmlElement* statusElement;

                     statusElement = commandsElement->NextSiblingElement();
                     if (   statusElement
                         && (0 == strcmp("status",statusElement->Value())))
                     {
                        TiXmlElement* statusChildElement;
                     
                        // mPidFile <= sipXecs-process/status/pid
                        statusChildElement = statusElement->FirstChildElement();
                        if (statusChildElement)
                        {
                           if (0 == strcmp("pid",statusChildElement->Value()))
                           {
                              // the optional pid element is present
                              if (   textContent(process->mPidFile, statusChildElement)
                                  && !process->mPidFile.isNull())
                              {
                                 // advance the statusChildElement to the first log element, if any
                                 statusChildElement = statusChildElement->NextSiblingElement();
                              }
                              else
                              {
                                 definitionValid = false;
                                 XmlErrorMsg(processDefinitionDoc,errorMsg);
                                 OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                                               "SipxProcess::createFromDefinition "
                                               "'pid' element is empty"
                                               " - if present, it must be a pathname %s",
                                               errorMsg.data()
                                               );

                              }
                           }
                           /*
                            * There is no pid element, which is allowed.
                            * If statusChildElement is non-NULL, it should point to a 'log' element.
                            * Leave it for the 'log' loop below.
                            */
                        }

                        // mLogFiles <- sipXecs-process/status/log
                        while (definitionValid && statusChildElement)
                        {
                           if (0 == strcmp("log",statusChildElement->Value()))
                           {
                              UtlString logPath;
                              if (textContent(logPath, statusChildElement) && !logPath.isNull())
                              {
                                 FileResource* logFileResource =
                                    FileResource::logFileResource(logPath, process);
                                 
                                 process->mLogFiles.append(logFileResource);
                              
                                 // advance the statusChildElement to the first log element, if any
                                 statusChildElement = statusChildElement->NextSiblingElement();
                              }
                              else
                              {
                                 definitionValid = false;
                                 XmlErrorMsg(processDefinitionDoc,errorMsg);
                                 OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                                               "SipxProcess::createFromDefinition "
                                               "'log' element is empty"
                                               " - if present, it must be a pathname %s",
                                               errorMsg.data()
                                               );
                              }
                           }
                           else
                           {
                              definitionValid = false;
                              XmlErrorMsg(processDefinitionDoc,errorMsg);
                              OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                                            "SipxProcess::createFromDefinition "
                                            "'%s' element is invalid here: expected 'log'",
                                            statusChildElement->Value()                
                                            );
                           }
                        }
                     }
                     else
                     {
                        definitionValid = false;
                     
                        XmlErrorMsg(processDefinitionDoc,errorMsg);
                        OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                                      "required 'status' element is missing %s",
                                      errorMsg.data()
                                      );
                     }
                  }
                  
                  if (definitionValid)
                  {
                     // Parse the 'resources' elements
                     TiXmlElement* resourcesElement;

                     resourcesElement = statusElement->NextSiblingElement();
                     if (resourcesElement)
                     {
                        if (0 == strcmp("resources",resourcesElement->Value()))
                        {
                           TiXmlElement* resourceElement;
                           for (resourceElement = resourcesElement->FirstChildElement();
                                definitionValid && resourceElement;
                                resourceElement = resourceElement->NextSiblingElement()
                                )
                           {
                              /*
                               * We do not validate the element name here, because
                               * we don't know all valid resource element types.  That
                               * is done in the factory we call to parse the element.
                               *
                               * If this resource is required for starting or stopping,
                               * then the requireResourceToStart and/or checkResourceBeforeStop
                               * methods on the new process is called to add the new
                               * resource to the appropriate list.
                               */
                              definitionValid =
                                 SipxResource::parse(processDefinitionDoc,resourceElement,process);
                           }
                        }
                        else
                        {
                           definitionValid = false;
                           XmlErrorMsg(processDefinitionDoc,errorMsg);
                           OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                                         "SipxProcess::createFromDefinition "
                                         "'%s' element is invalid here: expected 'resources'",
                                         resourcesElement->Value()                
                                         );
                        }
                     }
                     else
                     {
                        definitionValid = false;
                        XmlErrorMsg(processDefinitionDoc,errorMsg);
                        OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                                      "required 'resources' element is missing %s",
                                      errorMsg.data()
                                      );
                     }
                  }
                  
                  if (definitionValid)
                  {
                     process->mState = Disabled;

                     SipxProcessManager::getInstance()->save(process);

                     process->readPersistentState();

                     {
                        OsLock mutex(process->mLock);
                        /*
                         * Now that the SipxProcessManager could return this, locking is
                         * important.  Both persistDesiredState and readConfigurationVersion
                         * require that the lock be held.
                         */
                        if (Undefined == process->mDesiredState)
                        {
                           process->mDesiredState = Disabled; // default is disabled.
                           process->persistDesiredState();
                        }
                        process->readConfigurationVersion();
                     } // end lock
                  }
                  else
                  {
                     // something is wrong, so get rid of the invalid SipxProcess object
                     delete process;
                     process = NULL;
                  }
               }
               else
               {
                  OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SipxProcess::createFromDefinition "
                                "failed to create SipxProcess object for '%s'", name.data());
               }
            }
         }
         else
         {
            XmlErrorMsg(processDefinitionDoc,errorMsg);
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                          "invalid root element '%s' in namespace '%s'\n"
                          "should be '%s' in namespace '%s' %s",
                          rootElementName, definitionNamespace,
                          SipXecsProcessRootElement, SipXecsProcessNamespace,
                          errorMsg.data()
                          );
         }

      }
      else
      {
         XmlErrorMsg(processDefinitionDoc,errorMsg);
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::createFromDefinition "
                       "root element not found in '%s': %s",
                       definitionFile.data(), errorMsg.data()
                       );
      }
   }
   else
   {
      UtlString errorMsg;
      XmlErrorMsg(processDefinitionDoc,errorMsg);
      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                    "SipxProcess::createFromDefinition failed to load '%s': %s",
                    definitionFile.data(), errorMsg.data()
                    );
   }

   return process;
}

/// Return the SipxProcessResource for this SipxProcess.
SipxProcessResource* SipxProcess::resource()
{
   OsLock mutex(mLock);
   
   return mSelfResource;
}

/// Return the current state of the SipxProcess.
SipxProcess::State SipxProcess::getState()
{
   OsLock mutex(mLock);
   
   return mState;
}

/// Return whether or not the service for this process should be Running.
bool SipxProcess::isEnabled()
{
   OsLock mutex(mLock);
   
   return (Running == mDesiredState);
}
/**< @returns true if the desired state of this service is Running;
 *            this does not indicate anything about the current state of
 *            the service: for that see getState
 */

/// Set the persistent desired state of the SipxProcess to Running.
void SipxProcess::enable()
{
   OsLock mutex(mLock);
   
   mDesiredState = Running;
   persistDesiredState();

   triggerServiceCheck(CheckState);
}

/// Set the persistent desired state of the SipxProcess to Disabled.
void SipxProcess::disable()
{
   OsLock mutex(mLock);
   
   mDesiredState = Disabled;
   persistDesiredState();

   triggerServiceCheck(CheckState);
}

/// Shutting down sipXsupervisor, so shut down the service.
void SipxProcess::shutdown()
{
   OsLock mutex(mLock);
   
   //This does not affect the persistent state of the service.
   // @TODO - trigger fsm shutdown event
}


/// Notify the SipxProcess that some configuration change has occurred.
void SipxProcess::configurationChange(const SipxResource& changedResource)
{
   UtlString changedResourceDescription;
   changedResource.appendDescription(changedResourceDescription);
   
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "SipxProcess[%s]::configurationChange(%s)",
                 data(), changedResourceDescription.data());
   
   triggerServiceCheck(ConfigurationChange);
}
   
void SipxProcess::readConfigurationVersion()
{
   // caller must be holding mLock

   OsPath persistentConfigVersionPath(SipXecsService::Path(SipXecsService::VarDirType,
                                                           SipxProcessConfigVersionDir)
                                      + OsPath::separator + data());
   OsFile persistentConfigVersionFile(persistentConfigVersionPath);

   if (OS_SUCCESS == persistentConfigVersionFile.open(OsFile::READ_ONLY))
   {
      if (OS_SUCCESS == persistentConfigVersionFile.readLine(mConfigVersion))
      {
         OsSysLog::add(FAC_SUPERVISOR, PRI_INFO,
                       "SipxProcess[%s]::readConfigurationVersion mConfigVersion='%s'",
                       data(), mConfigVersion.data());
      }
      else
      {
         // apparently, open read-only can return success when the file is not there.
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                       "SipxProcess[%s]::readConfigurationVersion read of '%s' failed"
                       " (ok if process has never been configured)",
                       data(), persistentConfigVersionPath.data());
      }

      persistentConfigVersionFile.close();
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_WARNING,
                    "SipxProcess[%s]::readConfigurationVersion open of '%s' failed"
                    " (ok if process has never been configured)",
                    data(), persistentConfigVersionPath.data());
   }
}

/// Check whether or not the configuration version matches the process version.
bool SipxProcess::configurationVersionMatches()
{
   OsLock mutex(mLock);
   bool versionMatches = (0==mConfigVersion.compareTo(mVersion, UtlString::matchCase));

   if (versionMatches) 
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "SipxProcess[%s]::configurationVersionMatches true",
                    data());
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "SipxProcess[%s]::configurationVersionMatches false:"
                    " process '%s' != config '%s'",
                    data(), mVersion.data(), mConfigVersion.data());
   }
   
   return versionMatches;
}

   

/// Set the version stamp value of the configuration.
void SipxProcess::setConfigurationVersion(const UtlString& newConfigVersion)
{
   OsLock mutex(mLock);
   
   if (0!=newConfigVersion.compareTo(mConfigVersion,UtlString::matchCase))
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "SipxProcess[%s]::setConfigurationVersion"
                    " '%s' -> '%s'", data(), mConfigVersion.data(), newConfigVersion.data());

      mConfigVersion = newConfigVersion;
      
      OsPath persistentConfigVersionDirPath  // normally {prefix}/var/sipxecs/process-state
         = SipXecsService::Path(SipXecsService::VarDirType, SipxProcessConfigVersionDir);
   
      OsDir persistentConfigVersionDir(persistentConfigVersionDirPath);
      OsPath persistentConfigVersionPath(persistentConfigVersionDirPath
                                         + OsPath::separator + data());
      OsFile persistentConfigVersionFile(persistentConfigVersionPath);

      if (!persistentConfigVersionDir.exists()) // does the directory exist?
      {
         if (OS_SUCCESS==OsFileSystem::createDir(persistentConfigVersionDirPath,
                                                 TRUE /* create parents */))
         {
            OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "SipxProcess::setConfigurationVersion "
                          "created directory '%s'",
                          persistentConfigVersionDirPath.data());
         }
         else 
         {
            OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SipxProcess[%s]::setConfigurationVersion "
                          "directory create failed for '%s'",
                          data(), persistentConfigVersionDirPath.data());
         }
      }

      if (OS_SUCCESS==persistentConfigVersionFile.open(OsFile::CREATE))
      {
         size_t bytesWritten;
         if (OS_SUCCESS!=persistentConfigVersionFile.write(mConfigVersion.data(),
                                                           mConfigVersion.length(),
                                                           bytesWritten))
         {
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess[%s]::setConfigurationVersion "
                          "write to '%s' failed", data(), persistentConfigVersionPath.data());
         }
      }
      else
      {
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                       "SipxProcess[%s]::setConfigurationVersion create of '%s' failed",
                       data(), persistentConfigVersionPath.data());
      }

      triggerServiceCheck(ConfigurationVersionUpdate);
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                    "SipxProcess[%s]::setConfigurationVersion "
                    "new value '%s' matches existing value.",
                    data(),mConfigVersion.data());
   }
   
}



/// Get the version stamp value of the configuration.
void SipxProcess::getConfigurationVersion(UtlString& version)
{
   version.remove(0);
   OsLock mutex(mLock);
   
   if (!mConfigVersion.isNull())
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_INFO, "SipxProcess[%s]::getConfigurationVersion"
                    " '%s'", data(), mConfigVersion.data());
      version = mConfigVersion;
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                    "SipxProcess[%s]::getConfigurationVersion - no value set",
                    data());
   }
}

/// Tell the SipxProcessTask thread to check the service process.
void SipxProcess::triggerServiceCheck(Event event)
{
   // @TODO 
   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "SipxProcess[%s]::triggerServiceCheck called.", data());
}
   
/// Check that all resources on the mRequiredResources list are ready so this can start.
bool SipxProcess::resourcesAreReady()
{
   return false;                // @TODO 
}


/// Compare actual process state to the desired state, and attempt to change it if needed.
void SipxProcess::checkService(Event event)
{
   bool waiting = false;
   while (!waiting)
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "SipxProcess[%s]::checkService "
                    " mState=%s  mDesiredState=%s event=%s",
                    data(), state(mState), state(mDesiredState), eventName(event));

      OsLock mutex(mLock); // note that is taken here and released at the bottom of the loop.
      switch (mState)
      {
      case Undefined:
      case Disabled:
         {
            switch (event)
            {
            case Startup:
            case ConfigurationChange:
            case ConfigurationVersionUpdate:
               if (Running==mDesiredState)
               {
                  mState = ConfigurationMismatch; 
               }
               else
               {
                  waiting = true;
               }
               break;

            case TestPass:
            case TestFail:
            case ProcessRunning:
               unexpectedEvent("checkService",event);
               waiting = true;
               break;

            case CheckState:
            case ProcessExit:
            case Shutdown:
               // no-op
               waiting = true;
               break;

            default:
               OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT,
                             "SipxProcess[%s]::checkService invalid event %d",
                             data(), event);
               waiting = true;
            }
            break;

         case ConfigurationMismatch:
            {
               if (Running == mDesiredState)
               {
                  if (configurationVersionMatches())
                  {
                     mState = ResourceRequired;
                  }
                  else
                  {
                     waiting = true;
                  }
               }
               else
               {
                  unexpectedEvent("checkService",event);
               }
            }
            break;

         case ResourceRequired:
            {
               if (Running == mDesiredState)
               {
                  if (resourcesAreReady())
                  {
                     mState = Testing;
                     // @TODO trigger configtest
                  }

                  waiting = true;
               }
               else
               {
                  unexpectedEvent("checkService",event);
               }
            }
            break;

         case Testing:
            {
               // @TODO - wait for successful test completion

               // HACK - just pretend the test passed:
               mState = Starting;
               // @TODO execute start command
               waiting = true;
            }
            break;

         case ConfigurationTestFailed:
            {
               switch (event)
               {
               case ConfigurationChange:
               case ConfigurationVersionUpdate:
                  if (Running==mDesiredState)
                  {
                     mState = ConfigurationMismatch; 
                  }
                  else
                  {
                     waiting = true;
                  }
                  break;

               case Startup:
               case ProcessRunning:
                  unexpectedEvent("checkService",event);
                  waiting = true;
                  break;

               case CheckState:
                  // @TODO - rerun the test
                  waiting = true;
                  break;
               
               case TestPass:
                  if (Running == mDesiredState)
                  {
                     // @TODO - execute the start command
                     waiting = true;
                  }
                  else
                  {
                     // @TODO - return indication that the test passed
                     mState = mDesiredState;
                  }
                  break;
               
               case ProcessExit:
               case TestFail:
                  if (Running == mDesiredState)
                  {
                     // @TODO - restart the health timer to retry
                     waiting = true;
                  }
                  else
                  {
                     // @TODO - return indication that the test failed
                     mState = mDesiredState;
                  }
                  break;
               
               case Shutdown:
                  // no-op
                  waiting = true;
                  break;

               default:
                  OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT,
                                "SipxProcess[%s]::checkService invalid event %d",
                                data(), event);
                  waiting = true;
               }
            }
            break;

         case Starting:
            {
               switch (event)
               {
               case ConfigurationChange:
               case ConfigurationVersionUpdate:
                  if (Running==mDesiredState)
                  {
                     mState = ConfigurationMismatch; 
                  }
                  else
                  {
                     waiting = true;
                  }
                  break;

               case Startup:
                  unexpectedEvent("checkService",event);
                  waiting = true;
                  break;

               case ProcessRunning:
               case CheckState:
                  if (Running == mDesiredState)
                  {
                     if (false)    // @TODO is our system process executing?
                     {
                        mState = Running;
                     }
                  }
                  waiting = true;
                  break;
               
               case TestPass:
                  if (Running == mDesiredState)
                  {
                     // @TODO - execute the start command
                     waiting = true;
                  }
                  else
                  {
                     // @TODO - return indication that the test passed
                     mState = mDesiredState;
                  }
                  break;
               
               case ProcessExit:
               case TestFail:
                  if (Running == mDesiredState)
                  {
                     // @TODO - restart the health timer to retry
                     waiting = true;
                  }
                  else
                  {
                     // @TODO - return indication that the test failed
                     mState = mDesiredState;
                  }
                  break;
               
               case Shutdown:
                  // no-op
                  waiting = true;
                  break;

               default:
                  OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT,
                                "SipxProcess[%s]::checkService invalid event %d",
                                data(), event);
                  waiting = true;
               }
            
               waiting = true;
            }
            break;

         case Running:
            {
               if (false)          // @TODO is our system process executing?
               {
                  mState = Failed;
               }
               else
               {
                  waiting = true;
               }
            }         
            break;

         case AwaitingReferences:
            {
               // @TODO
            }
            break;

         case Stopping:
            {
               // @TODO
            }
            break;

         case Failed:
            {
               // @TODO
            }
            break;

         default:
            OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SipxProcess::checkService invalid mState '%d'",
                          mState);
            break;
         }
      }
   }
}


/// Determine whether or not the values in a containable are comparable.
UtlContainableType SipxProcess::getContainableType() const
{
   return TYPE;
}

void SipxProcess::requireResource(SipxResource* resource)
{
   OsLock mutex(mLock);

   mRequiredResources.append(resource);
}

void SipxProcess::resourceIsOptional(SipxResource* resource)
{
   OsLock mutex(mLock);

   mRequiredResources.removeReference(resource);
}

/// Translate enum of the event to the string
const char* SipxProcess::eventName(Event event)
{
   const char* eventName = NULL;
   if ( event >= Startup && event <= Shutdown )
   {
      eventName = SipxProcessEventName[event];
   }
   else
   {
      eventName = "INVALID_EVENT_VALUE";
   }
   return eventName;
}



/// Translate the string from of the state name to the enum
SipxProcess::State SipxProcess::state(const UtlString& stringStateValue)
{
   int stateValue;
   for ( stateValue = Failed;
         (   stateValue > Undefined
          && 0 != stringStateValue.compareTo(SipxProcessStateName[stateValue], UtlString::matchCase)
          );
         stateValue--
        )
   {}
   return static_cast<State>(stateValue);
}
   
/// Translate enum of the state to the string
const char* SipxProcess::state(State stateValue)
{
   const char* stateName = NULL;
   if ( stateValue >= Undefined && stateValue <= Failed )
   {
      stateName = SipxProcessStateName[stateValue];
   }
   else
   {
      stateName = "INVALID_STATE_VALUE";
   }
   return stateName;
}

/// Save the persistent desired state.
void SipxProcess::persistDesiredState()
{
   // caller must be holding mLock.

   OsPath persistentStateDirPath  // normally {prefix}/var/sipxecs/process-state
      = SipXecsService::Path(SipXecsService::VarDirType, SipxProcessStateDir);
   
   OsDir persistentStateDir(persistentStateDirPath);

   if (!persistentStateDir.exists())
   {
      if (OS_SUCCESS == OsFileSystem::createDir(persistentStateDirPath, TRUE /* create parents */))
      {
         OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG, "SipxProcess::persistDesiredState "
                       "created directory '%s'",
                       persistentStateDirPath.data());
      }
      else
      {
         OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SipxProcess::persistDesiredState "
                       "directory create failed for '%s'",
                       persistentStateDirPath.data());
      }
   }

   OsPath persistentStatePath(persistentStateDirPath + OsPath::separator + data());
   OsFile persistentStateFile(persistentStatePath);
   
   if (OS_SUCCESS==persistentStateFile.open(OsFile::CREATE))
   {
      UtlString persistentState(state(mDesiredState));
      
      size_t bytesWritten;
      if (OS_SUCCESS != persistentStateFile.write(persistentState.data(),
                                                  persistentState.length(),
                                                  bytesWritten))
      {
         OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SipxProcess::persistDesiredState "
                       "write to '%s' failed", persistentStatePath.data());
      }
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcess::persistDesiredState open of '%s' failed",
                    persistentStatePath.data());
   }
}

/// Read the persistent desired state into mDesiredState.
void SipxProcess::readPersistentState()
{
   OsLock mutex(mLock);
   
   mDesiredState = Undefined;

   OsPath persistentStatePath(SipXecsService::Path(SipXecsService::VarDirType, SipxProcessStateDir)
                              + OsPath::separator + data());
   OsFile persistentStateFile(persistentStatePath);
   
   if (OS_SUCCESS == persistentStateFile.open(OsFile::READ_ONLY))
   {
      UtlString persistentStateString;
      if (OS_SUCCESS != persistentStateFile.readLine(persistentStateString))
      {
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                       "SipxProcess::readPersistentState read failed. "
                       "persistentStatePath = '%s'",
                       persistentStatePath.data());
      }

      mDesiredState = state(persistentStateString);
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_WARNING,
                    "SipxProcess::readPersistentState open of '%s' failed",
                    persistentStatePath.data());
   }
}

void SipxProcess::unexpectedEvent(const char* methodName, Event event)
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                 "SipxProcess[%s]::%s %s event unexpected; states: current=%s desired=%s",
                 data(), methodName, eventName(event), state(mState), state(mDesiredState)
                 );
}



/// destructor
SipxProcess::~SipxProcess()
{
   OsLock mutex(mLock);

   // @TODO shut down task

   if (mConfigtest)
   {
      delete mConfigtest;
   }
   if (mStart)
   {
      delete mStart;
   }
   if (mStop)
   {
      delete mStop;
   }
   if (mReconfigure)
   {
      delete mReconfigure;
   }

   mRequiredResources.removeAll();
};
