//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "xmlparser/tinyxml.h"
#include "xmlparser/XmlErrorMsg.h"
#include "xmlparser/ExtractContent.h"

#include "sipXecsService/SipXecsService.h"

#include "ProcessCmd.h"

// DEFINES
// CONSTANTS
// TYPEDEFS
// FORWARD DECLARATIONS

/// initializes by parsing a Command type element from sipXecs-process schema
ProcessCmd* ProcessCmd::parseCommandDefinition(const TiXmlDocument& processDefinitionDoc,
                                               const TiXmlElement* commandElement // any 'Command'
                                               )
{
   ProcessCmd* processCmd = NULL;

   bool      definitionValid = true;
   UtlString errorMsg;

   UtlString defaultDir;
   UtlString user;
   UtlString execute;

   const TiXmlElement* commandChildElement;

   // sipXecs-process/commands/<Command>/defaultDir
   commandChildElement = commandElement->FirstChildElement();
   if (commandChildElement)
   {
      if (0 == strcmp("defaultDir",commandChildElement->Value()))
      {
         // the optional defaultDir element is present
         if (   textContent(defaultDir, commandChildElement)
             && !defaultDir.isNull())
         {
            // advance to the commandChildElement to the user element, if any
            commandChildElement = commandChildElement->NextSiblingElement();
         }
         else
         {
            definitionValid = false;
            XmlErrorMsg(processDefinitionDoc,errorMsg);
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "ProcessCmd::parseCommandDefinition "
                          "'defaultDir' element is empty"
                          " - if present, it must be a pathname %s",
                          errorMsg.data()
                          );

         }
      }
      else
      {
         defaultDir = SipXecsService::Path(SipXecsService::LogDirType,"");

         /*
          * There is no defaultDir element, which is allowed.
          * If commandChildElement is non-NULL, it should point to
          * either a 'user' or 'execute' element.
          */
      }
   }

   // sipXecs-process/commands/<Command>/user
   if (definitionValid && commandChildElement)
   {
      if (0 == strcmp("user",commandChildElement->Value()))
      {
         // the optional 'user' element is present
         if (   textContent(user, commandChildElement)
             && !user.isNull())
         {
            // advance to the commandChildElement to the user element, if any
            commandChildElement = commandChildElement->NextSiblingElement();
         }
         else
         {
            definitionValid = false;
            XmlErrorMsg(processDefinitionDoc,errorMsg);
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "ProcessCmd::parseCommandDefinition "
                          "'user' element is empty"
                          " - if present, it must be a valid username %s",
                          errorMsg.data()
                          );

         }
      }
      else
      {
         /*
          * There is no user element, which is allowed.
          * commandChildElement should point to an 'execute' element.
          */
         user = SipXecsService::User();
      }
   }

   // sipXecs-process/commands/<Command>/execute
   if (definitionValid && commandChildElement)
   {
      if (0 == strcmp("execute",commandChildElement->Value()))
      {
         // the required 'execute' element is present
         if (   textContent(execute, commandChildElement)
             && !execute.isNull())
         {
            // advance to the commandChildElement to the first parameter element, if any
            commandChildElement = commandChildElement->NextSiblingElement();
         }
         else
         {
            definitionValid = false;
            XmlErrorMsg(processDefinitionDoc,errorMsg);
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "ProcessCmd::parseCommandDefinition "
                          "'execute' element is empty"
                          " - it must be a valid executable %s",
                          errorMsg.data()
                          );

         }
      }
      else
      {
         definitionValid = false;
         XmlErrorMsg(processDefinitionDoc,errorMsg);
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "ProcessCmd::parseCommandDefinition "
                       "'execute' element is missing %s",
                       errorMsg.data()
                       );

      }
   }

   if (definitionValid)
   {
      if ((processCmd = new ProcessCmd(execute, defaultDir, user)))
      {
         // sipXecs-process/commands/<Command>/parameter
         while (definitionValid && commandChildElement)
         {
            if (0 == strcmp("parameter",commandChildElement->Value()))
            {
               UtlString* parameter = new UtlString;
               if (textContent(*parameter, commandChildElement) && !parameter->isNull())
               {
                  processCmd->mParameters.append(parameter);

                  // advance to the commandChildElement to the first log element, if any
                  commandChildElement = commandChildElement->NextSiblingElement();
               }
               else
               {
                  delete parameter;

                  definitionValid = false;
                  XmlErrorMsg(processDefinitionDoc,errorMsg);
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "ProcessCmd::parseCommandDefinition "
                                "'parameter' element is empty"
                                " - if present, it must have text content %s",
                                errorMsg.data()
                                );
               }
            }
            else
            {
               definitionValid = false;
               XmlErrorMsg(processDefinitionDoc,errorMsg);
               OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "ProcessCmd::parseCommandDefinition "
                             "'%s' element is invalid here: expected 'parameter'",
                             commandChildElement->Value()
                             );
            }
         }

         if (!definitionValid)
         {
            delete processCmd;
            processCmd = NULL;
         }
      }
      else
      {
         OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "ProcessCmd::parseCommandDefinition "
                       "unable to allocate ProcessCmd'"
                       );
      }
   }

   return processCmd;
}

/// Execute the command.
void ProcessCmd::execute()
{
   OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE, "ProcessCmd::execute "
                 "STUB - NOT IMPLEMENTED");
}


/// destructor
ProcessCmd::~ProcessCmd()
{
}

ProcessCmd::ProcessCmd(const UtlString& execute,
                       const UtlString& workingDirectory,
                       const UtlString& user
                       ) :
   mWorkingDirectory(workingDirectory),
   mUser(user),
   mExecutable(execute)
{
}
