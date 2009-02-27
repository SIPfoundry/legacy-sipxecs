//
//
// Copyright (C) 2009 Nortel, certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
// $$
//////////////////////////////////////////////////////////////////////////////
// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "os/OsConfigDb.h"
#include "os/OsFileSystem.h"
#include "xmlparser/XmlErrorMsg.h"
#include "sipXecsService/SipXecsService.h"
#include "SipxProcessManager.h"
#include "SipxProcess.h"

#define DEBUG

// MACROS
// EXTERNAL FUNCTIONS
// EXTERNAL VARIABLES
// CONSTANTS
const char* OLD_PROCESS_DEFINITION_NAME_PATTERN = ".*.process.xml$";

// STRUCTS
// TYPEDEFS
// FORWARD DECLARATIONS
// GLOBALS


void upgradeFrom3_0()
{
   // 3.0 had process definition files in /etc/sipxpbx/process.d
   UtlString oldProcessDefinitionDirectory =
      SipXecsService::Path(SipXecsService::ConfigurationDirType, "process-old.d");

   OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"upgradeFrom3_0: searching '%s'",
                 oldProcessDefinitionDirectory.data()
                 );

   if ( !OsFileSystem::exists( oldProcessDefinitionDirectory) )
   {
      return;
   }

   OsFileIterator definitions(oldProcessDefinitionDirectory);
   OsPath    oldProcessDefinitionFile;
   bool okToRemoveDir = true;   // set to false if any pre-4.0 process file cannot be upgraded
   for ( OsStatus iteratorStatus = definitions.findFirst(oldProcessDefinitionFile,
                                                OLD_PROCESS_DEFINITION_NAME_PATTERN,
                                                OsFileIterator::FILES);
         OS_SUCCESS == iteratorStatus;
         iteratorStatus = definitions.findNext(oldProcessDefinitionFile)
        )
   {
      OsPath oldProcessDefinitionPath( oldProcessDefinitionDirectory
                                   +OsPath::separator
                                   +oldProcessDefinitionFile
                                   );
      OsSysLog::add(FAC_SUPERVISOR, PRI_INFO,"upgradeFrom3_0: reading pre-4.0 process def '%s'",
                    oldProcessDefinitionPath.data()
                    );

      // read the process name and the watchdog enable setting,
      // and enable the process if necessary
      TiXmlDocument processDefinitionDoc(oldProcessDefinitionPath);

      bool definitionValid = true;
      UtlString errorMsg;
      if ( processDefinitionDoc.LoadFile() )
      {
         TiXmlElement *subroot = processDefinitionDoc.RootElement();
         if (subroot != NULL)
         {
            bool bEnabled=false;
            const char *enableString = subroot->Attribute("enable");
            if (enableString == NULL || strcmp(enableString, "true")==0)
            {
               bEnabled=true;
            }

            TiXmlElement* processDefElement = subroot->FirstChildElement("process_definitions");
            if (processDefElement)
            {
               TiXmlNode *pGroupNode = processDefElement->FirstChild("group");
               if (pGroupNode)
               {
                  TiXmlElement *processElement = pGroupNode->FirstChildElement("process");
                  if (processElement)
                  {
                     const char *pMsg = processElement->Attribute("name");
                     UtlString processName = pMsg;

                     // enable or disable process based on setting in old process def file
                     SipxProcess* newProcess;
                     if ((newProcess=SipxProcessManager::getInstance()->findProcess(processName)))
                     {
                        if (bEnabled)
                        {
                           OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE,
                                 "upgradeFrom3_0: enabling process '%s'", processName.data());
                           newProcess->enable();
                        }
                        else
                        {
                           OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE,
                                 "upgradeFrom3_0: disabling process '%s'", processName.data());
                           newProcess->disable();
                        }

                        if (OS_SUCCESS == OsFileSystem::remove(oldProcessDefinitionPath))
                        {
                           OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE,
                                 "upgradeFrom3_0: removing pre-4.0 process def file '%s'",
                                 oldProcessDefinitionPath.data());
                        }
                        else
                        {
                           OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                                 "upgradeFrom3_0: failed to remove pre-4.0 process def file '%s'",
                                 oldProcessDefinitionPath.data());
                        }
                     }
                     else
                     {
                        OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
                              "upgradeFrom3_0: could not find process '%s'", processName.data());
                        okToRemoveDir = false;
                     }
                  }
                  else
                  {
                     definitionValid = false;
                  }
               }
               else
               {
                  definitionValid = false;
               }
            }
            else
            {
               definitionValid = false;
            }
         }
         else
         {
            definitionValid = false;
         }
      }
      else
      {
         definitionValid = false;
      }

      if ( !definitionValid )
      {
         // Invalid document format.  Log the issue and continue to the next process xml file.
         XmlErrorMsg(processDefinitionDoc, errorMsg);
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR,
               "upgradeFrom3_0: ignoring invalid pre-4.0 process xml file '%s' (%s)",
               oldProcessDefinitionFile.data(), errorMsg.data());
         okToRemoveDir = false;
      }
   }

   // remove the old process defn directory (succeeds only if it is empty)
   if (OS_SUCCESS == OsFileSystem::remove(oldProcessDefinitionDirectory))
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE,
            "upgradeFrom3_0: all process data has been migrated from "
            "pre-4.0 process def directory '%s', "
            "and the directory has been deleted.",
            oldProcessDefinitionDirectory.data());
   }
   else
   {
      // rmdir may fail because files still exist (e.g. editor backup files).
      // Log whether upgrade succeeded or not.
      if ( okToRemoveDir )
      {
            OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE,
                  "upgradeFrom3_0: all process data has been migrated from "
                  "pre-4.0 process def directory '%s', "
                  "and the directory may safely be deleted.",
                  oldProcessDefinitionDirectory.data());
      }
      else
      {
            OsSysLog::add(FAC_SUPERVISOR, PRI_WARNING,
                  "upgradeFrom3_0: some process data could not be migrated from "
                  "pre-4.0 process def directory '%s'. ",
                  oldProcessDefinitionDirectory.data());
      }
   }
}

