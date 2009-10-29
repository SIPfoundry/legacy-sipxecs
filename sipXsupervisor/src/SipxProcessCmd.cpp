//
// Copyright (C) 2008 Pingtel Corp., certain elements licensed under a Contributor Agreement.
// Contributors retain copyright to elements licensed under a Contributor Agreement.
// Licensed to the User under the LGPL license.
//
//////////////////////////////////////////////////////////////////////////////

// SYSTEM INCLUDES

// APPLICATION INCLUDES
#include "os/OsSysLog.h"
#include "utl/UtlSListIterator.h"
#include "xmlparser/tinyxml.h"
#include "xmlparser/XmlErrorMsg.h"
#include "xmlparser/ExtractContent.h"

#include "sipXecsService/SipXecsService.h"

#include "SipxProcessCmd.h"

// DEFINES
// CONSTANTS
const UtlContainableType SipxProcessCmd::TYPE = "SipxProcessCmd";

// TYPEDEFS
// FORWARD DECLARATIONS

/// initializes by parsing a Command type element from sipXecs-process schema
SipxProcessCmd* SipxProcessCmd::parseCommandDefinition(const TiXmlDocument& processDefinitionDoc,
                                               const TiXmlElement* commandElement // any 'Command'
                                               )
{
   SipxProcessCmd* processCmd = NULL;

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
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcessCmd::parseCommandDefinition "
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
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcessCmd::parseCommandDefinition "
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
            OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcessCmd::parseCommandDefinition "
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
         OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcessCmd::parseCommandDefinition "
                       "'execute' element is missing %s",
                       errorMsg.data()
                       );

      }
   }

   if (definitionValid)
   {
      if ((processCmd = new SipxProcessCmd(execute, defaultDir, user)))
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
                  OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcessCmd::parseCommandDefinition "
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
               OsSysLog::add(FAC_SUPERVISOR, PRI_ERR, "SipxProcessCmd::parseCommandDefinition "
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
         OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SipxProcessCmd::parseCommandDefinition "
                       "unable to allocate SipxProcessCmd'"
                       );
      }
   }

   return processCmd;
}

/// Execute the command.
void SipxProcessCmd::execute(SipxProcessCmdOwner* owner)
{
   if ( isRunning() )
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SipxProcessCmd::execute %s: process is already running ",
                 data());
      // kill the previous one?
   }

   ExecuteMsg message(owner);
   postMessage( message );
}


bool SipxProcessCmd::isRunning()
{
   UtlBoolean status = false;

   status = mProcess.isRunning();

   return status;
}

/// Kill the command.
void SipxProcessCmd::kill()
{
   mProcess.kill();
}

UtlBoolean SipxProcessCmd::handleMessage( OsMsg& rMsg )
{
   UtlBoolean handled = FALSE;
   ExecuteMsg* pMsg = dynamic_cast <ExecuteMsg*> ( &rMsg );
   switch ( rMsg.getMsgType() )
   {
   case OsMsg::OS_SHUTDOWN:
      requestShutdown();
      handled = TRUE;
      break;

   case OsMsg::OS_EVENT:
   {
      switch ( pMsg->getMsgSubType() )
      {
      case ExecuteMsg::EXECUTE:
         executeInTask(pMsg->getOwner());
         handled = TRUE;
         break;
      default:
         OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT,
                       "SipxProcessCmd::handleMessage: '%s' unhandled message subtype %d.%d",
                       mName.data(), rMsg.getMsgType(), rMsg.getMsgSubType());
         break;
      }
      break;
   }

   default:
      OsSysLog::add(FAC_ALARM, PRI_CRIT,
                    "SipxProcessCmd::handleMessage: '%s' unhandled message type %d.%d",
                    mName.data(), rMsg.getMsgType(), rMsg.getMsgSubType());
      break;
   }

   return handled;
}


void SipxProcessCmd::executeInTask(SipxProcessCmdOwner* owner)
{
   UtlString args[mParameters.entries()+1];
   UtlSListIterator parameterListIterator(mParameters);
   UtlString* pParameter;
   UtlString argString;
   ssize_t i=0;
   while ( (pParameter = dynamic_cast<UtlString*> (parameterListIterator())) )
   {
      args[i++] = *pParameter;
      argString.append(" ");
      argString.append(*pParameter);
   }
   args[i] = NULL;
   OsSysLog::add(FAC_SUPERVISOR, PRI_NOTICE, "SipxProcessCmd::execute %s %s",
                 mExecutable.data(), argString.data());

   int rc;

   if ( (rc=mProcess.launch(mExecutable, &args[0], mWorkingDirectory,
                             mProcess.NormalPriorityClass, FALSE,
                             FALSE /*don't automatically reap children*/))
       == OS_SUCCESS )
   {
      owner->evCommandStarted(this);
      OsSysLog::add(FAC_SUPERVISOR, PRI_DEBUG,"'%s: process running, pid %ld",
                    mExecutable.data(), (long)mProcess.getPID());

      //now wait around for the thing to finish
      UtlString stdoutMsg;
      UtlString stderrMsg;
      while ( (rc = mProcess.getOutput(&stdoutMsg, &stderrMsg)) > 0)
      {
         if (stdoutMsg.length() > 0)
         {
            owner->evCommandOutput(this, PRI_INFO, stdoutMsg);
         }
         if (stderrMsg.length() > 0)
         {
            owner->evCommandOutput(this, PRI_ERR, stderrMsg);
         }
      }

      //0 means wait until it's finished
      rc = mProcess.wait(0);
      owner->evCommandStopped(this, rc);
   }
   else
   {
      OsSysLog::add(FAC_SUPERVISOR, PRI_CRIT, "SipxProcessCmd::execute %s %s failed",
                    mExecutable.data(), argString.data());
      owner->evCommandStopped(this, rc);
   }
}


/// Determine whether or not the values in a containable are comparable.
UtlContainableType SipxProcessCmd::getContainableType() const
{
   return TYPE;
}


/// destructor
SipxProcessCmd::~SipxProcessCmd()
{
   if (mProcess.isRunning())
   {
      mProcess.kill();
   }

   mParameters.destroyAll();

   waitUntilShutDown();
}

SipxProcessCmd::SipxProcessCmd(const UtlString& execute,
                               const UtlString& workingDirectory,
                               const UtlString& user
                               ) :
   UtlString(execute),
   OsServerTask("SipxProcessCmd-%d"),
   mWorkingDirectory(workingDirectory),
   mUser(user),
   mExecutable(execute)
{
   mParameters.removeAll();
   // start the task which will listen for messages and launch programs in the background
   start();
}

//////////////////////////////////////////////////////////////////////////////
ExecuteMsg::ExecuteMsg(//EventSubType eventSubType,
                               SipxProcessCmdOwner* owner
                               ) :
   OsMsg( OS_EVENT, ExecuteMsg::EXECUTE ),
   mOwner( owner )
{
}

// deep copy of alarm and parameters
ExecuteMsg::ExecuteMsg( const ExecuteMsg& rhs) :
   OsMsg( OS_EVENT, rhs.getMsgSubType() ),
   mOwner( rhs.getOwner() )
{
}

ExecuteMsg::~ExecuteMsg()
{
}

OsMsg* ExecuteMsg::createCopy( void ) const
{
   return new ExecuteMsg( *this );
}

