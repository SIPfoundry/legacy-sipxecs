//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "utl/UtlTokenizer.h"
#include "os/OsSysLog.h"
#include "xmlparser/tinyxml.h"
#include "xmlparser/XmlErrorMsg.h"
#include "xmlparser/ExtractContent.h"
#include "sipXecsService/SipXecsService.h"

#include "SipxResource.h"
#include "SipxProcessManager.h"
#include "SipxSupervisorProcess.h"

// DEFINES
// CONSTANTS

extern const char* SipXecsProcessRootElement;
extern const char* SipXecsProcessNamespace;

extern const char* SUPERVISOR_CONFIG_SETTINGS_FILE;
extern const char* SUPERVISOR_CONFIG_PREFIX;    // prefix for config file entries

// TYPEDEFS
// STATICS INITIALIZATION
// FORWARD DECLARATIONS

/// constructor
SipxSupervisorProcess::SipxSupervisorProcess(const UtlString& name,
                         const UtlString& version,
                         const OsPath& definitionFile
                         ) :
   SipxProcess(name, version, definitionFile)
{
};


/// Read the supervisor process definition and return a process if definition is valid.
// This function is based on SipxProcess::createFromDefinition and changes made here
// may apply there as well.
SipxProcess* SipxSupervisorProcess::createFromDefinition(const OsPath& definitionFile)
{
   SipxProcess* process= NULL;
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
         if (rootElementName && definitionNamespace && 0 == strcmp(rootElementName,
               SipXecsProcessRootElement) && 0 == strcmp(definitionNamespace,
               SipXecsProcessNamespace) )
         {
            // step through the top level elements

            bool definitionValid = true;

            TiXmlElement* nameElement= NULL;
            UtlString name;
            TiXmlElement* versionElement= NULL;
            UtlString version;
            TiXmlElement* resourcesElement= NULL;

            // Get the 'name' element
            nameElement = processDefElem->FirstChildElement();
            if (nameElement && (0 == strcmp("name", nameElement->Value())))
            {

               if ( !textContent(name, nameElement) || name.isNull() )
               {
                  definitionValid = false;
                  XmlErrorMsg(processDefinitionDoc, errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                        "SipxSupervisorProcess::createFromDefinition "
                           "'name' element content is invalid %s", errorMsg.data() );
               }
               else if ((process = processManager->findProcess(name)))
               {
                  definitionValid = false;
                  XmlErrorMsg(processDefinitionDoc, errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                        "SipxSupervisorProcess::createFromDefinition "
                           "duplicate process name '%s'\n"
                           "  %s\n"
                           "  previously defined in '%s'", name.data(), errorMsg.data(),
                        process->mDefinitionFile.data() );
                  process = NULL; // so that we don't return or delete it
               }
            }
            else
            {
               definitionValid = false;
               XmlErrorMsg(processDefinitionDoc, errorMsg);
               OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                     "SipxSupervisorProcess::createFromDefinition "
                        "required 'name' element is missing %s", errorMsg.data() );
            }

            if (definitionValid && (name != SUPERVISOR_PROCESS_NAME))
            {
               // this should be the special definition for the Supervisor itself.
               definitionValid = false;
               OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                     "SipxSupervisorProcess::createFromDefinition "
                        "required 'name' element is not %s",
                     SUPERVISOR_PROCESS_NAME);
            }

            // Get the 'version' element
            if (definitionValid)
            {
               if ( (versionElement = processDefElem->FirstChildElement("version")))
               {
                  if ( !textContent(version, versionElement) || version.isNull() )
                  {
                     definitionValid = false;
                     XmlErrorMsg(processDefinitionDoc, errorMsg);
                     OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                           "SipxSupervisorProcess::createFromDefinition "
                              "'version' element content is invalid %s", errorMsg.data() );
                  }
               }
               else
               {
                  definitionValid = false;
                  XmlErrorMsg(processDefinitionDoc, errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                        "SipxSupervisorProcess::createFromDefinition "
                           "required 'version' element is missing %s", errorMsg.data() );
               }
            }

            // Get the 'resources' element
            if (definitionValid)
            {
               if ( !(resourcesElement = processDefElem->FirstChildElement("resources")))
               {
                  definitionValid = false;
                  XmlErrorMsg(processDefinitionDoc, errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                        "SipxSupervisorProcess::createFromDefinition "
                           "required 'resources' element is missing %s", errorMsg.data() );
               }
            }

            /* All the required top level elements have been found, so create
             * the SipxSupervisorProcess object and invoke the parsers for the components. */
            if (definitionValid)
            {
               if ((process = new SipxSupervisorProcess(SUPERVISOR_PROCESS_NAME, version,
                     definitionFile)))
               {
                  // Parse the 'resources' elements
                  if (resourcesElement)
                  {
                     TiXmlElement* resourceElement;
                     for (resourceElement = resourcesElement->FirstChildElement();
                           definitionValid && resourceElement;
                           resourceElement = resourceElement->NextSiblingElement() )
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
                        definitionValid = SipxResource::parse(processDefinitionDoc,
                              resourceElement, process);
                     }
                  }
               }

               if (definitionValid)
               {
                  SipxProcessManager::getInstance()->save(process);
               }

            }
         }
         else
         {
            XmlErrorMsg(processDefinitionDoc, errorMsg);
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
               "SipxSupervisorProcess::createFromDefinition "
               "invalid root element '%s' in namespace '%s'\n"
               "should be '%s' in namespace '%s' %s", rootElementName, definitionNamespace,
                  SipXecsProcessRootElement, SipXecsProcessNamespace, errorMsg.data() );
         }

      }
      else
      {
         XmlErrorMsg(processDefinitionDoc, errorMsg);
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
            "SipxSupervisorProcess::createFromDefinition "
            "root element not found in '%s': %s", definitionFile.data(), errorMsg.data() );
      }

   }

   return process;
}


