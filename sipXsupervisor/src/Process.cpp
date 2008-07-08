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
#include "ProcessCmd.h"
#include "SipxResource.h"
#include "Process.h"

// DEFINES
// CONSTANTS

const UtlContainableType Process::TYPE = "Process";

const char* SipXecsProcessRootElement = "sipXecs-process";
const char* SipXecsProcessNamespace =
   "http://www.sipfoundry.org/sipX/schema/xml/sipXecs-process-01-00";

// TYPEDEFS
// FORWARD DECLARATIONS

/// constructor
Process::Process(const UtlString& name, const UtlString& version) :
   UtlString(name),
   mLock(OsBSem::Q_PRIORITY, OsBSem::FULL),
   mVersion(version),
   mDesiredState(Undefined),
   mpProcessTask(NULL),
   mConfigtest(NULL),
   mStart(NULL),
   mStop(NULL),
   mReconfigure(NULL),
   mMyResource(NULL)
{
};

/// Read a process definition and return a process if definition is valid.
Process* Process::createFromDefinition(const OsPath& definitionFile)
{
   Process* process = NULL;
   
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
                  XmlErrorMsg(&processDefinitionDoc,errorMsg);
                  OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                "'name' element content is invalid.  %s",
                                errorMsg.data()
                                );
               }
            }
            else
            {
               definitionValid = false;
               XmlErrorMsg(&processDefinitionDoc,errorMsg);
               OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                             "required 'name' element is missing.  %s",
                             errorMsg.data()
                             );
            }
   
            // Get the 'version' element
            if (   definitionValid
                && (versionElement = nameElement->NextSiblingElement())
                && (0 == strcmp("version",versionElement->Value())))
            {
               if (   textContent(version, versionElement)
                   && ! version.isNull()
                   )
               {
                  definitionValid = false;
                  XmlErrorMsg(&processDefinitionDoc,errorMsg);
                  OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                "'version' element content is invalid.  %s",
                                errorMsg.data()
                                );
               }
            }
            else
            {
               XmlErrorMsg(&processDefinitionDoc,errorMsg);
               OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                             "required 'version' element is missing.  %s",
                             errorMsg.data()
                             );
            }

            // Get the 'commands' element
            if (   definitionValid
                && (commandsElement = versionElement->NextSiblingElement())
                && (0 == strcmp("commands",commandsElement->Value())))
            {
               // defer parsing commands until Process object is created below
            }
            else
            {
               definitionValid = false;
               XmlErrorMsg(&processDefinitionDoc,errorMsg);
               OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                             "required 'commands' element is missing.  %s",
                             errorMsg.data()
                             );
            }

            // Get the 'status' element
            if (   definitionValid
                && (statusElement = commandsElement->NextSiblingElement())
                && (0 == strcmp("status",statusElement->Value())))
            {
               // defer parsing status until Process object is created below
            }
            else
            {
               definitionValid = false;
               XmlErrorMsg(&processDefinitionDoc,errorMsg);
               OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                             "required 'status' element is missing.  %s",
                             errorMsg.data()
                             );
            }

            // Get the 'resources' element
            if (   definitionValid
                && (resourcesElement = statusElement->NextSiblingElement()))
            {
               const char* elementName = resourcesElement->Value();
               if (0 != strcmp("resources",elementName))
               {
                  definitionValid = false;
                  XmlErrorMsg(&processDefinitionDoc,errorMsg);
                  OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                "invalid '%s' element: expected 'resources'  %s",
                                elementName, errorMsg.data()
                             );
               }
            }

            /* All the required top level elements have been found, so create
             * the Process object and invoke the parsers for the components. */
            if (definitionValid)
            {
               if ((process = new Process(name, version)))
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
                            ProcessCmd::parseCommandDefinition(configtestCmdElement)))
                     {
                        definitionValid = false;
                        XmlErrorMsg(&processDefinitionDoc,errorMsg);
                        OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                      "'configtest' element is invalid.  %s",
                                      errorMsg.data()
                                      );
                     }
                  }
                  else
                  {
                     definitionValid = false;
                     XmlErrorMsg(&processDefinitionDoc,errorMsg);
                     OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                   "required 'configtest' element is missing.  %s",
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
                               ProcessCmd::parseCommandDefinition(startCmdElement)))
                        {
                           definitionValid = false;
                           XmlErrorMsg(&processDefinitionDoc,errorMsg);
                           OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                         "'start' element is invalid.  %s",
                                         errorMsg.data()
                                         );
                        }
                     }
                     else
                     {
                        definitionValid = false;
                        XmlErrorMsg(&processDefinitionDoc,errorMsg);
                        OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                      "required 'start' element is missing.  %s",
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
                               ProcessCmd::parseCommandDefinition(stopCmdElement)))
                        {
                           definitionValid = false;
                           XmlErrorMsg(&processDefinitionDoc,errorMsg);
                           OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                         "'stop' element is invalid.  %s",
                                         errorMsg.data()
                                         );
                        }
                     }
                     else
                     {
                        definitionValid = false;
                        XmlErrorMsg(&processDefinitionDoc,errorMsg);
                        OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                      "required 'stop' element is missing.  %s",
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
                              ProcessCmd::parseCommandDefinition(reconfigureCmdElement)))
                        {
                           definitionValid = false;
                           XmlErrorMsg(&processDefinitionDoc,errorMsg);
                           OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                         "'reconfigure' element is invalid.  %s",
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
                                 // @TODO ? try to validate the path?

                                 // advance to the statusChildElement to the first log element, if any
                                 statusChildElement = statusChildElement->NextSiblingElement();
                              }
                              else
                              {
                                 definitionValid = false;
                                 XmlErrorMsg(&processDefinitionDoc,errorMsg);
                                 OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                               "'pid' element is empty"
                                               " - if present, it must be a pathname.  %s",
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
                              UtlString* logPath = new UtlString;
                              if (textContent(*logPath, statusChildElement) && !logPath->isNull())
                              {
                                 // @TODO ? try to validate the path?

                                 process->mLogFiles.append(logPath);
                              
                                 // advance to the statusChildElement to the first log element, if any
                                 statusChildElement = statusChildElement->NextSiblingElement();
                              }
                              else
                              {
                                 delete logPath;

                                 definitionValid = false;
                                 XmlErrorMsg(&processDefinitionDoc,errorMsg);
                                 OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                               "'log' element is empty"
                                               " - if present, it must be a pathname.  %s",
                                               errorMsg.data()
                                               );

                              }
                           }
                           else
                           {
                              definitionValid = false;
                              XmlErrorMsg(&processDefinitionDoc,errorMsg);
                              OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                            "'%s' element is invalid here: expected 'log'",
                                            statusChildElement->Value()                
                                            );
                           }
                        }
                     }
                     else
                     {
                        definitionValid = false;
                     
                        XmlErrorMsg(&processDefinitionDoc,errorMsg);
                        OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                      "required 'status' element is missing.  %s",
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
                                definitionValid && resourcesElement;
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
                              definitionValid = SipxResource::parse(resourceElement, process);
                           }
                        }
                        else
                        {
                           definitionValid = false;
                           XmlErrorMsg(&processDefinitionDoc,errorMsg);
                           OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                         "'%s' element is invalid here: expected 'resources'",
                                         resourcesElement->Value()                
                                         );
                        }
                     }
                     else
                     {
                        definitionValid = false;
                     
                        XmlErrorMsg(&processDefinitionDoc,errorMsg);
                        OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                                      "required 'resources' element is missing.  %s",
                                      errorMsg.data()
                                      );
                     }
                  }
                  
                  if (definitionValid)
                  {
                     // Process object has been successfully created.
                     // @TODO 
                  }
                  else
                  {
                     delete process;
                  }
               }
               else
               {
                  OsSysLog::add(FAC_WATCHDOG, PRI_CRIT, "Process::createFromDefinition "
                                "failed to create Process object for '%s'", name.data());
               }
            }
         }
         else
         {
            XmlErrorMsg(&processDefinitionDoc,errorMsg);
            OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                          "invalid document type '%s' in namespace '%s'\n"
                          "should be '%s' in namespace '%s' %s",
                          rootElementName, definitionNamespace,
                          SipXecsProcessRootElement, SipXecsProcessNamespace,
                          errorMsg.data()
                          );
         }

      }
      else
      {
         XmlErrorMsg(&processDefinitionDoc,errorMsg);
         OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition "
                       "root element not found in '%s': %s",
                       definitionFile.data(), errorMsg.data()
                       );
      }
   }
   else
   {
      UtlString errorMsg;
      XmlErrorMsg(&processDefinitionDoc,errorMsg);
      OsSysLog::add(FAC_WATCHDOG, PRI_ERR, "Process::createFromDefinition failed to load '%s': %s",
                    definitionFile.data(), errorMsg.data()
                    );
   }

   return process;
}