/// Return whether or not the service for this process is Running.
bool SipxSupervisorProcess::isRunning()
{
   return true;
}


bool SipxSupervisorProcess::enable()
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                 "SipxSupervisorProcess::enable: supervisor cannot be enabled");
   return false;
}

bool SipxSupervisorProcess::disable()
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                 "SipxSupervisorProcess::disable: supervisor cannot be disabled");
   return false;
}

bool SipxSupervisorProcess::restart()
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                 "SipxSupervisorProcess::restart: supervisor cannot be restarted");
   return false;
}

void SipxSupervisorProcess::shutdown()
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                 "SipxSupervisorProcess::shutdown: supervisor cannot be shutdown");
}



void SipxSupervisorProcess::evCommandStarted(const SipxProcessCmd* command)
{
}

void SipxSupervisorProcess::evCommandStopped(const SipxProcessCmd* command, int rc)
{
   if (command == mConfigtest)
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                    "SipxSupervisorProcess[%s]::evCommandStopped mConfigtest %s, rc=%d",
                    data(), command->data(), rc);
   }
}

void SipxSupervisorProcess::evCommandOutput(const SipxProcessCmd* command,
                                        OsSysLogPriority pri,
                                        UtlString output)
{
   // The output is sure to contain newlines, and may contain several lines.
   // Clean it up before dispatching.
   UtlTokenizer tokenizer(output);
   UtlString    msg;
   while ( tokenizer.next(msg, "\r\n") )
   {
      logCommandOutput(pri, msg);
   }
}


/// Notify the SipxProcess that some configuration change has occurred.
void SipxSupervisorProcess::configurationChange(const SipxResource& changedResource)
{
   UtlString changedResourceDescription;
   changedResource.appendDescription(changedResourceDescription);

   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,
                 "SipxSupervisorProcess[%s]::configurationChange(%s)",
                 data(), changedResourceDescription.data());

   // tell Supervisor to reload its config files
   SipXecsService::setLogPriority(SUPERVISOR_CONFIG_SETTINGS_FILE, SUPERVISOR_CONFIG_PREFIX );
}

/// Notify the SipxSupervisorProcess that some configuration change has occurred.
void SipxSupervisorProcess::configurationVersionChange()
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_WARNING,
                 "SipxSupervisorProcess[%s]::configurationVersionChange(%s) IGNORED",
                 data(), mConfigVersion.data());
}

void SipxSupervisorProcess::startConfigTest()
{
}

void SipxSupervisorProcess::killConfigTest()
{
}

void SipxSupervisorProcess::startProcess()
{
}

void SipxSupervisorProcess::stopProcess()
{
}



/// destructor
SipxSupervisorProcess::~SipxSupervisorProcess()
{
};