/// Return the current state of the Process.
Process::State Process::getState()
{
   return Undefined;            // @TODO 
}

/// Return whether or not the service for this process should be Running.
bool Process::isEnabled()
{
   return false;                // @TODO 
}
/**< @returns true if the desired state of this service is Running;
 *            this does not indicate anything about the current state of
 *            the service: for that see getState
 */

/// Set the persistent desired state of the Process to Running.
void Process::enable()
{
}

/// Set the persistent desired state of the Process to Disabled.
void Process::disable()
{
}

/// Shutting down sipXsupervisor, so shut down the service.
void Process::shutdown()
{
   //This does not affect the persistent state of the service.
}



/// Notify the Process that some configuration change has occurred.
void Process::configurationChange(const SipxResource& changedResource)
{
}
   
/// Notify the Process that the version stamp value of the configuration has changed.
void Process::configurationVersionChange()
{
}
   
/// Compare actual process state to the desired state, and attempt to change it if needed.
void Process::checkService()
{
}
   
/// Determine whether or not the values in a containable are comparable.
UtlContainableType Process::getContainableType() const
{
   return TYPE;
}

void Process::requireResourceToStart(const SipxResource* requiredResource)
{
}

void Process::checkResourceBeforeStop(const SipxResource* requiredResource)
{
}

/// Save the persistent desired state.
void Process::persistDesiredState(State persistentState ///< may only be Disabled or Running
                                  )
{
}

/// Read the persistent desired state.
Process::State Process::readPersistentState()
{
   return mDesiredState;
}

/// destructor
Process::~Process()
{
};
